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
#include <image_rotate_16x16.h>
#include "RotateImagePlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x00000022 };

// Angle property
static PropertyDescriptor angleProperty =
{ XVT_R4, "Angle", "angle", "Angle to rotate by.", PropertyFlag_None };

// Fill Color property
static PropertyDescriptor fillColorProperty =
{ XVT_ARGB, "Fill color", "fillColor", "Color used to fill empty space.", PropertyFlag_None };

// Resize To Fit property
static PropertyDescriptor resizeToFitProperty =
{ XVT_Bool, "Resize To Fit", "resizeToFit", "Resize image to fit the entire rotated image or keep same size.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &angleProperty, &fillColorProperty, &resizeToFitProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** RotateImagePlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_Transformation,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Rotate Image",
    "RotateImage",
    "Rotates image counter clockwise by the specified angle.",

    /* Long description */
    "The plug-in performs image rotation by the specified angle using Bilinear interpolation."
    ,
    &image_rotate_16x16,
    0,
    RotateImagePlugin,

    sizeof( pluginProperties ) / sizeof( PropertyDescriptor* ),
    pluginProperties,
    PluginInitializer,
    0,
    0  // no dynamic properties update
);

// Complete properties description by initializing those parts, which were not
// initialized during properties array declaration
static void PluginInitializer( )
{
    // Angle property
    angleProperty.DefaultValue.type       = XVT_R4;
    angleProperty.DefaultValue.value.fVal = 0.0f;

    // Fill Color property
    fillColorProperty.DefaultValue.type               = XVT_ARGB;
    fillColorProperty.DefaultValue.value.argbVal.argb = 0xFF000000;

    // Resize To Fit property
    resizeToFitProperty.DefaultValue.type          = XVT_Bool;
    resizeToFitProperty.DefaultValue.value.boolVal = false;
}
