/*
    JPEG images handling plug-ins of Computer Vision Sandbox

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
#include "JpegExporterPlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000002, 0x00000002 };

// Quality property
static PropertyDescriptor qualityProperty =
{ XVT_U1, "Quality", "quality", "JPEG saving quality.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &qualityProperty,
};

// Let the class itself know description of its properties
const PropertyDescriptor** JpegExporterPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_Default,

    PluginType_ImageExporter,
    PluginVersion,
    "JPEG Exporter",
    "JpegExporter",
    "JPEG files saving plug-in.",

    "The plug-in allows saving images in JPEG format.",
    0,
    0,
    JpegExporterPlugin,

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
    qualityProperty.DefaultValue.type        = XVT_U1;
    qualityProperty.DefaultValue.value.usVal = 85;

    qualityProperty.MinValue.type        = XVT_U1;
    qualityProperty.MinValue.value.usVal = 0;

    qualityProperty.MaxValue.type        = XVT_U1;
    qualityProperty.MaxValue.value.usVal = 100;
}
