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
#include "VideoRepeaterPlugin.hpp"
#include <image_video_repeater_plugin_16x16.h>

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000010, 0x00000001 };

// Repeater ID property
static PropertyDescriptor repeaterIdProperty =
{ XVT_String, "Repeater ID", "id", "ID to associate with the repeater (used by push plug-in).", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &repeaterIdProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** VideoRepeaterPlugin::propertiesDescription = ( const PropertyDescriptor** ) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_VirtualVideoSource,

    PluginType_VideoSource,
    PluginVersion,
    "Video Repeater",
    "VideoRepeater",
    "Video repeater plug-in, which replays video pushed into it.",

    "This plug-in is aimed to provide a video source, which is comprised from images pushed into it. On its own this "
    "plug-in does nothing - it does not generate any images. All it does is waiting until an image is pushed into "
    "it using <a href='{AF000003-00000000-00000010-00000002}'>Video Repeater Push</a> plug-in. Once an image is pushed, "
    "the video source will generate a new frame event with a copy of the pushed image. Using the concept of video "
    "repeaters it is possible to re-translate an arbitrary video source into multiple repeaters, where each may have its "
    "own video processing graph.<br><br>"

    "<b>Note:</b> in order for this plug-in to accept images from a push plug-in, both must be configured with the same "
    "<b>Repeater ID</b>.<br><br>"

    "More information : <a href = 'http://www.cvsandbox.com/cvsandbox/tutorials/video_repeaters/'>Video Repeaters</a>"
    ,
    &image_video_repeater_plugin_16x16,
    0,
    VideoRepeaterPlugin,

    sizeof( pluginProperties ) / sizeof( PropertyDescriptor* ),
    pluginProperties,
    0,
    0,
    0
);
