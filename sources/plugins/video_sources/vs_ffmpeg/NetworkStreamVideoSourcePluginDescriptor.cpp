/*
    FFMPEG video source plug-ins of Computer Vision Sandbox

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
#include <image_network_stream_plugin_16x16.h>
#include "NetworkStreamVideoSourcePlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x0000000B, 0x00000001 };

// MJpeg URL property
static PropertyDescriptor urlProperty =
{ XVT_String, "Stream URL", "url", "URL of a network video stream.", PropertyFlag_None };
// User name property
static PropertyDescriptor userNameProperty =
{ XVT_String, "User name", "userName", "User name used for authentication.", PropertyFlag_None };
// Password property
static PropertyDescriptor passwordProperty =
{ XVT_String, "Password", "password", "Password used for authentication.", PropertyFlag_None };
// ProbSize property
static PropertyDescriptor probSizeProperty =
{ XVT_U4, "Probing size", "probSize", "Maximum size of the data (bytes) to read from network stream to determine format of the input container. Decrease it to reduce video start-up delay.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &urlProperty, &userNameProperty, &passwordProperty, &probSizeProperty
};

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_VideoSource,

    PluginType_VideoSource,
    PluginVersion,
    "Network Stream Video Source",
    "NetworkStreamVideoSource",
    "Plug-in to access network video streams (like RTSP) using FFmpeg library.",

    /* Long description */
    "The plug-in allows to access different network video streams like RTSP, which are provided by "
    "IP video surveillance cameras or other sources providing video streams over network. It utilizes "
    "FFmpeg library to access network video streams, so the full list of supported  protocols depends "
    "on the capabilities of the library. For now the plug-in is mainly targeted RTSP streams.<br><br>"

    "If the video source (IP camera) is configured to require authentication, then <b>User name</b> and "
    "<b>Password</b> properties must be set. Otherwise those should be left blank."
    ,
    &image_network_stream_plugin_16x16,
    nullptr,
    NetworkStreamVideoSourcePlugin,

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
    // probing size Interval
    probSizeProperty.DefaultValue.type = XVT_U4;
    probSizeProperty.DefaultValue.value.uiVal = 5000000;

    probSizeProperty.MinValue.type = XVT_U4;
    probSizeProperty.MinValue.value.uiVal = 32;

    probSizeProperty.MaxValue.type = XVT_U4;
    probSizeProperty.MaxValue.value.uiVal = 50000000;
}
