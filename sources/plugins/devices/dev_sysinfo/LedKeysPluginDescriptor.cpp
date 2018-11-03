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
#include "LedKeysPlugin.hpp"

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 1 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x0000000A, 0x00000001 };

// NUM lock property
static PropertyDescriptor numLockProperty =
{ XVT_Bool, "Num lock", "numLock", "Num Lock key status.", PropertyFlag_ReadOnly | PropertyFlag_Dynamic };

// CAPS lock property
static PropertyDescriptor capsLockProperty =
{ XVT_Bool, "Caps lock", "capsLock", "Caps Lock key status.", PropertyFlag_ReadOnly | PropertyFlag_Dynamic };

// SCROLL lock property
static PropertyDescriptor scrollLockProperty =
{ XVT_Bool, "Scroll lock", "scrollLock", "Scroll Lock key status.", PropertyFlag_ReadOnly | PropertyFlag_Dynamic };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &numLockProperty, &capsLockProperty, &scrollLockProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** LedKeysPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_Default,

    PluginType_Device,
    PluginVersion,
    "LED Keys",
    "LedKeys",
    "Allows to get status of LED keys (Num/Caps/Scroll lock).",

    "The plug-in allows to query status of the system \"LED keys\". When reading its properties it tells if the "
    "corresponding LED key is On or Off.",
    0,
    0,
    LedKeysPlugin,

    sizeof( pluginProperties ) / sizeof( PropertyDescriptor* ),
    pluginProperties,
    0,
    0, // no clean-up
    0  // no dynamic properties update
);
