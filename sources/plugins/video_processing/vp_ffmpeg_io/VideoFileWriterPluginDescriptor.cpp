    /*
    FFMPEG video writing plug-ins of Computer Vision Sandbox

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
#include <image_video_16x16.h>
#include "VideoFileWriterPlugin.hpp"

static void PluginInitializer( );
static void PluginCleaner( );
static XErrorCode UpdateAddTimeStampProperty( PropertyDescriptor* desc, const xvariant* parentValue );
static XErrorCode UpdateVideoSplitDependentProperties( PropertyDescriptor* desc, const xvariant* parentValue );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 2 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x0000000C, 0x00000001 };

// Destination Folder property
static PropertyDescriptor destinationFolderProperty =
{ XVT_String, "Destination Folder", "folder", "Folder to write video files to.", PropertyFlag_PreferredEditor_FolderBrowser };
// File Name property
static PropertyDescriptor fileNameProperty =
{ XVT_String, "File Name", "fileName", "Base file name of the video files.", PropertyFlag_None };
// Codec property
static PropertyDescriptor codecProperty =
{ XVT_U1, "Codec", "codec", "Codec to use for compression.", PropertyFlag_SelectionByIndex };
// Bit Rate property
static PropertyDescriptor bitRateProperty =
{ XVT_U2, "Bit Rate", "bitRate", "Bit rate to use for compression (kbps).", PropertyFlag_None };
// Frame Rate property
static PropertyDescriptor frameRateProperty =
{ XVT_U1, "Frame Rate", "frameRate", "Frame rate of the output video files.", PropertyFlag_None };
// Sync Presentation Time property
static PropertyDescriptor syncPresentationTimeProperty =
{ XVT_Bool, "Sync Presentation Time", "syncPresentationTime", "Synchronize or not frames presentation time with video source.", PropertyFlag_None };
// Add Time Stamp property
static PropertyDescriptor addTimeStampProperty =
{ XVT_Bool, "Add Time Stamp", "addTimeStamp", "Specifies if time stamp should be added to video files' names.", PropertyFlag_Dependent };
// Split Video Files property
static PropertyDescriptor splitVideoFilesProperty =
{ XVT_Bool, "Split Video Files", "splitVideoFiles", "Specifies if multiple files should be created of certain length.", PropertyFlag_None };
// Fragment Lenght property
static PropertyDescriptor fragmentLengthProperty =
{ XVT_U1, "Fragment Length", "fragmentLength", "Video fragment length (in minutes) when splitting video into multiple files.", PropertyFlag_Dependent };
// Remove Old Files property
static PropertyDescriptor removeOldFilesProperty =
{ XVT_Bool, "Remove Old Files", "removeOldFiles", "Specifies if old files should be removed or not.", PropertyFlag_Dependent };
// Directory Size Limit property
static PropertyDescriptor directorySizeLimitProperty =
{ XVT_U4, "Directory Size Limit", "directorySizeLimit", "Directory size limit (Mb) after which old files will get deleted to free up space for new files.", PropertyFlag_Dependent };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &destinationFolderProperty, &fileNameProperty, &codecProperty, &bitRateProperty, &frameRateProperty, &syncPresentationTimeProperty,
    &addTimeStampProperty, &splitVideoFilesProperty, &fragmentLengthProperty, &removeOldFilesProperty, &directorySizeLimitProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** VideoFileWriterPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_VideoProcessing,

    PluginType_VideoProcessing,
    PluginVersion,
    "Video File Writer",
    "VideoFileWriter",
    "Plug-in to write video files using FFmpeg library.",

    /* Long description */
    "The plug-in can be used for writing video frames into single video file at the specified destination folder "
    "or for writing multiple video files (splitting video into chunks). While writing single files it is possible to "
    "specify if file names should contain date/time stamp. If time stamp is not added, then existing file with the same "
    "name is always overwritten. Adding time stamp however will result in creating new files every time.<br><br>"

    "When plug-in is configured to split video files, it will create those with the specified length in minutes. "
    "<b>Note:</b> the actual duration of the result video files may end up being different in the case if video "
    "source frame rate is different from the configured frame rate of the video files. If video source's frame "
    "rate is lower than video file's frame rate, then playing the video file will make it look like everything "
    "was moving faster than it was. To avoid that it is required to enable <b>Sync presentation time</b> option, "
    "so presentation time of video frames will be kept in sync (if possible) with the time each frame was "
    "provided by a video source.<br><br>"

    "Enabling video splitting will allow creating video archive of the specified size. In this case the plug-in will "
    "monitor the size of the destination folder. If folder size reaches the specified limit, the plug-in will delete "
    "old files to free up space for new video files. <b>Note:</b> in the case if video archive should be made "
    "available for multiple video sources/cameras, it is required to write them into separate folders since the plug-in "
    "checks the total size of the destination folder (not just the size of files it has written since it started to run)."
    ,
    &image_video_16x16,
    0,
    VideoFileWriterPlugin,

    sizeof( pluginProperties ) / sizeof( PropertyDescriptor* ),
    pluginProperties,
    PluginInitializer,
    PluginCleaner,
    0
);

