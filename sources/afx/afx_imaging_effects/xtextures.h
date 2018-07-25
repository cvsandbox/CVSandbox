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
#ifndef CVS_XTEXTURES_H
#define CVS_XTEXTURES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <xtypes.h>
#include <ximage.h>

/* Allocates texture image - 8bpp grayscale image of the specified size */
XErrorCode XImageAllocateTexture( int32_t width, int32_t height, ximage** texture );

/* Allocates texture image - 8bpp grayscale image of the specified size (image memory is not initialized with zeros) */
XErrorCode XImageAllocateTextureRaw( int32_t width, int32_t height, ximage** texture );

/* Apply the specified texture to the image.
 *
 * texture          Texture to apply to the specified image.
 * image            Image to apply texture to.
 * amountToKeep     Amount of source image to keep for sure, [0, 1].
 * textureBaseLevel Level of the texture which would correspond to 1.0 multiplier. If this level
 *                  is set to 255, then the texture can only darken an image (or keep it as is for
 *                  for texture 255 values). If this value is set smaller, then the texture can
 *                  make image brighter for texture values greater than this level (since multiplier
 *                  is "textureValue/textureBaseLevel").
 */
XErrorCode XImageApplyTexture( const ximage* texture, ximage* image, float amountToKeep, uint8_t textureBaseLevel );

// ===== Texture generation functions =====

/* Generate textile looking texture.
 *
 * texture          8bpp grayscale image to fill with the texture.
 * randNumber       Random number to create randomness of the texture.
 * stitchSize       Textile's "stitch" size in pixels.
 * stitchOffset     Offset of the stitch in the texture (phase).
 */
XErrorCode GenerateTextileTexture( ximage* texture, uint16_t randNumber, uint8_t stitchSize, uint8_t stitchOffset );

/* Generate marble looking texture.
 *
 * texture          8bpp grayscale image to fill with the texture.
 * randNumber       Random number to create randomness of the texture.
 * ??
 */
XErrorCode GenerateMarbleTexture( ximage* texture, uint16_t randNumber, float xPeriod, float yPeriod );

/* Generate clouds looking texture.
 *
 * texture          8bpp grayscale image to fill with the texture.
 * randNumber       Random number to create randomness of the texture.
 */
XErrorCode GenerateCloudsTexture( ximage* texture, uint16_t randNumber );

/* Generate grain texture.
 *
 * texture          8bpp grayscale image to fill with the texture.
 * randNumber       Random number to create randomness of the texture.
 * spacing          Maximum distance between grain lines. The actual distance between lines is a random number in the [1, spacing] range.
 * density          Specifies grain density, [0, 1].
 * isVertical       Specifies if the generated texture shall contain vertical or horizontal grain.
 */
XErrorCode GenerateGrainTexture( ximage* texture, uint16_t randNumber, uint16_t spacing, float density, bool isVertical );

/* ===== Frames and Borders textures =====
   ---------------------------------------
 */

/* Generate fuzzy border texture.
 *
 * texture          8bpp grayscale image to fill with the texture.
 * randNumber       Random number to create randomness of the texture.
 * borderWidth      Width of the border to create.
 * gradientWidth    Gradient width in the border.
 * waviness         Waviness factor of the border.
 */
XErrorCode GenerateFuzzyBorderTexture( ximage* texture, uint16_t randNumber, uint16_t borderWidth, uint16_t gradientWidth, uint16_t waviness );

/* Generate rounded border texture.
 *
 * texture          8bpp grayscale image to fill with the texture.
 * borderWidth      Width of the border to create.
 * xRoundness       X radius of the rounded area.
 * yRoundness       Y radius of the rounded area.
 * xRoundnessShift  Shift of the rounded area in X direction.
 * yRoundnessShift  Shift of the rounded area in Y direction.
 * addBloor         Bloor border or not.
 */
XErrorCode GenerateRoundedBorderTexture( ximage* texture, uint16_t borderWidth, uint16_t xRoundness, uint16_t yRoundness, uint16_t xRoundnessShift, uint16_t yRoundnessShift, bool addBloor );

#ifdef __cplusplus
}
#endif

#endif // CVS_XTEXTURES_H
