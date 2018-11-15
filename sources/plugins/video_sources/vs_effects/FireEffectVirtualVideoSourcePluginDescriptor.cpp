/*
    Virtual video source plug-ins of Computer Vision Sandbox

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
#include "FireEffectVirtualVideoSourcePlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000009, 0x00000001 };

// Frame Width property
static PropertyDescriptor frameWidthProperty =
{ XVT_U2, "Frame Width", "frameWidth", "Width of video frames to generate.", PropertyFlag_None };
// Frame Height property
static PropertyDescriptor frameHeightProperty =
{ XVT_U2, "Frame Height", "frameHeight", "Height of video frames to generate.", PropertyFlag_None };
// Frame Rate property
static PropertyDescriptor frameRateProperty =
{ XVT_U2, "Frame Rate", "frameRate", "Video frame rate to generate.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &frameWidthProperty, &frameHeightProperty, &frameRateProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** FireEffectVirtualVideoSourcePlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_VirtualVideoSource,

    PluginType_VideoSource,
    PluginVersion,
    "Fire Effect Virtual Video Source",
    "FireEffectVirtualVideoSource",
    "Plug-in providing virtual video source with fire effect.",
    "Some help about this routine.",
    0, //&image_jpeg_stream_16x16,
    0,
    FireEffectVirtualVideoSourcePlugin,

    sizeof( pluginProperties ) / sizeof( PropertyDescriptor* ),
    pluginProperties,
    PluginInitializer,
    0,
    0
);

// Complete properties description by initializing those parts, which were not
// initialized during properties array declaration
static void PluginInitializer( )
{
    // Frame Width
    frameWidthProperty.DefaultValue.type         = XVT_U2;
    frameWidthProperty.DefaultValue.value.usVal  = 320;

    frameWidthProperty.MinValue.type             = XVT_U2;
    frameWidthProperty.MinValue.value.usVal      = 100;

    frameWidthProperty.MaxValue.type             = XVT_U2;
    frameWidthProperty.MaxValue.value.usVal      = 1024;

    // Frame Height
    frameHeightProperty.DefaultValue.type        = XVT_U2;
    frameHeightProperty.DefaultValue.value.usVal = 240;

    frameHeightProperty.MinValue.type            = XVT_U2;
    frameHeightProperty.MinValue.value.usVal     = 100;

    frameHeightProperty.MaxValue.type            = XVT_U2;
    frameHeightProperty.MaxValue.value.usVal     = 768;

    // Frame Rate
    frameRateProperty.DefaultValue.type          = XVT_U2;
    frameRateProperty.DefaultValue.value.usVal   = 15;

    frameRateProperty.MinValue.type              = XVT_U2;
    frameRateProperty.MinValue.value.usVal       = 1;

    frameRateProperty.MaxValue.type              = XVT_U2;
    frameRateProperty.MaxValue.value.usVal       = 300;
}
