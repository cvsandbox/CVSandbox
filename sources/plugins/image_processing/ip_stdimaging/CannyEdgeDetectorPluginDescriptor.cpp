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
#include <image_canny_edge_detector_16x16.h>
#include "CannyEdgeDetectorPlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x00000033 };

// Sigma property
static PropertyDescriptor sigmaProperty =
{ XVT_R4, "Sigma", "sigma", "Gaussian sigma value to specify blurring strength.", PropertyFlag_None };
// Radius property
static PropertyDescriptor radiusProperty =
{ XVT_U1, "Radius", "radius", "Blurring radius.", PropertyFlag_None };
// Low Threshold property
static PropertyDescriptor lowThresholdProperty =
{ XVT_U2, "Low Threshold", "lowThreshold", "Low threshold for edge hysteresis.", PropertyFlag_None };
// High Threshold property
static PropertyDescriptor highThresholdProperty =
{ XVT_U2, "High Threshold", "highThreshold", "High threshold for edge hysteresis.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &sigmaProperty, &radiusProperty, &lowThresholdProperty, &highThresholdProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** CannyEdgeDetectorPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_EdgeDetector,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Canny Edge Detector",
    "CannyEdgeDetector",
    "Appies Canny edge detector to the specified image.",

    /* Long description */
    "The plug-in implements <a href='https://en.wikipedia.org/wiki/Canny_edge_detector'>Canny edge detector</a>. "
    "First it applies Gaussian blur with the specified <b>sigma</b> and <b>radius</b> values. Then it uses "
    "<a href='https://en.wikipedia.org/wiki/Sobel_operator'>Sobel operator</a> to find edges' gradients and "
    "their orientation. After performing non-maximum suppression, it finally does edge hysteresis - if gradient's "
    "value is higher than <b>High Threshold</b>, then it is an edge pixel for sure (strong edge); if it is lower than "
    "<b>Low Threshold</b> - it is not an edge pixel; and if gradient's value is between those two thresholds - "
    "the pixel becomes an edge pixel only if one of its 8 neighbours is a strong edges."
    ,
    &image_canny_edge_detector_16x16,
    0,
    CannyEdgeDetectorPlugin,

    XARRAY_SIZE( pluginProperties ),
    pluginProperties,
    PluginInitializer,
    nullptr,
    nullptr  // no dynamic properties update
);

// Complete properties description by initializing those parts, which were not 
// initialized during properties array declaration
static void PluginInitializer( )
{
    // Sigma property
    sigmaProperty.DefaultValue.type = XVT_R4;
    sigmaProperty.DefaultValue.value.fVal = 1.4f;

    sigmaProperty.MinValue.type = XVT_R4;
    sigmaProperty.MinValue.value.fVal = 0.1f;

    sigmaProperty.MaxValue.type = XVT_R4;
    sigmaProperty.MaxValue.value.fVal = 10.0f;

    // Radius property
    radiusProperty.DefaultValue.type = XVT_U1;
    radiusProperty.DefaultValue.value.ubVal = 2;

    radiusProperty.MinValue.type = XVT_U1;
    radiusProperty.MinValue.value.ubVal = 1;

    radiusProperty.MaxValue.type = XVT_U1;
    radiusProperty.MaxValue.value.ubVal = 10;

    // Low Threshold property
    lowThresholdProperty.DefaultValue.type = XVT_U2;
    lowThresholdProperty.DefaultValue.value.usVal = 20;

    lowThresholdProperty.MinValue.type = XVT_U2;
    lowThresholdProperty.MinValue.value.usVal = 1;

    lowThresholdProperty.MaxValue.type = XVT_U2;
    lowThresholdProperty.MaxValue.value.usVal = 1000;

    // High Threshold property
    highThresholdProperty.DefaultValue.type = XVT_U2;
    highThresholdProperty.DefaultValue.value.usVal = 100;

    highThresholdProperty.MinValue.type = XVT_U2;
    highThresholdProperty.MinValue.value.usVal = 1;

    highThresholdProperty.MaxValue.type = XVT_U2;
    highThresholdProperty.MaxValue.value.usVal = 1000;
}
