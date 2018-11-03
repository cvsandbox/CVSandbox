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
#include "PowerInfoPlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x0000000A, 0x00000002 };

// Battery Charge property
static PropertyDescriptor batteryChargeProperty =
{ XVT_U1, "Battery charge", "batteryCharge", "The percentage of full battery charge remaining.", PropertyFlag_ReadOnly | PropertyFlag_Dynamic};
// Power Connected property
static PropertyDescriptor powerConnectedProperty =
{ XVT_Bool, "Power is connected", "powerIsConnected", "Tells if AC power supply is connected to the system or not.", PropertyFlag_ReadOnly | PropertyFlag_Dynamic };
// Batter Is Charging property
static PropertyDescriptor batteryIsChargingProperty =
{ XVT_Bool, "Battery is charging", "batteryIsCharging", "Tells if battery is now charging or not.", PropertyFlag_ReadOnly | PropertyFlag_Dynamic };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &batteryChargeProperty, &powerConnectedProperty, &batteryIsChargingProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** PowerInfoPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_Default,

    PluginType_Device,
    PluginVersion,
    "Power Info",
    "PowerInfo",
    "Provides information about power status of the system.",

    "The plug-in provides number of properties, which tell status of battery if it is available in the system - "
    "remaining charge level, power connection and battery charging indicators.",
    0,
    0,
    PowerInfoPlugin,

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
    batteryChargeProperty.MinValue.type = XVT_U1;
    batteryChargeProperty.MinValue.value.ubVal = 0;

    batteryChargeProperty.MaxValue.type = XVT_U1;
    batteryChargeProperty.MaxValue.value.ubVal = 100;
}
