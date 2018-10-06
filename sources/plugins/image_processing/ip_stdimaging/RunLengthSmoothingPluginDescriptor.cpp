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
#include <image_run_length_smoothing_16x16.h>
#include "RunLengthSmoothingPlugin.hpp"

static void PluginInitializer( );
static void PluginCleaner( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x00000037 };

// Type property
static PropertyDescriptor gapTypeProperty =
{ XVT_U1, "Gap Type", "gapType", "Gap type to fill.", PropertyFlag_SelectionByIndex };

// Maximum Gap property
static PropertyDescriptor maxGapProperty =
{ XVT_U2, "Maximum Gap", "maxGap", "Maximum size of gaps to to fill.", PropertyFlag_None };


// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &gapTypeProperty, &maxGapProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** RunLengthSmoothingPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_Morphology,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Run Length Smoothing",
    "RunLengthSmoothing",
    "Fill horizontal/vertical gaps between objects.",

    /* Long description */
    "The plug-in implements run length smoothing algorithm, which fills horizontal/vertical gaps between objects (non-black pixels). "
    "All gaps smaller or equal to the specified limit are filled to connect objects or their parts."
    ,
    &image_run_length_smoothing_16x16,
    nullptr,
    RunLengthSmoothingPlugin,

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
    gapTypeProperty.DefaultValue.type = XVT_U1;
    gapTypeProperty.DefaultValue.value.ubVal = 0;

    gapTypeProperty.MinValue.type = XVT_U1;
    gapTypeProperty.MinValue.value.ubVal = 0;

    gapTypeProperty.MaxValue.type = XVT_U1;
    gapTypeProperty.MaxValue.value.ubVal = 1;

    gapTypeProperty.ChoicesCount = 2;
    gapTypeProperty.Choices = new xvariant[2];

    gapTypeProperty.Choices[0].type = XVT_String;
    gapTypeProperty.Choices[0].value.strVal = XStringAlloc( "Horizontal" );

    gapTypeProperty.Choices[1].type = XVT_String;
    gapTypeProperty.Choices[1].value.strVal = XStringAlloc( "Vertical" );

    // Maximum Gap property
    maxGapProperty.DefaultValue.type = XVT_U2;
    maxGapProperty.DefaultValue.value.usVal = 10;

    maxGapProperty.MinValue.type = XVT_U2;
    maxGapProperty.MinValue.value.usVal = 1;

    maxGapProperty.MaxValue.type = XVT_U2;
    maxGapProperty.MaxValue.value.usVal = 1000;
}

// Clean-up plug-in - deallocate strings
static void PluginCleaner( )
{
    for ( int i = 0; i < gapTypeProperty.ChoicesCount; i++ )
    {
        XVariantClear( &gapTypeProperty.Choices[i] );
    }

    delete [] gapTypeProperty.Choices;
}
