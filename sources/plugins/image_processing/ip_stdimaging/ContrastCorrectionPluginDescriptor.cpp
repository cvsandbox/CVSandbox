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
#include <image_contrast_correction_16x16.h>
#include "ContrastCorrectionPlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 1 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x00000021 };

// Factor property
static PropertyDescriptor factorProperty =
{ XVT_R4, "Factor", "factor", "Contrast correction factor.", PropertyFlag_None };
// Process Red property
static PropertyDescriptor precessRedProperty =
{ XVT_Bool, "Process red", "processRed", "Process red channel or not.", PropertyFlag_None };
// Process Green property
static PropertyDescriptor precessGreenProperty =
{ XVT_Bool, "Process green", "processGreen", "Process green channel or not.", PropertyFlag_None };
// Process Blue property
static PropertyDescriptor precessBlueProperty =
{ XVT_Bool, "Process blue", "processBlue", "Process blue channel or not.", PropertyFlag_None };


// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &factorProperty, &precessRedProperty, &precessGreenProperty, &precessBlueProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** ContrastCorrectionPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;


// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_ColorFilter,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Contrast Correction",
    "ContrastCorrection",
    "Performs contrast correction for the given image.",

    /* Long description */
    "The plug-in performs contrast correction for the given image by applying a S-shape curve to its intensities. "
    "Contrast is increased for positive <b>factor</b> value or decreased otherwise.<br><br>"

    "This image processing filter can be applied as to all RGB channels of a color image, as to individual ones only. For "
    "grayscale image it is always applied to intensity values."
    ,
    &image_contrast_correction_16x16,
    0,
    ContrastCorrectionPlugin,

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
    // Factor property
    factorProperty.DefaultValue.type = XVT_R4;
    factorProperty.DefaultValue.value.fVal = 0.2f;

    factorProperty.MinValue.type = XVT_R4;
    factorProperty.MinValue.value.fVal = -10.0f;

    factorProperty.MaxValue.type = XVT_R4;
    factorProperty.MaxValue.value.fVal = 10.0f;

    // Process Red  property
    precessRedProperty.DefaultValue.type = XVT_Bool;
    precessRedProperty.DefaultValue.value.boolVal = true;

    // Process Green property
    precessGreenProperty.DefaultValue.type = XVT_Bool;
    precessGreenProperty.DefaultValue.value.boolVal = true;

    // Process Blue property
    precessBlueProperty.DefaultValue.type = XVT_Bool;
    precessBlueProperty.DefaultValue.value.boolVal = true;
}
