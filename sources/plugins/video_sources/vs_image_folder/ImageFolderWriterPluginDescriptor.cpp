/*
    Image folder plug-ins of Computer Vision Sandbox

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
#include <image_image_folder_writer_16x16.h>
#include "ImageFolderWriterPlugin.hpp"

static void PluginInitializer( );
static void PluginCleaner( );
static XErrorCode UpdateQualityProperties( PropertyDescriptor* desc, const xvariant* parentValue );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 1 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000013, 0x00000001 };

// Folder property
static PropertyDescriptor folderProperty =
{ XVT_String, "Folder", "folder", "Folder to write image files to.", PropertyFlag_PreferredEditor_FolderBrowser };
// File Name Prefix property
static PropertyDescriptor fileNamePrefixProperty =
{ XVT_String, "File Name Prefix", "fileNamePrefix", "Prefix to add to image file names. The rest of file name is time stamp.", PropertyFlag_None };
// Frame Interval property
static PropertyDescriptor frameIntervalProperty =
{ XVT_U2, "Frame Interval", "frameInterval", "Specifies time interval (ms) between writing images.", PropertyFlag_None };
// Image Type property
static PropertyDescriptor imageTypeProperty =
{ XVT_U1, "Image Type", "imageType", "Type of images to write.", PropertyFlag_SelectionByIndex };
// Quality property
static PropertyDescriptor qualityProperty =
{ XVT_U1, "Quality", "quality", "Image compression quality (JPEG only).", PropertyFlag_Dependent };


// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &folderProperty, &fileNamePrefixProperty, &frameIntervalProperty,
    &imageTypeProperty, &qualityProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** ImageFolderWriterPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_VideoProcessing,

    PluginType_VideoProcessing,
    PluginVersion,
    "Image Folder Writer",
    "ImageFolderWriter",
    "Plug-in to write images into the specified folder.",

    /* Long description */
    "The plug-in writes images into the specified folder using the set time interval between frames. If the time "
    "interval is set to zero (or is less than the time interval between frames provided by a video source), then every "
    "frame is written as an image. However, if the set time interval is higher, then the plug-in can be used to create "
    "time lapse image slide-show. Using <a href='{AF000003-00000000-00000005-00000003}'>Image Folder Video Source</a> plug-in "
    "with a video writing plug-in, it is possible to make a time lapse video afterwards."
    ,
    &image_image_folder_writer_16x16,
    nullptr,
    ImageFolderWriterPlugin,

    XARRAY_SIZE( pluginProperties ),
    pluginProperties,
    PluginInitializer,
    PluginCleaner,
    nullptr
);

// Complete properties description by initializing those parts, which were not
// initialized during properties array declaration
static void PluginInitializer( )
{
    // Frame Interval property
    frameIntervalProperty.DefaultValue.type = XVT_U2;
    frameIntervalProperty.DefaultValue.value.usVal = 1000;

    frameIntervalProperty.MinValue.type = XVT_U2;
    frameIntervalProperty.MinValue.value.usVal = 0;

    frameIntervalProperty.MaxValue.type = XVT_U2;
    frameIntervalProperty.MaxValue.value.usVal = 60000;

    // Image Type property
    imageTypeProperty.DefaultValue.type = XVT_U1;
    imageTypeProperty.DefaultValue.value.ubVal = 0;

    imageTypeProperty.MinValue.type = XVT_U1;
    imageTypeProperty.MinValue.value.ubVal = 0;

    imageTypeProperty.MaxValue.type = XVT_U1;
    imageTypeProperty.MaxValue.value.ubVal = 1;

    imageTypeProperty.ChoicesCount = 2;
    imageTypeProperty.Choices = new xvariant[2];

    imageTypeProperty.Choices[0].type = XVT_String;
    imageTypeProperty.Choices[0].value.strVal = XStringAlloc( "JPEG images" );

    imageTypeProperty.Choices[1].type = XVT_String;
    imageTypeProperty.Choices[1].value.strVal = XStringAlloc( "PNG images" );

    // Quality property
    qualityProperty.DefaultValue.type = XVT_U1;
    qualityProperty.DefaultValue.value.ubVal = 90;

    qualityProperty.MinValue.type = XVT_U1;
    qualityProperty.MinValue.value.ubVal = 0;

    qualityProperty.MaxValue.type = XVT_U1;
    qualityProperty.MaxValue.value.ubVal = 100;

    qualityProperty.ParentProperty = 3;
    qualityProperty.Updater = UpdateQualityProperties;
}

// Clean-up plug-in - deallocate strings
static void PluginCleaner( )
{
    for ( int i = 0; i < imageTypeProperty.ChoicesCount; i++ )
    {
        XVariantClear( &imageTypeProperty.Choices[i] );
    }

    delete[] imageTypeProperty.Choices;
}

static XErrorCode UpdateQualityProperties( PropertyDescriptor* desc, const xvariant* parentValue )
{
    XErrorCode ret = ErrorFailed;
    uint8_t    codec;

    ret = XVariantToUByte( parentValue, &codec );

    if ( ret == SuccessCode )
    {
        if ( codec == 0 )
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
