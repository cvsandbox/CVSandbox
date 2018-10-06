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
#include <image_erode_edges_16x16.h>
#include "ErodeEdgesPlugin.hpp"

static void PluginInitializer( );
static void PluginCleaner( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x00000036 };

// Type property
static PropertyDescriptor edgeTypeProperty =
{ XVT_U1, "Edge Type", "edgeType", "Edge type to erode.", PropertyFlag_SelectionByIndex };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &edgeTypeProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** ErodeEdgesPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_Morphology,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Erode Edges",
    "ErodeEdges",
    "Erode horizontal/vertical objects' edges.",

    /* Long description */
    "The plug-in erodes horizontal/vertical edges of objects in grayscale images. Usually those image "
    "are result of some edge detection operator, segmentation, etc. When eroding horizontal edges, the "
    "plug-in removes all pixels which are not connected to any of the 3 neighbours above or below. "
    "Similar with vertical edges - it removes pixels which are not connected to any of the 3 neighbours on "
    "the left or right."
    ,
    &image_erode_edges_16x16,
    nullptr,
    ErodeEdgesPlugin,

    sizeof( pluginProperties ) / sizeof( PropertyDescriptor* ),
    pluginProperties,
    PluginInitializer,
    PluginCleaner,
    nullptr  // no dynamic properties update
);

// Complete properties description by initializing those parts, which were not 
// initialized during properties array declaration
static void PluginInitializer( )
{
    // Edge Type property
    edgeTypeProperty.DefaultValue.type = XVT_U1;
    edgeTypeProperty.DefaultValue.value.ubVal = 0;

    edgeTypeProperty.MinValue.type = XVT_U1;
    edgeTypeProperty.MinValue.value.ubVal = 0;

    edgeTypeProperty.MaxValue.type = XVT_U1;
    edgeTypeProperty.MaxValue.value.ubVal = 1;

    edgeTypeProperty.ChoicesCount = 2;
    edgeTypeProperty.Choices = new xvariant[2];

    edgeTypeProperty.Choices[0].type = XVT_String;
    edgeTypeProperty.Choices[0].value.strVal = XStringAlloc( "Horizontal" );

    edgeTypeProperty.Choices[1].type = XVT_String;
    edgeTypeProperty.Choices[1].value.strVal = XStringAlloc( "Vertical" );
}

// Clean-up plug-in - deallocate strings
static void PluginCleaner( )
{
    for ( int i = 0; i < edgeTypeProperty.ChoicesCount; i++ )
    {
        XVariantClear( &edgeTypeProperty.Choices[i] );
    }

    delete [] edgeTypeProperty.Choices;
}
