/*
    DirectShow video source plug-ins of Computer Vision Sandbox

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

#include <string>
#include <map>
#include <algorithm>
#include <iplugincpp.hpp>
#include <image_local_device_16x16.h>
#include "DirectShowVideoSourcePlugin.hpp"

using namespace std;
using namespace CVSandbox;
using namespace CVSandbox::Video;
using namespace CVSandbox::Video::DirectShow;

static map<string, vector<XDeviceCapabilities>> videoCapabilitiesCache;
static map<string, vector<XDevicePinInfo>>      videoInputsCache;
static vector<XDeviceCapabilities>              lastAccessedCapabilities;

static void PluginInitializer( );
static void PluginCleaner( );
static XErrorCode UpdateProperties( PluginDescriptor* desc );
static XErrorCode UpdateResolutionProperty( PropertyDescriptor* desc, const xvariant* parentValue );
static XErrorCode UpdateFrameRateProperty( PropertyDescriptor* desc, const xvariant* parentValue );
static XErrorCode UpdateVideoInputProperty( PropertyDescriptor* desc, const xvariant* parentValue );
static XErrorCode UpdateExposureProperty( PropertyDescriptor* desc, const xvariant* parentValue );

// Version of the plug-in
static xversion PluginVersion = { 1, 1, 2 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000004, 0x00000001 };

// Device property
static PropertyDescriptor deviceProperty =
{ XVT_String, "Device", "device", "Local video device to capture images from.", PropertyFlag_SelectionByValue | PropertyFlag_RuntimeConfiguration };
// Resolution property
static PropertyDescriptor resolutionProperty =
{ XVT_String, "Resolution", "resolution", "Video device resolution to use.", PropertyFlag_SelectionByValue | PropertyFlag_RuntimeConfiguration | PropertyFlag_Dependent };
// Frame Rate property
static PropertyDescriptor frameRateProperty =
{ XVT_U2, "Frame Rate", "frameRate", "Frame rate to set (if supported).", PropertyFlag_RuntimeConfiguration | PropertyFlag_Dependent };
// Video input property
static PropertyDescriptor videoInputProperty =
{ XVT_U1, "Video input", "videoInput", "Video input to use (if supported).", PropertyFlag_SelectionByIndex | PropertyFlag_RuntimeConfiguration | PropertyFlag_Dependent };

// >>>>> Device run-time properties

// Brightness property
static PropertyDescriptor brightnessProperty =
{ XVT_I4, "Brightness", "brightness", "Brightness level of camera images.", PropertyFlag_DeviceRuntimeConfiguration };
// Contrast property
static PropertyDescriptor contrastProperty =
{ XVT_I4, "Contrast", "contrast", "Contrast level of camera images.", PropertyFlag_DeviceRuntimeConfiguration };
// Saturation property
static PropertyDescriptor saturationProperty =
{ XVT_I4, "Saturation", "saturation", "Saturation level of camera images.", PropertyFlag_DeviceRuntimeConfiguration };
// Hue property
static PropertyDescriptor hueProperty =
{ XVT_I4, "Hue", "hue", "Hue level of camera images.", PropertyFlag_DeviceRuntimeConfiguration };
// Sharpness property
static PropertyDescriptor sharpnessProperty =
{ XVT_I4, "Sharpness", "sharpness", "Sharpness of camera images.", PropertyFlag_DeviceRuntimeConfiguration };
// Gamma property
static PropertyDescriptor gammaProperty =
{ XVT_I4, "Gamma", "gamma", "Gamma level of camera images.", PropertyFlag_DeviceRuntimeConfiguration };

// Color Image property
static PropertyDescriptor colorImagesProperty =
{ XVT_Bool, "Color Image", "colorImages", "Specifies if camera should provide color or gray image.", PropertyFlag_DeviceRuntimeConfiguration };
// Back-light Compensation property
static PropertyDescriptor backlightCompensationProperty =
{ XVT_Bool, "Back-light Compensation", "backlightCompensation", "Specifies if back-light compensation should be on or off.", PropertyFlag_DeviceRuntimeConfiguration };

// Automatic Exposure property
static PropertyDescriptor automaticExposureProperty =
{ XVT_Bool, "Automatic Exposure", "automaticExposure", "Specifies if exposure is controlled automatically or not.", PropertyFlag_DeviceRuntimeConfiguration };
// Exposure property
static PropertyDescriptor exposureProperty =
{ XVT_I4, "Exposure", "exposure", "Sets camera's exposure level (in log base 2 seconds).", PropertyFlag_DeviceRuntimeConfiguration | PropertyFlag_Dependent };

// <<<<<

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &deviceProperty, &resolutionProperty, &frameRateProperty, &videoInputProperty,

    &brightnessProperty, &contrastProperty, &saturationProperty,
    &hueProperty, &sharpnessProperty, &gammaProperty,

    &colorImagesProperty, &backlightCompensationProperty,

    &automaticExposureProperty, &exposureProperty
};

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_VideoSource,

    PluginType_VideoSource,
    PluginVersion,
    "Local Capture Device",
    "LocalCaptureDevice",
    "Plug-in to access local capture devices (USB cameras, capture boards, etc) exposed through DirectShow interface.",

    /* Long description */
    "The plug-in is aimed to allow capturing video from different video devices supporting DirectShow interfaces. Most "
    "common examples of those are laptops' integrated cameras, USB web cameras and different frame grabbers. However the list "
    "may expand virtually to everything what provides a DirectShow driver exposing certain interface (IP video surveillance "
    "cameras, high-end industrial cameras, virtual cameras, etc).<br><br>"

    "The list of DirectShow compatible devices is discovered at runtime by the plug-in and the corresponding Device property "
    "is populated allowing to select the required one. <b>Resolution</b> and <b>Video input</b> properties depend on the selected video "
    "device and so their possible values are populated on video device selection.<br><br>"

    "<b>Note:</b> Setting the <b>Resolution</b> property makes sure the video source provides video frames of the requested size. The frame rate "
    "specified there is a target average frame rate defined by camera's drivers, which may not be always "
    "met due to certain camera/system constraints (different camera's settings, like auto exposure, for example, USB bus throughput, etc). "
    "Some cameras allow overriding default average frame rate, which can be done using <b>Frame Rate</b> property, if it is enabled. "
    "If so, its value can be set to one of the values in the available range. However, camera may not support every frame rate value "
    "from the advertised range. For example, some cameras may show [15-30] frame rate range for a certain resolution, but support "
    "only two values from it - 15 and 30. While other cameras may support other values from the range as well.<br><br>"

    "Some cameras exposed through DirectShow interface provide configuration of properties like brightness, contrast, saturation, etc. "
    "Those are exposed as run time properties by this plug-in. If certain property is supported by particular camera, it can be set "
    "while the video source is running."
    ,
    &image_local_device_16x16,
    nullptr,
    DirectShowVideoSourcePlugin,

    sizeof( pluginProperties ) / sizeof( PropertyDescriptor* ),
    pluginProperties,
    PluginInitializer,
    PluginCleaner,
    UpdateProperties
);

