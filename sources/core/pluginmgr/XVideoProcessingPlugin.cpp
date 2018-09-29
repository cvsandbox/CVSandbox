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

#include "XVideoProcessingPlugin.hpp"
#include <algorithm>

using namespace std;
using namespace CVSandbox;

XVideoProcessingPlugin::XVideoProcessingPlugin( void* plugin, bool ownIt ) :
    XPlugin( plugin, PluginType_VideoProcessing, ownIt ),
    mSupportedPixelFormats( )
{
    int32_t pixelFormatsCount = 0;

    // collect supported pixel formats
    SVideoProcessingPlugin* vpp = static_cast<SVideoProcessingPlugin*>( plugin );

    if ( vpp->GetSupportedPixelFormats( vpp, nullptr, &pixelFormatsCount ) == ErrorTooSmallBuffer )
    {
        XPixelFormat* pixelFormats = new XPixelFormat[pixelFormatsCount];

        if ( vpp->GetSupportedPixelFormats( vpp, pixelFormats, &pixelFormatsCount ) == SuccessCode )
        {
            mSupportedPixelFormats.resize( pixelFormatsCount );
            copy( pixelFormats, pixelFormats + pixelFormatsCount, mSupportedPixelFormats.begin( ) );
        }

        delete [] pixelFormats;
    }
}

XVideoProcessingPlugin::~XVideoProcessingPlugin( )
{
}

// Create plug-in wrapper
const shared_ptr<XVideoProcessingPlugin> XVideoProcessingPlugin::Create( void* plugin, bool ownIt )
{
    return shared_ptr<XVideoProcessingPlugin>( new XVideoProcessingPlugin( plugin, ownIt ) );
}

// Check if the plug-in does changes to input video frames or not
bool XVideoProcessingPlugin::IsReadOnlyMode( ) const
{
    SVideoProcessingPlugin* vpp = static_cast<SVideoProcessingPlugin*>( mPlugin );
    return vpp->IsReadOnlyMode( vpp );
}

// Check if certain pixel format is supported by the plug-in
bool XVideoProcessingPlugin::IsPixelFormatSupported( XPixelFormat inputPixelFormat ) const
{
    return ( std::find( mSupportedPixelFormats.begin( ), mSupportedPixelFormats.end( ), inputPixelFormat ) != mSupportedPixelFormats.end( ) );
}

// Get pixel formats supported by the plug-in
const vector<XPixelFormat> XVideoProcessingPlugin::GetSupportedPixelFormats( ) const
{
    return mSupportedPixelFormats;
}

// Process the specified video frame
XErrorCode XVideoProcessingPlugin::ProcessImage( const shared_ptr<XImage>& image ) const
{
    SVideoProcessingPlugin* vpp = static_cast<SVideoProcessingPlugin*>( mPlugin );
    return vpp->ProcessImage( vpp, image->ImageData( ) );
}

// Reset run time state of the video processing plug-in
void XVideoProcessingPlugin::Reset( )
{
    SVideoProcessingPlugin* vpp = static_cast<SVideoProcessingPlugin*>( mPlugin );
    return vpp->Reset( vpp );
}
