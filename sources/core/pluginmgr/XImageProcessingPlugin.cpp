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

#include "XImageProcessingPlugin.hpp"
#include <algorithm>

using namespace std;
using namespace CVSandbox;

XImageProcessingPlugin::XImageProcessingPlugin( void* plugin, bool ownIt ) :
    XPlugin( plugin, PluginType_ImageProcessing, ownIt ),
    mSupportedFormats( )
{
    int32_t pixelFormatsCount = 0;

    // collect supported pixel format from a plugin
    SImageProcessingPlugin* ipf = static_cast<SImageProcessingPlugin*>( plugin );

    if ( ipf->GetSupportedPixelFormats( ipf, nullptr, &pixelFormatsCount ) == ErrorTooSmallBuffer )
    {
        XPixelFormat* inputFormats = new XPixelFormat[pixelFormatsCount];

        if ( ipf->GetSupportedPixelFormats( ipf, inputFormats, &pixelFormatsCount ) == SuccessCode )
        {
            mSupportedFormats.resize( pixelFormatsCount );

            copy( inputFormats, inputFormats + pixelFormatsCount, mSupportedFormats.begin( ) );
        }

        delete [] inputFormats;
    }
}

XImageProcessingPlugin::~XImageProcessingPlugin( )
{
}

// Create plug-in wrapper
const shared_ptr<XImageProcessingPlugin> XImageProcessingPlugin::Create( void* plugin, bool ownIt )
{
    return shared_ptr<XImageProcessingPlugin>( new XImageProcessingPlugin( plugin, ownIt ) );
}

// Check if certain pixel format is supported by the filter
bool XImageProcessingPlugin::IsPixelFormatSupported( XPixelFormat pixelFormat ) const
{
    return ( std::find( mSupportedFormats.begin( ), mSupportedFormats.end( ), pixelFormat ) != mSupportedFormats.end( ) );
}

// Get pixel formats supported by the plug-in
const vector<XPixelFormat> XImageProcessingPlugin::GetSupportedPixelFormats( ) const
{
    return mSupportedFormats;
}

// Process the specified image
XErrorCode XImageProcessingPlugin::ProcessImage( const shared_ptr<XImage>& image ) const
{
    SImageProcessingPlugin* ip = static_cast<SImageProcessingPlugin*>( mPlugin );
    return ip->ProcessImage( ip, image->ImageData( ) );
}
