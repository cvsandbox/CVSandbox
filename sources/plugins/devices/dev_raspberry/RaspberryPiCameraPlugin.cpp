/*
    Raspberry Pi plug-ins for Computer Vision Sandbox

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

#include "RaspberryPiCameraPlugin.hpp"
#include <string.h>
#include <XJpegHttpStream.hpp>
#include <XMjpegHttpStream.hpp>
#include <XHttpCommandsThread.hpp>
#include <XVariant.hpp>
#include <XMutex.hpp>

using namespace std;
using namespace CVSandbox;
using namespace CVSandbox::Video;
using namespace CVSandbox::Video::MJpeg;
using namespace CVSandbox::Threading;

namespace Private
{
    const static char* CAMERA_CONFIG_URL = "/camera/config";

    const static char* SupportedAwbModes[] =
    {
        "Off", "Auto", "Sunlight", "Cloudy", "Shade", "Tungsten",
        "Fluorescent", "Incandescent", "Flash", "Horizon"
    };

    const static char* SupportedExposureModes[] =
    {
        "Off", "Auto", "Night", "NightPreview", "Backlight", "Spotlight",
        "Sports", "Snow", "Beach", "VeryLong", "FixedFps", "AntiShake", "Fireworks"
    };

    const static char* SupportedExposureMeteringModes[] =
    {
        "Average", "Spot", "Backlit", "Matrix"
    };

    const static char* SupportedImageEffects[] =
    {
        "None", "Negative", "Solarize", "Sketch", "Denoise", "Emboss", "OilPaint",
        "Hatch", "Gpen", "Pastel", "WaterColor", "Film", "Blur", "Saturation",
        "ColorSwap", "WashedOut", "Posterise", "ColorPoint", "ColorBalance", "Cartoon"
    };

    enum class Protocol
    {
        MJpeg = 0,
        Jpeg  = 1
    };

    class VideoSourceListner : public IVideoSourceListener
    {
    public:
        virtual void OnNewImage( const shared_ptr<const XImage>& image )
        {
            VideoSource_OnNewImage( image );
        }
        virtual void OnError( const string& errorMessage )
        {
            VideoSource_OnError( errorMessage );
        }

        virtual void VideoSource_OnNewImage( const shared_ptr<const XImage>& image ) = 0;
        virtual void VideoSource_OnError( const string& errorMessage ) = 0;
    };

    class HttpCommandResultListener : public IHttpCommandResultListener
    {
        virtual void OnCompletion( uint32_t requestId, const uint8_t* data, uint32_t dataLength )
        {
            HttpCommand_OnCompletion( requestId, data, dataLength );
        }
        virtual void OnError( uint32_t requestId, const string& errorMessage )
        {
            HttpCommand_OnError( requestId, errorMessage );
        }

        virtual void HttpCommand_OnCompletion( uint32_t requestId, const uint8_t* data, uint32_t dataLength ) = 0;
        virtual void HttpCommand_OnError( uint32_t requestId, const string& errorMessage ) = 0;
    };

    class RaspberryPiCameraPluginData : public VideoSourceListner, public HttpCommandResultListener
    {
    public:
        RaspberryPiCameraPluginData( ) :
            Sync( ), Device( ), CommandsThread( XHttpCommandsThread::Create( ) ),
            Address( ), UserName( ), Password( ),
            Port( 8000 ), FrameInterval( 100 ), AccessProtocol( Protocol::MJpeg ),
            Brightness( 50 ), Contrast( 0 ), Saturation( 0 ), Sharpness( 0 ),
            HorizontalFlip( false ), VerticalFlip( false ), VideoStabilization( false ),
            AwbMode( 1 ), ExposureMode( 1 ), ExposureMeteringMode( 0 ), ImageEffect( 0 ),
            UserCallbacks( { 0 } ), UserParam( nullptr ),
            VersionRequestId( 0 ), VersionRequestCounter( 0 )
        {
        }

        virtual void VideoSource_OnNewImage( const shared_ptr<const XImage>& image );
        virtual void VideoSource_OnError( const string& errorMessage );

        virtual void HttpCommand_OnCompletion( uint32_t requestId, const uint8_t* data, uint32_t dataLength );
        virtual void HttpCommand_OnError( uint32_t requestId, const string& errorMessage );

        void StopFakeCamera( );

        void ApplyCurrentConfiguration( );

    public:
        XMutex                          Sync;
        shared_ptr<IVideoSource>        Device;
        shared_ptr<XHttpCommandsThread> CommandsThread;
        string                          Address;
        string                          UserName;
        string                          Password;
        uint16_t                        Port;
        uint16_t                        FrameInterval;
        Protocol                        AccessProtocol;

        int32_t                         Brightness;
        int32_t                         Contrast;
        int32_t                         Saturation;
        int32_t                         Sharpness;

        bool                            HorizontalFlip;
        bool                            VerticalFlip;
        bool                            VideoStabilization;

        uint8_t                         AwbMode;
        uint8_t                         ExposureMode;
        uint8_t                         ExposureMeteringMode;
        uint8_t                         ImageEffect;

        VideoSourcePluginCallbacks      UserCallbacks;
        void*                           UserParam;

        uint32_t                        VersionRequestId;
        uint32_t                        VersionRequestCounter;
    };
}

RaspberryPiCameraPlugin::RaspberryPiCameraPlugin( ) :
    mData( new ::Private::RaspberryPiCameraPluginData( ) )
{
}

RaspberryPiCameraPlugin::~RaspberryPiCameraPlugin( )
{
    delete mData;
}

void RaspberryPiCameraPlugin::Dispose( )
{
    delete this;
}

// Get specified property value of the plug-in
XErrorCode RaspberryPiCameraPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type         = XVT_String;
        value->value.strVal = XStringAlloc( mData->Address.c_str( ) );
        break;

    case 1:
        value->type         = XVT_U2;
        value->value.usVal  = mData->Port;
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
        value->type         = XVT_U1;
        value->value.ubVal  = static_cast<uint8_t>( mData->AccessProtocol );
        break;

    case 5:
        value->type         = XVT_U2;
        value->value.usVal  = mData->FrameInterval;
        break;

    case 6:
        value->type         = XVT_I4;
        value->value.iVal   = mData->Brightness;
        break;

    case 7:
        value->type         = XVT_I4;
        value->value.iVal   = mData->Contrast;
        break;

    case 8:
        value->type         = XVT_I4;
        value->value.iVal   = mData->Saturation;
        break;

    case 9:
        value->type         = XVT_I4;
        value->value.iVal   = mData->Sharpness;
        break;

    case 10:
        value->type         = XVT_Bool;
        value->value.iVal   = mData->HorizontalFlip;
        break;

    case 11:
        value->type         = XVT_Bool;
        value->value.iVal   = mData->VerticalFlip;
        break;

    case 12:
        value->type         = XVT_Bool;
        value->value.iVal   = mData->VideoStabilization;
        break;

    case 13:
        value->type         = XVT_U1;
        value->value.ubVal  = mData->AwbMode;
        break;

    case 14:
        value->type         = XVT_U1;
        value->value.ubVal  = mData->ExposureMode;
        break;

    case 15:
        value->type         = XVT_U1;
        value->value.ubVal  = mData->ExposureMeteringMode;
        break;

    case 16:
        value->type         = XVT_U1;
        value->value.ubVal  = mData->ImageEffect;
        break;

    default:
        ret = ErrorInvalidProperty;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode RaspberryPiCameraPlugin::SetProperty( int32_t id, const xvariant* value )
{
    const static char* runTimePropertyNames[] =
    {
        "brightness", "contrast", "saturation", "sharpness", "hflip", "vflip", "videostabilisation",
        "awb", "expmode", "expmeteringmode", "effect"
    };

    XScopedLock lock( &mData->Sync );

    XErrorCode  ret                   = SuccessCode;
    const char* propertyNameToSet     = nullptr;
    int32_t     propertyValueToSet    = 0;
    const char* propertyValueToSetStr = nullptr;

    xvariant convertedValue;
    XVariantInit( &convertedValue );

    // make sure property value has expected type
    ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 17, &convertedValue );

    if ( ret == SuccessCode )
    {
        switch ( id )
        {
        case 0:
            mData->Address = convertedValue.value.strVal;
            break;

        case 1:
            mData->Port = convertedValue.value.usVal;
            break;

        case 2:
            mData->UserName = convertedValue.value.strVal;
            break;

        case 3:
            mData->Password = convertedValue.value.strVal;
            break;

        case 4:
            mData->AccessProtocol = static_cast<::Private::Protocol>( XINRANGE( convertedValue.value.ubVal, 0, 1 ) );
            break;

        case 5:
            mData->FrameInterval = convertedValue.value.usVal;
            break;

        case 6:
            propertyValueToSet = mData->Brightness = convertedValue.value.iVal;
            break;

        case 7:
            propertyValueToSet = mData->Contrast = convertedValue.value.iVal;
            break;

        case 8:
            propertyValueToSet = mData->Saturation = convertedValue.value.iVal;
            break;

        case 9:
            propertyValueToSet = mData->Sharpness = convertedValue.value.iVal;
            break;

        case 10:
            mData->HorizontalFlip = convertedValue.value.boolVal;
            propertyValueToSet    = ( convertedValue.value.boolVal ) ? 1 : 0;
            break;

        case 11:
            mData->VerticalFlip = convertedValue.value.boolVal;
            propertyValueToSet  = ( convertedValue.value.boolVal ) ? 1 : 0;
            break;

        case 12:
            mData->VideoStabilization = convertedValue.value.boolVal;
            propertyValueToSet        = ( convertedValue.value.boolVal ) ? 1 : 0;
            break;

        case 13:
            mData->AwbMode = XINRANGE( convertedValue.value.ubVal, 0, XARRAY_SIZE( ::Private::SupportedAwbModes ) - 1 );
            propertyValueToSetStr = ::Private::SupportedAwbModes[mData->AwbMode];
            break;

        case 14:
            mData->ExposureMode = XINRANGE( convertedValue.value.ubVal, 0, XARRAY_SIZE( ::Private::SupportedExposureModes ) - 1 );
            propertyValueToSetStr = ::Private::SupportedExposureModes[mData->ExposureMode];
            break;

        case 15:
            mData->ExposureMeteringMode = XINRANGE( convertedValue.value.ubVal, 0, XARRAY_SIZE( ::Private::SupportedExposureMeteringModes ) - 1 );
            propertyValueToSetStr = ::Private::SupportedExposureMeteringModes[mData->ExposureMeteringMode];
            break;

        case 16:
            mData->ImageEffect = XINRANGE( convertedValue.value.ubVal, 0, XARRAY_SIZE( ::Private::SupportedImageEffects ) - 1 );
            propertyValueToSetStr = ::Private::SupportedImageEffects[mData->ImageEffect];
            break;
        }

        if ( ( id >= 6 ) && ( id <= 16 ) )
        {
            propertyNameToSet = runTimePropertyNames[id - 6];
        }

        // set run time property if camera is running
        if ( ( propertyNameToSet != nullptr ) && ( mData->CommandsThread->IsRunning( ) ) )
        {
            char commandBuffer[64];

            if ( propertyValueToSetStr == nullptr )
            {
                sprintf( commandBuffer, "{\"%s\":%d}", propertyNameToSet, propertyValueToSet );
            }
            else
            {
                sprintf( commandBuffer, "{\"%s\":\"%s\"}", propertyNameToSet, propertyValueToSetStr );
            }

            mData->CommandsThread->EnqueuePostRequest( ::Private::CAMERA_CONFIG_URL, commandBuffer );
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}

// The device runtime properties we have don't have dynamic limits, so nothing really to update here
XErrorCode RaspberryPiCameraPlugin::UpdateDescription( PluginDescriptor* /* descriptor */ )
{

    return SuccessCode;
}

