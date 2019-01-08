/*
    Computer vision library of Computer Vision Sandbox

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

#pragma once
#ifndef CVS_XVISION_H
#define CVS_XVISION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <xtypes.h>
#include <ximage.h>

// Build integral image for the specified image (RGB channel must be specified for color images)
XErrorCode BuildIntegralImage( const ximage* image, ximage* integralImage, XRGBComponent rgbChannel );
// Build integral and squared integral images for the specified image (RGB channel must be specified for color images)
XErrorCode BuildIntegralImage2( const ximage* image, ximage* integralImage, ximage* sqIntegralImage, XRGBComponent rgbChannel );

// ===== Detection and tracking of square binary glyphs =====

// Information about detected glyph
typedef struct _detectedGlyphInfo
{
    xrect  BoundingRect;
    xpoint Quadrilateral[4];
    char*  Code;
}
DetectedGlyphInfo;

// Glyph detection context containing information about detected glyphs, as well as internal data structures required for detection
typedef struct _glyphDetectionContext
{
    void*              Data;
    uint32_t           DetectedGlyphsCount;
    DetectedGlyphInfo* DetectedGlyphs;
}
GlyphDetectionContext;

// Free glyph detection context allocated by glyph detection functions
void FreeGlyphDetectionContext( GlyphDetectionContext** pContext );

// Find glyphs in the specified image. Context is allocated and can be reused by subsequent call.
XErrorCode FindGlyphs( const ximage* image, uint32_t glyphSize, uint32_t maxGlyphs, GlyphDetectionContext** pContext );


// ===== Detection and recognition of bar codes =====

// Information about detected 1d bar code
typedef struct _detectedBarcodeInfo
{
    xrect  BoundingRect;
    xpoint Quadrilateral[4];
    bool   IsVertical;
}
DetectedBarcodeInfo;

// Bar code detection context containing information about detected bar codes, as well as internal data structures required for detection
typedef struct _barcodeDetectionContext
{
    void*                Data;
    uint32_t             DetectedBarcodesCount;
    DetectedBarcodeInfo* DetectedBarcodes;
}
BarcodeDetectionContext;

// Free bar code detection context allocated by bar code detection functions
void FreeBarcodeDetectionContext( BarcodeDetectionContext** pContext );

// Find 1D linear bar codes in the specified image. Context is allocated and can be reused by subsequent call.
XErrorCode FindBarcodes( const ximage* image, uint32_t maxBarcodes, BarcodeDetectionContext** pContext );

#ifdef __cplusplus
}
#endif

#endif // CVS_XIMAGING_H
