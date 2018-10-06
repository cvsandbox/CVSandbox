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
#include <image_mirror_16x16.h>
#include "MirrorPlugin.hpp"

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x0000000B };

// Xmirror property
static PropertyDescriptor xMirrorProperty =
{ XVT_Bool, "X Mirror", "xMirror", "Mirror or not around X axis.", PropertyFlag_None };

// Ymirror property
static PropertyDescriptor yMirrorProperty =
{ XVT_Bool, "Y Mirror", "yMirror", "Mirror or not around Y axis.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &xMirrorProperty, &yMirrorProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** MirrorPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_Transformation,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Mirror Image",
    "MirrorImage",
    "Mirrors image around X and/or Y axis.",

    /* Long description */
    "The plug-in performs image mirroring around X axis, which is a horizontal mirror turning image up side "
    "down, or Y axis - a vertical mirror changing left/right direction in the image."
    ,
    &image_mirror_16x16,
    0, 
    MirrorPlugin,

    sizeof( pluginProperties ) / sizeof( PropertyDescriptor* ),
    pluginProperties,
    0, // no initializer
    0, // no clean-up
    0  // no dynamic properties update
);
