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
#include <image_morphology_plugin_16x16.h>
#include "MorphologyOperatorPlugin.hpp"

static void PluginInitializer( );
static void PluginCleaner( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x00000030 };

// Operator Type property
static PropertyDescriptor opeartorTypeProperty =
{ XVT_U1, "Operator Type", "operatorType", "Type of morphology operator to apply.", PropertyFlag_SelectionByIndex };

// Structuring Element property
static PropertyDescriptor structuringElementProperty =
{ XVT_I1 | XVT_Array2d, "Structuring Element", "structuringElement", "Shape of structuring element to use.", PropertyFlag_PreferredEditor_MorphologyStructuringElement };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &opeartorTypeProperty, &structuringElementProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** MorphologyOperatorPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_Morphology,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Morphology Operator",
    "MorphologyOperator",

    "Applies selected morphology operator using the specified structuring element.",
    
    /* Long description */
    "The core operators are <b>Erosion</b> and <b>Dilatation</b>. Erosion calculates minimum value of pixels, "
    "which correspond to value 1 in the structuring element (pixels, which correspond to zeros in the structuring "
    "element, are ignored). Dilatation calculates maximum value of those "
    "pixels. For binary images it has an effect of shrinking or growing object areas (white pixels).<br><br> "

    "<b>Opening</b> operator is a result of applying Erosion followed by Dilatation with the same structuring "
    "element. This removes small objects, but keeps big objects same size. <b>Closing</b> operator is a "
    "result of applying Dilatation followed by Erosion - fills small holes in objects.<br><br>"

    "<b>Top Hat</b> operator is a result of subtracting the image produced by Opening from the source image - "
    "shows all pixels, which were removed by Erosion, but could not be restored by Dilatation. "
    "<b>Bottom Hat</b> operator is a result of subtracting the source image from the image produced by Closing - "
    "shows all pixels, which were added by Dilatation, but could not be removed by Erosion.<br><br>"

    "See also: <a href='{AF000003-00000000-00000001-00000032}'>Hit and Miss</a>"
    ,
    &image_morphology_plugin_16x16,
    0,
    MorphologyOperatorPlugin,

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
    static const char* operatorNames[] = { "Erosion", "Dilatation", "Opening", "Closing", "Top Hat", "Bottom Hat" };

    // Type property
    opeartorTypeProperty.DefaultValue.type = XVT_U1;
    opeartorTypeProperty.DefaultValue.value.ubVal = 0;

    opeartorTypeProperty.MinValue.type = XVT_U1;
    opeartorTypeProperty.MinValue.value.ubVal = 0;

    opeartorTypeProperty.MaxValue.type = XVT_U1;
    opeartorTypeProperty.MaxValue.value.ubVal = XARRAY_SIZE( operatorNames ) - 1;

    opeartorTypeProperty.ChoicesCount = XARRAY_SIZE( operatorNames );
    opeartorTypeProperty.Choices = new xvariant[opeartorTypeProperty.ChoicesCount];

    for ( int i = 0; i < opeartorTypeProperty.ChoicesCount; i++ )
    {
        opeartorTypeProperty.Choices[i].type = XVT_String;
        opeartorTypeProperty.Choices[i].value.strVal = XStringAlloc( operatorNames[i] );
    }

    // set information about min/max values for the elements of the array property
    structuringElementProperty.MinValue.type = XVT_I1;
    structuringElementProperty.MinValue.value.sVal = 0;

    structuringElementProperty.MaxValue.type = XVT_I1;
    structuringElementProperty.MaxValue.value.sVal = 1;
}

// Clean-up plug-in - deallocate strings
static void PluginCleaner( )
{
    for ( int i = 0; i < opeartorTypeProperty.ChoicesCount; i++ )
    {
        XVariantClear( &opeartorTypeProperty.Choices[i] );
    }

    delete[] opeartorTypeProperty.Choices;
}
