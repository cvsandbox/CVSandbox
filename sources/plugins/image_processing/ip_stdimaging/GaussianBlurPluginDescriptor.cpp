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
#include <image_gaussian_blur_16x16.h>
#include "GaussianBlurPlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x00000019 };

// Sigma property
static PropertyDescriptor sigmaProperty =
{ XVT_R4, "Sigma", "sigma", "Gaussian sigma value to specify blurring strength.", PropertyFlag_None };
// Radius property
static PropertyDescriptor radiusProperty =
{ XVT_U1, "Radius", "radius", "Blurring radius.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &sigmaProperty, &radiusProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** GaussianBlurPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_ImageSmoothing,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Gaussian Blur",
    "GaussianBlur",
    "Performs image blurring using Gaussian kernel.",

    "This plug-in performs image blurring by doing <a href='{AF000003-00000000-00000001-00000031}'>convolution</a> using "
    "a Gaussian kernel. The width/height of the kernel equals to <b>radius * 2 + 1</b>. The <b>sigma</b> value determines "
    "blurring strength.<br><br>"

    "The Gaussian kernel values are calculated as: exp( -(x<sup>2</sup> + y<sup>2</sup>) / (2 * sigma<sup>2</sup>) )</sup>, "
    "where <b>x</b> and <b>y</b> run from <b>-radius</b> to <b>radius</b>."
    ,
    &image_gaussian_blur_16x16,
    0,
    GaussianBlurPlugin,

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
