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
#include <image_textile_16x16.h>
#include "TextileTexturePlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000006, 0x00000005 };

// Stitch Size property
static PropertyDescriptor stitchSizeProperty =
{ XVT_U1, "Stitch Size", "stitchSize", "Specifies stitch size of the textile texture.", PropertyFlag_None };
// Stitch Offset property
static PropertyDescriptor stitchOffsetProperty =
{ XVT_U1, "Stitch Offset", "stitchOffset", "Specifies stitch offset (phase) in the texture.", PropertyFlag_None };
// Amount To Keep property
static PropertyDescriptor amountToKeepProperty =
{ XVT_R4, "Amount To Kepp", "amountToKeep", "Specifies amount to keep of the source image's intesities, [0, 1].", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &stitchSizeProperty,
    &stitchOffsetProperty,
    &amountToKeepProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** TextileTexturePlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

/*
// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_TextureEffect,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Textile Texture",
    "TextileTexture",
    "Applies textile texture effect to an image.",
    "Some help about this routine.",
	&image_textile_16x16,
	0,
    TextileTexturePlugin,

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
    // stich size
	stitchSizeProperty.DefaultValue.type = XVT_U1;
	stitchSizeProperty.DefaultValue.value.ubVal = 7;

	stitchSizeProperty.MinValue.type = XVT_U1;
	stitchSizeProperty.MinValue.value.ubVal = 3;

	stitchSizeProperty.MaxValue.type = XVT_U1;
	stitchSizeProperty.MaxValue.value.ubVal = 30;

    // stich offset
	stitchOffsetProperty.DefaultValue.type = XVT_U1;
	stitchOffsetProperty.DefaultValue.value.ubVal = 0;

	stitchOffsetProperty.MinValue.type = XVT_U1;
	stitchOffsetProperty.MinValue.value.ubVal = 0;

	stitchOffsetProperty.MaxValue.type = XVT_U1;
	stitchOffsetProperty.MaxValue.value.ubVal = 30;

    // amount to keep
	amountToKeepProperty.DefaultValue.type = XVT_R4;
	amountToKeepProperty.DefaultValue.value.fVal = 0.5f;

	amountToKeepProperty.MinValue.type = XVT_R4;
	amountToKeepProperty.MinValue.value.fVal = 0.0f;

	amountToKeepProperty.MaxValue.type = XVT_R4;
	amountToKeepProperty.MaxValue.value.fVal = 1.0f;
}
