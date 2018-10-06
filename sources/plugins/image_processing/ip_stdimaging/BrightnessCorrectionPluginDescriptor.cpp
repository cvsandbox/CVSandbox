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
#include <image_gamma_correction_16x16.h>
#include "BrightnessCorrectionPlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 1 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x00000020 };

// Factor property
static PropertyDescriptor factorProperty =
{ XVT_R4, "Factor", "factor", "Brightness correction factor.", PropertyFlag_None };
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
const PropertyDescriptor** BrightnessCorrectionPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;


// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_ColorFilter,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Brightness Correction",
    "BrightnessCorrection",
    "Performs brightness correction for the given image.",

    /* Long description */
    "The plug-in performs brightness correction for the given image - increasing or decreasing image's brightness. "
    "Every pixels' value is remapped to a new one according to the equation: out = in<sup>gamma</sup>, where <b>in</b> and "
    "<b>out</b> are pixels' input/output values in the [0, 1] range. Note: the above equation is a standard gamma correction "
    "filter. The <b>gamma</b> value is set to <b>1-factor</b> for negative factors (decreasing brightness) or "
    "<b>1/(factor+1)</b> for positive factors (increasing brightness).<br><br>"

    "This image processing filter can be applied as to all RGB channels of a color image, as to individual ones only. For "
    "grayscale image it is always applied to intensity values."
    ,
    &image_gamma_correction_16x16,
    0,
    BrightnessCorrectionPlugin,

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
