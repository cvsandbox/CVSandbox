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
#include <image_pixellate_16x16.h>
#include "PixellatePlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x0000000C };

// Pixel Width property
static PropertyDescriptor pixelWidthProperty =
{ XVT_U1, "Pixel Width", "pixelWidth", "Width of the blocks to make.", PropertyFlag_None };
// Pixel Height property
static PropertyDescriptor pixelHeightProperty =
{ XVT_U1, "Pixel Height", "pixelHeight", "Height of the blocks to make.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &pixelWidthProperty, &pixelHeightProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** PixellatePlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_ImageEffect,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Pixellate",
    "Pixellate",
    "Performs pixellation of the specified image - creates effect of bigger pixels.",

    /* Long description */
    "The plug-in fills the image with rectangular blocks of the specified size, where each block's color is calculated as the "
    "average color of the pixels from the original image belonging to the block. The result image produces effect of a magnified "
    "lower resolution image."
    ,
    &image_pixellate_16x16,
    0,
    PixellatePlugin,

    sizeof( pluginProperties ) / sizeof( PropertyDescriptor* ),
    pluginProperties,
    PluginInitializer,
    0, // no clean-up
    0  // no dynamic properties update
);

// Complete properties description by initializing those parts, which were not 
// initialized during properties array declaration
static void PluginInitializer( )
{
    // Pixel Width property
    pixelWidthProperty.DefaultValue.type = XVT_U1;
    pixelWidthProperty.DefaultValue.value.ubVal = 8;

    pixelWidthProperty.MinValue.type = XVT_U1;
    pixelWidthProperty.MinValue.value.ubVal = 1;

    pixelWidthProperty.MaxValue.type = XVT_U1;
    pixelWidthProperty.MaxValue.value.ubVal = 128;

    // Pixel Height property
    pixelHeightProperty.DefaultValue.type = XVT_U1;
    pixelHeightProperty.DefaultValue.value.ubVal = 8;

    pixelHeightProperty.MinValue.type = XVT_U1;
    pixelHeightProperty.MinValue.value.ubVal = 1;

    pixelHeightProperty.MaxValue.type = XVT_U1;
    pixelHeightProperty.MaxValue.value.ubVal = 128;
}
