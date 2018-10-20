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
#include <image_colorize_16x16.h>
#include "ColorizePlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 1 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000006, 0x0000000E };

// Hue property
static PropertyDescriptor hueProperty =
{ XVT_U2, "Hue", "hue", "Hue value to set for all pixels of an image.", PropertyFlag_PreferredEditor_HuePicker };

// Saturation property
static PropertyDescriptor saturationProperty =
{ XVT_U1, "Saturation", "saturation", "Saturation value to set for all pixels of an image.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &hueProperty, &saturationProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** ColorizePlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_ColorEffect,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Colorize",
    "Colorize",
    "Sets hue and saturation of all pixels to the specified values.",

    /* Long description */
    "The plug-in changes hue and saturation values of all pixel in the specified image. It converts pixel value from RGB "
    "to HSV color space, then sets the desired values of <b>Hue</b> and <b>Saturation</b>, and finally converts it back "
    "to RGB color space."
    ,
    &image_colorize_16x16,
    0,
    ColorizePlugin,

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
    // hue property
    hueProperty.DefaultValue.type = XVT_U2;
    hueProperty.DefaultValue.value.sVal = 0;

    hueProperty.MinValue.type = XVT_U2;
    hueProperty.MinValue.value.sVal = 0;

    hueProperty.MaxValue.type = XVT_U2;
    hueProperty.MaxValue.value.sVal = 359;

    // saturation property
    saturationProperty.DefaultValue.type = XVT_U1;
    saturationProperty.DefaultValue.value.ubVal = 100;

    saturationProperty.MinValue.type = XVT_U1;
    saturationProperty.MinValue.value.ubVal = 0;

    saturationProperty.MaxValue.type = XVT_U1;
    saturationProperty.MaxValue.value.ubVal = 100;
}
