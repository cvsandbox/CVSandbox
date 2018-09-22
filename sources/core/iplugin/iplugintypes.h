/*
    Plug-ins' interface library of Computer Vision Sandbox

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

#pragma once
#ifndef CVS_IPLUGINTYPES_H
#define CVS_IPLUGINTYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <ximage.h>
#include "iplugin.h"

// ===== Some helper functions which may help in implementation of different plug-ins =====

// Helper function which provides default implementation for the "GetPixelFormatTranslations" method
// of image processing filter plugin
XErrorCode GetPixelFormatTranslationsImpl( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count,
                                           const XPixelFormat* supportedInputFormats, const XPixelFormat* supportedOutputFormats,
                                           int32_t supportedFormatsCount );

// Helper function to check if the specified pixel format is in the array of supported formats
bool IsPixelFormatSupportedImpl( const XPixelFormat* supportedInputFormats, int32_t count, XPixelFormat formatToCheck );

// Helper function which provides default implementation for the "GetSupportedExtensions" method
// of image importing/exporting plug-in
XErrorCode GetSupportedExtensionsImpl( const char** supportedExtensionsIn, int32_t inCount,
                                       xstring* supportedExtensionsOut, int32_t* outCount );

// Helper function which provides default implementation for the "GetSupportedImageFormats" method of some plug-ins
XErrorCode GetSupportedPixelFormatsImpl( const XPixelFormat* supportedPixelFormatsIn, int32_t inCount,
                                         XPixelFormat* supportedPixelFormatsOut, int32_t* outCount );


// ===== Base plug-in interface =====
struct SPluginBase_;

// Dispose a plugin
typedef void (*PluginBase_Dispose)( struct SPluginBase_* me );
// Get property of a plug-in
typedef XErrorCode (*PluginBase_GetProperty)( struct SPluginBase_* me, int32_t id, xvariant* value );
// Set property of a plug-in
typedef XErrorCode (*PluginBase_SetProperty)( struct SPluginBase_* me, int32_t id, const xvariant* value );
// Get value of an indexed property (of array type)
typedef XErrorCode (*PluginBase_GetIndexedProperty)( struct SPluginBase_* me, int32_t id, uint32_t index, xvariant* value );
// Set value of an indexed property (of array type)
typedef XErrorCode (*PluginBase_SetIndexedProperty)( struct SPluginBase_* me, int32_t id, uint32_t index, const xvariant* value );
// Call function of a plug-in
typedef XErrorCode (*PluginBase_CallFunction)( struct SPluginBase_* me, int32_t id, xvariant* returnValue, const xarray* arguments );
// Update description of device run time configuration properties
typedef XErrorCode( *PluginBase_UpdateDescription)( struct SPluginBase_* me, PluginDescriptor* descriptor );

typedef struct SPluginBase_
{
    PluginBase_Dispose              Dispose;
    PluginBase_GetProperty          GetProperty;
    PluginBase_SetProperty          SetProperty;
    PluginBase_GetIndexedProperty   GetIndexedProperty;
    PluginBase_SetIndexedProperty   SetIndexedProperty;
    PluginBase_CallFunction         CallFunction;
    PluginBase_UpdateDescription    UpdateDescription;
}
SPluginBase;

// ===== Interface for image processing filter plug-in =====
struct SImageProcessingFilterPlugin_;

// Check if image processing filter can process image itself (not putting result to another image)
typedef bool (*IPFPlugin_CanProcessInPlace)( struct SImageProcessingFilterPlugin_* me );
// Get pixel format translation table - supported input formats and result output formats
typedef XErrorCode (*IPFPlugin_GetPixelFormatTranslations)( struct SImageProcessingFilterPlugin_* me,
                                                            XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count );
// Process specified source image leaving it as it is, but creating new image as a result
typedef XErrorCode (*IPFPlugin_ProcessImage)( struct SImageProcessingFilterPlugin_* me, const ximage* src, ximage** dst );
// Process specified image in place (change it)
typedef XErrorCode (*IPFPlugin_ProcessImageInPlace)( struct SImageProcessingFilterPlugin_* me, ximage* src );

typedef struct SImageProcessingFilterPlugin_
{
    SPluginBase Base;
    IPFPlugin_CanProcessInPlace          CanProcessInPlace;
    IPFPlugin_GetPixelFormatTranslations GetPixelFormatTranslations;
    IPFPlugin_ProcessImage               ProcessImage;
    IPFPlugin_ProcessImageInPlace        ProcessImageInPlace;
}
SImageProcessingFilterPlugin;


// ===== Interface for image processing filter plug-in which uses 2 images to produce one =====
struct SImageProcessingFilterPlugin2_;

// Check if image processing filter can process image itself (not putting result to another image)
typedef bool (*IPFPlugin2_CanProcessInPlace)( struct SImageProcessingFilterPlugin2_* me );
// Get pixel format translation table - supported input formats and result output formats
typedef XErrorCode (*IPFPlugin2_GetPixelFormatTranslations)( struct SImageProcessingFilterPlugin2_* me,
                                                             XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count );
// Get human understandable description/purpose of the second image required by the filter
typedef xstring (*IPFPlugin2_GetSecondImageDescription)( struct SImageProcessingFilterPlugin2_* me );
// Get supported size of the second image
typedef XSupportedImageSize (*IPFPlugin2_GetSecondImageSupportedSize)( struct SImageProcessingFilterPlugin2_* me );
// Get supported pixel format of the second image for the given format of the source image
typedef XPixelFormat (*IPFPlugin2_GetSecondImageSupportedFormat)( struct SImageProcessingFilterPlugin2_* me,
                                                                  XPixelFormat inputPixelFormat );
// Process specified source image leaving it as it is, but creating new image as a result
typedef XErrorCode (*IPFPlugin2_ProcessImage)( struct SImageProcessingFilterPlugin2_* me, const ximage* src, const ximage* src2, ximage** dst );
// Process specified image in place (change it)
typedef XErrorCode (*IPFPlugin2_ProcessImageInPlace)( struct SImageProcessingFilterPlugin2_* me, ximage* src, const ximage* src2 );

typedef struct SImageProcessingFilterPlugin2_
{
    SPluginBase                              Base;
    IPFPlugin2_CanProcessInPlace             CanProcessInPlace;
    IPFPlugin2_GetPixelFormatTranslations    GetPixelFormatTranslations;
    IPFPlugin2_GetSecondImageDescription     GetSecondImageDescription;
    IPFPlugin2_GetSecondImageSupportedSize   GetSecondImageSupportedSize;
    IPFPlugin2_GetSecondImageSupportedFormat GetSecondImageSupportedFormat;
    IPFPlugin2_ProcessImage                  ProcessImage;
    IPFPlugin2_ProcessImageInPlace           ProcessImageInPlace;
}
SImageProcessingFilterPlugin2;

// ===== Interface for image processing plug-in =====
struct SImageProcessingPlugin_;

// Get pixel formats supported by the image processing routine
typedef XErrorCode( *IPPlugin_GetSupportedPixelFormats )( struct SImageProcessingPlugin_* me, XPixelFormat* pixelFormats, int32_t* count );
// Process specified source image calculating whatever it needs to
typedef XErrorCode( *IPPlugin_ProcessImage )( struct SImageProcessingPlugin_* me, const ximage* image );

typedef struct SImageProcessingPlugin_
{
    SPluginBase Base;
    IPPlugin_GetSupportedPixelFormats GetSupportedPixelFormats;
    IPPlugin_ProcessImage             ProcessImage;
}
SImageProcessingPlugin;


// ===== Interface for image importing plug-in =====
struct SImageImportingPlugin_;

// Get some short description of the file type
typedef xstring (*IImpPlugin_GetFileTypeDescription)( struct SImageImportingPlugin_* me );
// Get file extensions supported by the importer
typedef XErrorCode (*IImpPlugin_GetSupportedExtensions)( struct SImageImportingPlugin_* me, xstring* fileExtensions, int32_t* count );
// Load image from the specified file
typedef XErrorCode (*IImpPlugin_ImportImage)( struct SImageImportingPlugin_* me, xstring fileName, ximage** image );

typedef struct SImageImportingPlugin_
{
    SPluginBase Base;
    IImpPlugin_GetFileTypeDescription GetFileTypeDescription;
    IImpPlugin_GetSupportedExtensions GetSupportedExtensions;
    IImpPlugin_ImportImage            ImportImage;
}
SImageImportingPlugin;


// ===== Interface for image exporting plug-in =====
struct SImageExportingPlugin_;

// Get some short description of the file type
typedef xstring (*IExpPlugin_GetFileTypeDescription)( struct SImageExportingPlugin_* me );
// Get file extensions supported by the exporter
typedef XErrorCode (*IExpPlugin_GetSupportedExtensions)( struct SImageExportingPlugin_* me, xstring* fileExtensions, int32_t* count );
// Get pixel formats supported by the exporter
typedef XErrorCode (*IExpPlugin_GetSupportedPixelFormats)( struct SImageExportingPlugin_* me, XPixelFormat* pixelFormats, int32_t* count );
// Save image to the specified file
typedef XErrorCode (*IExpPlugin_ExportImage)( struct SImageExportingPlugin_* me, xstring fileName, const ximage* image );

typedef struct SImageExportingPlugin_
{
    SPluginBase Base;
    IExpPlugin_GetFileTypeDescription   GetFileTypeDescription;
    IExpPlugin_GetSupportedExtensions   GetSupportedExtensions;
    IExpPlugin_GetSupportedPixelFormats GetSupportedPixelFormats;
    IExpPlugin_ExportImage              ExportImage;
}
SImageExportingPlugin;


// ===== Interface for video source plug-in =====
struct SVideoSourcePlugin_;

// Callback type to provide video frames from plug-in
typedef void( *VideoSourcePluginCallback_NewImage )( void* userParam, const ximage* image );
// Callback type to provide error messages from video source
typedef void( *VideoSourcePluginCallback_ErrorMessage )( void* userParam, const char* errorMessage );

typedef struct VideoSourcePluginCallbacks_
{
    VideoSourcePluginCallback_NewImage      NewImageCallback;
    VideoSourcePluginCallback_ErrorMessage  ErrorMessageCallback;
}
VideoSourcePluginCallbacks;

// Start video source so it initializes and begins providing video frames
typedef XErrorCode (*VSPlugin_Start)( struct SVideoSourcePlugin_* me );
// Signal video to stop, so it could finalize and cleanup
typedef void (*VSPlugin_SignalToStop)( struct SVideoSourcePlugin_* me );
// Wait till video source (its thread) stops
typedef void (*VSPlugin_WaitForStop)( struct SVideoSourcePlugin_* me );
// Check if video source (its thread) is still running
typedef bool (*VSPlugin_IsRunning)( struct SVideoSourcePlugin_* me );
// Terminate video source - call *ONLY* if video source looks to be frozen and does not stop
// by itself when signaled (ideally this method should not exist and be called at all)
typedef void (*VSPlugin_Terminate)( struct SVideoSourcePlugin_* me );
// Get number of frames received since the the start of the video source
typedef uint32_t (*VSPlugin_FramesReceived)( struct SVideoSourcePlugin_* me ) ;
// Set callbacks for the video source
typedef void( *VSPlugin_SetCallbacks )( struct SVideoSourcePlugin_* me, const VideoSourcePluginCallbacks* callbacks, void* userParam );

typedef struct SVideoSourcePlugin_
{
    SPluginBase Base;

    VSPlugin_Start              Start;
    VSPlugin_SignalToStop       SignalToStop;
    VSPlugin_WaitForStop        WaitForStop;
    VSPlugin_IsRunning          IsRunning;
    VSPlugin_Terminate          Terminate;
    VSPlugin_FramesReceived     FramesReceived;
    VSPlugin_SetCallbacks       SetCallbacks;
}
SVideoSourcePlugin;


// ===== Interface for image generation plug-in =====
struct SImageGenerationPlugin_;

// Generates an image
typedef XErrorCode (*IGPlugin_GenerateImage)( struct SImageGenerationPlugin_* me, ximage** dst );

typedef struct SImageGenerationPlugin_
{
    SPluginBase Base;
    IGPlugin_GenerateImage GenerateImage;
}
SImageGenerationPlugin;


// ===== Interface for device plug-in =====
struct SDevicePlugin_;

// Connect to device
typedef XErrorCode (*DPlugin_Connect)( struct SDevicePlugin_* me );
// Disconnect from device
typedef void (*DPlugin_Disconnect)( struct SDevicePlugin_* me );
// Check if connected to a device
typedef bool (*DPlugin_IsConnected)( struct SDevicePlugin_* me );

typedef struct SDevicePlugin_
{
    SPluginBase Base;

    DPlugin_Connect     Connect;
    DPlugin_Disconnect  Disconnect;
    DPlugin_IsConnected IsConnected;
}
SDevicePlugin;


// ===== Interface for video processing plug-in =====
struct SVideoProcessingPlugin_;

// Check if the plug-in does changes to input video frames or not
typedef bool( *VPPlugin_IsReadOnlyMode )( struct SVideoProcessingPlugin_* me );
// Get pixel formats supported by the video processing plug-in
typedef XErrorCode( *VPPlugin_GetSupportedPixelFormats )( struct SVideoProcessingPlugin_* me, XPixelFormat* pixelFormats, int32_t* count );
// Process the specified image
typedef XErrorCode( *VPPlugin_ProcessImage )( struct SVideoProcessingPlugin_* me, ximage* src );
// Reset run time state of the video processing plug-in
typedef void( *VPPlugin_Reset )( struct SVideoProcessingPlugin_* me );

typedef struct SVideoProcessingPlugin_
{
    SPluginBase Base;
    VPPlugin_IsReadOnlyMode             IsReadOnlyMode;
    VPPlugin_GetSupportedPixelFormats   GetSupportedPixelFormats;
    VPPlugin_ProcessImage               ProcessImage;
    VPPlugin_Reset                      Reset;
}
SVideoProcessingPlugin;


// ===== Interface for scripting engine plug-in =====
struct SScriptingEnginePlugin_;

// Callback type to get name of the host running scripting engine plug-in
typedef xstring( *ScriptingEnginePluginCallback_GetHostName )( void* userParam );
// Callback type to get version of the host running scripting engine plug-in
typedef void( *ScriptingEnginePluginCallback_GetHostVersion )( void* userParam, xversion* version );
// Callback type to print a string message at the host
typedef void( *ScriptingEnginePluginCallback_PrintString )( void* userParam, xstring message );
// Callback type to create plug-in instance
typedef XErrorCode( *ScriptingEnginePluginCallback_CreatePluginInstance )( void* userParam, xstring pluginName,
                                                                           PluginDescriptor** pDescriptor,
                                                                           void** pPlugin );
// Callback type to get current image available on the host side
typedef XErrorCode( *ScriptingEnginePluginCallback_GetImage )( void* userParam, ximage** image );
// Callback type to set/replace current image on the host side
typedef XErrorCode( *ScriptingEnginePluginCallback_SetImage )( void* userParam, ximage* image );
// Callback type to get xvariant variable from the host side
typedef XErrorCode( *ScriptingEnginePluginCallback_GetVariable )( void* userParam, xstring name, xvariant* value );
// Callback type to store xvariant variable on the host side
typedef XErrorCode( *ScriptingEnginePluginCallback_SetVariable )( void* userParam, xstring name, const xvariant* value );
// Callback type to get image variable from the host side
typedef XErrorCode( *ScriptingEnginePluginCallback_GetImageVariable )( void* userParam, xstring name, ximage** value );
// Callback type to store image variable on the host side
typedef XErrorCode( *ScriptingEnginePluginCallback_SetImageVariable )( void* userParam, xstring name, const ximage* value );
// Callback type to get video source associated with the running script (if any)
typedef XErrorCode( *ScriptingEnginePluginCallback_GetVideoSource )( void* userParam,
                                                                     PluginDescriptor** pDescriptor,
                                                                     void** pPlugin );

typedef struct ScriptingEnginePluginCallbacks_
{
    ScriptingEnginePluginCallback_GetHostName           GetHostName;
    ScriptingEnginePluginCallback_GetHostVersion        GetHostVersion;
    ScriptingEnginePluginCallback_PrintString           PrintString;
    ScriptingEnginePluginCallback_CreatePluginInstance  CreatePluginInstance;
    ScriptingEnginePluginCallback_GetImage              GetImage;
    ScriptingEnginePluginCallback_SetImage              SetImage;
    ScriptingEnginePluginCallback_GetVariable           GetVariable;
    ScriptingEnginePluginCallback_SetVariable           SetVariable;
    ScriptingEnginePluginCallback_GetImageVariable      GetImageVariable;
    ScriptingEnginePluginCallback_SetImageVariable      SetImageVariable;
    ScriptingEnginePluginCallback_GetVideoSource        GetVideoSource;
}
ScriptingEnginePluginCallbacks;

// Set callback to interface with the calling host
typedef void( *SEPlugin_SetCallbacks )( struct SScriptingEnginePlugin_* me, const ScriptingEnginePluginCallbacks* callbacks, void* userParam );
// Get default file name extension for the engine's scripts
typedef xstring( *SEPlugin_GetDefaultExtension )( struct SScriptingEnginePlugin_* me );
// Perform any necessary initialization of the scripting engine
typedef XErrorCode( *SEPlugin_Init )( struct SScriptingEnginePlugin_* me );
// Set script file to be loaded (this might be also exposed as plug-ins property,
// but duplicated for clients who would like to rely on the set interface)
typedef void( *SEPlugin_SetScriptFile )( struct SScriptingEnginePlugin_* me, xstring fileName );
// Load previously set script from file
typedef XErrorCode( *SEPlugin_LoadScript )( struct SScriptingEnginePlugin_* me );
// Run initialization part of the loaded script (any code outside of Main)
typedef XErrorCode( *SEPlugin_InitScript )( struct SScriptingEnginePlugin_* me );
// Run the Main function of the script
typedef XErrorCode( *SEPlugin_RunScript )( struct SScriptingEnginePlugin_* me );
// Get error message which scripting engine may generate if LoadScript() returns ErrorFailedLoadingScript or InitScript()/RunScript() return ErrorFailedRunningScript
// (return 0 if no error message is available)
typedef xstring( *SEPlugin_GetLastErrorMessage )( struct SScriptingEnginePlugin_* me );

typedef struct SScriptingEnginePlugin_
{
    SPluginBase Base;

    SEPlugin_SetCallbacks           SetCallbacks;
    SEPlugin_GetDefaultExtension    GetDefaultExtension;
    SEPlugin_Init                   Init;
    SEPlugin_SetScriptFile          SetScriptFile;
    SEPlugin_LoadScript             LoadScript;
    SEPlugin_InitScript             InitScript;
    SEPlugin_RunScript              RunScript;
    SEPlugin_GetLastErrorMessage    GetLastErrorMessage;
}
SScriptingEnginePlugin;

#ifdef __cplusplus
}
#endif

#endif // CVS_IPLUGINTYPES_H
