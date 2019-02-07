/*
    Gamepad device plug-in for Computer Vision Sandbox

    Copyright (C) 2011-2019, cvsandbox
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
#include "GamepadPlugin.hpp"
#include "image_gamepad_16x16.h"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000018, 0x00000001 };

// Device ID property
static PropertyDescriptor deviceIdProperty =
{ XVT_U2, "Device Id", "deviceId", "Device ID to connect to (starts from 0 for the first device).", PropertyFlag_None };

// Device Name property
static PropertyDescriptor deviceNameProperty =
{ XVT_String, "Device Name", "deviceName", "Device name.", PropertyFlag_ReadOnly };

// Axes Count property
static PropertyDescriptor axesCountProperty =
{ XVT_U2, "Axes Count", "axesCount", "Number of available axes.", PropertyFlag_ReadOnly };

// Buttons Count property
static PropertyDescriptor buttonsCountProperty =
{ XVT_U2, "Buttons Count", "buttonsCount", "Number of available buttons.", PropertyFlag_ReadOnly };

// Axes Values property
static PropertyDescriptor axesValuesProperty =
{ XVT_R4 | XVT_Array, "Axes Values", "axesValues", "Current values of the available axes.", PropertyFlag_ReadOnly | PropertyFlag_Dynamic };

// Buttons State property
static PropertyDescriptor buttonsStateProperty =
{ XVT_Bool | XVT_Array, "Buttons State", "buttonsState", "Current state of the buttons.", PropertyFlag_ReadOnly | PropertyFlag_Dynamic };

// Point Of View property
static PropertyDescriptor pointOfViewProperty =
{ XVT_R4, "Point Of View", "pointOfView", "Current position of the point-of-view control.", PropertyFlag_ReadOnly | PropertyFlag_Dynamic };


// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &deviceIdProperty, &deviceNameProperty, &buttonsCountProperty, &axesCountProperty,
    &axesValuesProperty, &buttonsStateProperty, &pointOfViewProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** GamepadPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_Default,

    PluginType_Device,
    PluginVersion,
    "Gamepad",
    "Gamepad",
    "Plug-in to access gamepads and joysticks.",

    "The plug-in allows accessing game pads and joysticks connected to the systems. To establish connection with a "
    "game pads, it is required specify its ID, which normally starts from 0 for the first available device and increases "
    "for others. Once connection is done, the plug-in allows to query game pad's buttons' state, axes' positions and "
    "point of view control."
    ,

    &image_gamepad_16x16,
    nullptr,
    GamepadPlugin,

    XARRAY_SIZE( pluginProperties ),
    pluginProperties,
    PluginInitializer,
    nullptr, // no clean-up
    nullptr  // no dynamic properties update
);

// Complete properties description by initializing those parts, which were not
// initialized during properties array declaration
static void PluginInitializer( )
{
    // Device ID property
    deviceIdProperty.DefaultValue.type        = XVT_U2;
    deviceIdProperty.DefaultValue.value.usVal = 0;

    deviceIdProperty.MinValue.type        = XVT_U2;
    deviceIdProperty.MinValue.value.usVal = 0;

    deviceIdProperty.MaxValue.type        = XVT_U2;
    deviceIdProperty.MaxValue.value.usVal = 15;
}
