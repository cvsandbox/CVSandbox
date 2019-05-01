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

#include "XDetectionPlugin.hpp"
#include <algorithm>

using namespace std;
using namespace CVSandbox;

XDetectionPlugin::XDetectionPlugin( void* plugin, bool ownIt ) :
    XPlugin( plugin, PluginType_Detection, ownIt ),
    mSupportedPixelFormats( )
{
    int32_t pixelFormatsCount = 0;

    // collect supported pixel formats
    SDetectionPlugin* dtp = static_cast<SDetectionPlugin*>( plugin );

    if ( dtp->GetSupportedPixelFormats( dtp, nullptr, &pixelFormatsCount ) == ErrorTooSmallBuffer )
    {
        XPixelFormat* pixelFormats = new XPixelFormat[pixelFormatsCount];

        if ( dtp->GetSupportedPixelFormats( dtp, pixelFormats, &pixelFormatsCount ) == SuccessCode )
        {
            mSupportedPixelFormats.resize( pixelFormatsCount );
            copy( pixelFormats, pixelFormats + pixelFormatsCount, mSupportedPixelFormats.begin( ) );
        }

        delete [] pixelFormats;
    }
}

XDetectionPlugin::~XDetectionPlugin( )
{
}

// Create plug-in wrapper
const shared_ptr<XDetectionPlugin> XDetectionPlugin::Create( void* plugin, bool ownIt )
{
    return shared_ptr<XDetectionPlugin>( new XDetectionPlugin( plugin, ownIt ) );
}

// Check if the plug-in does changes to input video frames or not
bool XDetectionPlugin::IsReadOnlyMode( ) const
{
    SDetectionPlugin* dtp = static_cast<SDetectionPlugin*>( mPlugin );
    return dtp->IsReadOnlyMode( dtp );
}

// Check if certain pixel format is supported by the plug-in
bool XDetectionPlugin::IsPixelFormatSupported( XPixelFormat inputPixelFormat ) const
{
    return ( std::find( mSupportedPixelFormats.begin( ), mSupportedPixelFormats.end( ), inputPixelFormat ) != mSupportedPixelFormats.end( ) );
}

// Get pixel formats supported by the plug-in
const vector<XPixelFormat> XDetectionPlugin::GetSupportedPixelFormats( ) const
{
    return mSupportedPixelFormats;
}

// Process the specified video frame
XErrorCode XDetectionPlugin::ProcessImage( const shared_ptr<XImage>& image ) const
{
    SDetectionPlugin* dtp = static_cast<SDetectionPlugin*>( mPlugin );
    return dtp->ProcessImage( dtp, image->ImageData( ) );
}

// Check if detection was triggered
bool XDetectionPlugin::Detected( ) const
{
    SDetectionPlugin* dtp = static_cast<SDetectionPlugin*>( mPlugin );
    return dtp->Detected( dtp );
}

// Reset run time state of the video processing plug-in
void XDetectionPlugin::Reset( )
{
    SDetectionPlugin* dtp = static_cast<SDetectionPlugin*>( mPlugin );
    return dtp->Reset( dtp );
}