// Start video source so it initializes and begins providing video frames
XErrorCode RaspberryPiCameraPlugin::Start( )
{
    XScopedLock lock( &mData->Sync );
    XErrorCode  ret = SuccessCode;

    if ( mData->Address.empty( ) )
    {
        ret = ErrorInvalidConfiguration;
    }
    else
    {
        char* baseUrl = new char[mData->Address.length( ) + 32];

        sprintf( baseUrl, "http://%s:%u", mData->Address.c_str( ), mData->Port );

        // configure commands thread
        mData->CommandsThread->SetBaseAddress( baseUrl );
        mData->CommandsThread->SetAuthenticationCredentials( mData->UserName, mData->Password );

        // configure video source device
        if ( mData->AccessProtocol == ::Private::Protocol::Jpeg )
        {
            shared_ptr<XJpegHttpStream> device = XJpegHttpStream::Create( );

            strcat( baseUrl, "/camera/jpeg" );

            device->SetJpegUrl( baseUrl );
            device->SetAuthenticationCredentials( mData->UserName, mData->Password );
            device->SetFrameInterval( mData->FrameInterval );

            mData->Device = device;
        }
        else
        {
            shared_ptr<XMjpegHttpStream> device = XMjpegHttpStream::Create( );

            strcat( baseUrl, "/camera/mjpeg" );

            device->SetMjpegUrl( baseUrl );
            device->SetAuthenticationCredentials( mData->UserName, mData->Password );

            mData->Device = device;
        }

        // set listeners for both video source and commands' thread
        if ( ( mData->UserCallbacks.NewImageCallback != nullptr ) || ( mData->UserCallbacks.ErrorMessageCallback != nullptr ) )
        {
            mData->Device->SetListener( static_cast<IVideoSourceListener*>( mData ) );
        }
        if ( mData->UserCallbacks.ErrorMessageCallback != nullptr )
        {
            mData->CommandsThread->SetListener( static_cast<IHttpCommandResultListener*>( mData ) );
        }

        delete [] baseUrl;

        // ignore anything in the commands' queue - current configuration will be applied soon anyway
        mData->CommandsThread->ClearRequestsQueue( );

        // start commands thread and JPEG/MJPEG device
        ret = mData->CommandsThread->Start( );

        if ( ret == SuccessCode )
        {
            // enqueue version request, so we could check if we are really talking to Pi camera
            mData->VersionRequestCounter = 0;
            mData->VersionRequestId      = mData->CommandsThread->EnqueueGetRequest( "/version" );
            // apply existing configuration
            mData->ApplyCurrentConfiguration( );

            ret = mData->Device->Start( );

            if ( ret != SuccessCode )
            {
                mData->CommandsThread->SignalToStop( );
            }
        }
    }

    return ret;
}

