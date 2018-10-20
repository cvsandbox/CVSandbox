/*
    Image processing tools plug-ins of Computer Vision Sandbox

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
#include <image_fade_images_16x16.h>
#include "FadeImagesPlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000007, 0x00000008 };

// Factor property
static PropertyDescriptor factorProperty =
{ XVT_R4, "Factor", "factor", "Specfies amount of the first image to keep. The rest (1 - factor) is taken from the second image.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &factorProperty,
};

// Let the class itself know description of its properties
const PropertyDescriptor** FadeImagesPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_TwoImageFilters,

    PluginType_ImageProcessingFilter2,
    PluginVersion,
    "Fade Images",
    "FadeImages",
    "Fades one image into another creating transition effect.",

    "The plug-in sets pixels' values of result image to weighted sum of corresponding pixels' values from the two "
    "provided images. The first image's weight is <b>factor</b>, while second image's weight is <b>1 - factor</b>. "
    "This leads to result being equal to <b>factor * image1 + (1 - factor) * image2</b>.",

    &image_fade_images_16x16,
    nullptr,
    FadeImagesPlugin,

    sizeof( pluginProperties ) / sizeof( PropertyDescriptor* ),
    pluginProperties,
    PluginInitializer,
    nullptr, // no clean-up
    nullptr  // no dynamic properties update
);

// Complete properties description by initializing those parts, which were not
// initialized during properties array declaration
static void PluginInitializer( )
{
    factorProperty.DefaultValue.type = XVT_R4;
    factorProperty.DefaultValue.value.fVal = 0.5f;

    factorProperty.MinValue.type = XVT_R4;
    factorProperty.MinValue.value.fVal = 0.0f;

    factorProperty.MaxValue.type = XVT_R4;
    factorProperty.MaxValue.value.fVal = 1.0f;
}
