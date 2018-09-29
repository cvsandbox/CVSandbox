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
#ifndef CVS_IPLUGINTYPESCPP_HPP
#define CVS_IPLUGINTYPESCPP_HPP

#include "iplugin.h"
#include "iplugintypes.h"

// ===== Base plug-in interface =====
class IPluginBase
{
protected:
    virtual ~IPluginBase( ) { };

public:
    // Dispose a plug-in
    virtual void Dispose( ) = 0;

    // Get/Set property of a plug-in
    virtual XErrorCode GetProperty( int32_t id, xvariant* value ) const = 0;
    virtual XErrorCode SetProperty( int32_t id, const xvariant* value ) = 0;

    // Get/Set indexed property of a plug-in - not supported by default, since most plug-ins will not use this API
    virtual XErrorCode GetIndexedProperty( int32_t id, uint32_t index, xvariant* value ) const
    {
        XUNREFERENCED_PARAMETER( id )
        XUNREFERENCED_PARAMETER( index )
        XUNREFERENCED_PARAMETER( value )
        return ErrorUnsupportedInterface;
    }
    virtual XErrorCode SetIndexedProperty( int32_t id, uint32_t index, const xvariant* value )
    {
        XUNREFERENCED_PARAMETER( id )
        XUNREFERENCED_PARAMETER( index )
        XUNREFERENCED_PARAMETER( value )
        return ErrorUnsupportedInterface;
    }

    // Call function of a plug-in
    virtual XErrorCode CallFunction( int32_t id, xvariant* returnValue, const xarray* arguments )
    {
        XUNREFERENCED_PARAMETER( id )
        XUNREFERENCED_PARAMETER( returnValue )
        XUNREFERENCED_PARAMETER( arguments )
        return ErrorUnsupportedInterface;
    }

    // Update description of device run time configuration properties
    virtual XErrorCode UpdateDescription( PluginDescriptor* descriptor )
    {
        XUNREFERENCED_PARAMETER( descriptor )
        return ErrorUnsupportedInterface;
    }
};

// ===== Interface for image processing filter plug-in =====
class IImageProcessingFilterPlugin : public IPluginBase
{
public:
    // Check if image processing filter can process image itself (not putting result to another image)
    virtual bool CanProcessInPlace( ) = 0;
    // Get pixel format translation table - supported input formats and result output formats
    virtual XErrorCode GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count ) = 0;
    // Process specified source image leaving it as it is, but creating new image as a result
    virtual XErrorCode ProcessImage( const ximage* src, ximage** dst ) = 0;
    // Process specified image in place (change it)
    virtual XErrorCode ProcessImageInPlace( ximage* src ) = 0;
};

// ===== Interface for image processing filter plug-in which uses 2 images to produce one =====
class IImageProcessingFilterPlugin2 : public IPluginBase
{
public:
    // Check if image processing filter can process image itself (not putting result to another image)
    virtual bool CanProcessInPlace( ) = 0;
    // Get pixel format translation table - supported input formats and result output formats
    virtual XErrorCode GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count ) = 0;
    // Get human understandable description/purpose of the second image required by the filter
    virtual xstring GetSecondImageDescription( ) = 0;
    // Get supported size of the second image
    virtual XSupportedImageSize GetSecondImageSupportedSize( ) = 0;
    // Get supported pixel format of the second image for the given format of the source image
    virtual XPixelFormat GetSecondImageSupportedFormat( XPixelFormat inputPixelFormat ) = 0;
    // Process specified source image leaving it as it is, but creating new image as a result
    virtual XErrorCode ProcessImage( const ximage* src, const ximage* src2, ximage** dst ) = 0;
    // Process specified image in place (change it)
    virtual XErrorCode ProcessImageInPlace( ximage* src, const ximage* src2 ) = 0;
};

// ===== Interface for image processing plug-in =====
class IImageProcessingPlugin : public IPluginBase
{
public:
    // Get pixel formats supported by the plug-in
    virtual XErrorCode GetSupportedPixelFormats( XPixelFormat* pixelFormats, int32_t* count ) = 0;
    // Process the specified image
    virtual XErrorCode ProcessImage( const ximage* image ) = 0;
};

