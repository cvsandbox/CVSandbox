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
#include <image_objects_thinning_16x16.h>
#include "ObjectsThinningPlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x00000038 };

// Thinning Amount property
static PropertyDescriptor thinningAmountProperty =
{ XVT_U2, "Thinning Amount", "thinningAmount", "Edge thickness to remove from objects.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &thinningAmountProperty,
};

// Let the class itself know description of its properties
const PropertyDescriptor** ObjectsThinningPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_Morphology,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Objects Thinning",
    "ObjectsThinning",
    "Removes pixels from objects' edges.",

    /* Long description */
    "The plug-in removes pixels from objects' edges in a grayscale image with binary data - black and white pixels "
    "(background and objects), which is result of some segmentation routine. Its result is similar to "
    "<a href='{AF000003-00000000-00000001-0000001B}'>Erosion operator</a> with square structuring element. However, "
    "this plug-in utilizes distance transformation, which makes its performance independent to the value of thinning "
    "amount.<br><br>"

    "See also: <a href='{AF000003-00000000-00000001-0000003A}'>Objects Thickening</a>, "
    "<a href='{AF000003-00000000-00000001-00000039}'>Objects Edges</a>"
    ,
    &image_objects_thinning_16x16,
    nullptr,
    ObjectsThinningPlugin,

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
    thinningAmountProperty.DefaultValue.type = XVT_U2;
    thinningAmountProperty.DefaultValue.value.usVal = 3;

    thinningAmountProperty.MinValue.type = XVT_U2;
    thinningAmountProperty.MinValue.value.usVal = 1;

    thinningAmountProperty.MaxValue.type = XVT_U2;
    thinningAmountProperty.MaxValue.value.usVal = 1000;
}
