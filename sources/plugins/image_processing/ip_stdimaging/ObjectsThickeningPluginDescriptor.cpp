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
#include <image_objects_thickening_16x16.h>
#include "ObjectsThickeningPlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x0000003A };

// Growing Amount property
static PropertyDescriptor growingAmountProperty =
{ XVT_U2, "Growing Amount", "growingAmount", "Thickness amount to add to objects' edges.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &growingAmountProperty,
};

// Let the class itself know description of its properties
const PropertyDescriptor** ObjectsThickeningPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_Morphology,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Objects Thickening",
    "ObjectsThickening",
    "Grows objects edges by the specified amount.",

    /* Long description */
    "The plug-in adds pixels to objects' edges in a grayscale image with binary data - black and white pixels "
    "(background and objects), which is result of some segmentation routine. Its result is similar to "
    "<a href='{AF000003-00000000-00000001-0000001C}'>Dilatation operator</a> with square structuring element. However, "
    "this plug-in utilizes distance transformation, which makes its performance independent to the value of growing "
    "amount.<br><br>"

    "See also: <a href='{AF000003-00000000-00000001-00000038}'>Objects Thinning</a>"
    ,
    &image_objects_thickening_16x16,
    nullptr,
    ObjectsThickeningPlugin,

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
    growingAmountProperty.DefaultValue.type = XVT_U2;
    growingAmountProperty.DefaultValue.value.usVal = 3;

    growingAmountProperty.MinValue.type = XVT_U2;
    growingAmountProperty.MinValue.value.usVal = 1;

    growingAmountProperty.MaxValue.type = XVT_U2;
    growingAmountProperty.MaxValue.value.usVal = 1000;
}