// Complete properties description by initializing those parts, which were not 
// initialized during properties array declaration
static void PluginInitializer( )
{
    // no choices for now, but it will be updated when needed
    deviceProperty.ChoicesCount = 0;
    deviceProperty.Choices      = nullptr;
    XVariantInit( &deviceProperty.DefaultValue );

    // configure resolution property
    resolutionProperty.ChoicesCount   = 0;
    resolutionProperty.Choices        = nullptr;
    resolutionProperty.ParentProperty = 0;
    resolutionProperty.Updater        = UpdateResolutionProperty;
    XVariantInit( &resolutionProperty.DefaultValue );

    // configure frame rate property
    frameRateProperty.ParentProperty = 1;
    frameRateProperty.Updater        = UpdateFrameRateProperty;
    XVariantInit( &frameRateProperty.DefaultValue );

    // configure video input property
    videoInputProperty.ChoicesCount   = 0;
    videoInputProperty.Choices        = nullptr;
    videoInputProperty.ParentProperty = 0;
    videoInputProperty.Updater        = UpdateVideoInputProperty;
    XVariantInit( &videoInputProperty.DefaultValue );

    // configure exposure property
    exposureProperty.ParentProperty = 12;
    exposureProperty.Updater        = UpdateExposureProperty;
}

// Clean-up plugin
static void PluginCleaner( )
{
    videoCapabilitiesCache.clear( );
    videoInputsCache.clear( );
    lastAccessedCapabilities.clear( );
}

