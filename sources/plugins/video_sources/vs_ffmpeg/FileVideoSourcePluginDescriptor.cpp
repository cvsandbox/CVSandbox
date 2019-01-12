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
#include <image_video_16x16.h>
#include "FileVideoSourcePlugin.hpp"

static void PluginInitializer( );
static XErrorCode UpdateFrameIntervalProperty( PropertyDescriptor* desc, const xvariant* parentValue );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 1 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x0000000B, 0x00000002 };

// File Name property
static PropertyDescriptor fileNameProperty =
{ XVT_String, "File Name", "fileName", "Video file to play.", PropertyFlag_PreferredEditor_FileBrowser };
// Override Frame Interval property
static PropertyDescriptor overrideFrameIntervalProperty =
{ XVT_Bool, "Override Frame Interval", "overrideFrameInterval", "Use custom frame interval or not.", PropertyFlag_None };
// Frame Interval property
static PropertyDescriptor frameIntervalProperty =
{ XVT_U2, "Frame Interval", "frameInterval", "Desired frame interval between video frames (ms).", PropertyFlag_Dependent };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &fileNameProperty, &overrideFrameIntervalProperty, &frameIntervalProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** FileVideoSourcePlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_VirtualVideoSource,

    PluginType_VideoSource,
    PluginVersion,
    "Video File",
    "VideoFile",
    "Plug-in to play video files using FFmpeg library.",

    /* Long description */
    "The plug-in plays the specified video file. No audio is provided, only video comes. By default it plays the video "
    "using the frame rate specified in the file. However the play speed can be overridden by specifying custom <b>frame "
    "interval</b>.<br><br>"

    "<b>Note</b>: when default frame rate is used, frames' presentation time is still ignored. As the result video may play faster, "
    "if it contains less frames per second than its FPS says, but they have presentation time associated to control "
    "playback. The plug-in is not aimed to provide video player functionality, but mostly to play previously saved videos for "
    "image processing and computer vision projects."
    ,
    &image_video_16x16,
    nullptr,
    FileVideoSourcePlugin,

    XARRAY_SIZE( pluginProperties ),
    pluginProperties,
    PluginInitializer,
    nullptr,
    nullptr
);

// Complete properties description by initializing those parts, which were not 
// initialized during properties array declaration
static void PluginInitializer( )
{
    overrideFrameIntervalProperty.DefaultValue.type = XVT_Bool;
    overrideFrameIntervalProperty.DefaultValue.value.boolVal = false;

    frameIntervalProperty.DefaultValue.type = XVT_U2;
    frameIntervalProperty.DefaultValue.value.usVal = 40;

    frameIntervalProperty.MinValue.type = XVT_U2;
    frameIntervalProperty.MinValue.value.usVal = 0;

    frameIntervalProperty.MaxValue.type = XVT_U2;
    frameIntervalProperty.MaxValue.value.usVal = 60000;

    frameIntervalProperty.ParentProperty = 1;
    frameIntervalProperty.Updater = UpdateFrameIntervalProperty;
}

static XErrorCode UpdateFrameIntervalProperty( PropertyDescriptor* desc, const xvariant* parentValue )
{
    XErrorCode ret = ErrorFailed;
    bool       override;

    ret = XVariantToBool( parentValue, &override );

    if ( ret == SuccessCode )
    {
        if ( override )
        {
            desc->Flags &= ( ~PropertyFlag_Disabled );
        }
        else
        {
            desc->Flags |= PropertyFlag_Disabled;
        }
    }

    return ret;
}
