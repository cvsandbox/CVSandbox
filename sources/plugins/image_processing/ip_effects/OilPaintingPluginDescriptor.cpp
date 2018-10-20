/*
    Image processing effects plug-ins of Computer Vision Sandbox

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
#include "OilPaintingPlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000006, 0x0000000F };

// Radius property
static PropertyDescriptor radiusProperty =
{ XVT_U1, "Radius", "radius", "Oil painting radius.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &radiusProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** OilPaintingPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

/*
// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_ImageEffect,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Oil Painting",
    "OilPainting",
    "Applies simple oil painting effect to the selected image.",
    "Some help about this routine.",
    0, //&image_erosion_3s_16x16,
    0,
    OilPaintingPlugin,

    sizeof( pluginProperties ) / sizeof( PropertyDescriptor* ),
    pluginProperties,
    PluginInitializer,
	0,
    0  // no dynamic properties update
);
*/

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
}
