/*
    System Info plug-ins of Computer Vision Sandbox

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
#include "PerformanceInfoPlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x0000000A, 0x00000003 };

// Processors Count property
static PropertyDescriptor processorsCountProperty =
{ XVT_U4, "Processors Count", "processorsCount", "Number of CPUs/cores in the system.", PropertyFlag_ReadOnly };
// System Load property
static PropertyDescriptor systemLoadProperty =
{ XVT_R4, "System Load", "systemLoad", "Total CPU(s) load, %.", PropertyFlag_ReadOnly | PropertyFlag_Dynamic };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &processorsCountProperty, &systemLoadProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** PerformanceInfoPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_Default,

    PluginType_Device,
    PluginVersion,
    "Performance Info",
    "PerformanceInfo",
    "Provides information about system performance.",

    "The plug-in provides system's performance information, like number of CPU/cores available in the system "
    "and total CPU load.",
    0,
    0,
    PerformanceInfoPlugin,

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
    systemLoadProperty.MinValue.type = XVT_R4;
    systemLoadProperty.MinValue.value.fVal = 0;

    systemLoadProperty.MaxValue.type = XVT_R4;
    systemLoadProperty.MaxValue.value.fVal = 100;
}
