/*
    DirectShow video source plug-ins of Computer Vision Sandbox

    Copyright (C) 2011-2020, cvsandbox
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

#include "DirectShowVideoSourcePlugin.hpp"
#include <string.h>
#include <XVariant.hpp>
#include <algorithm>
#include <ximaging_formats.h>

using namespace std;
using namespace CVSandbox;
using namespace CVSandbox::Video;
using namespace CVSandbox::Video::DirectShow;

// Some of the camera's properties available through plug-in's API
static const XVideoProperty cameraIntProperties[] =
{
    XVideoProperty::Brightness, XVideoProperty::Contrast, XVideoProperty::Saturation,
    XVideoProperty::Hue, XVideoProperty::Sharpness, XVideoProperty::Gamma
};
static const XVideoProperty cameraBoolProperties[] =
{
    XVideoProperty::ColorEnable, XVideoProperty::BacklightCompensation
};

// extern some API defines in the plug-in descriptor source file
extern string DeviceNameToString( const XDeviceName& device );
extern XDeviceName DeviceNameFromString( const string& str );
extern string ResolutionToString( const XDeviceCapabilities& cap );
extern XDeviceCapabilities ResolutionFromString( const string& str, bool* foundMinMaxFps = nullptr );
extern vector<XDeviceCapabilities> GetDeviceCapabillities( const string& deviceMoniker );

// Some private data
namespace Private
{
    class DirectShowVideoSourcePluginData : public IVideoSourceListener
    {
    public:
        DirectShowVideoSourcePluginData( ) :
            UserCallbacks( { 0 } ), UserParam( 0 ),
            DecodedImage( nullptr ),
            HaveCachedExposure( false ), HaveCachedExposureAuto( false ),
            CachedExposure( 0 ), CachedExposureAuto( false )
        {
        }

        ~DirectShowVideoSourcePluginData( )
        {
            XImageFree( &DecodedImage );
        }

        virtual void OnNewImage( const std::shared_ptr<const XImage>& image );
        virtual void OnError( const std::string& errorMessage );

    public:
        VideoSourcePluginCallbacks  UserCallbacks;
        void*                       UserParam;
        ximage*                     DecodedImage;

        bool    HaveCachedExposure;
        bool    HaveCachedExposureAuto;

        int32_t CachedExposure;
        bool    CachedExposureAuto;
    };
}


DirectShowVideoSourcePlugin::DirectShowVideoSourcePlugin( ) :
    mData( new ::Private::DirectShowVideoSourcePluginData( ) ),
    mDevice( XLocalVideoDevice::Create( ) ), mDeviceName( ), mResolution( ), mFrameRate( 0 ), mVideoInput( 0 )
{
    vector<XDeviceName> devices = XLocalVideoDevice::GetAvailableDevices( );

    if ( devices.size( ) > 0 )
    {
        mDevice->SetDeviceMoniker( devices[0].Moniker( ) );
        // remember the name for further interaction with property manager
        mDeviceName = DeviceNameToString( devices[0] );
        mResolution = ResolutionToString( XDeviceCapabilities( ) );
    }
}

DirectShowVideoSourcePlugin::~DirectShowVideoSourcePlugin( )
{
    delete mData;
}

void DirectShowVideoSourcePlugin::Dispose( )
{
    delete this;
}

// Get property of the plug-in
XErrorCode DirectShowVideoSourcePlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type         = XVT_String;
        value->value.strVal = XStringAlloc( mDeviceName.c_str( ) );
        break;

    case 1:
        value->type         = XVT_String;
        value->value.strVal = XStringAlloc( mResolution.c_str( ) );
        break;

    case 2:
        value->type         = XVT_U2;
        value->value.usVal  = mFrameRate;
        break;

    case 3:
        value->type         = XVT_U1;
        value->value.ubVal  = mVideoInput;
        break;

    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
        ret = mDevice->GetVideoProperty( cameraIntProperties[id - 4], &( value->value.iVal ) );

        if ( ret == SuccessCode )
        {
            value->type = XVT_I4;
        }
        break;

    case 10:
    case 11:
        {
            int32_t propertyValue = 0;

            ret = mDevice->GetVideoProperty( cameraBoolProperties[id - 10], &propertyValue );

            if ( ret == SuccessCode )
            {
                value->type = XVT_Bool;
                value->value.boolVal = ( propertyValue == 1 );
            }
        }
        break;

    case 12:
        {
            int32_t propertyValue  = 0;

            ret = mDevice->GetCameraProperty( XCameraProperty::Exposure, &propertyValue, &( value->value.boolVal ) );

            if ( ret == SuccessCode )
            {
                value->type = XVT_Bool;
            }
        }
        break;

    case 13:
        ret = mDevice->GetCameraProperty( XCameraProperty::Exposure, &( value->value.iVal ) );

        if ( ret == SuccessCode )
        {
            value->type = XVT_I4;
        }
        break;

    default:
        ret = ErrorInvalidProperty;
    }

    return ret;
}

// Set property of the plug-in
XErrorCode DirectShowVideoSourcePlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode ret = SuccessCode;
    XVariant   xvar( *value );

    switch ( id )
    {
    // video device
    case 0:
        if ( xvar.Type( ) == XVT_String )
        {
            string      str        = xvar.ToString( &ret );
            XDeviceName deviceName = DeviceNameFromString( str );

            if ( !deviceName.Moniker( ).empty( ) )
            {
                if ( mDevice->SetDeviceMoniker( deviceName.Moniker( ) ) )
                {
                    mDeviceName = str;
                }
                else
                {
                    ret = ErrorCannotSetPropertyWhileRunning;
                }
            }
            else
            {
                ret = ErrorInvalidFormat;
            }
        }
        else
        {
            ret = ErrorIncompatibleTypes;
        }
        break;

    // resolution
    case 1:
        if ( xvar.Type( ) == XVT_String )
        {
            string              str           = xvar.ToString( &ret );
            bool                foundMinMax   = false;
            XDeviceCapabilities resolutionSet = ResolutionFromString( str, &foundMinMax );

            // in case the resolution string is missing min/max frame rate info, try finding it
            if ( !foundMinMax )
            {
                XDeviceName                 deviceName    = DeviceNameFromString( mDeviceName );
                vector<XDeviceCapabilities> supportedCaps = GetDeviceCapabillities( deviceName.Moniker( ) );

                vector<XDeviceCapabilities>::const_iterator capsIt = std::find( supportedCaps.begin( ), supportedCaps.end( ), resolutionSet );

                if ( capsIt != supportedCaps.end( ) )
                {
                    resolutionSet = *capsIt;
                }
            }

            if ( ( mFrameRate < resolutionSet.MinimumFrameRate( ) ) || ( mFrameRate > resolutionSet.MaximumFrameRate( ) ) )
            {
                mFrameRate = static_cast<uint16_t>( resolutionSet.AverageFrameRate( ) );
            }

            if ( mDevice->SetResolution( resolutionSet, mFrameRate ) )
            {
                mResolution = str;
            }
            else
            {
                ret = ErrorCannotSetPropertyWhileRunning;
            }
        }

        break;

    // frame rate
    case 2:
        {
            uint16_t requestedFps = xvar.ToUShort( &ret );

            if ( ret == SuccessCode )
            {
                XDeviceCapabilities cap = ResolutionFromString( mResolution );

                if ( mDevice->SetResolution( cap, requestedFps ) )
                {
                    mFrameRate = requestedFps;
                }
                else
                {
                    ret = ErrorCannotSetPropertyWhileRunning;
                }
            }
        }
        break;

    // video input
    case 3:
        mVideoInput = xvar.ToUByte( );
        break;

    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
        {
            int32_t propertyValue = xvar.ToInt( &ret );

            if ( ret == SuccessCode )
            {
                ret = mDevice->SetVideoProperty( cameraIntProperties[id - 4], propertyValue );
            }
        }
        break;

    case 10:
    case 11:
        {
            bool propertyBoolValue = xvar.ToBool( &ret );

            if ( ret == SuccessCode )
            {
                ret = mDevice->SetVideoProperty( cameraBoolProperties[id - 10], ( propertyBoolValue ) ? 1 : 0 );
            }
        }
        break;

    case 12:    // Automatic Exposure
    case 13:    // Exposure
        {
            int32_t    propertyValue      = 0;
            int32_t    intValue           = 0;
            bool       isAutomaticControl = false;
            bool       boolValue          = false;

            // convert to required type
            if ( id == 12 )
            {
                boolValue = xvar.ToBool( &ret );
            }
            else
            {
                intValue = xvar.ToInt( &ret );
            }

            if ( ret == SuccessCode )
            {
                // get current settings of Exposure control
                ret = mDevice->GetCameraProperty( XCameraProperty::Exposure, &propertyValue, &isAutomaticControl );

                // set the property user requested
                if ( id == 12 )
                {
                    isAutomaticControl = boolValue;
                }
                else
                {
                    propertyValue = intValue;
                }

                // check if the other half of the property was retieved successfully
                if ( ret == ErrorDeivceNotReady )
                {
                    // device is not yet running, so can not get current settings for exposure control.
                    // need to do some magic here then. if user already set it before, then get the cached version.
                    // if not, try to come up with something sensible values.
                    if ( id == 12 )
                    {
                        // user is setting automatic exposure, so need resolve exposure value
                        propertyValue = ( mData->HaveCachedExposure ) ? mData->CachedExposure : -5;

                        mData->HaveCachedExposureAuto = true;
                        mData->CachedExposureAuto     = isAutomaticControl;
                    }
                    else
                    {
                        // user is setting exposure value, so need to resolve automatic control
                        isAutomaticControl = ( mData->HaveCachedExposureAuto ) ? mData->CachedExposureAuto : false;

                        mData->HaveCachedExposure = true;
                        mData->CachedExposure     = propertyValue;
                    }

                    ret = SuccessCode;
                }
                else
                {
                    mData->HaveCachedExposure     = false;
                    mData->HaveCachedExposureAuto = false;
                }

                if ( ret == SuccessCode )
                {
                    ret = mDevice->SetCameraProperty( XCameraProperty::Exposure, propertyValue, isAutomaticControl );
                }
            }
        }
        break;

    default:
        ret = ErrorInvalidProperty;
        break;
    }

    return ret;
}

// Update description of device run time configuration properties
XErrorCode DirectShowVideoSourcePlugin::UpdateDescription( PluginDescriptor* descriptor )
{
    XErrorCode ret = SuccessCode;

    if ( descriptor == nullptr )
    {
        ret = ErrorNullParameter;
    }
    else if ( !mDevice->IsDeviceRunning( ) )
    {
        ret = ErrorDeivceNotReady;
    }
    else
    {
        // video configuration properties
        if ( !mDevice->IsVideoConfigSupported( ) )
        {
            // device does not support any video configuration, hide all properties
            for ( int i = 4; i <= 11; i++ )
            {
                descriptor->Properties[i]->Flags |= PropertyFlag_Hidden;
            }
        }
        else
        {
            int32_t minValue, maxValue, defaultValue, stepSize;
            bool    isAutoSupported;

            for ( int i = 4; i <= 9; i++ )
            {
                if ( mDevice->GetVideoPropertyRange( cameraIntProperties[i - 4], &minValue, &maxValue, &stepSize, &defaultValue, &isAutoSupported ) != SuccessCode )
                {
                    descriptor->Properties[i]->Flags |= PropertyFlag_Hidden;
                }
                else
                {
                    descriptor->Properties[i]->Flags &= ( ~PropertyFlag_Hidden );

                    descriptor->Properties[i]->DefaultValue.type = XVT_I4;
                    descriptor->Properties[i]->DefaultValue.value.iVal = defaultValue;

                    descriptor->Properties[i]->MinValue.type = XVT_I4;
                    descriptor->Properties[i]->MinValue.value.iVal = minValue;

                    descriptor->Properties[i]->MaxValue.type = XVT_I4;
                    descriptor->Properties[i]->MaxValue.value.iVal = maxValue;
                }
            }

            for ( int i = 10; i <= 11; i++ )
            {
                if ( mDevice->GetVideoPropertyRange( cameraBoolProperties[i - 10], &minValue, &maxValue, &stepSize, &defaultValue, &isAutoSupported ) != SuccessCode )
                {
                    descriptor->Properties[i]->Flags |= PropertyFlag_Hidden;
                }
                else
                {
                    descriptor->Properties[i]->Flags &= ( ~PropertyFlag_Hidden );

                    descriptor->Properties[i]->DefaultValue.type = XVT_Bool;
                    descriptor->Properties[i]->DefaultValue.value.boolVal = ( defaultValue == 1 );
                }
            }
        }

        // camera control properties
        if ( !mDevice->IsCameraConfigSupported( ) )
        {
            descriptor->Properties[12]->Flags |= PropertyFlag_Hidden;
            descriptor->Properties[13]->Flags |= PropertyFlag_Hidden;
        }
        else
        {
            int32_t minValue, maxValue, defaultValue, stepSize;
            bool    isAutoSupported;

            if ( mDevice->GetCameraPropertyRange( XCameraProperty::Exposure, &minValue, &maxValue, &stepSize, &defaultValue, &isAutoSupported ) != SuccessCode )
            {
                descriptor->Properties[12]->Flags |= PropertyFlag_Hidden;
                descriptor->Properties[13]->Flags |= PropertyFlag_Hidden;
            }
            else
            {
                descriptor->Properties[12]->Flags &= ( ~PropertyFlag_Hidden );
                descriptor->Properties[13]->Flags &= ( ~PropertyFlag_Hidden );

                descriptor->Properties[12]->DefaultValue.type = XVT_Bool;
                descriptor->Properties[12]->DefaultValue.value.boolVal = isAutoSupported;

                descriptor->Properties[13]->DefaultValue.type = XVT_I4;
                descriptor->Properties[13]->DefaultValue.value.iVal = defaultValue;

                descriptor->Properties[13]->MinValue.type = XVT_I4;
                descriptor->Properties[13]->MinValue.value.iVal = minValue;

                descriptor->Properties[13]->MaxValue.type = XVT_I4;
                descriptor->Properties[13]->MaxValue.value.iVal = maxValue;
            }
        }
    }

    return ret;
}

// Start video source so it initializes and begins providing video frames
XErrorCode DirectShowVideoSourcePlugin::Start( )
{
    vector<XDevicePinInfo> inputs = mDevice->GetInputVideoPins( );

    mDevice->PreferJpegEncoding( true );

    if ( ( !inputs.empty( ) ) && ( mVideoInput < static_cast<uint8_t>( inputs.size( ) ) ) )
    {
        mDevice->SetVideoInput( inputs[mVideoInput] );
    }

    mData->HaveCachedExposure     = false;
    mData->HaveCachedExposureAuto = false;

    return mDevice->Start( );
}

// Signal video to stop, so it could finalize and cleanup
void DirectShowVideoSourcePlugin::SignalToStop( )
{
    mDevice->SignalToStop( );
}

// Wait till video source (its thread) stops
void DirectShowVideoSourcePlugin::WaitForStop( )
{
    mDevice->WaitForStop( );
}

// Check if video source (its thread) is still running
bool DirectShowVideoSourcePlugin::IsRunning( )
{
    return mDevice->IsRunning( );
}

// Terminate video source - call *ONLY* if video source looks to be frozen and does not stop
// by itself when signaled (ideally this method should not exist and be called at all)
void DirectShowVideoSourcePlugin::Terminate( )
{
    mDevice->Terminate( );
}

// Get number of frames received since the the start of the video source
uint32_t DirectShowVideoSourcePlugin::FramesReceived( )
{
    return mDevice->FramesReceived( );
}

// Set callbacks for the video source
void DirectShowVideoSourcePlugin::SetCallbacks( const VideoSourcePluginCallbacks* callbacks, void* userParam )
{
    // first unsubscribe from events - this will make sure our local events are done
    mDevice->SetListener( 0 );

    if ( callbacks != 0 )
    {
        memcpy( &mData->UserCallbacks, callbacks, sizeof( mData->UserCallbacks ) );
        mData->UserParam = userParam;

        mDevice->SetListener( mData );
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
void DirectShowVideoSourcePluginData::OnNewImage( const std::shared_ptr<const XImage>& image )
{
    if ( ( UserCallbacks.NewImageCallback != nullptr ) && ( image ) )
    {
        if ( image->Format( ) != XPixelFormatJPEG )
        {
            UserCallbacks.NewImageCallback( UserParam, image->ImageData( ) );
        }
        else
        {
            if ( XDecodeJpegFromMemory( image->ImageData( )->data, image->ImageData( )->width, &DecodedImage ) == SuccessCode )
            {
                UserCallbacks.NewImageCallback( UserParam, DecodedImage );
            }
            else
            {
                OnError( "Failed decoding JPEG image" );
            }
        }
    }
}

void DirectShowVideoSourcePluginData::OnError( const std::string& errorMessage )
{
    if ( UserCallbacks.ErrorMessageCallback != nullptr )
    {
        UserCallbacks.ErrorMessageCallback( UserParam, errorMessage.c_str( ) );
    }
}

} // namespace Private
