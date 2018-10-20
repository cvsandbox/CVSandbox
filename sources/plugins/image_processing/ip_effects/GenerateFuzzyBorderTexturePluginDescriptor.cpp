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
//#include <image_textile_16x16.h>
#include "GenerateFuzzyBorderTexturePlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000006, 0x00000011 };

// Width property
static PropertyDescriptor widthProperty =
{ XVT_U2, "Width", "width", "Specifies width of the texture image.", PropertyFlag_None };
// Height property
static PropertyDescriptor heightProperty =
{ XVT_U2, "Height", "height", "Specifies height of the texture image.", PropertyFlag_None };
// Border Width property
static PropertyDescriptor borderWidthProperty =
{ XVT_U2, "Border Width", "borderWidth", "Specifies width of the border to create.", PropertyFlag_None };
// Gradient Width property
static PropertyDescriptor gradientWidthProperty =
{ XVT_U2, "Gradient Width", "gradientWidth", "Specifies width of the gradient in the border.", PropertyFlag_None };
// Waviness property
static PropertyDescriptor wavinessProperty =
{ XVT_U2, "Waviness", "waviness", "Specifies waviness of the border.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &widthProperty,
    &heightProperty,
    &borderWidthProperty,
    &gradientWidthProperty,
    &wavinessProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** GenerateFuzzyBorderTexturePlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_ImageGenerator,

    PluginType_ImageGenerator,
    PluginVersion,
    "Generate Fuzzy Border Texture",
    "GenerateFuzzyBorderTexture",
    "Generates fuzzy border texture.",
    "Some help about this routine.",
    0, // &image_textile_16x16,
    0,
    GenerateFuzzyBorderTexturePlugin,

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
    // width
    widthProperty.DefaultValue.type = XVT_U2;
    widthProperty.DefaultValue.value.usVal = 640;

    widthProperty.MinValue.type = XVT_U2;
    widthProperty.MinValue.value.usVal = 8;

    widthProperty.MaxValue.type = XVT_U2;
    widthProperty.MaxValue.value.usVal = 10000;

    // height
    heightProperty.DefaultValue.type = XVT_U2;
    heightProperty.DefaultValue.value.usVal = 480;

    heightProperty.MinValue.type = XVT_U2;
    heightProperty.MinValue.value.usVal = 8;

    heightProperty.MaxValue.type = XVT_U2;
    heightProperty.MaxValue.value.usVal = 10000;

    // border width
    borderWidthProperty.DefaultValue.type = XVT_U2;
    borderWidthProperty.DefaultValue.value.usVal = 16;

    borderWidthProperty.MinValue.type = XVT_U2;
    borderWidthProperty.MinValue.value.usVal = 2;

    borderWidthProperty.MaxValue.type = XVT_U2;
    borderWidthProperty.MaxValue.value.usVal = 100;

    // gradient width
    gradientWidthProperty.DefaultValue.type = XVT_U2;
    gradientWidthProperty.DefaultValue.value.usVal = 12;

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
