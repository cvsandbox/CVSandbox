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
#include <image_rotate_hue_16x16.h>
#include "RotateHuePlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000006, 0x00000017 };

// Hue Angle property
static PropertyDescriptor hueAngleProperty =
{ XVT_U2, "Hue Angle", "hueAngle", "Angle to shift hue value of all pixels by.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &hueAngleProperty,
};

// Let the class itself know description of its properties
const PropertyDescriptor** RotateHuePlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_ColorEffect,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Rotate Hue",
    "RotateHue",
    "Rotates hue value of all pixels to the specified angle.",

    /* Long description */
    "The plug-in changes hue value of all pixel by shifting it by the specified hue angle (adding <b>hueAngle</b> to current "
    "hue value of all pixels). This creates hue rotation effect. The plug-in converts pixels' values from RGB to HSL color space, "
    "then shifts their Hue by the specified value and finally converts it back to RGB color space."
    ,
    &image_rotate_hue_16x16,
    0,
    RotateHuePlugin,

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
    hueAngleProperty.DefaultValue.type        = XVT_U2;
    hueAngleProperty.DefaultValue.value.usVal = 0;

    hueAngleProperty.MinValue.type            = XVT_U2;
    hueAngleProperty.MinValue.value.usVal     = 0;

    hueAngleProperty.MaxValue.type            = XVT_U2;
    hueAngleProperty.MaxValue.value.usVal     = 359;
}