// Complete properties description by initializing those parts, which were not
// initialized during properties array declaration
static void PluginInitializer( )
{
    static const char* codecName[] =
    {
        "MPEG-4 Video", "Windows Media Video 7", "Windows Media Video 8", "MS MPEG-4 Video v2",
        "MS MPEG-4 Video v3", "H.263", "Flash Video", "MPEG-1/2 Video"
    };

    // configure Bit Rate property
    bitRateProperty.DefaultValue.type = XVT_U2;
    bitRateProperty.DefaultValue.value.usVal = 10000;

    bitRateProperty.MinValue.type = XVT_U2;
    bitRateProperty.MinValue.value.usVal = 50;

    // configure Frame Rate property
    frameRateProperty.DefaultValue.type = XVT_U1;
    frameRateProperty.DefaultValue.value.ubVal = 30;

    frameRateProperty.MinValue.type = XVT_U1;
    frameRateProperty.MinValue.value.ubVal = 1;

    frameRateProperty.MaxValue.type = XVT_U1;
    frameRateProperty.MaxValue.value.ubVal = 60;

    // configure Codec property
    codecProperty.ChoicesCount = XARRAY_SIZE( codecName );
    codecProperty.Choices      = new xvariant[codecProperty.ChoicesCount];

    for ( int i = 0; i < codecProperty.ChoicesCount; i++ )
    {
        codecProperty.Choices[i].type         = XVT_String;
        codecProperty.Choices[i].value.strVal = XStringAlloc( codecName[i] );
    }

    codecProperty.DefaultValue.type = XVT_U1;
    codecProperty.DefaultValue.value.ubVal = 0;

    codecProperty.MinValue.type = XVT_U1;
    codecProperty.MinValue.value.ubVal = 0;

    codecProperty.MaxValue.type = XVT_U1;
    codecProperty.MaxValue.value.ubVal = static_cast<uint8_t>( codecProperty.ChoicesCount - 1 );

    // add Time Stamp property
    addTimeStampProperty.ParentProperty = 7;
    addTimeStampProperty.Updater = UpdateAddTimeStampProperty;

    // fragment Length property
    fragmentLengthProperty.DefaultValue.type = XVT_U1;
    fragmentLengthProperty.DefaultValue.value.ubVal = 60;

    fragmentLengthProperty.MinValue.type = XVT_U1;
    fragmentLengthProperty.MinValue.value.ubVal = 1;

    fragmentLengthProperty.MaxValue.type = XVT_U1;
    fragmentLengthProperty.MaxValue.value.ubVal = 120;

    fragmentLengthProperty.ParentProperty = 7;
    fragmentLengthProperty.Updater = UpdateVideoSplitDependentProperties;

    // remove Old Files Property
    removeOldFilesProperty.ParentProperty = 7;
    removeOldFilesProperty.Updater = UpdateVideoSplitDependentProperties;

    // directory Size Limit Property
    directorySizeLimitProperty.DefaultValue.type = XVT_U4;
    directorySizeLimitProperty.DefaultValue.value.uiVal = 10000;

    directorySizeLimitProperty.MinValue.type = XVT_U4;
    directorySizeLimitProperty.MinValue.value.uiVal = 100;

    directorySizeLimitProperty.ParentProperty = 7;
    directorySizeLimitProperty.Updater = UpdateVideoSplitDependentProperties;
}

// Clean-up plug-in - deallocate strings
static void PluginCleaner( )
{
    for ( int i = 0; i < codecProperty.ChoicesCount; i++ )
    {
        XVariantClear( &codecProperty.Choices[i] );
    }

    delete[] codecProperty.Choices;
}

XErrorCode UpdateAddTimeStampProperty( PropertyDescriptor* desc, const xvariant* parentValue )
{
    XErrorCode ret = ErrorFailed;
    bool       boolParentValue;

    ret = XVariantToBool( parentValue, &boolParentValue );

    if ( ret == SuccessCode )
    {
        if ( boolParentValue )
        {
            desc->Flags |= PropertyFlag_Disabled;
        }
        else
        {
            desc->Flags &= ( ~PropertyFlag_Disabled );
        }
    }

    return ret;
}

XErrorCode UpdateVideoSplitDependentProperties( PropertyDescriptor* desc, const xvariant* parentValue )
{
    XErrorCode ret = ErrorFailed;
    bool       boolParentValue;

    ret = XVariantToBool( parentValue, &boolParentValue );

    if ( ret == SuccessCode )
    {
        if ( boolParentValue )
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
