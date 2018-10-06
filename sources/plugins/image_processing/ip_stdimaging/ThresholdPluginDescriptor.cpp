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
#include <image_threshold_16x16.h>
#include "ThresholdPlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 1 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x00000001 };

// Threshold property
static PropertyDescriptor thresholdProperty =
{ XVT_U2, "Threshold", "threshold", "Threshold value used for binarization.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &thresholdProperty,
};

// Let the class itself know description of its properties
const PropertyDescriptor** ThresholdPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_Thresholding,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Threshold",
    "Threshold",
    "Performs thresholding of the specified grayscale image.",

    /* Long description */
    "The plug-in performs thresholding of grayscale images by setting all pixels' values equal or greater than "
    "the specified threshold to white, the rest of pixels' values are set to black."
    ,
    &image_threshold_16x16,
    0, 
    ThresholdPlugin,

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
    thresholdProperty.DefaultValue.type = XVT_U2;
    thresholdProperty.DefaultValue.value.usVal = 128;
}
