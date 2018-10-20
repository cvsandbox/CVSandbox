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
#include "GenerateRoundedBorderTexturePlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000006, 0x00000015 };

// Width property
static PropertyDescriptor widthProperty =
{ XVT_U2, "Width", "width", "Specifies width of the texture image.", PropertyFlag_None };
// Height property
static PropertyDescriptor heightProperty =
{ XVT_U2, "Height", "height", "Specifies height of the texture image.", PropertyFlag_None };
// Border Width property
static PropertyDescriptor borderWidthProperty =
{ XVT_U2, "Border Width", "borderWidth", "Specifies width of the border to create.", PropertyFlag_None };
// X Roundness property
static PropertyDescriptor xRoundnessProperty =
{ XVT_U2, "X Roundness", "xRoundness", "Specifies borders X roundness.", PropertyFlag_None };
// Y Roundness property
static PropertyDescriptor yRoundnessProperty =
{ XVT_U2, "Y Roundness", "yRoundness", "Specifies borders Y roundness.", PropertyFlag_None };
// Roundness X Shift property
static PropertyDescriptor xRoundnessShiftProperty =
{ XVT_U2, "Roundness X Shift", "xRoundnessShift", "Specifies shift of the rounded corders in X direction.", PropertyFlag_None };
// Roundness Y Shift property
static PropertyDescriptor yRoundnessShiftProperty =
{ XVT_U2, "Roundness Y Shift", "yRoundnessShift", "Specifies shift of the rounded corders in Y direction.", PropertyFlag_None };
// Add Bloor property
static PropertyDescriptor addBloorProperty =
{ XVT_Bool, "Add Bloor", "addBloor", "Specifies border should be bloored or not.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &widthProperty,
    &heightProperty,
    &borderWidthProperty,
    &xRoundnessProperty,
    &yRoundnessProperty,
    &xRoundnessShiftProperty,
    &yRoundnessShiftProperty,
    &addBloorProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** GenerateRoundedBorderTexturePlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_ImageGenerator,

    PluginType_ImageGenerator,
    PluginVersion,
    "Generate Rounded Border Texture",
    "GenerateRoundedBorderTexture",
    "Generates rounded border texture.",
    "Some help about this routine.",
    0, // &image_textile_16x16,
    0,
    GenerateRoundedBorderTexturePlugin,

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

    // X roundness
    xRoundnessProperty.DefaultValue.type = XVT_U2;
    xRoundnessProperty.DefaultValue.value.usVal = 5;

    xRoundnessProperty.MinValue.type = XVT_U2;
    xRoundnessProperty.MinValue.value.usVal = 0;

    xRoundnessProperty.MaxValue.type = XVT_U2;
    xRoundnessProperty.MaxValue.value.usVal = 5000;

    // Y roundness
    yRoundnessProperty.DefaultValue.type = XVT_U2;
    yRoundnessProperty.DefaultValue.value.usVal = 5;

    yRoundnessProperty.MinValue.type = XVT_U2;
    yRoundnessProperty.MinValue.value.usVal = 0;

    yRoundnessProperty.MaxValue.type = XVT_U2;
    yRoundnessProperty.MaxValue.value.usVal = 5000;

    // X roundess shift
    xRoundnessShiftProperty.DefaultValue.type = XVT_U2;
    xRoundnessShiftProperty.DefaultValue.value.usVal = 0;

    xRoundnessShiftProperty.MinValue.type = XVT_U2;
    xRoundnessShiftProperty.MinValue.value.usVal = 0;

    xRoundnessShiftProperty.MaxValue.type = XVT_U2;
    xRoundnessShiftProperty.MaxValue.value.usVal = 5000;

    // Y roundess shift
    yRoundnessShiftProperty.DefaultValue.type = XVT_U2;
    yRoundnessShiftProperty.DefaultValue.value.usVal = 0;

    yRoundnessShiftProperty.MinValue.type = XVT_U2;
    yRoundnessShiftProperty.MinValue.value.usVal = 0;

    yRoundnessShiftProperty.MaxValue.type = XVT_U2;
    yRoundnessShiftProperty.MaxValue.value.usVal = 5000;

    // Add Bloor property
    addBloorProperty.DefaultValue.type = XVT_Bool;
    addBloorProperty.DefaultValue.value.boolVal = true;
}
