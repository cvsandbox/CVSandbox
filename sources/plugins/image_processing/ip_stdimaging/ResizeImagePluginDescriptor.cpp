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
#include <image_resize_image_16x16.h>
#include "ResizeImagePlugin.hpp"

static void PluginInitializer( );
static void PluginCleaner( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x00000015 };

// New width property
static PropertyDescriptor newWidthProperty =
{ XVT_I4, "Width", "width", "New image width to resize to.", PropertyFlag_None };

// New height property
static PropertyDescriptor newHeightProperty =
{ XVT_I4, "Height", "height", "New image height to resize to.", PropertyFlag_None };

// Interpolation property
static PropertyDescriptor interpolationProperty =
{ XVT_U1, "Interpolation", "interpolation", "Interpolation algorithm to use.", PropertyFlag_SelectionByIndex };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &newWidthProperty, &newHeightProperty, &interpolationProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** ResizeImagePlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_Transformation,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Resize Image",
    "ResizeImage",
    "Resizes image to the specified size.",

    /* Long description */
    "The plug-in resizes images to the specified size using one of the supported interpolation algorithms."
    ,
    &image_resize_image_16x16,
    0, 
    ResizeImagePlugin,

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
    // New Width property
    newWidthProperty.DefaultValue.type = XVT_I4;
    newWidthProperty.DefaultValue.value.iVal = 640;

    newWidthProperty.MinValue.type = XVT_I4;
    newWidthProperty.MinValue.value.iVal = 1;

    newWidthProperty.MaxValue.type = XVT_I4;
    newWidthProperty.MaxValue.value.iVal = 10000;

    // New Height property
    newHeightProperty.DefaultValue.type = XVT_I4;
    newHeightProperty.DefaultValue.value.iVal = 480;

    newHeightProperty.MinValue.type = XVT_I4;
    newHeightProperty.MinValue.value.iVal = 1;

    newHeightProperty.MaxValue.type = XVT_I4;
    newHeightProperty.MaxValue.value.iVal = 10000;

    // Interpolation property
    interpolationProperty.DefaultValue.type = XVT_U1;
    interpolationProperty.DefaultValue.value.ubVal = 0;

    interpolationProperty.ChoicesCount = 2;
    interpolationProperty.Choices = new xvariant[2];

    interpolationProperty.Choices[0].type = XVT_String;
    interpolationProperty.Choices[0].value.strVal = XStringAlloc( "Nearest Neighbor" );

    interpolationProperty.Choices[1].type = XVT_String;
    interpolationProperty.Choices[1].value.strVal = XStringAlloc( "Bilinear" );

    interpolationProperty.MaxValue.type = XVT_U1;
    interpolationProperty.MaxValue.value.ubVal = (uint8_t)( interpolationProperty.ChoicesCount - 1 );
}

// Clean-up plug-in - deallocate strings
static void PluginCleaner( )
{
    for ( int i = 0; i < interpolationProperty.ChoicesCount; i++ )
    {
        XVariantClear( &interpolationProperty.Choices[i] );
    }

    delete [] interpolationProperty.Choices;
}
