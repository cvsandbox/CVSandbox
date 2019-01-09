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

#include "JpegStreamVideoSourcePlugin.hpp"
#include <string.h>
#include <XJpegHttpStream.hpp>
#include <XVariant.hpp>

using namespace std;
using namespace CVSandbox;
using namespace CVSandbox::Video;
using namespace CVSandbox::Video::MJpeg;

namespace Private
{
    class JpegStreamVideoSourcePluginData : public IVideoSourceListener
    {
    public:
        JpegStreamVideoSourcePluginData( const shared_ptr<XJpegHttpStream>& device ) :
            Device( device ), JpegUrl( ), UserName( ), Password( ), ForceBasicAuthorization( false ), FrameIntervalMs( 100 ),
            UserCallbacks( { 0 } ), UserParam( 0 )
        {
        }

        virtual void OnNewImage( const shared_ptr<const XImage>& image );
        virtual void OnError( const string& errorMessage );

    public:
        shared_ptr<XJpegHttpStream> Device;
        string                      JpegUrl;
        string                      UserName;
        string                      Password;
        bool                        ForceBasicAuthorization;
        uint16_t                    FrameIntervalMs;

        VideoSourcePluginCallbacks  UserCallbacks;
        void*                       UserParam;
    };
}

JpegStreamVideoSourcePlugin::JpegStreamVideoSourcePlugin( ) :
    mData( new ::Private::JpegStreamVideoSourcePluginData( XJpegHttpStream::Create( ) ) )
{
}

JpegStreamVideoSourcePlugin::~JpegStreamVideoSourcePlugin( )
{
    delete mData;
}

void JpegStreamVideoSourcePlugin::Dispose( )
{
    delete this;
}

// Get specified property value of the plug-in
XErrorCode JpegStreamVideoSourcePlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type         = XVT_String;
        value->value.strVal = XStringAlloc( mData->JpegUrl.c_str( ) );
        break;

    case 1:
        value->type        = XVT_U2;
        value->value.usVal = mData->FrameIntervalMs;
        break;

    case 2:
        value->type         = XVT_String;
        value->value.strVal = XStringAlloc( mData->UserName.c_str( ) );
        break;

    case 3:
        value->type         = XVT_String;
        value->value.strVal = XStringAlloc( mData->Password.c_str( ) );
        break;

    case 4:
        value->type          = XVT_Bool;
        value->value.boolVal = mData->ForceBasicAuthorization;
        break;

    default:
        ret = ErrorInvalidProperty;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode JpegStreamVideoSourcePlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode ret = SuccessCode;
    XVariant   xvar( *value );

    if ( ( id == 0 ) || ( id == 2 ) || ( id == 3 ) )
    {
        if ( xvar.Type( ) == XVT_String )
        {
            string str = xvar.ToString( &ret );

            if ( ret == SuccessCode )
            {
                switch ( id )
                {
                case 0:
                    if ( mData->Device->SetJpegUrl( str ) )
                    {
                        mData->JpegUrl = str;
                    }
                    else
                    {
                        ret = ErrorCannotSetPropertyWhileRunning;
                    }
                    break;
                case 2:
                    if ( mData->Device->SetAuthenticationCredentials( str, mData->Password ) )
                    {
                        mData->UserName = str;
                    }
                    else
                    {
                        ret = ErrorCannotSetPropertyWhileRunning;
                    }
                    break;
                case 3:
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
    else if ( id == 1 )
    {
        uint16_t frameInterval = xvar.ToUShort( );

        if ( mData->Device->SetFrameInterval( frameInterval ) )
        {
            mData->FrameIntervalMs = frameInterval;
        }
        else
        {
            ret = ErrorCannotSetPropertyWhileRunning;
        }
    }
    else if ( id == 4 )
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
XErrorCode JpegStreamVideoSourcePlugin::Start( )
{
    return mData->Device->Start( );
}

// Signal video to stop, so it could finalize and clean-up
void JpegStreamVideoSourcePlugin::SignalToStop( )
{
    mData->Device->SignalToStop( );
}

// Wait till video source (its thread) stops
void JpegStreamVideoSourcePlugin::WaitForStop( )
{
    mData->Device->WaitForStop( );
}

// Check if video source (its thread) is still running
bool JpegStreamVideoSourcePlugin::IsRunning( )
{
    return mData->Device->IsRunning( );
}

// Terminate video source - call *ONLY* if video source looks to be frozen and does not stop
// by itself when signalled (ideally this method should not exist and be called at all)
void JpegStreamVideoSourcePlugin::Terminate( )
{
    mData->Device->Terminate( );
}

// Get number of frames received since the the start of the video source
uint32_t JpegStreamVideoSourcePlugin::FramesReceived( )
{
    return mData->Device->FramesReceived( );
}

// Set callbacks for the video source
void JpegStreamVideoSourcePlugin::SetCallbacks( const VideoSourcePluginCallbacks* callbacks, void* userParam )
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

// Handle new image arrived from the video source
void JpegStreamVideoSourcePluginData::OnNewImage( const std::shared_ptr<const XImage>& image )
{
    if ( ( UserCallbacks.NewImageCallback != 0 ) && ( image ) )
    {
        UserCallbacks.NewImageCallback( UserParam, image->ImageData( ) );
    }
}

// Handle error message arrived from the video source
void JpegStreamVideoSourcePluginData::OnError( const std::string& errorMessage )
{
    if ( UserCallbacks.ErrorMessageCallback != 0 )
    {
        UserCallbacks.ErrorMessageCallback( UserParam, errorMessage.c_str( ) );
    }
}

} // namespace Private