// Create string from device name - used to build selection list for device property
string DeviceNameToString( const XDeviceName& device )
{
    string str = device.Name( ) + "\n" + device.Moniker( );
    return str;
}
// Create device name from string - here we mostly interested in moniker string to set to video source
XDeviceName DeviceNameFromString( const string& str )
{
    XDeviceName deviceName = XDeviceName( string( ), string( ) );
    int         newLinePos = str.find( '\n' );

    if ( newLinePos == static_cast<int>( string::npos ) )
    {
        if ( str.find( "@device" ) == 0 )
        {
            deviceName = XDeviceName( str, string( ) );
        }
    }
    else
    {
        deviceName = XDeviceName( str.substr( newLinePos + 1 ), str.substr( 0, newLinePos ) );
    }

    return deviceName;
}
// Create string from resolution
string ResolutionToString( const XDeviceCapabilities& cap )
{
    string resolutionString;

    if ( cap.Width( ) == 0 )
    {
        resolutionString = "Default";
    }
    else
    {
        char temp[64];

        sprintf( temp, "%d x %d, %d bpp, %d fps\n%d %d", cap.Width( ), cap.Height( ), cap.BitCount( ), cap.AverageFrameRate( ),
                                                         cap.MinimumFrameRate( ), cap.MaximumFrameRate( ) );

        resolutionString = string( temp );
    }

    return resolutionString;
}
// Create resolution from string
XDeviceCapabilities ResolutionFromString( const string& str, bool* foundMinMaxFps = nullptr )
{
    XDeviceCapabilities cap;
    int                 newLinePos = str.find( '\n' );
    int                 width      = 0, height = 0, bpp = 0, fps = 0, minFps = 0, maxFps = 0;

    if ( sscanf( str.c_str( ), "%d x %d, %d bpp, %d", &width, &height, &bpp, &fps ) == 4 )
    {
        if ( ( newLinePos != static_cast<int>( string::npos ) ) &&
             ( sscanf( str.substr( newLinePos + 1 ).c_str( ), "%d %d", &minFps, &maxFps ) == 2 ) )
        {
            if ( foundMinMaxFps ) *foundMinMaxFps = true;
        }
        else
        {
            if ( foundMinMaxFps ) *foundMinMaxFps = false;
            minFps = maxFps = fps;
        }

        cap = XDeviceCapabilities( width, height, bpp, fps, maxFps, minFps );
    }

    return cap;
}

// Custom comparer to sort resolutions
static bool DeviceResolutionsSorter( const XDeviceCapabilities& cap1, const XDeviceCapabilities& cap2 )
{
    bool ret = ( cap1.BitCount( ) > cap2.BitCount( ) );

    if ( cap1.BitCount( ) == cap2.BitCount( ) )
    {
        ret = ( cap1.Width( ) < cap2.Width( ) );

        if ( cap1.Width( ) == cap2.Width( ) )
        {
            ret = ( cap1.Height( ) < cap2.Height( ) );
        }
    }

    return ret;
}

// Get supported capabilities for the specified device
vector<XDeviceCapabilities> GetDeviceCapabillities( const string& deviceMoniker )
{
    vector<XDeviceCapabilities>     capabilities;
    auto                            cacheIt = videoCapabilitiesCache.find( deviceMoniker );

    if ( cacheIt != videoCapabilitiesCache.end( ) )
    {
        capabilities = cacheIt->second;
    }
    else
    {
        // get resolutions for the specified device
        shared_ptr<XLocalVideoDevice> defaultDevice = XLocalVideoDevice::Create( deviceMoniker );

        capabilities = defaultDevice->GetCapabilities( );

        // sort available video capabilities
        sort( capabilities.begin( ), capabilities.end( ), DeviceResolutionsSorter );

        // add empty capability, which will result in default value
        capabilities.insert( capabilities.begin( ), XDeviceCapabilities( ) );

        // cache it
        videoCapabilitiesCache.insert( pair<string, vector<XDeviceCapabilities>>( deviceMoniker, capabilities ) );
    }

    return capabilities;
}

