/*
    Image processing effects plug-ins of Computer Vision Sandbox

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
//#include <image_textile_16x16.h>
#include "GenerateGrainTexturePlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000006, 0x00000013 };

// Width property
static PropertyDescriptor widthProperty =
{ XVT_U2, "Width", "width", "Specifies width of the texture image.", PropertyFlag_None };
// Height property
static PropertyDescriptor heightProperty =
{ XVT_U2, "Height", "height", "Specifies height of the texture image.", PropertyFlag_None };
// Spacing property
static PropertyDescriptor spacingProperty =
{ XVT_U2, "Spacing", "spacing", "Specifies maximum value for the random spacing between grain lines.", PropertyFlag_None };
// Density property
static PropertyDescriptor densityProperty =
{ XVT_R4, "Density", "density", "Specifies grain density.", PropertyFlag_None };
// IsVertical property
static PropertyDescriptor isVerticalProperty =
{ XVT_Bool, "IsVertical", "isVertical", "Specifies grain density.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &widthProperty, &heightProperty, &spacingProperty, &densityProperty, &isVerticalProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** GenerateGrainTexturePlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_ImageGenerator,

    PluginType_ImageGenerator,
    PluginVersion,
    "Generate Grain Texture",
    "GenerateGrainTexture",
    "Generates vertical/horizontal grain texture.",
    "Some help about this routine.",
    0, // &image_textile_16x16,
    0,
    GenerateGrainTexturePlugin,

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
    // width
    widthProperty.DefaultValue.type = XVT_U2;
    widthProperty.DefaultValue.value.usVal = 640;

    widthProperty.MinValue.type = XVT_U2;
    widthProperty.MinValue.value.usVal = 16;

    widthProperty.MaxValue.type = XVT_U2;
    widthProperty.MaxValue.value.usVal = 10000;

    // height
    heightProperty.DefaultValue.type = XVT_U2;
    heightProperty.DefaultValue.value.usVal = 480;

    heightProperty.MinValue.type = XVT_U2;
    heightProperty.MinValue.value.usVal = 16;

    heightProperty.MaxValue.type = XVT_U2;
    heightProperty.MaxValue.value.usVal = 10000;

    // spacing
    spacingProperty.DefaultValue.type = XVT_U2;
    spacingProperty.DefaultValue.value.usVal = 50;

    spacingProperty.MinValue.type = XVT_U2;
    spacingProperty.MinValue.value.usVal = 5;

    spacingProperty.MaxValue.type = XVT_U2;
    spacingProperty.MaxValue.value.usVal = 500;

    // density
    densityProperty.DefaultValue.type = XVT_R4;
    densityProperty.DefaultValue.value.fVal = 0.5f;

    densityProperty.MinValue.type = XVT_R4;
    densityProperty.MinValue.value.fVal = 0.0f;

    densityProperty.MaxValue.type = XVT_R4;
    densityProperty.MaxValue.value.fVal = 1.0f;

    // isVertical
    isVerticalProperty.DefaultValue.type = XVT_Bool;
    isVerticalProperty.DefaultValue.value.boolVal = true;
}
