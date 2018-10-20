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
#include "ApplyTexturePlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000006, 0x0000000B };

// Amount To Keep property
static PropertyDescriptor amountToKeepProperty =
{ XVT_R4, "Amount To Kepp", "amountToKeep", "Specifies amount to keep of the source image's intesities, [0, 1].", PropertyFlag_None };
// Texture Base Level property
static PropertyDescriptor textureBaseLevelProperty =
{ XVT_U1, "Texture Base Level", "textureBaseLevel", "Level of the texture which would correspond to 1.0 multiplier.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &amountToKeepProperty, &textureBaseLevelProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** ApplyTexturePlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

/*
// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_Default,

    PluginType_ImageProcessingFilter2,
    PluginVersion,
    "Apply Texture",
    "ApplyTexture",
    "Applies selected texture to an image.",
    "Some help about this routine.",
    0,//&image_textile_16x16,
    0,
    ApplyTexturePlugin,

    sizeof( pluginProperties ) / sizeof( PropertyDescriptor* ),
    pluginProperties,
    PluginInitializer,
    0, // no clean-up
    0  // no dynamic properties update
);
*/

// Complete properties description by initializing those parts, which were not 
// initialized during properties array declaration
static void PluginInitializer( )
{
    // amount to keep
    amountToKeepProperty.DefaultValue.type = XVT_R4;
    amountToKeepProperty.DefaultValue.value.fVal = 0.5f;

    amountToKeepProperty.MinValue.type = XVT_R4;
    amountToKeepProperty.MinValue.value.fVal = 0.0f;

    amountToKeepProperty.MaxValue.type = XVT_R4;
    amountToKeepProperty.MaxValue.value.fVal = 1.0f;

    // texture base level
    textureBaseLevelProperty.DefaultValue.type = XVT_U1;
    textureBaseLevelProperty.DefaultValue.value.ubVal = 255;

    textureBaseLevelProperty.MinValue.type = XVT_U1;
    textureBaseLevelProperty.MinValue.value.ubVal = 32;

    textureBaseLevelProperty.MaxValue.type = XVT_U1;
    textureBaseLevelProperty.MaxValue.value.ubVal = 255;
}
