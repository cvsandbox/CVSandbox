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
#include <image_gaussian_sharpen_16x16.h>
#include "GaussianSharpenPlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x0000001A };

// Sigma property
static PropertyDescriptor sigmaProperty =
{ XVT_R4, "Sigma", "sigma", "Gaussian sigma value to specify sharpening strength.", PropertyFlag_None };
// Radius property
static PropertyDescriptor radiusProperty =
{ XVT_U1, "Radius", "radius", "Sharpening radius.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &sigmaProperty, &radiusProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** GaussianSharpenPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_ImageSmoothing,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Gaussian Sharpen",
    "GaussianSharpen",
    "Performs image sharpening using Gaussian kernel.",

    "This plug-in performs image sharpening by doing <a href='{AF000003-00000000-00000001-00000031}'>convolution</a> using "
    "a Gaussian sharpening kernel. The width/height of the kernel equals to <b>radius * 2 + 1</b>. The <b>sigma</b> value determines "
    "sharpening strength.<br><br>"

    "The sharpening kernel is based on <a href='{AF000003-00000000-00000001-00000019}'>Gaussian blur kernel</a>, where all elements "
    "of the kernel are negated except the center element, which is set to <b>2 * BlurKernelSum - CenterValue</b>. This creates a kernel "
    "with all negative values and a single positive high value in its center."
    ,
    &image_gaussian_sharpen_16x16,
    0,
    GaussianSharpenPlugin,

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
}
