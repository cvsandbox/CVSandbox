/*
    Motion detection plug-ins for Computer Vision Sandbox

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
#include "TwoFramesDifferenceDetectionPlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000020, 0x00000001 };

// Dummy property
static PropertyDescriptor dummyProperty =
{ XVT_U4, "Dummy", "dummy", "Dummy property, todo.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &dummyProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** TwoFramesDifferenceDetectionPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_Default,

    PluginType_Detection,
    PluginVersion,
    "Two Frames Difference",
    "TwoFramesDifference",
    "Plug-in to detect motion in video stream based on two frames difference.",

    /* Long description */
    "???"
    ,
    nullptr,
    nullptr,
    TwoFramesDifferenceDetectionPlugin,

    sizeof( pluginProperties ) / sizeof( PropertyDescriptor* ),
    pluginProperties,
    PluginInitializer,
    nullptr,
    nullptr
);

// Complete properties description by initializing those parts, which were not
// initialized during properties array declaration
static void PluginInitializer( )
{
}
