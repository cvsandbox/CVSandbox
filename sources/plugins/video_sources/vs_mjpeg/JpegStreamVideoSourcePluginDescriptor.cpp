/*
    JPEG and MJPEG HTTP video source plug-ins of Computer Vision Sandbox

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
#include <image_jpeg_stream_16x16.h>
#include "JpegStreamVideoSourcePlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 1 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000005, 0x00000001 };

// Jpeg URL property
static PropertyDescriptor jpegUrlProperty =
{ XVT_String, "JPEG URL", "url", "URL string of JPEG image to download continuously.", PropertyFlag_None };
// Frame Interval property
static PropertyDescriptor frameIntervalProperty =
{ XVT_U2, "Frame interval", "frameInterval", "Time interval (in milliseconds) between querying video frames.", PropertyFlag_None };
// User name property
static PropertyDescriptor userNameProperty =
{ XVT_String, "User name", "userName", "User name used for authentication.", PropertyFlag_None };
// Password property
static PropertyDescriptor passwordProperty =
{ XVT_String, "Password", "password", "Password used for authentication.", PropertyFlag_None };
// Force basic authentication property
static PropertyDescriptor forceBasicAuthenticationProperty =
{ XVT_Bool, "Force basic authentication", "forceBasicAuthentication", "Force basic authentication or negotiate it.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &jpegUrlProperty, &frameIntervalProperty, &userNameProperty, &passwordProperty, &forceBasicAuthenticationProperty
};

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_VideoSource,

    PluginType_VideoSource,
    PluginVersion,
    "JPEG HTTP Video Source",
    "JpegStreamVideoSource",
    "Plug-in to continuously download JPEG images from a given HTTP URL.",

    /* Long description */
    "The plug-in allows to continuously download JPEG images from a given HTTP URL, which usually "
    "points not to an image dynamically updating over time. A classical example can be an IP video "
    "surveillance camera, which provides an URL to query video snapshots. For example, many AXIS "
    "cameras allow capturing video snapshots using URL format like this: "
    "\"<u>http://&lt;ip_or_dns&gt;[:port]/axis-cgi/jpg/image.cgi</u>\". Other IP cameras may provide similar "
    "type of snapshot URLs, which can be found from documentation provided by the camera's "
    "manufacturer.<br><br>"

    "The plug-in queries new video snapshots at the configured <b>frame interval</b>. If it is set to 100 "
    "milliseconds, for example, then the maximum video frame rate can be 10 frames per second. "
    "If frame interval is set to 0 however, then the plug-in will keep querying video snapshots "
    "without making any pause in-between.<br><br>"

    "If the video source (IP camera) is configured to require authentication, then <b>User name</b> and "
    "<b>Password</b> properties must be set. Otherwise those should be left blank. <b>Note</b>: some camera "
    "models fail negotiating authentication method and so <b>basic authentication</b>must be forced "
    "for those."
    ,
    &image_jpeg_stream_16x16,
    nullptr,
    JpegStreamVideoSourcePlugin,

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
    // Frame Interval
    frameIntervalProperty.DefaultValue.type         = XVT_U2;
    frameIntervalProperty.DefaultValue.value.usVal  = 100;

    frameIntervalProperty.MinValue.type             = XVT_U2;
    frameIntervalProperty.MinValue.value.usVal      = 0;

    frameIntervalProperty.MaxValue.type             = XVT_U2;
    frameIntervalProperty.MaxValue.value.usVal      = 60000;

    // Force Basic Authentication
    forceBasicAuthenticationProperty.DefaultValue.type          = XVT_Bool;
    forceBasicAuthenticationProperty.DefaultValue.value.boolVal = false;
}
