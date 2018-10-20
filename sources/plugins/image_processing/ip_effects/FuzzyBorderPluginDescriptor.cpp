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
#include <image_fuzzy_border_16x16.h>
#include "FuzzyBorderPlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000006, 0x00000012 };

// Border Color property
static PropertyDescriptor borderColorProperty =
{ XVT_ARGB, "Border color", "borderColor", "Color of the border to create.", PropertyFlag_None };
// Border Width property
static PropertyDescriptor borderWidthProperty =
{ XVT_U2, "Border width", "borderWidth", "Specifies width of the border to create.", PropertyFlag_None };
// Gradient Width property
static PropertyDescriptor gradientWidthProperty =
{ XVT_U2, "Gradient width", "gradientWidth", "Specifies width of the gradient in the border.", PropertyFlag_None };
// Waviness property
static PropertyDescriptor wavinessProperty =
{ XVT_U2, "Waviness", "waviness", "Specifies waviness of the border.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &borderColorProperty, &borderWidthProperty, &gradientWidthProperty, &wavinessProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** FuzzyBorderPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_FrameGenerator,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Fuzzy Border",
    "FuzzyBorder",
    "Adds fuzzy border to the selected image.",

    /* Long description */
    "The plug-in adds a fuzzy border of the specified color and width to a given image. "
    "The <b>border width</b> property specifies the total width of the border to add, "
    "while the <b>gradient width</b> property specifies the portion of the border, which "
    "smoothly transitions into the processed image. <b>Note:</b> image size is preserved, "
    "so the added border takes some edge parts of the image."
    ,
    &image_fuzzy_border_16x16,
    0,
    FuzzyBorderPlugin,

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
    // border color
    borderColorProperty.DefaultValue.type = XVT_ARGB;
    borderColorProperty.DefaultValue.value.argbVal.argb = 0xFFFFFFFF;

    // border width
    borderWidthProperty.DefaultValue.type = XVT_U2;
    borderWidthProperty.DefaultValue.value.usVal = 16;

    borderWidthProperty.MinValue.type = XVT_U2;
    borderWidthProperty.MinValue.value.usVal = 2;

    borderWidthProperty.MaxValue.type = XVT_U2;
    borderWidthProperty.MaxValue.value.usVal = 100;

    // gradient width
    gradientWidthProperty.DefaultValue.type = XVT_U2;
    gradientWidthProperty.DefaultValue.value.usVal = 8;

    gradientWidthProperty.MinValue.type = XVT_U2;
    gradientWidthProperty.MinValue.value.usVal = 2;

    gradientWidthProperty.MaxValue.type = XVT_U2;
    gradientWidthProperty.MaxValue.value.usVal = 100;

    // waviness
    wavinessProperty.DefaultValue.type = XVT_U2;
    wavinessProperty.DefaultValue.value.usVal = 5;

    wavinessProperty.MinValue.type = XVT_U2;
    wavinessProperty.MinValue.value.usVal = 1;

    wavinessProperty.MaxValue.type = XVT_U2;
    wavinessProperty.MaxValue.value.usVal = 100;
}
