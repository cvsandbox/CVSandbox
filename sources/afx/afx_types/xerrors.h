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

#pragma once
#ifndef CVS_XERRORS_H
#define CVS_XERRORS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// WARNING: don't EVER change errors' values without updating xerros.c
typedef int32_t XErrorCode;

// ===== Standard error codes =====
static const XErrorCode SuccessCode                         = 0;
static const XErrorCode ErrorFailed                         = 1;  // General failure code
static const XErrorCode ErrorNullParameter                  = 2;  // Failed because of some parameters are null pointers
static const XErrorCode ErrorOutOfMemory                    = 3;  // Out of memory failure
static const XErrorCode ErrorTooSmallBuffer                 = 4;  // Amount of provided memory is not enough (given buffer is too small)
static const XErrorCode ErrorArgumentOutOfRange             = 5;  // One of the specified parameters is out of range
static const XErrorCode ErrorInvalidArgument                = 6;  // One of the specified paramters is not valid
static const XErrorCode ErrorNotImplemented                 = 7;  // The invoked API is not implemented
static const XErrorCode ErrorInvalidProperty                = 8;  // The property (property index) is not valid or does not exist
static const XErrorCode ErrorUnsupportedInterface           = 9;  // The interface in subject is not supported
static const XErrorCode ErrorFailedLoadingModule            = 10; // Failed loading specified module for some reason
static const XErrorCode ErrorInitializationFailed           = 11; // Failed to initialize (module/routine/etc)
static const XErrorCode ErrorIOFailure                      = 12; // General IO failure
static const XErrorCode ErrorInvalidConfiguration           = 13; // Failure due to invalid/missing configuration
static const XErrorCode ErrorCannotSetPropertyWhileRunning  = 14; // Cannot set the property; while object/plug-in is running
static const XErrorCode ErrorConnectionFailed               = 15; // Failed connecting to device/host/etc
static const XErrorCode ErrorNotConnected                   = 16; // Failed due to missing connection (the requested operation requires connection; which was not done or failed before)
static const XErrorCode ErrorReadOnlyProperty               = 17; // The property is read only
static const XErrorCode ErrorNotIndexedProperty             = 18; // The property is not indexed (not an array type)
static const XErrorCode ErrorInvalidArgumentCount           = 19; // Invalid number of arguments passed
static const XErrorCode ErrorInvalidFunction                = 20; // The function (function index) is not valid or does not exist
static const XErrorCode ErrorConfigurationNotSupported      = 21; // Configuration is not supported by device/object/whoever
static const XErrorCode ErrorDeivceNotReady                 = 22; // Device (whatever it might be) is not ready for the requested action
static const XErrorCode ErrorEOF                            = 23; // End of file/stream reached

// ===== Error code used in data conversion (by variant related functions or others) =====
static const XErrorCode ErrorInvalidType                    = 10001; // Specified type is not valid
static const XErrorCode ErrorIncompatibleTypes              = 10002; // Specified types are not compatible
static const XErrorCode ErrorInvalidFormat                  = 10003;
static const XErrorCode ErrorLimitsExceeded                 = 10004;
static const XErrorCode ErrorIndexOutOfBounds               = 10005; // Specified index is out array's bounds
static const XErrorCode ErrorInvalidArraySize               = 10006; // Size of the provided array is not valid

// ===== Error codes related to image processing functions =====
static const XErrorCode ErrorUnsupportedPixelFormat         = 11001;
static const XErrorCode ErrorImageParametersMismatch        = 11002;
static const XErrorCode ErrorImageIsTooSmall                = 11003;
static const XErrorCode ErrorImageIsTooBig                  = 11004;
static const XErrorCode ErrorInvalidImageSize               = 11005;
static const XErrorCode ErrorImagePaletteIsMissing          = 11006;

// ===== Error codes related to image encoding/decoding =====
static const XErrorCode ErrorUnknownImageFileFormat         = 12001;
static const XErrorCode ErrorFailedImageDecoding            = 12002;
static const XErrorCode ErrorFailedImageEncoding            = 12003;

// ===== Error codes related to video encoding/decoding =====
static const XErrorCode ErrorUnknownVideoFileFormat         = 12101;
static const XErrorCode ErrorCodecInitFailure               = 12102;
static const XErrorCode ErrorCodecNotFound                  = 12103;
static const XErrorCode ErrorVideoStreamMissing             = 12104;
static const XErrorCode ErrorFailedVideoDecoding            = 12105;

// ===== Error codes related to scripting engine =====
static const XErrorCode ErrorInitializingScripting          = 13001;
static const XErrorCode ErrorFailedLoadingScript            = 13002;
static const XErrorCode ErrorFailedRunningScript            = 13003;
static const XErrorCode ErrorScriptingNotInitialized        = 13004;
static const XErrorCode ErrorNoScriptLoaded                 = 13005;
static const XErrorCode ErrorNoEntryPointFound              = 13006;

// ===== Error codes related to plug-in's engine =====
static const XErrorCode ErrorPluginNotFound                 = 14001;
static const XErrorCode ErrorFailedPluginInstantiation      = 14002;

#ifdef __cplusplus
}
#endif

#endif // CVS_XERRORS_H
