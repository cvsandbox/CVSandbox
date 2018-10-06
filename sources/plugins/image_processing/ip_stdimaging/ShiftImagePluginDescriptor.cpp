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
#include <image_shift_image_16x16.h>
#include "ShiftImagePlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x00000023 };

// xShift property
static PropertyDescriptor xShiftProperty =
{ XVT_I4, "X Shift", "xShift", "Shift amount in X direction.", PropertyFlag_None };

// yShift property
static PropertyDescriptor yShiftProperty =
{ XVT_I4, "Y Shift", "yShift", "Shift amount in Y direction.", PropertyFlag_None };

// fillOpenSpace property
static PropertyDescriptor fillOpenSpaceProperty =
{ XVT_Bool, "Fill Open Space", "fillOpenSpace", "Fill or not open space uncovered by a shifted image.", PropertyFlag_None };

// fillColor property
static PropertyDescriptor fillColorProperty =
{ XVT_ARGB, "Fill Color", "fillColor", "Color used to fill open space.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &xShiftProperty, &yShiftProperty, &fillOpenSpaceProperty, &fillColorProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** ShiftImagePlugin::propertiesDescription = ( const PropertyDescriptor** ) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_Transformation,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Shift Image",
    "ShiftImage",
    "Shifts image by the specified number of pixels.",

    /* Long description */
    "The plug-in shifts image in X/Y direction by the specified number of pixels. The uncovered open space is filled with the "
    "specified color or left as is."
    ,
    &image_shift_image_16x16,
    0,
    ShiftImagePlugin,

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
    // xShift property
    xShiftProperty.DefaultValue.type = XVT_I4;
    xShiftProperty.DefaultValue.value.iVal = 0;

    xShiftProperty.MinValue.type = XVT_I4;
    xShiftProperty.MinValue.value.iVal = -10000;

    xShiftProperty.MaxValue.type = XVT_I4;
    xShiftProperty.MaxValue.value.iVal = 10000;

    // yShift property
    yShiftProperty.DefaultValue.type = XVT_I4;
    yShiftProperty.DefaultValue.value.iVal = 0;

    yShiftProperty.MinValue.type = XVT_I4;
    yShiftProperty.MinValue.value.iVal = -10000;

    yShiftProperty.MaxValue.type = XVT_I4;
    yShiftProperty.MaxValue.value.iVal = 10000;

    // fill or not
    fillOpenSpaceProperty.DefaultValue.type = XVT_Bool;
    fillOpenSpaceProperty.DefaultValue.value.boolVal = true;

    // fill color
    fillColorProperty.DefaultValue.type = XVT_ARGB;
    fillColorProperty.DefaultValue.value.argbVal.argb = 0xFF000000;
}
