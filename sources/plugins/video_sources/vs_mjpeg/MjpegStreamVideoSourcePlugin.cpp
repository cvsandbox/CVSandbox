/*
    JPEG and MJPEG HTTP video source plug-ins of Computer Vision Sandbox

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

#include "MjpegStreamVideoSourcePlugin.hpp"
#include <string.h>
#include <XMjpegHttpStream.hpp>
#include <XVariant.hpp>

using namespace std;
using namespace CVSandbox;
using namespace CVSandbox::Video;
using namespace CVSandbox::Video::MJpeg;

namespace Private
{
    class MjpegStreamVideoSourcePluginData : public IVideoSourceListener
    {
    public:
        MjpegStreamVideoSourcePluginData( const shared_ptr<XMjpegHttpStream>& device ) :
            Device( device ), MjpegUrl( ), UserName( ), Password( ), ForceBasicAuthorization( false ),
            UserCallbacks( { 0 } ), UserParam( 0 )
        {
        }

        virtual void OnNewImage( const shared_ptr<const XImage>& image );
        virtual void OnError( const string& errorMessage );

    public:
        shared_ptr<XMjpegHttpStream> Device;
        string                       MjpegUrl;
        string                       UserName;
        string                       Password;
        bool                         ForceBasicAuthorization;

        VideoSourcePluginCallbacks   UserCallbacks;
        void*                        UserParam;
    };
}

MjpegStreamVideoSourcePlugin::MjpegStreamVideoSourcePlugin( ) :
    mData( new ::Private::MjpegStreamVideoSourcePluginData( XMjpegHttpStream::Create( ) ) )
{
}

MjpegStreamVideoSourcePlugin::~MjpegStreamVideoSourcePlugin( )
{
    delete mData;
}

void MjpegStreamVideoSourcePlugin::Dispose( )
{
    delete this;
}

// Get specified property value of the plug-in
XErrorCode MjpegStreamVideoSourcePlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type         = XVT_String;
        value->value.strVal = XStringAlloc( mData->MjpegUrl.c_str( ) );
        break;

    case 1:
        value->type         = XVT_String;
        value->value.strVal = XStringAlloc( mData->UserName.c_str( ) );
        break;

    case 2:
        value->type         = XVT_String;
        value->value.strVal = XStringAlloc( mData->Password.c_str( ) );
        break;

    case 3:
        value->type          = XVT_Bool;
        value->value.boolVal = mData->ForceBasicAuthorization;
        break;

    default:
        ret = ErrorInvalidProperty;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode MjpegStreamVideoSourcePlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode ret = SuccessCode;
    XVariant   xvar( *value );

    if ( ( id >= 0 ) && ( id <= 2 ) )
    {
        if ( xvar.Type( ) == XVT_String )
        {
            string str = xvar.ToString( &ret );

            if ( ret == SuccessCode )
            {
                switch ( id )
                {
                case 0:
                    if ( mData->Device->SetMjpegUrl( str ) )
                    {
                        mData->MjpegUrl = str;
                    }
                    else
                    {
                        ret = ErrorCannotSetPropertyWhileRunning;
                    }
                    break;
                case 1:
                    if ( mData->Device->SetAuthenticationCredentials( str, mData->Password ) )
                    {
                        mData->UserName = str;
                    }
                    else
                    {
                        ret = ErrorCannotSetPropertyWhileRunning;
                    }
                    break;
                case 2:
                    if ( mData->Device->SetAuthenticationCredentials( mData->UserName, str ) )
                    {
                        mData->Password = str;
                    }
                    else
                    {
                        ret = ErrorCannotSetPropertyWhileRunning;
                    }
                    break;
                }
            }
        }
        else
        {
            ret = ErrorIncompatibleTypes;
        }
    }
    else if ( id == 3 )
    {
        bool forceBasic = xvar.ToBool( );

        if ( mData->Device->SetForceBasicAuthorization( forceBasic ) )
        {
            mData->ForceBasicAuthorization = forceBasic;
        }
        else
        {
            ret = ErrorCannotSetPropertyWhileRunning;
        }
    }
    else
    {
        ret = ErrorInvalidProperty;
    }

    return ret;
}

// Start video source so it initializes and begins providing video frames
XErrorCode MjpegStreamVideoSourcePlugin::Start( )
{
    return mData->Device->Start( );
}

// Signal video to stop, so it could finalize and clean-up
void MjpegStreamVideoSourcePlugin::SignalToStop( )
{
    mData->Device->SignalToStop( );
}

// Wait till video source (its thread) stops
void MjpegStreamVideoSourcePlugin::WaitForStop( )
{
    mData->Device->WaitForStop( );
}

// Check if video source (its thread) is still running
bool MjpegStreamVideoSourcePlugin::IsRunning( )
{
    return mData->Device->IsRunning( );
}

// Terminate video source - call *ONLY* if video source looks to be frozen and does not stop
// by itself when signalled (ideally this method should not exist and be called at all)
void MjpegStreamVideoSourcePlugin::Terminate( )
{
    mData->Device->Terminate( );
}

// Get number of frames received since the the start of the video source
uint32_t MjpegStreamVideoSourcePlugin::FramesReceived( )
{
    return mData->Device->FramesReceived( );
}

// Set callbacks for the video source
void MjpegStreamVideoSourcePlugin::SetCallbacks( const VideoSourcePluginCallbacks* callbacks, void* userParam )
{
    // first unsubscribe from events - this will make sure our local events are done
    mData->Device->SetListener( 0 );

    if ( callbacks != 0 )
    {
        memcpy( &mData->UserCallbacks, callbacks, sizeof( mData->UserCallbacks ) );
        mData->UserParam = userParam;

        mData->Device->SetListener( mData );
    }
    else
    {
        memset( &mData->UserCallbacks, 0, sizeof( mData->UserCallbacks ) );
        mData->UserParam = 0;
    }
}

namespace Private
{

// Handle new image arrived from video source
void MjpegStreamVideoSourcePluginData::OnNewImage( const std::shared_ptr<const XImage>& image )
{
    if ( ( UserCallbacks.NewImageCallback != 0 ) && ( image ) )
    {
        UserCallbacks.NewImageCallback( UserParam, image->ImageData( ) );
    }
}

// Handle error message arrived from the video source
void MjpegStreamVideoSourcePluginData::OnError( const std::string& errorMessage )
{
    if ( UserCallbacks.ErrorMessageCallback != 0 )
    {
        UserCallbacks.ErrorMessageCallback( UserParam, errorMessage.c_str( ) );
    }
}

} // namespace Private
