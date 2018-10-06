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
#include <image_color_filter_16x16.h>
#include "DistanceColorFilterPlugin.hpp"

static void PluginInitializer( );
static void PluginCleaner( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x0000002F };

static PropertyDescriptor distanceTypeProperty =
{ XVT_U1, "Distance Type", "distanceType", "Type of distance to calculate.", PropertyFlag_SelectionByIndex };

static PropertyDescriptor sampleColorProperty =
{ XVT_ARGB, "Sample Color", "sampleColor", "Sample color to calculate distance from.", PropertyFlag_None };

static PropertyDescriptor maxDistanceProperty =
{ XVT_U2, "Maximum Distance", "maxDistance", "Maximum distance from the sample color.", PropertyFlag_None };

static PropertyDescriptor fillOutsideProperty =
{ XVT_Bool, "Fill Outside", "fillOutside", "Flag telling to fill outside or inside of the color range.", PropertyFlag_None };

static PropertyDescriptor fillColorProperty =
{ XVT_ARGB, "Fill Color", "fillColor", "Color used to fill filtered areas.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &distanceTypeProperty, &sampleColorProperty, &maxDistanceProperty,
    &fillOutsideProperty, &fillColorProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** DistanceColorFilterPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_ColorFilter,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Distance Color Filter",
    "DistanceColorFilter",
    "Filters colors outside/inside certain distance from a sample color.",

    /* Long description */
    "The plug-in filters out pixels (sets them to the specified <b>fill color</b>) which colors have "
    "difference with the sample color higher than the specified maximum distance. Or, if <b>Fill Outside</b> "
    "is set to <b>false</b>, pixels having color within the distance from the sample color.<br><br>"

    "The distance is calculated as:<ul>"
    "<li><b>Euclidean</b>: D = sqrt( dr<sup>2</sup> + dg<sup>2</sup> + db<sup>2</sup> ),"
    "<li><b>Manhattan</b>: D = |dr| + |dg| + |db|,"
    "</ul>"
    "where <b>dr</b>, <b>dg</b>, <b>db</b> are differences of red, green and blue components between "
    "sample color and pixel color.<br><br>"

    "See also: <a href='{AF000003-00000000-00000001-00000006}'>Color Filter</a>"
    ,
    &image_color_filter_16x16,
    0,
    DistanceColorFilterPlugin,

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
    distanceTypeProperty.DefaultValue.type = XVT_U1;
    distanceTypeProperty.DefaultValue.value.ubVal = 0;

    distanceTypeProperty.MinValue.type = XVT_U1;
    distanceTypeProperty.MinValue.value.ubVal = 0;

    distanceTypeProperty.MaxValue.type = XVT_U1;
    distanceTypeProperty.MaxValue.value.ubVal = 1;

    distanceTypeProperty.ChoicesCount = 2;
    distanceTypeProperty.Choices = new xvariant[2];

    distanceTypeProperty.Choices[0].type = XVT_String;
    distanceTypeProperty.Choices[0].value.strVal = XStringAlloc( "Euclidean" );

    distanceTypeProperty.Choices[1].type = XVT_String;
    distanceTypeProperty.Choices[1].value.strVal = XStringAlloc( "Manhattan" );

    sampleColorProperty.DefaultValue.type = XVT_ARGB;
    sampleColorProperty.DefaultValue.value.argbVal.argb = 0xFFFFFFFF;

    maxDistanceProperty.DefaultValue.type = XVT_U2;
    maxDistanceProperty.DefaultValue.value.usVal = 50;

    maxDistanceProperty.MinValue.type = XVT_U2;
    maxDistanceProperty.MinValue.value.usVal = 0;

    maxDistanceProperty.MaxValue.type = XVT_U2;
    maxDistanceProperty.MaxValue.value.usVal = 440;

    fillOutsideProperty.DefaultValue.type = XVT_Bool;
    fillOutsideProperty.DefaultValue.value.boolVal = true;

    fillColorProperty.DefaultValue.type = XVT_ARGB;
    fillColorProperty.DefaultValue.value.argbVal.argb = 0xFF000000;
}

// Clean-up plug-in - deallocate strings
static void PluginCleaner( )
{
    for ( int i = 0; i < distanceTypeProperty.ChoicesCount; i++ )
    {
        XVariantClear( &distanceTypeProperty.Choices[i] );
    }

    delete[] distanceTypeProperty.Choices;
}
