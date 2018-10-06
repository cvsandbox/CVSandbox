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
#include <image_objects_edges_16x16.h>
#include "ObjectsEdgesPlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x00000039 };

// Edge Thickness property
static PropertyDescriptor edgeThicknessProperty =
{ XVT_U2, "Edge Thickness", "edgeThickness", "Edge thickness to keep in objects.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &edgeThicknessProperty,
};

// Let the class itself know description of its properties
const PropertyDescriptor** ObjectsEdgesPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_Morphology,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Objects Edges",
    "ObjectsEdges",
    "Keeps specified amount of objects' edges and removes pixels from inside.",

    /* Long description */
    "The plug-in keeps specified thickness of objects' edges while removing anything from the inside "
    "of found objects. It operates on grayscale images, which are result of some segmentation routine and "
    "contain only background (black pixels) and objects (non-black pixels). Since it utilizes distance "
    "transformation of the source image, its performance does not depend on edge thickness to keep.<br><br>"

    "See also: <a href='{AF000003-00000000-00000001-00000038}'>Objects Thinning</a>"
    ,
    &image_objects_edges_16x16,
    nullptr,
    ObjectsEdgesPlugin,

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
    edgeThicknessProperty.DefaultValue.type = XVT_U2;
    edgeThicknessProperty.DefaultValue.value.usVal = 3;

    edgeThicknessProperty.MinValue.type = XVT_U2;
    edgeThicknessProperty.MinValue.value.usVal = 1;

    edgeThicknessProperty.MaxValue.type = XVT_U2;
    edgeThicknessProperty.MaxValue.value.usVal = 1000;
}
