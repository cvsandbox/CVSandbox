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
#include <image_jitter_16x16.h>
#include "JitterPlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000006, 0x00000004 };

// Radius property
static PropertyDescriptor radiusProperty =
{ XVT_U1, "Radius", "radius", "Jittering radius to use for pixels' displacement.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &radiusProperty,
};

// Let the class itself know description of its properties
const PropertyDescriptor** JitterPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_ImageEffect,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Jitter",
    "Jitter",
    "Performs jittering of the image by displacing pixels randomly.",

    /* Long description */
    "The plug-in does pixel's jittering - it displaces every pixel randomly within the specified <b>radius</b>."
    ,
    &image_jitter_16x16,
    0,
    JitterPlugin,

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
    radiusProperty.DefaultValue.type = XVT_U1;
    radiusProperty.DefaultValue.value.ubVal = 3;

    radiusProperty.MinValue.type = XVT_U1;
    radiusProperty.MinValue.value.ubVal = 0;

    radiusProperty.MaxValue.type = XVT_U1;
    radiusProperty.MaxValue.value.ubVal = 100;
}
