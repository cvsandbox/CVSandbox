/*
    Plug-ins' management library of Computer Vision Sandbox

    Copyright (C) 2011-2019, cvsandbox
    http://www.cvsandbox.com/contacts.html

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "XImageProcessingFilterPlugin.hpp"
#include <algorithm>
#include <assert.h>

using namespace std;
using namespace CVSandbox;

XImageProcessingFilterPlugin::XImageProcessingFilterPlugin( void* plugin, bool ownIt ) :
    XPlugin( plugin, PluginType_ImageProcessingFilter, ownIt ),
    mSupportedInputFormats( ),
    mSupportedOutputFormats( )
{
    int32_t pixelFormatsCount = 0;

    // collec supported pixel format from a plugin
    SImageProcessingFilterPlugin* ipf = static_cast<SImageProcessingFilterPlugin*>( plugin );

    if ( ipf->GetPixelFormatTranslations( ipf, nullptr, nullptr, &pixelFormatsCount ) == ErrorTooSmallBuffer )
    {
        XPixelFormat* inputFormats  = new XPixelFormat[pixelFormatsCount];
        XPixelFormat* outputFormats = new XPixelFormat[pixelFormatsCount];

        if ( ipf->GetPixelFormatTranslations( ipf, inputFormats, outputFormats, &pixelFormatsCount ) == SuccessCode )
        {
            mSupportedInputFormats.resize( pixelFormatsCount );
            mSupportedOutputFormats.resize( pixelFormatsCount );

            copy( inputFormats, inputFormats + pixelFormatsCount, mSupportedInputFormats.begin( ) );
            copy( outputFormats, outputFormats + pixelFormatsCount, mSupportedOutputFormats.begin( ) );
        }

        delete [] inputFormats;
        delete [] outputFormats;
    }
}

XImageProcessingFilterPlugin::~XImageProcessingFilterPlugin( )
{
}

// Create plug-in wrapper
const shared_ptr<XImageProcessingFilterPlugin> XImageProcessingFilterPlugin::Create( void* plugin, bool ownIt )
{
    return shared_ptr<XImageProcessingFilterPlugin>( new XImageProcessingFilterPlugin( plugin, ownIt ) );
}

// Check if the image processing filter can process images by modifying them
// (without creating new image as a result)
bool XImageProcessingFilterPlugin::CanProcessInPlace( ) const
{
    SImageProcessingFilterPlugin* ipf = static_cast<SImageProcessingFilterPlugin*>( mPlugin );
    return ipf->CanProcessInPlace( ipf );
}

// Check if certain pixel format is supported by the filter
bool XImageProcessingFilterPlugin::IsPixelFormatSupported( XPixelFormat inputPixelFormat ) const
{
    return ( GetOutputPixelFormat( inputPixelFormat ) != XPixelFormatUnknown );
}

// Check if certain input pixel format is supported by the filter and return corresponding output pixel format
XPixelFormat XImageProcessingFilterPlugin::GetOutputPixelFormat( XPixelFormat inputPixelFormat ) const
{
    XPixelFormat ret = XPixelFormatUnknown;

    for ( size_t i = 0, n = mSupportedInputFormats.size( ); i < n; i++ )
    {
        if ( mSupportedInputFormats[i] == inputPixelFormat )
        {
            ret = mSupportedOutputFormats[i];
            break;
        }
    }

    return ret;
}

// Get pixel formats supported by the plug-in
const vector<XPixelFormat> XImageProcessingFilterPlugin::GetSupportedPixelFormats( ) const
{
    return mSupportedInputFormats;
}

// Process image by modifying its data
XErrorCode XImageProcessingFilterPlugin::ProcessImage( const shared_ptr<XImage>& image ) const
{
    SImageProcessingFilterPlugin* ipf = static_cast<SImageProcessingFilterPlugin*>( mPlugin );
    return ipf->ProcessImageInPlace( ipf, image->ImageData( ) );
}

// Process image and create new image as a result
XErrorCode XImageProcessingFilterPlugin::ProcessImage( const shared_ptr<const XImage>& src, shared_ptr<XImage>& dst ) const
{
    XErrorCode  ret       = ErrorUnsupportedInterface;
    ximage*     dstCImage = ( !dst ) ? nullptr : dst->ImageData( );

    SImageProcessingFilterPlugin* ipf = static_cast<SImageProcessingFilterPlugin*>( mPlugin );
    ret = ipf->ProcessImage( ipf, src->ImageData( ), &dstCImage );

    if ( !dst )
    {
        // if destination was not allocated before, we do it only on success
        if ( ret == SuccessCode )
        {
            dst = XImage::Create( &dstCImage, true );
        }
        else
        {
            // plug-ins should not really allocate anything new when fail, but lets be safe
            assert( !dstCImage );
            XImageFree( &dstCImage );
        }
    }
    else
    {
        // if destination was already set before, we may need to reset it again
        if ( dst->ImageData( ) != dstCImage )
        {
            // the image we had before was reallocated and so now
            // dst objects wraps a dangling pointer - need to reset it
            dst->Reset( dstCImage );
        }
    }

    return ret;
}
