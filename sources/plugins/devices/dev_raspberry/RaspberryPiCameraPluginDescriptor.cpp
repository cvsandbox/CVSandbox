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

#include <iplugincpp.hpp>
#include <image_raspberry_camera_16x16.h>
#include "RaspberryPiCameraPlugin.hpp"

static void PluginInitializer( );
static void PluginCleaner( );
static XErrorCode UpdateFrameIntervalProperty( PropertyDescriptor* desc, const xvariant* parentValue );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000014, 0x00000001 };

// Address property
static PropertyDescriptor jpegUrlProperty =
{ XVT_String, "Address", "address", "IP address or DNS name of Raspberry Pi.", PropertyFlag_None };
// Port property
static PropertyDescriptor portProperty =
{ XVT_U2, "Port", "port", "Port number to connect to.", PropertyFlag_None };
// User name property
static PropertyDescriptor userNameProperty =
{ XVT_String, "User name", "userName", "User name used for authentication.", PropertyFlag_None };
// Password property
static PropertyDescriptor passwordProperty =
{ XVT_String, "Password", "password", "Password used for authentication.", PropertyFlag_None };
// Protocol property
static PropertyDescriptor protocolProperty =
{ XVT_U1, "Protocol", "protocol", "Specifies protocol to use for accessing camera.", PropertyFlag_SelectionByIndex };
// Frame Interval property
static PropertyDescriptor frameIntervalProperty =
{ XVT_U2, "Frame interval", "frameInterval", "Time interval (in milliseconds) between querying video frames.", PropertyFlag_Dependent | PropertyFlag_Disabled };

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
// Sharpness property
static PropertyDescriptor sharpnessProperty =
{ XVT_I4, "Sharpness", "sharpness", "Sharpness of camera images.", PropertyFlag_DeviceRuntimeConfiguration };

// Horizontal flip property
static PropertyDescriptor horizontalFlipProperty =
{ XVT_Bool, "Horizontal flip", "hflip", "Enable horizontal flipping of camera images or not.", PropertyFlag_DeviceRuntimeConfiguration };
// Vertical flip property
static PropertyDescriptor verticalFlipProperty =
{ XVT_Bool, "Vertical flip", "vflip", "Enable vertical flipping of camera images or not.", PropertyFlag_DeviceRuntimeConfiguration };
// Video stabilization property
static PropertyDescriptor videoStabilisationProperty =
{ XVT_Bool, "Video stabilization", "videoStabilisation", "Enable video stabilisation or not.", PropertyFlag_DeviceRuntimeConfiguration };

// White balance property
static PropertyDescriptor whiteBalanceProperty =
{ XVT_U1, "White balance", "whiteBalance", "Automatic white balance mode.", PropertyFlag_SelectionByIndex | PropertyFlag_DeviceRuntimeConfiguration };
// Exposure mode property
static PropertyDescriptor exposureModeProperty =
{ XVT_U1, "Exposure mode", "exposureMode", "Exposure mode used by the camera.", PropertyFlag_SelectionByIndex | PropertyFlag_DeviceRuntimeConfiguration };
// Exposure metering property
static PropertyDescriptor exposureMeteringProperty =
{ XVT_U1, "Exposure metering", "exposureMetering", "Exposure metering mode used by the camera.", PropertyFlag_SelectionByIndex | PropertyFlag_DeviceRuntimeConfiguration };
// Image effect property
static PropertyDescriptor imageEffectProperty =
{ XVT_U1, "Image effect", "imageEffect", "Select an effect to apply to camera images.", PropertyFlag_SelectionByIndex | PropertyFlag_DeviceRuntimeConfiguration };

// <<<<<

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &jpegUrlProperty, &portProperty, &userNameProperty, &passwordProperty,
    &protocolProperty, &frameIntervalProperty,

    &brightnessProperty, &contrastProperty, &saturationProperty, &sharpnessProperty,
    &horizontalFlipProperty, &verticalFlipProperty, &videoStabilisationProperty,

    &whiteBalanceProperty, &exposureModeProperty, &exposureMeteringProperty, &imageEffectProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** RaspberryPiCameraPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_VideoSource,

    PluginType_VideoSource,
    PluginVersion,
    "Raspberry Pi Camera",
    "RaspberryPiCamera",
    "Plug-in to capture images from remote Raspberry Pi camera running cam2web application.",

    /* Long description */
    "The plug-in allows capturing images from remote Raspberry Pi camera, if the device is running "
    "<a href='https://github.com/cvsandbox/cam2web'>cam2web</a> application, which allows streaming "
    "camera image to Web as JPEG/MJPEG streams.<br><br>"

    "Apart from standard connection properties, which allow to specify Pi's address, port, user credentials, "
    "etc., there are number of run time properties, which control image acquisition parameters."
    ,
    &image_raspberry_camera_16x16,
    nullptr,
    RaspberryPiCameraPlugin,

    sizeof( pluginProperties ) / sizeof( PropertyDescriptor* ),
    pluginProperties,
    PluginInitializer,
    PluginCleaner,
    nullptr
);

