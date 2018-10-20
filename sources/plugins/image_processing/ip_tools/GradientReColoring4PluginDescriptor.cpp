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
#include <image_gradient_grayscale_recoloring4_16x16.h>
#include "GradientReColoring4Plugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000007, 0x00000006 };

static PropertyDescriptor startColorProperty =
{ XVT_ARGB, "Start color", "startColor", "Color used to start re-coloring from (min values).", PropertyFlag_None };

static PropertyDescriptor middleColor1Property =
{ XVT_ARGB, "Middle color 1", "middleColor1", "First middle color (at the 1st threshold value).", PropertyFlag_None };

static PropertyDescriptor middleColor2Property =
{ XVT_ARGB, "Middle color 2", "middleColor2", "Second middle color (at the 2nd threshold value).", PropertyFlag_None };

static PropertyDescriptor middleColor3Property =
{ XVT_ARGB, "Middle color 3", "middleColor3", "Third middle color (at the 3rd threshold value).", PropertyFlag_None };

static PropertyDescriptor endColorProperty =
{ XVT_ARGB, "End color", "endColor", "Color used to end re-coloring at (max values).", PropertyFlag_None };

static PropertyDescriptor threshold1Property =
{ XVT_U1, "Threshold 1", "threshold1", "Threshold value where the first gradient changes to the second gradient.", PropertyFlag_None };

static PropertyDescriptor threshold2Property =
{ XVT_U1, "Threshold 2", "threshold2", "Threshold value where the second gradient changes to the third gradient.", PropertyFlag_None };

static PropertyDescriptor threshold3Property =
{ XVT_U1, "Threshold 3", "threshold3", "Threshold value where the third gradient changes to the fourth gradient.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &startColorProperty, &middleColor1Property, &middleColor2Property, &middleColor3Property, &endColorProperty,
    &threshold1Property, &threshold2Property, &threshold3Property
};

// Let the class itself know description of its properties
const PropertyDescriptor** GradientReColoring4Plugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_ColorFilter,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Grayscale Gradient Recoloring 4",
    "GrayscaleGradientRecoloring4",
    "Recolors grayscale image into color image by mapping grayscale values to four gradients between the specified five colors.",

    /* Long description */
    "The plug-in performs recoloring of grayscale images to color images by using four linear gradients between the "
    "five specified colors. Grayscale pixel values are mapped linearly to the folowing color ranges: <ul>"
    "<li>[0, threshold1] - mapped to colors between the <b>start color</b> and the <b>1<sup>st</sup> middle color</b>;"
    "<li>[threshold1, threshold2] - mapped to colors between the <b>1<sup>st</sup> middle color</b> and the <b>2<sup>nd</sup> middle color</b>;"
    "<li>[threshold2, threshold3] - mapped to colors between the <b>2<sup>nd</sup> middle color</b> and the <b>3<sup>rd</sup> middle color</b>;"
    "<li>[threshold3, 255] - mapped to colors between the <b>3<sup>rd</sup> middle color</b> and the <b>end color</b>."
    ,
    &image_gradient_grayscale_recoloring4_16x16,
    0,
    GradientReColoring4Plugin,

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

    middleColor1Property.DefaultValue.type = XVT_ARGB;
    middleColor1Property.DefaultValue.value.argbVal.argb = 0xFFA0FF80;

    middleColor2Property.DefaultValue.type = XVT_ARGB;
    middleColor2Property.DefaultValue.value.argbVal.argb = 0xFF005000;

    middleColor3Property.DefaultValue.type = XVT_ARGB;
    middleColor3Property.DefaultValue.value.argbVal.argb = 0xFF50A080;

    endColorProperty.DefaultValue.type = XVT_ARGB;
    endColorProperty.DefaultValue.value.argbVal.argb = 0xFFC0E0FF;

    threshold1Property.DefaultValue.type = XVT_U1;
    threshold1Property.DefaultValue.value.ubVal = 63;

    threshold2Property.DefaultValue.type = XVT_U1;
    threshold2Property.DefaultValue.value.ubVal = 127;

    threshold3Property.DefaultValue.type = XVT_U1;
    threshold3Property.DefaultValue.value.ubVal = 191;
}
