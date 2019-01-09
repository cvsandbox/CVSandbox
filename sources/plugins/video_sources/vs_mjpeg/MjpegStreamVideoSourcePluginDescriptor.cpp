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
#include <image_mjpeg_stream_16x16.h>
#include "MjpegStreamVideoSourcePlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 1 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000005, 0x00000002 };

// MJpeg URL property
static PropertyDescriptor jpegUrlProperty =
{ XVT_String, "MJPEG URL", "url", "URL string of MJPEG stream to download.", PropertyFlag_None };
// User name property
static PropertyDescriptor userNameProperty =
{ XVT_String, "User name", "userName", "User name used for authentication.", PropertyFlag_None };
// Password property
static PropertyDescriptor passwordProperty =
{ XVT_String, "Password", "password", "Password used for authentication.", PropertyFlag_None };
// Force basic authentication property
static PropertyDescriptor forceBasicAuthenticationProperty =
{ XVT_Bool, "Force Basic Authentication", "forceBasicAuthentication", "Force basic authentication or negotiate it.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &jpegUrlProperty, &userNameProperty, &passwordProperty, &forceBasicAuthenticationProperty
};

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_VideoSource,

    PluginType_VideoSource,
    PluginVersion,
    "MJPEG HTTP Video Source",
    "MjpegStreamVideoSource",
    "Plug-in to download MJPEG stream from a given HTTP URL.",

    /* Long description */
    "The plug-in allows to download JPEG images from a given HTTP URL of a MJPEG stream - an endless stream "
    "of JPEG images coming one after another separated with certain delimiter. A classical example can be "
    "an IP video surveillance camera, which provides an URL to query video stream in a form of MJPEG stream. "
    "For example, many AXIS cameras allow capturing MJPEG video streams using URL format like this: "
    "\"<u>http://&lt;ip_or_dns&gt;[:port]/axis-cgi/mjpg/video.cgi</u>\". Other IP cameras may provide similar type "
    "of MJPEG stream URLs, which can be found from documentation provided by the camera's manufacturer.<br><br>"

    "If the video source (IP camera) is configured to require authentication, then <b>User name</b> and "
    "<b>Password</b> properties must be set. Otherwise those should be left blank. <b>Note</b>: some camera "
    "models fail negotiating authentication method and so <b>basic authentication</b>must be forced "
    "for those."
    ,
    &image_mjpeg_stream_16x16,
    nullptr,
    MjpegStreamVideoSourcePlugin,

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
    // Force Basic Authentication
    forceBasicAuthenticationProperty.DefaultValue.type          = XVT_Bool;
    forceBasicAuthenticationProperty.DefaultValue.value.boolVal = false;
}
