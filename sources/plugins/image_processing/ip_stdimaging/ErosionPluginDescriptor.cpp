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
#include <image_erosion_16x16.h>
#include "ErosionPlugin.hpp"

static void PluginInitializer( );
static void PluginCleaner( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x0000001B };

// Radius property
static PropertyDescriptor radiusProperty =
{ XVT_U1, "Radius", "radius", "Radius of erosion structuring element.", PropertyFlag_None };

// Type property
static PropertyDescriptor typeProperty =
{ XVT_U1, "Shape", "shape", "Shape of structuring element to use.", PropertyFlag_SelectionByIndex };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &radiusProperty, &typeProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** ErosionPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_Morphology,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Erosion",
    "Erosion",
    "Applies erosion morphology operator using a structuring element of the specified size and shape.",

    /* Long description */
    "The plug-in performs erosion mathematical morphology operator. It assigns pixels in the output image "
    "to the minimum value of neighbour pixels (including the corresponding pixel itself) in the input image.<br><br>"

    "This version of the erosion operator is designed to process color or grayscale images using a structuring "
    "element of custom size and shape. The <b>radius</b> property controls the size of the structuring element, "
    "which is (2 * radius + 1) x (2 * radius + 1). For example, structuring element has size 5x5 if radius value "
    "is set to 2. The <b>shape</b> property controls number of neighbour pixels from the input image to use "
    "for MIN operator. Below are the possible shapes for 7x7 structuring element ('*' corresponds to neighbour "
    "pixels to use):<br><br>"

    "<b>Square</b> (49 neighbours):<br><tt>"
    "* * * * * * *<br>"
    "* * * * * * *<br>"
    "* * * * * * *<br>"
    "* * * * * * *<br>"
    "* * * * * * *<br>"
    "* * * * * * *<br>"
    "* * * * * * *</tt><br><br>"

    "<b>Circle</b> (37 neighbours):<br><tt>"
    ". . * * * . .<br>"
    ". * * * * * .<br>"
    "* * * * * * *<br>"
    "* * * * * * *<br>"
    "* * * * * * *<br>"
    ". * * * * * .<br>"
    ". . * * * . .</tt><br><br>"

    "<b>Diamond</b> (25 neighbours):<br><tt>"
    ". . . * . . .<br>"
    ". . * * * . .<br>"
    ". * * * * * .<br>"
    "* * * * * * *<br>"
    ". * * * * * .<br>"
    ". . * * * . .<br>"
    ". . . * . . .</tt><br><br>"

    "See also: <a href='{AF000003-00000000-00000001-0000001C}'>Dilatation operator</a>"
    ,
    &image_erosion_16x16,
    0,
    ErosionPlugin,

    sizeof( pluginProperties ) / sizeof( PropertyDescriptor* ),
    pluginProperties,
    PluginInitializer,
    PluginCleaner,
    0  // no dynamic properties update
);

// Complete properties description by initializing those parts, which were not 
// initialized during properties array declaration
static void PluginInitializer( )
{
    // Radius property
    radiusProperty.DefaultValue.type = XVT_U1;
    radiusProperty.DefaultValue.value.ubVal = 2;

    radiusProperty.MinValue.type = XVT_U1;
    radiusProperty.MinValue.value.ubVal = 1;

    radiusProperty.MaxValue.type = XVT_U1;
    radiusProperty.MaxValue.value.ubVal = 10;

    // Type property
    typeProperty.DefaultValue.type = XVT_U1;
    typeProperty.DefaultValue.value.ubVal = SEType_Square;

    typeProperty.MinValue.type = XVT_U1;
    typeProperty.MinValue.value.ubVal = SEType_Square;

    typeProperty.MaxValue.type = XVT_U1;
    typeProperty.MaxValue.value.ubVal = SEType_Diamond;

    typeProperty.ChoicesCount = 3;
    typeProperty.Choices = new xvariant[3];

    typeProperty.Choices[SEType_Square].type = XVT_String;
    typeProperty.Choices[SEType_Square].value.strVal = XStringAlloc( "Square" );

    typeProperty.Choices[SEType_Circle].type = XVT_String;
    typeProperty.Choices[SEType_Circle].value.strVal = XStringAlloc( "Circle" );

    typeProperty.Choices[SEType_Diamond].type = XVT_String;
    typeProperty.Choices[SEType_Diamond].value.strVal = XStringAlloc( "Diamond" );
}

// Clean-up plug-in - deallocate strings
static void PluginCleaner( )
{
    for ( int i = 0; i < typeProperty.ChoicesCount; i++ )
    {
        XVariantClear( &typeProperty.Choices[i] );
    }

    delete [] typeProperty.Choices;
}
