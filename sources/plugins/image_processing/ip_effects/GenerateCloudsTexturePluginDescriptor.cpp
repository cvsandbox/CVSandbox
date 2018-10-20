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
#include "GenerateCloudsTexturePlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000006, 0x0000000A };

// Width property
static PropertyDescriptor widthProperty =
{ XVT_U2, "Width", "width", "Specifies width of the texture image.", PropertyFlag_None };
// Height property
static PropertyDescriptor heightProperty =
{ XVT_U2, "Height", "height", "Specifies height of the texture image.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &widthProperty,
    &heightProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** GenerateCloudsTexturePlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_ImageGenerator,

    PluginType_ImageGenerator,
    PluginVersion,
    "Generate Clouds Texture",
    "GenerateCloudsTexture",
    "Generates clouds looking texture.",
    "Some help about this routine.",
	0, // &image_textile_16x16,
	0,
    GenerateCloudsTexturePlugin,

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
}
