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
#include <image_add_images_16x16.h>
#include "AddImagesPlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x00000028 };

// Factor property
static PropertyDescriptor factorProperty =
{ XVT_R4, "Factor", "factor", "Specifies amount of the second image to add.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &factorProperty,
};

// Let the class itself know description of its properties
const PropertyDescriptor** AddImagesPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_TwoImageFilters,

    PluginType_ImageProcessingFilter2,
    PluginVersion,
    "Add Images",
    "AddImages",
    "Adds one image to another of the same size and format.",

    "The plug-in sets pixels' values of result image to sum of corresponding pixels' values from the two "
    "provided images. The second image is multiplied by the specified factor. So result is: "
    "<b>image1 + image2 * factor</b>.<br><br>"

    "See also: <a href='{AF000003-00000000-00000001-00000029}'>Subtract Images</a>",

    &image_add_images_16x16,
    nullptr,
    AddImagesPlugin,

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
    factorProperty.DefaultValue.value.fVal = 1;

    factorProperty.MinValue.type = XVT_R4;
    factorProperty.MinValue.value.fVal = 0;

    factorProperty.MaxValue.type = XVT_R4;
    factorProperty.MaxValue.value.fVal = 1;
}
