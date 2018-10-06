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
#include <image_edge_detector_plugin_16x16.h>
#include "EdgeDetectorPlugin.hpp"

static void PluginInitializer( );
static void PluginCleaner( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 1 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x00000013 };

// Detector type property
static PropertyDescriptor detecterTypeProperty =
{ XVT_U1, "Detector type", "type", "Type of edge detector to use.", PropertyFlag_SelectionByIndex };

// Scale intensity property
static PropertyDescriptor scaleIntensityProperty =
{ XVT_Bool, "Scale intensity", "scale", "Scale intensities in [0, 255] range or not.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &detecterTypeProperty, &scaleIntensityProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** EdgeDetectorPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_EdgeDetector,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Edge Detector",
    "EdgeDetector",
    "Performs one of the specified edge detection routines",

    /* Long description */
    "The plug-in provides different simple edge detection techniques. For a pixel <b>x</b> and 8 of its neighbours, "
    "the corresponding pixel of destination image is calculated using one of the possible equations.<br><br><tt>"
    "P1 P2 P3<br>"
    "P8&nbsp;&nbsp;X P4<br>"
    "P7 P6 P5</tt><br><br>"
    "<b>Difference</b> detector: max( |P1-P5|, |P2-P6|, |P3-P7|, |P4-P8| );<br>"
    "<b>Homogeneity</b> detector: max( |x-P1|, |x-P2|, |x-P3|, |x-P4|, |x-P5|, |x-P6|, |x-P7|, |x-P8| );<br>"
    "<b>Sobel</b> detector: |P1 + 2P2 + P3 - P7 - 2P6 - P5| + |P3 + 2P4 + P5 - P1 - 2P8 - P7|.<br><br>"

    "Since the above calculations can not be done for edge pixels, those are set value 0.<br><br>"

    "If <b>scale intensity</b> is enabled, then result edge image pixel values are scaled, so they "
    "spread in the [0, 255] range."
    ,
    &image_edge_detector_plugin_16x16,
    0,
    EdgeDetectorPlugin,

    XARRAY_SIZE( pluginProperties ),
    pluginProperties,
    PluginInitializer,
    PluginCleaner,
    nullptr  // no dynamic properties update
);

// Complete properties description by initializing those parts, which were not 
// initialized during properties array declaration
static void PluginInitializer( )
{
    static const char* detectorNames[] = { "Difference", "Homogeneity", "Sobel" };

    detecterTypeProperty.DefaultValue.type = XVT_U1;
    detecterTypeProperty.DefaultValue.value.ubVal = 0;

    detecterTypeProperty.MinValue.type = XVT_U1;
    detecterTypeProperty.MinValue.value.ubVal = 0;

    detecterTypeProperty.MaxValue.type = XVT_U1;
    detecterTypeProperty.MaxValue.value.ubVal = XARRAY_SIZE( detectorNames ) - 1;

    detecterTypeProperty.ChoicesCount = XARRAY_SIZE( detectorNames );
    detecterTypeProperty.Choices = new xvariant[detecterTypeProperty.ChoicesCount];

    for ( int i = 0; i < detecterTypeProperty.ChoicesCount; i++ )
    {
        detecterTypeProperty.Choices[i].type = XVT_String;
        detecterTypeProperty.Choices[i].value.strVal = XStringAlloc( detectorNames[i] );
    }

    scaleIntensityProperty.DefaultValue.type = XVT_Bool;
    scaleIntensityProperty.DefaultValue.value.boolVal = false;
}

// Clean-up plug-in - deallocate strings
static void PluginCleaner( )
{
    for ( int i = 0; i < detecterTypeProperty.ChoicesCount; i++ )
    {
        XVariantClear( &detecterTypeProperty.Choices[i] );
    }

    delete[] detecterTypeProperty.Choices;
}
