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
#include <image_diff_images_thresholded_16x16.h>
#include "DiffImagesThresholdedPlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x00000027 };

// Threshold property
static PropertyDescriptor thresholdProperty =
{ XVT_I2, "Threshold", "threshold", "Difference threshold value used to set pixel to white or black.", PropertyFlag_None };
// Hi Color property
static PropertyDescriptor hiColorProperty =
{ XVT_ARGB, "Hi Color", "hiColor", "Color value to set for differences above the threshold.", PropertyFlag_None };
// Low Color property
static PropertyDescriptor lowColorProperty =
{ XVT_ARGB, "Low Color", "lowColor", "Color value to set for differences below the threshold.", PropertyFlag_None };
// Diff Pixels property
static PropertyDescriptor diffPixelsProperty =
{ XVT_U4, "Diff Pixels", "diffPixels", "Number of pixels set to white color (crossed the threshold).", PropertyFlag_ReadOnly };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &thresholdProperty, &hiColorProperty, &lowColorProperty, &diffPixelsProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** DiffImagesThresholdedPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_TwoImageFilters,

    PluginType_ImageProcessingFilter2,
    PluginVersion,
    "Diff Images Thresholded",
    "DiffImagesThresholded",
    "Calculates thresholded absolute difference between two images, ( abs(image1 - image2) >= threshold ) ? hiColor : lowColor.",

    "The plug-in sets pixels' values of result image to <b>hiColor</b>, if absolute difference of corresponding pixels' in "
    "provided images is greater than or equal to the specified threshold. Otherwise pixels in result image are set to <b>lowColor</b>. "
    "For color images, the sum of RGB absolute differences is used.<br><br>"
    
    "The plug-in also provides number of thresholded pixels (set to hi color) in the last processed image, which can be used to estimate "
    "difference between images and perform some actions based on that.<br><br>"

    "See also: <a href='{AF000003-00000000-00000001-00000018}'>Diff Images</a>",

    &image_diff_images_thresholded_16x16,
    nullptr,
    DiffImagesThresholdedPlugin,

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
    thresholdProperty.DefaultValue.type       = XVT_I2;
    thresholdProperty.DefaultValue.value.sVal = 30;

    thresholdProperty.MinValue.type       = XVT_I2;
    thresholdProperty.MinValue.value.sVal = 1;

    thresholdProperty.MaxValue.type       = XVT_I2;
    thresholdProperty.MaxValue.value.sVal = 765;

    hiColorProperty.DefaultValue.type = XVT_ARGB;
    hiColorProperty.DefaultValue.value.argbVal.argb = 0xFFFFFFFF;

    lowColorProperty.DefaultValue.type = XVT_ARGB;
    lowColorProperty.DefaultValue.value.argbVal.argb = 0xFF000000;
}