// Signal video to stop, so it could finalize and cleanup
void RaspberryPiCameraPlugin::SignalToStop( )
{
    XScopedLock lock( &mData->Sync );

    if ( mData->Device )
    {
        mData->Device->SignalToStop( );
    }

    mData->CommandsThread->SignalToStop( );
}

// Wait till video source (its thread) stops
void RaspberryPiCameraPlugin::WaitForStop( )
{
    XScopedLock lock( &mData->Sync );

    if ( mData->Device )
    {
        mData->Device->WaitForStop( );
    }

    mData->CommandsThread->WaitForStop( );
}

// Check if video source (its thread) is still running
bool RaspberryPiCameraPlugin::IsRunning( )
{
    XScopedLock lock( &mData->Sync );

    return ( mData->Device ) ? mData->Device->IsRunning( ) : false;
}

// Terminate video source - call *ONLY* if video source looks to be frozen and does not stop
// by itself when signaled (ideally this method should not exist and be called at all)
void RaspberryPiCameraPlugin::Terminate( )
{
    XScopedLock lock( &mData->Sync );

    if ( mData->Device )
    {
        mData->Device->Terminate( );
    }

    mData->CommandsThread->SignalToStop( );
    mData->CommandsThread->WaitForStop( );
}

// Get number of frames received since the the start of the video source
uint32_t RaspberryPiCameraPlugin::FramesReceived( )
{
    XScopedLock lock( &mData->Sync );

    return ( mData->Device ) ? mData->Device->FramesReceived( ) : 0;
}

