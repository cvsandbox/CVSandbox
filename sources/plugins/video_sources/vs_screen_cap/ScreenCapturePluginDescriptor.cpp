/*
    Screen capture plug-ins of Computer Vision Sandbox

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
#include "ScreenCapturePlugin.hpp"
#include <image_screen_16x16.h>
#include <stdio.h>

static void PluginInitializer( );
static void PluginCleaner( );
static XErrorCode UpdateProperties( PluginDescriptor* desc );
static XErrorCode UpdateScreenProperties( PropertyDescriptor* desc, const xvariant* parentValue );
static XErrorCode UpdateAreaProperties( PropertyDescriptor* desc, const xvariant* parentValue );
static XErrorCode UpdateWindowProperties( PropertyDescriptor* desc, const xvariant* parentValue );
static XErrorCode UpdateResizeProperties( PropertyDescriptor* desc, const xvariant* parentValue );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000012, 0x00000001 };

// Capture Mode property
static PropertyDescriptor captureModeProperty =
{ XVT_U1, "Capture Mode", "captureMode", "Specifies part of the screen to capture.", PropertyFlag_SelectionByIndex };
// Screen property
static PropertyDescriptor screenProperty =
{ XVT_U1, "Screen", "screen", "Specifies which screen to capture on sytems with multiple screens.", PropertyFlag_SelectionByIndex | PropertyFlag_RuntimeConfiguration | PropertyFlag_Dependent };
// Capture Point property
static PropertyDescriptor capturePointProperty =
{ XVT_Point, "Capture Point", "capturePoint", "Coordinates of the top-left rectangle's corner to capture.", PropertyFlag_Dependent };
// Capture Size property
static PropertyDescriptor captureSizeProperty =
{ XVT_Size, "Capture Size", "captureSize", "Size of the area to capture.", PropertyFlag_Dependent };
// Window Title property
static PropertyDescriptor windowTitleProperty =
{ XVT_String, "Window Title", "windowTitle", "Title of the window to capture.", PropertyFlag_Dependent };
// Frame Rate property
static PropertyDescriptor frameRateProperty =
{ XVT_U2, "Frame Rate", "frameRate", "Specifies capture frame rate.", PropertyFlag_None };

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
    &captureModeProperty, &screenProperty, &capturePointProperty, &captureSizeProperty, &windowTitleProperty,
    &frameRateProperty, &resizeImageProperty, &outputSizeProperty, &keepAspectRatioProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** ScreenCapturePlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_VirtualVideoSource,

    PluginType_VideoSource,
    PluginVersion,
    "Screen Capture",
    "ScreenCapture",
    "Video source which provides frames of the captured screen.",

    "The plug-in allows capturing content of the systems' screen. It can capture "
    "either entire content of one of the screens (in case multiple displays are available "
    "in the system), a specified rectangular area or content of the specified window.<br><br>"

    "In the case of capturing window, its position and size are tracked, so that the video "
    "source always follows it. If window is overlapped by another window, then parts of the "
    "overlapping window are captured as well (capturing the visible content of the window's "
    "rectangle). The <b>Window Title</b> does not need to be set to exact title of the window, "
    "but needs to specify a sub string of the window's title to capture. If multiple windows "
    "have the specified sub string in their title, then the first one is captured. If the window "
    "being captured is closed, the video source will try finding another with the same title. "
    "If no windows found having the title, the video source generates empty (filled with black color)"
    "image until a new window with the specified title is opened. Windows' title comparison is case "
    "insensitive."
    ,

    &image_screen_16x16,
    nullptr,
    ScreenCapturePlugin,

    XARRAY_SIZE( pluginProperties ),
    pluginProperties,
    PluginInitializer,
    PluginCleaner,
    UpdateProperties
);

// Complete properties description by initializing those parts, which were not
// initialized during properties array declaration
void PluginInitializer( )
{
    // Capture Mode property
    captureModeProperty.DefaultValue.type = XVT_U1;
    captureModeProperty.DefaultValue.value.ubVal = 0;

    captureModeProperty.MinValue.type = XVT_U1;
    captureModeProperty.MinValue.value.ubVal = 0;

    captureModeProperty.MaxValue.type = XVT_U1;
    captureModeProperty.MaxValue.value.ubVal = 2;

    captureModeProperty.ChoicesCount = 3;
    captureModeProperty.Choices = new xvariant[3];

    captureModeProperty.Choices[0].type = XVT_String;
    captureModeProperty.Choices[0].value.strVal = XStringAlloc( "Screen" );

    captureModeProperty.Choices[1].type = XVT_String;
    captureModeProperty.Choices[1].value.strVal = XStringAlloc( "Area" );

    captureModeProperty.Choices[2].type = XVT_String;
    captureModeProperty.Choices[2].value.strVal = XStringAlloc( "Window" );

    // Screen property
    screenProperty.ParentProperty = 0;
    screenProperty.Updater = UpdateScreenProperties;

    // Capture Point property
    capturePointProperty.MinValue.type = XVT_I4;
    capturePointProperty.MinValue.value.iVal = -10000;
    capturePointProperty.MaxValue.type = XVT_I4;
    capturePointProperty.MaxValue.value.iVal = 10000;

    capturePointProperty.ParentProperty = 0;
    capturePointProperty.Updater = UpdateAreaProperties;

    // Capture Size property
    captureSizeProperty.MinValue.type = XVT_I4;
    captureSizeProperty.MinValue.value.iVal = 16;
    captureSizeProperty.MaxValue.type = XVT_I4;
    captureSizeProperty.MaxValue.value.iVal = 4000;

    captureSizeProperty.ParentProperty = 0;
    captureSizeProperty.Updater = UpdateAreaProperties;

    // Window Title property
    windowTitleProperty.ParentProperty = 0;
    windowTitleProperty.Updater = UpdateWindowProperties;

    // Frame Rate property
    frameRateProperty.DefaultValue.type = XVT_U2;
    frameRateProperty.DefaultValue.value.usVal = 20;
    frameRateProperty.MinValue.type = XVT_U2;
    frameRateProperty.MinValue.value.usVal = 1;
    frameRateProperty.MaxValue.type = XVT_U2;
    frameRateProperty.MaxValue.value.usVal = 60;

    // Resize Image property
    resizeImageProperty.DefaultValue.type = XVT_Bool;
    resizeImageProperty.DefaultValue.value.boolVal = false;

    // Keep Aspect Ration property
    keepAspectRatioProperty.DefaultValue.type = XVT_Bool;
    keepAspectRatioProperty.DefaultValue.value.boolVal = true;

    keepAspectRatioProperty.ParentProperty = 6;
    keepAspectRatioProperty.Updater = UpdateResizeProperties;

    // Output Size property
    outputSizeProperty.DefaultValue.type = XVT_Size;
    outputSizeProperty.DefaultValue.value.sizeVal.width  = 640;
    outputSizeProperty.DefaultValue.value.sizeVal.height = 480;

    outputSizeProperty.MinValue.type = XVT_I4;
    outputSizeProperty.MinValue.value.iVal = 16;

    outputSizeProperty.MaxValue.type = XVT_I4;
    outputSizeProperty.MaxValue.value.iVal = 4096;

    outputSizeProperty.ParentProperty = 6;
    outputSizeProperty.Updater = UpdateResizeProperties;
}

// Clean-up plug-in
void PluginCleaner( )
{
    for ( int i = 0; i < captureModeProperty.ChoicesCount; i++ )
    {
        XVariantClear( &captureModeProperty.Choices[i] );
    }

    delete[] captureModeProperty.Choices;
}

// Update property listing available screens
static XErrorCode UpdateProperties( PluginDescriptor* desc )
{
    std::list<xrect>    displays = ScreenCapturePlugin::GetDisplays( );
    PropertyDescriptor* prop     = desc->Properties[1];
    char                buffer[64];
    int                 i = 0;

    // free old choices
    FreePropertyChoices( prop );
    XVariantClear( &prop->DefaultValue );
    XVariantClear( &prop->MinValue );
    XVariantClear( &prop->MaxValue );

    prop->ChoicesCount = static_cast<int16_t>( displays.size( ) );

    if ( prop->ChoicesCount > 0 )
    {
        prop->Choices = static_cast<xvariant*>( XCAlloc( prop->ChoicesCount, sizeof( xvariant ) ) );

        for ( std::list<xrect>::const_iterator it = displays.begin( ); it != displays.end( ); ++it, ++i )
        {
            sprintf( buffer, "%d: %d,%d - %dx%d", i + 1, it->x1, it->y1, it->x2 - it->x1, it->y2 - it->y1 );

            prop->Choices[i].type = XVT_String;
            prop->Choices[i].value.strVal = XStringAlloc( buffer );
        }

        prop->DefaultValue.type = XVT_U1;
        prop->DefaultValue.value.uiVal = 0;

        prop->MinValue.type = XVT_U1;
        prop->MinValue.value.uiVal = 0;

        prop->MaxValue.type = XVT_U1;
        prop->MaxValue.value.uiVal = prop->ChoicesCount - 1;
    }

    return SuccessCode;
}

XErrorCode UpdateScreenProperties( PropertyDescriptor* desc, const xvariant* parentValue )
{
    XErrorCode ret = ErrorFailed;
    uint8_t    captureMode;

    ret = XVariantToUByte( parentValue, &captureMode );

    if ( ret == SuccessCode )
    {
        if ( captureMode == 0 )
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

XErrorCode UpdateAreaProperties( PropertyDescriptor* desc, const xvariant* parentValue )
{
    XErrorCode ret = ErrorFailed;
    uint8_t    captureMode;

    ret = XVariantToUByte( parentValue, &captureMode );

    if ( ret == SuccessCode )
    {
        if ( captureMode == 1 )
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

XErrorCode UpdateWindowProperties( PropertyDescriptor* desc, const xvariant* parentValue )
{
    XErrorCode ret = ErrorFailed;
    uint8_t    captureMode;

    ret = XVariantToUByte( parentValue, &captureMode );

    if ( ret == SuccessCode )
    {
        if ( captureMode == 2 )
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
