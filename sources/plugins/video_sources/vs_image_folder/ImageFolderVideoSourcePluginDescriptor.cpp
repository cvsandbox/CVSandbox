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
#include <image_folder_images_16x16.h>
#include "ImageFolderVideoSourcePlugin.hpp"

static void PluginInitializer( );
static void PluginCleaner( );
static XErrorCode UpdateResizeProperties( PropertyDescriptor* desc, const xvariant* parentValue );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 1 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000005, 0x00000003 };

// Image Type property
static PropertyDescriptor imageTypeProperty =
{ XVT_U1, "Image Type", "imageType", "Type of images to look for in a folder.", PropertyFlag_SelectionByIndex };
// Folder property
static PropertyDescriptor folderProperty =
{ XVT_String, "Folder", "folder", "Folder to read image files from.", PropertyFlag_PreferredEditor_FolderBrowser };
// Frame Interval property
static PropertyDescriptor frameIntervalProperty =
{ XVT_U2, "Frame Interval", "frameInterval", "Specifies time interval (ms) between images.", PropertyFlag_None };
// Cycle Images property
static PropertyDescriptor cycleImagesProperty =
{ XVT_Bool, "Cycle Images", "cycleImages", "Specifies if images must be cycled in an end-less loop.", PropertyFlag_None };
// Resize Image property
static PropertyDescriptor resizeImageProperty =
{ XVT_Bool, "Resize Image", "resizeImage", "Specifies if the captured images must be resized.", PropertyFlag_None };
// Output Size property
static PropertyDescriptor outputSizeProperty =
{ XVT_Size, "Output Size", "outputSize", "Size to resize captured images to.", PropertyFlag_Dependent };
// Keep Aspect Ratio property
static PropertyDescriptor keepAspectRatioProperty =
{ XVT_Bool, "Keep Aspect Ratio", "keepAspectRatio", "Specifies if aspect ratio must be kept while resizing.", PropertyFlag_Dependent };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &imageTypeProperty,
    &folderProperty, &frameIntervalProperty, &cycleImagesProperty,
    &resizeImageProperty, &outputSizeProperty, &keepAspectRatioProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** ImageFolderVideoSourcePlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_VirtualVideoSource,

    PluginType_VideoSource,
    PluginVersion,
    "Image Folder Video Source",
    "ImageFolderVideoSource",
    "Plug-in to provide images from a specified folder.",

    /* Long description */
    "The plug-in provides images from a given folder with the specified time interval between them. By default "
    "it plays all images and stops. However it can also cycle them in an end-less loop."
    ,
    &image_folder_images_16x16,
    nullptr,
    ImageFolderVideoSourcePlugin,

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

    // Frame Interval property
    frameIntervalProperty.DefaultValue.type = XVT_U2;
    frameIntervalProperty.DefaultValue.value.usVal = 1000;

    frameIntervalProperty.MinValue.type = XVT_U2;
    frameIntervalProperty.MinValue.value.usVal = 0;

    frameIntervalProperty.MaxValue.type = XVT_U2;
    frameIntervalProperty.MaxValue.value.usVal = 60000;

    // Cycle Images property
    cycleImagesProperty.DefaultValue.type = XVT_Bool;
    cycleImagesProperty.DefaultValue.value.boolVal = false;

    // Resize Image property
    resizeImageProperty.DefaultValue.type = XVT_Bool;
    resizeImageProperty.DefaultValue.value.boolVal = false;

    // Keep Aspect Ration property
    keepAspectRatioProperty.DefaultValue.type = XVT_Bool;
    keepAspectRatioProperty.DefaultValue.value.boolVal = true;

    keepAspectRatioProperty.ParentProperty = 4;
    keepAspectRatioProperty.Updater = UpdateResizeProperties;

    // Output Size property
    outputSizeProperty.DefaultValue.type = XVT_Size;
    outputSizeProperty.DefaultValue.value.sizeVal.width  = 640;
    outputSizeProperty.DefaultValue.value.sizeVal.height = 480;

    outputSizeProperty.MinValue.type = XVT_I4;
    outputSizeProperty.MinValue.value.iVal = 16;

    outputSizeProperty.MaxValue.type = XVT_I4;
    outputSizeProperty.MaxValue.value.iVal = 4096;

    outputSizeProperty.ParentProperty = 4;
    outputSizeProperty.Updater = UpdateResizeProperties;
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

static XErrorCode UpdateResizeProperties( PropertyDescriptor* desc, const xvariant* parentValue )
{
    XErrorCode ret = ErrorFailed;
    bool       isResizing;

    ret = XVariantToBool( parentValue, &isResizing );

    if ( ret == SuccessCode )
    {
        if ( isResizing )
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