// Update properties dynamically - get available video devices
XErrorCode UpdateProperties( PluginDescriptor* desc )
{
    if ( ( desc != 0 ) && ( desc->PropertiesCount >= 1 ) && ( desc->Properties != 0 ) )
    {
        PropertyDescriptor* devProp = desc->Properties[0];

        // free old choices
        FreePropertyChoices( devProp );
        XVariantClear( &devProp->DefaultValue );

        // get video device choices
        vector<XDeviceName> devices      = XLocalVideoDevice::GetAvailableDevices( );
        int                 devicesCount = devices.size( );

        devProp->ChoicesCount = static_cast<uint16_t>( devicesCount );
        devProp->Choices      = static_cast<xvariant*>( XCAlloc( devProp->ChoicesCount, sizeof( xvariant ) ) );

        for ( int i = 0; i < devicesCount; i++ )
        {
            devProp->Choices[i].type         = XVT_String;
            devProp->Choices[i].value.strVal = XStringAlloc( DeviceNameToString( devices[i] ).c_str( ) );
        }

        if ( devicesCount != 0 )
        {
            XVariantCopy( &devProp->Choices[0], &devProp->DefaultValue );
        }
    }

    return SuccessCode;
}

// Update dependent resolution property
XErrorCode UpdateResolutionProperty( PropertyDescriptor* desc, const xvariant* parentValue )
{
    XErrorCode ret = ErrorFailed;

    if ( ( desc == nullptr ) || ( parentValue == nullptr ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( parentValue->type != XVT_String )
    {
        ret = ErrorIncompatibleTypes;
    }
    else
    {
        XDeviceName device = DeviceNameFromString( string( parentValue->value.strVal ) );

        if ( device.Moniker( ).empty( ) )
        {
            ret = ErrorInvalidFormat;
        }
        else
        {
            FreePropertyChoices( desc );
            XVariantClear( &desc->DefaultValue );

            vector<XDeviceCapabilities> capabilities = GetDeviceCapabillities( device.Moniker( ) );

            desc->ChoicesCount = static_cast<uint16_t>( capabilities.size( ) );
            desc->Choices      = static_cast<xvariant*>( XCAlloc( desc->ChoicesCount, sizeof( xvariant ) ) );

            for ( int16_t i = 0; i < desc->ChoicesCount; i++ )
            {
                desc->Choices[i].type         = XVT_String;
                desc->Choices[i].value.strVal = XStringAlloc( ResolutionToString( capabilities[i] ).c_str( ) );
            }

            if ( desc->ChoicesCount > 0 )
            {
                desc->DefaultValue.type = XVT_String;
                XVariantCopy( &desc->Choices[0], &desc->DefaultValue );
            }

            lastAccessedCapabilities = capabilities;

            ret = SuccessCode;
        }
    }

    return ret;
}

// Update dependent frame rate property
XErrorCode UpdateFrameRateProperty( PropertyDescriptor* desc, const xvariant* parentValue )
{
    XErrorCode ret = ErrorFailed;

    if ( ( desc == nullptr ) || ( parentValue == nullptr ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( parentValue->type != XVT_String )
    {
        ret = ErrorIncompatibleTypes;
    }
    else
    {
        bool                foundMinMax   = false;
        XDeviceCapabilities resolutionSet = ResolutionFromString( string( parentValue->value.strVal ), &foundMinMax );

        // in case the resolution string is missing min/max frame rate info, try finding it
        if ( !foundMinMax )
        {
            // it is not safe to assume the last accessed capabilities list is for the same device, but should be OK for now
            vector<XDeviceCapabilities>::const_iterator capsIt = std::find( lastAccessedCapabilities.begin( ), lastAccessedCapabilities.end( ), resolutionSet );

            if ( capsIt != lastAccessedCapabilities.end( ) )
            {
                resolutionSet = *capsIt;
            }
        }

        desc->DefaultValue.type        = XVT_U2;
        desc->DefaultValue.value.usVal = static_cast<uint16_t>( resolutionSet.AverageFrameRate( ) );

        desc->MinValue.type            = XVT_U2;
        desc->MinValue.value.usVal     = static_cast<uint16_t>( resolutionSet.MinimumFrameRate( ) );

        desc->MaxValue.type            = XVT_U2;
        desc->MaxValue.value.usVal     = static_cast<uint16_t>( resolutionSet.MaximumFrameRate( ) );

        if ( resolutionSet.MinimumFrameRate( ) == resolutionSet.MaximumFrameRate( ) )
        {
            // nothing to chose from
            desc->Flags |= PropertyFlag_Disabled;
        }
        else
        {
            desc->Flags &= ( ~PropertyFlag_Disabled );
        }

        ret = SuccessCode;
    }

    return ret;
}

// Update dependent video input property
XErrorCode UpdateVideoInputProperty( PropertyDescriptor* desc, const xvariant* parentValue )
{
    XErrorCode ret = ErrorFailed;

    if ( ( desc == nullptr ) || ( parentValue == nullptr ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( parentValue->type != XVT_String )
    {
        ret = ErrorIncompatibleTypes;
    }
    else
    {
        XDeviceName device = DeviceNameFromString( string( parentValue->value.strVal ) );

        if ( device.Moniker( ).empty( ) )
        {
            ret = ErrorInvalidFormat;
        }
        else
        {
            FreePropertyChoices( desc );
            XVariantClear( &desc->DefaultValue );

            vector<XDevicePinInfo>  inputs;
            auto                    cacheIt = videoInputsCache.find( device.Moniker( ) );

            if ( cacheIt != videoInputsCache.end( ) )
            {
                inputs = cacheIt->second;
            }
            else
            {
                // get resolutions for the specified device
                shared_ptr<XLocalVideoDevice> defaultDevice = XLocalVideoDevice::Create( device.Moniker( ) );

                inputs = defaultDevice->GetInputVideoPins( );

                // cache it
                videoInputsCache.insert( pair<string, vector<XDevicePinInfo>>( device.Moniker( ), inputs ) );
            }

            desc->ChoicesCount = static_cast<uint16_t>( inputs.size( ) );
            if ( desc->ChoicesCount == 0 )
            {
                desc->ChoicesCount = 1;
            }
            desc->Choices = static_cast<xvariant*>( XCAlloc( desc->ChoicesCount, sizeof( xvariant ) ) );

            if ( inputs.size( ) > 0 )
            {
                char temp[64];

                for ( int16_t i = 0; i < desc->ChoicesCount; i++ )
                {
                    sprintf( temp, "Input %d - %s", i + 1, inputs[i].TypeToString( ).c_str( ) );
                    desc->Choices[i].type         = XVT_String;
                    desc->Choices[i].value.strVal = XStringAlloc( temp );
                }

                desc->Flags &= ( ~PropertyFlag_Disabled );
            }
            else
            {
                desc->Choices[0].type         = XVT_String;
                desc->Choices[0].value.strVal = XStringAlloc( "Unsupported" );

                desc->Flags |= PropertyFlag_Disabled;
            }

            desc->DefaultValue.type        = XVT_U1;
            desc->DefaultValue.value.ubVal = 0;

            desc->MaxValue.type        = XVT_U1;
            desc->MaxValue.value.ubVal = static_cast<uint8_t>( desc->ChoicesCount - 1 );

            ret = SuccessCode;
        }
    }

    return ret;
}

// Enable/disable Exposure property based on Automatic Exposure setting
XErrorCode UpdateExposureProperty( PropertyDescriptor* desc, const xvariant* parentValue )
{
    XErrorCode ret = ErrorFailed;
    bool       boolParentValue;

    ret = XVariantToBool( parentValue, &boolParentValue );

    if ( ret == SuccessCode )
    {
        if ( boolParentValue )
        {
            desc->Flags |= PropertyFlag_Disabled;
        }
        else
        {
            desc->Flags &= ( ~PropertyFlag_Disabled );
        }
    }

    return ret;
}
