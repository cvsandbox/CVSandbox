/*
    Core types library of Computer Vision Sandbox

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

#include "xtypes.h"
#include <stdio.h>

// Common error codes
static char* commonErrorNames[] =
{
    "Success code (no error)",
    "General failure",
    "One of the specified arguments is a null pointer",
    "Out of available memory",
    "Specified buffer is too small",
    "One of the specified parameters is out of range",
    "One of the specified paramters is not valid",
    "The invoked API is not implemented",
    "Invalid property specified (specified property does not exist)",
    "Unsupported interface",
    "Failed loading module",
    "Initialization failure",
    "General IO failure",
    "Failed due to invalid configuration",
    "Cannot set the property, while object/plug-in is running",
    "Failed connecting to device/host",
    "Failed due to missing connection",
    "The property is read only",
    "The property is not indexed (not an array)",
    "Invalid count of arguments passed",
    "Invalid function specified (specified function does not exist)",
    "Configuration is not supported",
    "Device is not ready",
    "End of file reached"
};

// Type conversion error codes
static char* typeConversionErrorNames[] =
{
    "Specified type is not valid",
    "Specified types are not compatible",
    "Invalid type format",
    "Type limits exceedeed",
    "Specified index is out of array's bounds",
    "Size of the provided array is not valid"
};

// Image processing error codes
static char* imageProcessingErrorNames[] =
{
    "Unsupported pixel format",
    "Image parameters mismatch",
    "The specified image is too small to be processed by the routine",
    "The specified image is too big to be processed by the routine",
    "The specified image has invalid width and/or height",
    "Image palette is missing"
};

// Error codes related to image encoding/decoding
static char* imageCodingErrorNames[] =
{
    "Unknown image file format",
    "Failed image decoding",
    "Failed image encoding",
};

// Error codes related to video encoding/decoding
static char* videoCodingErrorNames[] =
{
    "Unknown video file format",
    "Failed initializing video codec",
    "Video codec is not found",
    "Video stream is missing",
    "Failed video decoding"
};

// Error codes related to scripting engine
static char* scriptingEngineErrorNames[] =
{
    "Failed initializing scripting engine",
    "Failed loading/compiling the specified script",
    "Failed running the specified script",
    "Scripting engine is not initialized",
    "No script is loaded",
    "No entry point found in the script"
};

// Error codes related to plug-in's engine
static char* pluginEngineErrorNames[] =
{
    "The specified plug-in was not found",
    "Failed creating instance of the specified plug-in",
};

// Get description for the specified error code
xstring XErrorGetDescription( XErrorCode error )
{
    char* errorDesc = 0;
    xstring ret = 0;

    // ===== Standard error codes =====
    if ( ( error >= SuccessCode ) && ( error <= ErrorEOF ) )
    {
        errorDesc = commonErrorNames[error - SuccessCode];
    }
    // ===== Error code used in data conversion (by variant related functions or others) =====
    else if ( ( error >= ErrorInvalidType ) && ( error <= ErrorInvalidArraySize ) )
    {
        errorDesc = typeConversionErrorNames[error - ErrorInvalidType];
    }
    // ===== Error codes related to image processing functions =====
    else if ( ( error >= ErrorUnsupportedPixelFormat ) && ( error <= ErrorImagePaletteIsMissing ) )
    {
        errorDesc = imageProcessingErrorNames[error - ErrorUnsupportedPixelFormat];
    }
    // ===== Error codes related to image encoding/decoding =====
    else if ( ( error >= ErrorUnknownImageFileFormat ) && ( error <= ErrorFailedImageEncoding ) )
    {
        errorDesc = imageCodingErrorNames[error - ErrorUnknownImageFileFormat];
    }
    // ===== Error codes related to video encoding/decoding =====
    else if ( ( error >= ErrorUnknownVideoFileFormat ) && ( error <= ErrorFailedVideoDecoding ) )
    {
        errorDesc = videoCodingErrorNames[error - ErrorUnknownVideoFileFormat];
    }
    // ===== Error codes related to scripting engine =====
    else if ( ( error >= ErrorInitializingScripting ) && ( error <= ErrorNoEntryPointFound ) )
    {
        errorDesc = scriptingEngineErrorNames[error - ErrorInitializingScripting];
    }
    // ===== Error codes related to plug-in's engine =====
    else if ( ( error >= ErrorPluginNotFound ) && ( error <= ErrorFailedPluginInstantiation ) )
    {
        errorDesc = pluginEngineErrorNames[error - ErrorPluginNotFound];
    }

    if ( errorDesc != 0 )
    {
        ret = XStringAlloc( errorDesc );
    }
    else
    {
        char buffer[48];

        sprintf( buffer, "Unknown error code: %d", error );
        ret = XStringAlloc( buffer );
    }

    return ret;
}
