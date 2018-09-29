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

#include "XVideoSourcePlugin.hpp"

using namespace std;
using namespace CVSandbox;

XVideoSourcePlugin::XVideoSourcePlugin( void* plugin, bool ownIt ) :
    XPlugin( plugin, PluginType_VideoSource, ownIt ),
    mListener( nullptr )
{
}

XVideoSourcePlugin::~XVideoSourcePlugin( )
{
}

// Create plug-in wrapper
const shared_ptr<XVideoSourcePlugin> XVideoSourcePlugin::Create( void* plugin, bool ownIt )
{
    return shared_ptr<XVideoSourcePlugin>( new XVideoSourcePlugin( plugin, ownIt ) );
}

// Start video source so it initializes and begins providing video frames
XErrorCode XVideoSourcePlugin::Start( )
{
    SVideoSourcePlugin* vsp = static_cast<SVideoSourcePlugin*>( mPlugin );
    return vsp->Start( vsp );
}

// Signal video to stop, so it could finalize and clean-up
void XVideoSourcePlugin::SignalToStop( )
{
    SVideoSourcePlugin* vsp = static_cast<SVideoSourcePlugin*>( mPlugin );
    vsp->SignalToStop( vsp );
}

// Wait till video source (its thread) stops
void XVideoSourcePlugin::WaitForStop( )
{
    SVideoSourcePlugin* vsp = static_cast<SVideoSourcePlugin*>( mPlugin );
    vsp->WaitForStop( vsp );
}

// Check if video source (its thread) is still running
bool XVideoSourcePlugin::IsRunning( )
{
    SVideoSourcePlugin* vsp = static_cast<SVideoSourcePlugin*>( mPlugin );
    return vsp->IsRunning( vsp );
}

// Terminate video source - call *ONLY* if video source looks to be frozen and does not stop
// by itself when signalled (ideally this method should not exist and be called at all)
void XVideoSourcePlugin::Terminate( )
{
    SVideoSourcePlugin* vsp = static_cast<SVideoSourcePlugin*>( mPlugin );
    vsp->Terminate( vsp );
}

// Get number of frames received since the the start of the video source
uint32_t XVideoSourcePlugin::FramesReceived( )
{
    SVideoSourcePlugin* vsp = static_cast<SVideoSourcePlugin*>( mPlugin );
    return vsp->FramesReceived( vsp );
}

// Set video source plug-in listener
void XVideoSourcePlugin::SetListener( IVideoSourcePluginListener* listener )
{
    if ( listener != mListener )
    {
        SVideoSourcePlugin* vsp = static_cast<SVideoSourcePlugin*>( mPlugin );

        // first unsubscribe from events - this will make sure our local handlers are done
        vsp->SetCallbacks( vsp, nullptr, nullptr );

        mListener = listener;

        if ( listener != nullptr )
        {
            VideoSourcePluginCallbacks callbacks = { 0 };

            callbacks.NewImageCallback      = NewImageHandler;
            callbacks.ErrorMessageCallback  = ErrorMessageHandler;

            // subscribe again
            vsp->SetCallbacks( vsp, &callbacks, this );
        }
    }
}

// New image comes from a plug-in
void XVideoSourcePlugin::NewImageHandler( void* userParam, const ximage* image )
{
    XVideoSourcePlugin*      me           = static_cast<XVideoSourcePlugin*>( userParam );
    shared_ptr<const XImage> guardedImage = XImage::Create( image );

    if ( me->mListener != nullptr )
    {
        me->mListener->OnNewImage( guardedImage );
    }
}

// Error message comes from a plu-in
void XVideoSourcePlugin::ErrorMessageHandler( void* userParam, const char* errorMessage )
{
    XVideoSourcePlugin* me = static_cast<XVideoSourcePlugin*>( userParam );

    if ( me->mListener != nullptr )
    {
        me->mListener->OnError( string( errorMessage ) );
    }
}
