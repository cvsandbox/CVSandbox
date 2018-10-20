/*
    Image processing tools plug-ins of Computer Vision Sandbox

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
#include <image_move_towards_16x16.h>
#include "MoveTowardsImagesPlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000007, 0x00000009 };

// Step Size property
static PropertyDescriptor stepSizeProperty =
{ XVT_U1, "Step Size", "stepSize", "Specifies step size, which is maximum amount to move by.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &stepSizeProperty,
};

// Let the class itself know description of its properties
const PropertyDescriptor** MoveTowardsImagesPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_TwoImageFilters,

    PluginType_ImageProcessingFilter2,
    PluginVersion,
    "Move Towards Images",
    "MoveTowardsImages",
    "Moves specified image towards the target image by the specified amount.",

    "The plug-in updates the specified image in the way to decrease difference with the target image (second provided image) - "
    "image is moved towards the target image. The update equation is defined in the next way: "
    "<b>result = source + MIN( ABS( target - source ), step ) * SIGN( target - source )</b>."
    ,

    &image_move_towards_16x16,
    nullptr,
    MoveTowardsImagesPlugin,

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
    stepSizeProperty.DefaultValue.type = XVT_U1;
    stepSizeProperty.DefaultValue.value.ubVal = 1;

    stepSizeProperty.MinValue.type = XVT_U1;
    stepSizeProperty.MinValue.value.ubVal = 1;

    stepSizeProperty.MaxValue.type = XVT_U1;
    stepSizeProperty.MaxValue.value.ubVal = 255;
}
