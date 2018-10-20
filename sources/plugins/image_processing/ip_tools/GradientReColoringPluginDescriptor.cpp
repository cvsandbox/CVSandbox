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
#include <image_gradient_grayscale_recoloring_16x16.h>
#include "GradientReColoringPlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000007, 0x00000004 };

static PropertyDescriptor startColorProperty =
{ XVT_ARGB, "Start color", "startColor", "Color used to start re-coloring from (min values).", PropertyFlag_None };

static PropertyDescriptor endColorProperty =
{ XVT_ARGB, "End color", "endColor", "Color used to end re-coloring at (max values).", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &startColorProperty, &endColorProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** GradientReColoringPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_ColorFilter,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Grayscale Gradient Recoloring",
    "GrayscaleGradientRecoloring",
    "Recolors grayscale image into color image by mapping grayscale values to gradient between the specified two colors.",

    /* Long description */
    "The plug-in performs recoloring of grayscale images to color images by using a linear gradient between the "
    "two specified colors. So the 0 pixel values of grayscale are mapped to the <b>start color</b>, 255 values "
    "are mapped to the <b>end color</b> and the values in between are mapped linearly to the colors between the two specified."
    ,
    &image_gradient_grayscale_recoloring_16x16,
    0,
    GradientReColoringPlugin,

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

    endColorProperty.DefaultValue.type = XVT_ARGB;
    endColorProperty.DefaultValue.value.argbVal.argb = 0xFFC0FFC0;
}
