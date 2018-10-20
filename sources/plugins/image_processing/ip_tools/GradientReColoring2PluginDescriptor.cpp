/*
    Image processing tools plug-ins of Computer Vision Sandbox

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
#include <image_gradient_grayscale_recoloring2_16x16.h>
#include "GradientReColoring2Plugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000007, 0x00000005 };

static PropertyDescriptor startColorProperty =
{ XVT_ARGB, "Start color", "startColor", "Color used to start re-coloring from (min values).", PropertyFlag_None };

static PropertyDescriptor middleColorProperty =
{ XVT_ARGB, "Middle color", "middleColor", "Color used at the middle (at threshold value).", PropertyFlag_None };

static PropertyDescriptor endColorProperty =
{ XVT_ARGB, "End color", "endColor", "Color used to end re-coloring at (max values).", PropertyFlag_None };

static PropertyDescriptor thresholdProperty =
{ XVT_U1, "Threshold", "threshold", "Threshold value where the first gradient changes to the second gradient.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &startColorProperty, &middleColorProperty, &endColorProperty, &thresholdProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** GradientReColoring2Plugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_ColorFilter,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Grayscale Gradient Recoloring 2",
    "GrayscaleGradientRecoloring2",
    "Recolors grayscale image into color image by mapping grayscale values to two gradients between the specified three colors.",

    /* Long description */
    "The plug-in performs recoloring of grayscale images to color images by using two linear gradients between the "
    "three specified colors. Grayscale pixel values in the [0, threshold] range are mapped linearly to the colors "
    "between the <b>start color</b> and the <b>middle color</b>, while the pixel values in the [threshold, 255] "
    "range are mapped to the colors between the <b>middle color</b> and the <b>end color</b>."
    ,
    &image_gradient_grayscale_recoloring2_16x16,
    0,
    GradientReColoring2Plugin,

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
    startColorProperty.DefaultValue.type = XVT_ARGB;
    startColorProperty.DefaultValue.value.argbVal.argb = 0xFF004000;

    middleColorProperty.DefaultValue.type = XVT_ARGB;
    middleColorProperty.DefaultValue.value.argbVal.argb = 0xFF50A080;

    endColorProperty.DefaultValue.type = XVT_ARGB;
    endColorProperty.DefaultValue.value.argbVal.argb = 0xFFC0E0FF;

    thresholdProperty.DefaultValue.type = XVT_U1;
    thresholdProperty.DefaultValue.value.ubVal = 127;
}