// Complete properties description by initializing those parts, which were not
// initialized during properties array declaration
static void PluginInitializer( )
{
    // Port
    portProperty.DefaultValue.type = XVT_U2;
    portProperty.DefaultValue.value.usVal = 8000;

    portProperty.MinValue.type = XVT_U2;
    portProperty.MinValue.value.usVal = 1;

    portProperty.MaxValue.type = XVT_U2;
    portProperty.MaxValue.value.usVal = 65535;

    // Frame Interval
    frameIntervalProperty.DefaultValue.type = XVT_U2;
    frameIntervalProperty.DefaultValue.value.usVal = 100;

    frameIntervalProperty.MinValue.type = XVT_U2;
    frameIntervalProperty.MinValue.value.usVal = 0;

    frameIntervalProperty.MaxValue.type = XVT_U2;
    frameIntervalProperty.MaxValue.value.usVal = 60000;

    frameIntervalProperty.ParentProperty = 4;
    frameIntervalProperty.Updater = UpdateFrameIntervalProperty;

    // Protocol
    const static char* supportedProtocols[] = { "MJPEG", "JPEG" };

    InitSelectionProperty( &protocolProperty, supportedProtocols, XARRAY_SIZE( supportedProtocols ), 0 );

    // Brightness
    brightnessProperty.DefaultValue.type = XVT_I4;
    brightnessProperty.DefaultValue.value.iVal = 50;

    brightnessProperty.MinValue.type = XVT_I4;
    brightnessProperty.MinValue.value.iVal = 0;

    brightnessProperty.MaxValue.type = XVT_I4;
    brightnessProperty.MaxValue.value.iVal = 100;

    // Contrast
    contrastProperty.DefaultValue.type = XVT_I4;
    contrastProperty.DefaultValue.value.iVal = 0;

    contrastProperty.MinValue.type = XVT_I4;
    contrastProperty.MinValue.value.iVal = -100;

    contrastProperty.MaxValue.type = XVT_I4;
    contrastProperty.MaxValue.value.iVal = 100;

    // Saturation
    saturationProperty.DefaultValue.type = XVT_I4;
    saturationProperty.DefaultValue.value.iVal = 0;

    saturationProperty.MinValue.type = XVT_I4;
    saturationProperty.MinValue.value.iVal = -100;

    saturationProperty.MaxValue.type = XVT_I4;
    saturationProperty.MaxValue.value.iVal = 100;

    // Sharpness
    sharpnessProperty.DefaultValue.type = XVT_I4;
    sharpnessProperty.DefaultValue.value.iVal = 0;

    sharpnessProperty.MinValue.type = XVT_I4;
    sharpnessProperty.MinValue.value.iVal = -100;

    sharpnessProperty.MaxValue.type = XVT_I4;
    sharpnessProperty.MaxValue.value.iVal = 100;

    // Horizontal flip
    horizontalFlipProperty.DefaultValue.type = XVT_Bool;
    horizontalFlipProperty.DefaultValue.value.boolVal = false;

    // Vertical flip
    verticalFlipProperty.DefaultValue.type = XVT_Bool;
    verticalFlipProperty.DefaultValue.value.boolVal = false;

    // Video stabilization
    videoStabilisationProperty.DefaultValue.type = XVT_Bool;
    videoStabilisationProperty.DefaultValue.value.boolVal = false;

    // White balance
    const static char* supportedAwbModes[] =
    {
        "Off", "Auto", "Sunlight", "Cloudy", "Shade", "Tungsten",
        "Fluorescent", "Incandescent", "Flash", "Horizon"
    };

    InitSelectionProperty( &whiteBalanceProperty, supportedAwbModes, XARRAY_SIZE( supportedAwbModes ), 1 );

    // Exposure mode
    const static char* supportedExposureModes[] =
    {
        "Off", "Auto", "Night", "Night Preview", "Backlight", "Spotlight",
        "Sports", "Snow", "Beach", "Very Long", "Fixed Fps", "Anti Shake", "Fireworks"
    };

    InitSelectionProperty( &exposureModeProperty, supportedExposureModes, XARRAY_SIZE( supportedExposureModes ), 1 );

    // Exposure metering mode
    const static char* supportedExposureMeteringModes[] =
    {
        "Average", "Spot", "Backlit", "Matrix"
    };

    InitSelectionProperty( &exposureMeteringProperty, supportedExposureMeteringModes, XARRAY_SIZE( supportedExposureMeteringModes ), 0 );

    // Exposure metering mode
    const static char* supportedImageEffects[] =
    {
        "None", "Negative", "Solarize", "Sketch", "Denoise", "Emboss", "Oil Paint",
        "Hatch", "Gpen", "Pastel", "Water Color", "Film", "Blur", "Saturation",
        "Color Swap", "Washed Out", "Posterise", "Color Point", "Color Balance", "Cartoon"
    };

    InitSelectionProperty( &imageEffectProperty, supportedImageEffects, XARRAY_SIZE( supportedImageEffects ), 0 );
}

// Clean-up plugin - deallocate strings
static void PluginCleaner( )
{
    CleanSelectionProperty( &protocolProperty  );

    CleanSelectionProperty( &whiteBalanceProperty );
    CleanSelectionProperty( &exposureModeProperty );
    CleanSelectionProperty( &exposureMeteringProperty );
    CleanSelectionProperty( &imageEffectProperty );
}

// Enable/disable frame interval property depending on selected protol
static XErrorCode UpdateFrameIntervalProperty( PropertyDescriptor* desc, const xvariant* parentValue )
{
    XErrorCode ret = ErrorFailed;
    uint8_t    ubParentValue;

    ret = XVariantToUByte( parentValue, &ubParentValue );

    if ( ret == SuccessCode )
    {
        if ( ubParentValue == 1 )
        {
            desc->Flags &= ( ~PropertyFlag_Disabled );
        }
        else
        {
            desc->Flags |= PropertyFlag_Disabled;
        }
    }

    return ret;
}
