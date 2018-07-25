/*
    Imaging effects library of Computer Vision Sandbox

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
#ifndef CVS_XIMAGING_EFFECTS_H
#define CVS_XIMAGING_EFFECTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <xtypes.h>
#include <ximage.h>
#include <ximaging.h>

    // Perform pixellation of the specified image
XErrorCode ImagePixellate( ximage* src, uint8_t pixelWidth, uint8_t pixelHeight );
// Perform jittering on the image
XErrorCode ImageJitter( ximage* src, uint8_t radius );
// Create image with emboss effect
XErrorCode EmbossImage( const ximage* src, ximage* dst, float azimuth, float elevation, float depth );
// Create image with effect of light dropped at the image surface from the specified direction
XErrorCode ImageDropLight( const ximage* src, ximage* dst, float azimuth, float elevation, float depth );
// Reduce saturation level in a color image (change is in [0, 100] range - 0: no changes, 100: grayscale)
XErrorCode ReduceSaturation( const ximage* src, uint8_t change );
// Increase saturation level in a color image (change is in [0, 100] range - 0: no changes, 100: max increase).
XErrorCode IncreaseSaturation( const ximage* src, uint8_t change );
// Set hue and saturation of all pixels to the specified values
XErrorCode ColorizeImage( ximage* src, uint16_t hue, float saturation );
// Shift hue of all pixels by the specified value, which looks like hue rotation
XErrorCode RotateImageHue( ximage* src, uint16_t hueAngle );
// Applies simple oil painting effect to the specified image
XErrorCode OilPainting( const ximage* src, ximage* dst, uint8_t radius );
// Create vignetting effect on the specified image
XErrorCode MakeVignetteImage( ximage* src, float startWidthFactor, float endWidthFactor, bool decreaseBrightness, bool decreaseSaturation );

#ifdef __cplusplus
}
#endif

#endif // CVS_XIMAGING_EFFECTS_H
