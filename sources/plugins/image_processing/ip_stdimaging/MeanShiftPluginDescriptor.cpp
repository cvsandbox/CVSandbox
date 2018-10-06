/*
    Standard image processing plug-ins of Computer Vision Sandbox

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
#include <image_mean_shift_16x16.h>
#include "MeanShiftPlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x00000035 };

// Radius property
static PropertyDescriptor radiusProperty =
{ XVT_U2, "Radius", "radius", "Radius of mean shift window.", PropertyFlag_None };

// Color Distance property
static PropertyDescriptor colorDistanceProperty =
{ XVT_U2, "Color distance", "colorDistance", "Maximum color distance for pixel to be taken into mean calculation.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &radiusProperty, &colorDistanceProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** MeanShiftPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_ImageSmoothing,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Mean Shift",
    "MeanShift",
    "Performs mean shift image smoothing.",

    /* Long description */
    "The plug-in calculates mean value of pixels in a window of the specified size (<b>radius</b> * 2 + 1). "
    "However, not all pixels from the window are taken into account. Each pixel's color is compared with color "
    "of the window's center pixel. If its Euclidean distance is not grater than the specified limit, then it "
    "is taken into calculation of the mean value. So, if center pixel has (cR, cG, cB) color, and the pixel "
    "being checked has (R, G, B) color, then <b>(R - cR)<sup>2</sup> + (G - cG)<sup>2</sup> + (B - cB)<sup>2</sup></b> "
    "must not be greater than <b>ColorDistance<sup>2</sup></b> in order for the pixel to be considered for mean "
    "calculation."
    ,
    &image_mean_shift_16x16,
    nullptr,
    MeanShiftPlugin,

    sizeof( pluginProperties ) / sizeof( PropertyDescriptor* ),
    pluginProperties,
    PluginInitializer,
    nullptr,
    nullptr  // no dynamic properties update
);

// Complete properties description by initializing those parts, which were not 
// initialized during properties array declaration
static void PluginInitializer( )
{
    // Radius property
    radiusProperty.DefaultValue.type = XVT_U2;
    radiusProperty.DefaultValue.value.usVal = 2;

    radiusProperty.MinValue.type = XVT_U2;
    radiusProperty.MinValue.value.usVal = 1;

    radiusProperty.MaxValue.type = XVT_U2;
    radiusProperty.MaxValue.value.usVal = 20;

    // Color Distance property
    colorDistanceProperty.DefaultValue.type = XVT_U2;
    colorDistanceProperty.DefaultValue.value.usVal = 10;

    colorDistanceProperty.MinValue.type = XVT_U2;
    colorDistanceProperty.MinValue.value.usVal = 1;

    colorDistanceProperty.MaxValue.type = XVT_U2;
    colorDistanceProperty.MaxValue.value.usVal = 100;
}
