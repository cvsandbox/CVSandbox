/*
    Image processing effects plug-ins of Computer Vision Sandbox

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

#include <iplugincpp.hpp>
#include <image_vignetting_16x16.h>
#include "VignettingPlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 1 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000006, 0x00000010 };

// Start Factor property
static PropertyDescriptor startFactorProperty =
{ XVT_R4, "Start Factor", "startFactor", "Width factor (%) which sets start of vignetting.", PropertyFlag_None };
// End Factor property
static PropertyDescriptor endFactorProperty =
{ XVT_R4, "End Factor", "endFactor", "Width factor (%) which sets end of vignetting.", PropertyFlag_None };
// Decrease Brightness property
static PropertyDescriptor decreaseBrightnessProperty =
{ XVT_Bool, "Decrease Brightness", "decreaseBrightness", "Specifies if brightness is decreased for the vignetting effect.", PropertyFlag_None };
// Decrease Saturation property
static PropertyDescriptor decreaseSaturationProperty =
{ XVT_Bool, "Decrease Saturation", "decreaseSaturation", "Specifies if saturation is decreased for the vignetting effect (color images only).", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &startFactorProperty, &endFactorProperty, &decreaseBrightnessProperty, &decreaseSaturationProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** VignettingPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_ImageEffect,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Vignetting",
    "Vignetting",
    "Creates image vignetting effect.",

    "The image processing plug-in create vignetting effect, which is a reduction of an image's brightness or saturation at the "
    "periphery compared to the image center.<br><br>"

    "The two main properties are the start and end factors of the vignetting effect, which are the percentages of image's radius "
    "(half of image's width) at which the effect starts decreasing brightness and/or saturation and finishes the effect. For example, "
    "lets suppose the <b>start factor</b> is set to 90% and the <b>end factor</b> is set to 150%. If the image's size is 640x480, then "
    "its radius is 320 pixels, which is half of its width. So for all pixels which are away from image's center for no more than 288 pixels "
    "(320 * 0.9) there are no changes at all. However pixels' brightness/saturation starts getting reduced for all other pixels, which are "
    "further away from center. This decrease is proportional to the distance from the effect's starting point. At the effect's end point, "
    "which is 480 (150% of 320) in our case, the effect should completely reduce brightness/saturation to 0. Note: although 150% for the end point "
    "is far beyond image's edges, this allows to control amount of vignetting effect at image's edges. If the end point was set to 100%, for example, "
    "then image's edges would be set to completely black/desaturated.<br><br>"

    "If saturation decrease is enabled, the routine gets slower, since it requires RGB to/from HSV conversion. If only brightness decrease is "
    "enabled, then it operates directly on RGB values of color images or on brightness values of grayscale images."
    ,
    &image_vignetting_16x16,
    0,
    VignettingPlugin,

    sizeof( pluginProperties ) / sizeof( PropertyDescriptor* ),
    pluginProperties,
    PluginInitializer,
    0, // no clean-up
    0  // no dynamic properties update
);

// Complete properties description by initializing those parts, which were not 
// initialized during properties array declaration
static void PluginInitializer( )
{
    // Start Factor property
    startFactorProperty.DefaultValue.type = XVT_R4;
    startFactorProperty.DefaultValue.value.fVal = 90.0f;

    startFactorProperty.MinValue.type = XVT_R4;
    startFactorProperty.MinValue.value.fVal = 10.0f;

    startFactorProperty.MaxValue.type = XVT_R4;
    startFactorProperty.MaxValue.value.fVal = 100.0f;

    // End Factor property
    endFactorProperty.DefaultValue.type = XVT_R4;
    endFactorProperty.DefaultValue.value.fVal = 150.0f;

    endFactorProperty.MinValue.type = XVT_R4;
    endFactorProperty.MinValue.value.fVal = 10.0f;

    endFactorProperty.MaxValue.type = XVT_R4;
    endFactorProperty.MaxValue.value.fVal = 500.0f;

    // Decrease Brightness property
    decreaseBrightnessProperty.DefaultValue.type = XVT_Bool;
    decreaseBrightnessProperty.DefaultValue.value.boolVal = true;

    // Decrease Saturation property
    decreaseSaturationProperty.DefaultValue.type = XVT_Bool;
    decreaseSaturationProperty.DefaultValue.value.boolVal = false;
}
