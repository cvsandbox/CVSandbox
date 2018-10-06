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
#include <image_objects_outline_16x16.h>
#include "ObjectsOutlinePlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x0000003B };

// Outline Thickness property
static PropertyDescriptor outlineThicknessProperty =
{ XVT_U2, "Outline Thickness", "outlineThickness", "Thickness of the outline to get around objects.", PropertyFlag_None };
// Outline Gap property
static PropertyDescriptor outlineGapProperty =
{ XVT_U2, "Outline Gap", "outlineGap", "Gap size between outline and objects.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &outlineThicknessProperty, &outlineGapProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** ObjectsOutlinePlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_Morphology,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Objects Outline",
    "ObjectsOutline",
    "Produces outline of the specified thickness around objects.",

    /* Long description */
    "The plug-in is similar to <a href = '{AF000003-00000000-00000001-00000039}'>Objects Edges</a> plug-in, but "
    "instead of keeping objects' edge pixels, it highlights pixels belonging to background area around objects. "
    "It operates on grayscale images, which are result of some segmentation routine and contain only background "
    "(black pixels) and objects (non-black pixels).<br><br>"

    "If outline gap is set to 0, same effect can be obtained by using <a href='{AF000003-00000000-00000001-0000003A}'>Objects Thickening</a> - "
    "first do objects' thickening and then <a href='{AF000003-00000000-00000001-00000029}'>subtract</a> source image from it. "
    "However, this plug-in avoids extra steps and operates directly on the result of distance transformation.<br><br>"

    "See also: <a href='{AF000003-00000000-00000001-00000039}'>Objects Edges</a>"
    ,
    &image_objects_outline_16x16,
    nullptr,
    ObjectsOutlinePlugin,

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
    outlineThicknessProperty.DefaultValue.type = XVT_U2;
    outlineThicknessProperty.DefaultValue.value.usVal = 3;

    outlineThicknessProperty.MinValue.type = XVT_U2;
    outlineThicknessProperty.MinValue.value.usVal = 1;

    outlineThicknessProperty.MaxValue.type = XVT_U2;
    outlineThicknessProperty.MaxValue.value.usVal = 1000;

    outlineGapProperty.DefaultValue.type = XVT_U2;
    outlineGapProperty.DefaultValue.value.usVal = 0;

    outlineGapProperty.MinValue.type = XVT_U2;
    outlineGapProperty.MinValue.value.usVal = 0;

    outlineGapProperty.MaxValue.type = XVT_U2;
    outlineGapProperty.MaxValue.value.usVal = 10;
}