// ===== Interface for image importing plug-in =====
class IImageImportingPlugin : public IPluginBase
{
public:
    // Get some short description of the file type
    virtual xstring GetFileTypeDescription( ) = 0;
    // Get file extensions supported by the importer
    virtual XErrorCode GetSupportedExtensions( xstring* fileExtensions, int32_t* count ) = 0;
    // Load image from the specified file
    virtual XErrorCode ImportImage( xstring fileName, ximage** image ) = 0;
};

// ===== Interface for image exporting plug-in =====
class IImageExportingPlugin : public IPluginBase
{
public:
    // Get some short description of the file type
    virtual xstring GetFileTypeDescription( ) = 0;
    // Get file extensions supported by the exporter
    virtual XErrorCode GetSupportedExtensions( xstring* fileExtensions, int32_t* count ) = 0;
    // Get pixel formats supported by the exporter
    virtual XErrorCode GetSupportedPixelFormats( XPixelFormat* pixelFormats, int32_t* count ) = 0;
    // Save image to the specified file
    virtual XErrorCode ExportImage( xstring fileName, const ximage* image ) = 0;
};

// ===== Interface for video source plug-in =====
class IVideoSourcePlugin : public IPluginBase
{
public:
    // Start video source so it initializes and begins providing video frames
    virtual XErrorCode Start( ) = 0;
    // Signal video to stop, so it could finalize and cleanup
    virtual void SignalToStop( ) = 0;
    // Wait till video source (its thread) stops
    virtual void WaitForStop( ) = 0;
    // Check if video source (its thread) is still running
    virtual bool IsRunning( ) = 0;

    // Terminate video source - call *ONLY* if video source looks to be frozen and does not stop
    // by itself when signaled (ideally this method should not exist and be called at all)
    virtual void Terminate( ) = 0;

    // Get number of frames received since the the start of the video source
    virtual uint32_t FramesReceived( ) = 0;

    // Set callbacks for the video source
    virtual void SetCallbacks( const VideoSourcePluginCallbacks* callbacks, void* userParam ) = 0;
};

// ===== Interface for image generation plug-in =====
class IImageGenerationPlugin : public IPluginBase
{
public:
    // Generates an image
    virtual XErrorCode GenerateImage( ximage** dst ) = 0;
};

// ===== Interface for device plug-in =====
class IDevicePlugin : public IPluginBase
{
public:
    // Connect to device
    virtual XErrorCode Connect( ) = 0;
    // Disconnect from device
    virtual void Disconnect( ) = 0;
    // Check if connected to a device
    virtual bool IsConnected( ) = 0;
};

// ===== Interface for video processing plug-in =====
class IVideoProcessingPlugin : public IPluginBase
{
public:
    // Check if the plug-in does changes to input video frames or not
    virtual bool IsReadOnlyMode( ) = 0;
    // Get pixel formats supported by the video processing plug-in
    virtual XErrorCode GetSupportedPixelFormats( XPixelFormat* pixelFormats, int32_t* count ) = 0;
    // Process the specified image
    virtual XErrorCode ProcessImage( ximage* src ) = 0;
    // Reset run time state of the video processing plug-in
    virtual void Reset( ) = 0;
};

// ===== Interface for scripting engine plug-in =====
class IScriptingEnginePlugin : public IPluginBase
{
public:
    // Set callback to interface with the calling host
    virtual void SetCallbacks( const ScriptingEnginePluginCallbacks* callbacks, void* userParam ) = 0;
    // Get default file name extension for the engine's scripts
    virtual xstring GetDefaultExtension( ) = 0;
    // Perform any necessary initialization of the scripting engine
    virtual XErrorCode Init( ) = 0;
    // Set script file to be loaded (this might be also exposed as plug-ins property,
    // but duplicated for clients who would like to rely on the set interface)
    virtual void SetScriptFile( xstring fileName ) = 0;
    // Load previously set script from file
    virtual XErrorCode LoadScript( ) = 0;
    // Run initialization part of the loaded script (any code outside of Main)
    virtual XErrorCode InitScript( ) = 0;
    // Run the Main function of the script
    virtual XErrorCode RunScript( ) = 0;
    // Get error message which scripting engine may generate if LoadScript() returns ErrorFailedLoadingScript or InitScript()/RunScript() return ErrorFailedRunningScript
    // (return nullptr if no error message is available)
    virtual xstring GetLastErrorMessage( ) = 0;
};

#endif // CVS_IPLUGINTYPESCPP_HPP
