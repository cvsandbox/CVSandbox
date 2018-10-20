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
#include <image_simple_posterization_16x16.h>
#include "SimplePosterizationPlugin.hpp"

static void PluginInitializer( );
static void PluginCleaner( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000007, 0x00000003 };

// Posterization Interval property
static PropertyDescriptor posterizationIntervalProperty =
{ XVT_U1, "Posterization interval", "interval", "Posterization interval used for color reduction.", PropertyFlag_None };

// Fill Type property
static PropertyDescriptor fillTypeProperty =
{ XVT_U1, "Fill type", "fillType", "Fill type of the posterization intervals.", PropertyFlag_SelectionByIndex };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &posterizationIntervalProperty,
    &fillTypeProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** SimplePosterizationPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_ColorFilter,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Simple Posterization",
    "SimplePosterization",
    "Performs simple posterization effect \"reducing colors\" in an image.",

    /* Long description */
    "The plug-in reduces number of colors used in an image, by splitting [0, 255] intensities range "
    "into intervals of the certain size and then replacing all values within the interval with either "
    "minimum, maximum or middle value. For color images it processes each RGB color plane individually."
    ,
    &image_simple_posterization_16x16,
    0,
    SimplePosterizationPlugin,

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
    // Posterization Interval property
    posterizationIntervalProperty.DefaultValue.type = XVT_U1;
    posterizationIntervalProperty.DefaultValue.value.ubVal = 16;

    posterizationIntervalProperty.MinValue.type = XVT_U1;
    posterizationIntervalProperty.MinValue.value.ubVal = 2;

    posterizationIntervalProperty.MaxValue.type = XVT_U1;
    posterizationIntervalProperty.MaxValue.value.ubVal = 128;

    // Fill Type property
    fillTypeProperty.DefaultValue.type = XVT_U1;
    fillTypeProperty.DefaultValue.value.ubVal = RangeMiddle;

    fillTypeProperty.MaxValue.type = XVT_U1;
    fillTypeProperty.MaxValue.value.ubVal = RangeMiddle;

    fillTypeProperty.ChoicesCount = 3;
    fillTypeProperty.Choices = new xvariant[3];

    fillTypeProperty.Choices[RangeMin].type = XVT_String;
    fillTypeProperty.Choices[RangeMin].value.strVal = XStringAlloc( "Min" );

    fillTypeProperty.Choices[RangeMax].type = XVT_String;
    fillTypeProperty.Choices[RangeMax].value.strVal = XStringAlloc( "Max" );

    fillTypeProperty.Choices[RangeMiddle].type = XVT_String;
    fillTypeProperty.Choices[RangeMiddle].value.strVal = XStringAlloc( "Middle" );
}

// Clean-up plugin - deallocate strings
static void PluginCleaner( )
{
    for ( int i = 0; i < 3; i++ )
    {
        XVariantClear( &fillTypeProperty.Choices[i] );
    }

    delete [] fillTypeProperty.Choices;
}
