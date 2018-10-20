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
#include <image_rounded_border_16x16.h>
#include "RoundedBorderPlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000006, 0x00000016 };

// Border Color property
static PropertyDescriptor borderColorProperty =
{ XVT_ARGB, "Border color", "borderColor", "Color of the border to create.", PropertyFlag_None };
// Border Width property
static PropertyDescriptor borderWidthProperty =
{ XVT_U2, "Border width", "borderWidth", "Specifies width of the border to create.", PropertyFlag_None };
// X Roundness property
static PropertyDescriptor xRoundnessProperty =
{ XVT_U2, "X roundness", "xRoundness", "Specifies corners' X roundness.", PropertyFlag_None };
// Y Roundness property
static PropertyDescriptor yRoundnessProperty =
{ XVT_U2, "Y roundness", "yRoundness", "Specifies corners' Y roundness.", PropertyFlag_None };
// Roundness X Shift property
static PropertyDescriptor xRoundnessShiftProperty =
{ XVT_U2, "Roundness X shift", "xRoundnessShift", "Specifies shift of the rounded corners' in X direction.", PropertyFlag_None };
// Roundness Y Shift property
static PropertyDescriptor yRoundnessShiftProperty =
{ XVT_U2, "Roundness Y shift", "yRoundnessShift", "Specifies shift of the rounded corners' in Y direction.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &borderColorProperty, &borderWidthProperty, &xRoundnessProperty, &yRoundnessProperty,
    &xRoundnessShiftProperty, &yRoundnessShiftProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** RoundedBorderPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_FrameGenerator,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Rounded Border",
    "RoundedBorder",
    "Adds rounded border to the selected image.",

    /* Long description */
    "The plug-in adds a rounded border of the specified color and width to a given image. "
    "<b>Note:</b> image size is preserved, so the added border takes some edge parts of the image."
    ,
    &image_rounded_border_16x16,
    0,
    RoundedBorderPlugin,

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
}
