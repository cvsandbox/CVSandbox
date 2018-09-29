/*
    Plug-ins' management library of Computer Vision Sandbox

    Copyright (C) 2011-2018, cvsandbox
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

#include "XImageProcessingFilterPlugin2.hpp"
#include <algorithm>

using namespace std;
using namespace CVSandbox;

XImageProcessingFilterPlugin2::XImageProcessingFilterPlugin2( void* plugin, bool ownIt ) :
    XPlugin( plugin, PluginType_ImageProcessingFilter, ownIt ),
    mSupportedInputFormats( ),
    mSupportedOutputFormats( )
{
    int32_t pixelFormatsCount = 0;

    // collect supported pixel format from a plug-in
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

XImageProcessingFilterPlugin2::~XImageProcessingFilterPlugin2( )
{
}

// Create plug-in wrapper
const shared_ptr<XImageProcessingFilterPlugin2> XImageProcessingFilterPlugin2::Create( void* plugin, bool ownIt )
{
    return shared_ptr<XImageProcessingFilterPlugin2>( new XImageProcessingFilterPlugin2( plugin, ownIt ) );
}

// Check if the image processing filter can process images by modifying them
// (without creating new image as a result)
bool XImageProcessingFilterPlugin2::CanProcessInPlace( ) const
{
    SImageProcessingFilterPlugin2* ipf = static_cast<SImageProcessingFilterPlugin2*>( mPlugin );
    return ipf->CanProcessInPlace( ipf );
}

// Check if certain pixel format is supported by the filter
bool XImageProcessingFilterPlugin2::IsPixelFormatSupported( XPixelFormat inputPixelFormat ) const
{
    return ( GetOutputPixelFormat( inputPixelFormat ) != XPixelFormatUnknown );
}

// Check if certain input pixel format is supported by the filter and return corresponding output pixel format
XPixelFormat XImageProcessingFilterPlugin2::GetOutputPixelFormat( XPixelFormat inputPixelFormat ) const
{
    XPixelFormat ret = XPixelFormatUnknown;

    for ( int i = 0, n = mSupportedInputFormats.size( ); i < n; i++ )
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
const vector<XPixelFormat> XImageProcessingFilterPlugin2::GetSupportedPixelFormats( ) const
{
    return mSupportedInputFormats;
}

// Get human understandable description/purpose of the second image required by the filter
string XImageProcessingFilterPlugin2::GetSecondImageDescription( ) const
{
    string desc;
    xstring xdesc = 0;

    SImageProcessingFilterPlugin2* ipf = static_cast<SImageProcessingFilterPlugin2*>( mPlugin );
    xdesc = ipf->GetSecondImageDescription( ipf );

    if ( xdesc != 0 )
    {
        desc = string( xdesc );
        XStringFree( &xdesc );
    }

    return desc;
}

// Get supported size of the second image
XSupportedImageSize XImageProcessingFilterPlugin2::GetSecondImageSupportedSize( ) const
{
    SImageProcessingFilterPlugin2* ipf = static_cast<SImageProcessingFilterPlugin2*>( mPlugin );
    return ipf->GetSecondImageSupportedSize( ipf );
}

// Get supported pixel format of the second image for the given format of the source image
XPixelFormat XImageProcessingFilterPlugin2::GetSecondImageSupportedFormat( XPixelFormat inputPixelFormat ) const
{
    SImageProcessingFilterPlugin2* ipf = static_cast<SImageProcessingFilterPlugin2*>( mPlugin );
    return ipf->GetSecondImageSupportedFormat( ipf, inputPixelFormat );
}

// Process image by modifying its data
XErrorCode XImageProcessingFilterPlugin2::ProcessImage( const shared_ptr<XImage>& src,
                                                        const shared_ptr<const XImage>& src2 ) const
{
    SImageProcessingFilterPlugin2* ipf = static_cast<SImageProcessingFilterPlugin2*>( mPlugin );
    return ipf->ProcessImageInPlace( ipf, src->ImageData( ), src2->ImageData( ) );
}

// Process image and create new image as a result
XErrorCode XImageProcessingFilterPlugin2::ProcessImage( const shared_ptr<const XImage>& src,
                                                        const shared_ptr<const XImage>& src2,
                                                        shared_ptr<XImage>& dst ) const
{
    XErrorCode  ret = ErrorUnsupportedInterface;
    ximage*     dstCImage = ( !dst ) ? nullptr : dst->GetImageDataOwnership( );

    SImageProcessingFilterPlugin2* ipf = static_cast<SImageProcessingFilterPlugin2*>( mPlugin );
    ret = ipf->ProcessImage( ipf, src->ImageData( ), src2->ImageData( ), &dstCImage );

    if ( ( ret == SuccessCode ) && ( dstCImage != 0 ) )
    {
        dst = XImage::Create( &dstCImage, true );
    }
    else
    {
        XImageFree( &dstCImage );
        dst.reset( );
    }

    return ret;
}
