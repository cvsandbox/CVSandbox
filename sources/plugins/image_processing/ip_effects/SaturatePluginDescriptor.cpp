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
#include <image_saturate_16x16.h>
#include "SaturatePlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 1 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000006, 0x0000000D };

// Hue property
static PropertyDescriptor saturationValueProperty =
{ XVT_I1, "Saturate By", "saturateBy", "Saturation change value.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &saturationValueProperty,
};

// Let the class itself know description of its properties
const PropertyDescriptor** SaturatePlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_ColorEffect,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Saturate",
    "Saturate",
    "Adjusts saturation level of the given color image.",

    /* Long description */
    "The plug-in increases or decreases saturation level of the given image by the specified amount. Every pixels' RGB "
    "values are converted to HSV color space, then saturation is adjusted and finally converted back to RGB. Positive "
    "saturation value results in saturation increase; otherwise decrease."
    ,
    &image_saturate_16x16,
    0,
    SaturatePlugin,

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
    saturationValueProperty.DefaultValue.type = XVT_I1;
    saturationValueProperty.DefaultValue.value.bVal = 10;

    saturationValueProperty.MinValue.type = XVT_I1;
    saturationValueProperty.MinValue.value.bVal = -100;

    saturationValueProperty.MaxValue.type = XVT_I1;
    saturationValueProperty.MaxValue.value.bVal = 100;
}