// Set callbacks for the video source
void RaspberryPiCameraPlugin::SetCallbacks( const VideoSourcePluginCallbacks* callbacks, void* userParam )
{
    {
        XScopedLock lock( &mData->Sync );

        // first unsubscribe from events - this will make sure our local events are done
        if ( mData->Device )
        {
            mData->Device->SetListener( nullptr );
        }
        mData->CommandsThread->SetListener( nullptr );
    }

    if ( callbacks != 0 )
    {
        memcpy( &mData->UserCallbacks, callbacks, sizeof( mData->UserCallbacks ) );
        mData->UserParam = userParam;

        {
            XScopedLock lock( &mData->Sync );

            if ( mData->Device )
            {
                mData->Device->SetListener( static_cast<IVideoSourceListener*>( mData ) );
            }
            mData->CommandsThread->SetListener( static_cast<IHttpCommandResultListener*>( mData ) );
        }
    }
    else
    {
        memset( &mData->UserCallbacks, 0, sizeof( mData->UserCallbacks ) );
        mData->UserParam = nullptr;
    }
}

namespace Private
{

// Handle new image arrived from video source
void RaspberryPiCameraPluginData::VideoSource_OnNewImage( const shared_ptr<const XImage>& image )
{
    if ( ( UserCallbacks.NewImageCallback != nullptr ) && ( image ) )
    {
        UserCallbacks.NewImageCallback( UserParam, image->ImageData( ) );
    }
}

// Handle error message arrived from the video source
void RaspberryPiCameraPluginData::VideoSource_OnError( const string& errorMessage )
{
    if ( UserCallbacks.ErrorMessageCallback != nullptr )
    {
        UserCallbacks.ErrorMessageCallback( UserParam, errorMessage.c_str( ) );
    }
}

// Handle completion of a HTTP command
void RaspberryPiCameraPluginData::HttpCommand_OnCompletion( uint32_t requestId, const uint8_t* data, uint32_t /* dataLength */ )
{
    if ( requestId == VersionRequestId )
    {
        if ( ( ( strstr( (char*) data, "\"product\":\"cam2web\"" )  == nullptr ) &&
               ( strstr( (char*) data, "\"product\":\"pirexbot\"" ) == nullptr ) )
            ||
               ( strstr( (char*) data, "\"platform\":\"RaspberryPi\"" ) == nullptr ) )
        {
            StopFakeCamera( );
        }
    }
    else
    {
        const char* ptr1 = strstr( (const char*) data, "\"status\":\"" );
        const char* ptr2 = nullptr;

        if ( ptr1 != nullptr )
        {
            ptr1 += 10;
            ptr2  = strchr( ptr1, '"' );

            if ( ptr2 != nullptr )
            {
                string propertySetStatus = string( ptr1, ptr2 );

                if ( propertySetStatus != "OK" )
                {
                    if ( UserCallbacks.ErrorMessageCallback != nullptr )
                    {
                        char* msg = new char[64 + propertySetStatus.length( )];

                        sprintf( msg, "Failed setting camera property: %s", propertySetStatus.c_str( ) );
                        UserCallbacks.ErrorMessageCallback( UserParam, msg );

                        delete [] msg;
                    }
                }
            }
        }
    }
}

// Handle error message arrived from the commands' thread
void RaspberryPiCameraPluginData::HttpCommand_OnError( uint32_t requestId, const string& errorMessage )
{
    if ( requestId == VersionRequestId )
    {
        // if we get "HTTP 404" for version request, then it is definitely not what we expect
        if ( errorMessage.find( "HTTP 404" ) == 0 )
        {
            StopFakeCamera( );
        }
        else
        {
            // if failed checking version, try few more times
            if ( VersionRequestCounter++ < 5 )
            {
                VersionRequestId = CommandsThread->EnqueueGetRequest( "/version" );
            }
        }
    }
    else
    {
        if ( UserCallbacks.ErrorMessageCallback != nullptr )
        {
            char* msg = new char[64 + errorMessage.length( )];

            sprintf( msg, "Failed setting camera property: %s", errorMessage.c_str( ) );
            UserCallbacks.ErrorMessageCallback( UserParam, msg );

            delete [] msg;
        }
    }
}

// Stop camera, which does not look like Raspberry Pi running cam2web
void RaspberryPiCameraPluginData::StopFakeCamera( )
{
    {
        XScopedLock lock( &Sync );

        if ( Device )
        {
            Device->SignalToStop( );
        }

        CommandsThread->SignalToStop( );
    }

    if ( UserCallbacks.ErrorMessageCallback != nullptr )
    {
        UserCallbacks.ErrorMessageCallback( UserParam, "Does not look like a Raspberry Pi camera" );
    }
}

// Apply all current configuration
void RaspberryPiCameraPluginData::ApplyCurrentConfiguration( )
{
    char commandBuffer[512];

    sprintf( commandBuffer, "{\"brightness\":%d, \"contrast\":%d, \"saturation\":%d, \"sharpness\":%d, "
        "\"hflip\":%d, \"vflip\":%d, \"videostabilisation\":%d, "
        "\"awb\":\"%s\", \"expmode\":\"%s\", \"expmeteringmode\":\"%s\", \"effect\":\"%s\" }",
        Brightness, Contrast, Saturation, Sharpness, HorizontalFlip, VerticalFlip, VideoStabilization,
        ::Private::SupportedAwbModes[AwbMode], ::Private::SupportedExposureModes[ExposureMode],
        ::Private::SupportedExposureMeteringModes[ExposureMeteringMode], ::Private::SupportedImageEffects[ImageEffect] );

    CommandsThread->EnqueuePostRequest( CAMERA_CONFIG_URL, commandBuffer );
}

} // namespace Private
