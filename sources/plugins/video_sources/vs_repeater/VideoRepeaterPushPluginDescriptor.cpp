/*
    Video repeater plug-ins of Computer Vision Sandbox

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
#include "VideoRepeaterPushPlugin.hpp"
#include <image_video_repeater_push_plugin_16x16.h>

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000010, 0x00000002 };

// Repeater ID property
static PropertyDescriptor repeaterIdProperty =
{ XVT_String, "Repeater ID", "id", "Repeater ID to push video into.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &repeaterIdProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** VideoRepeaterPushPlugin::propertiesDescription = ( const PropertyDescriptor** ) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_VideoProcessing,

    PluginType_VideoProcessing,
    PluginVersion,
    "Video Repeater Push",
    "VideoRepeaterPush",
    "Plug-in to push video into corresponding video repeater.",

    "This plug-in is used to push images into <a href='{AF000003-00000000-00000010-00000001}'>Video Repeater</a> "
    "with specified <b>Repeater ID</b>. Once image is pushed into repeater, it will generate a new video frame as "
    "any other video source would do.<br><br>"
    
    "More information : <a href = 'http://www.cvsandbox.com/cvsandbox/tutorials/video_repeaters/'>Video Repeaters</a>"
    ,
    &image_video_repeater_push_plugin_16x16,
    0,
    VideoRepeaterPushPlugin,

    sizeof( pluginProperties ) / sizeof( PropertyDescriptor* ),
    pluginProperties,
    0,
    0,
0
);
