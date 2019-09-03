/*
    Imaging library of Computer Vision Sandbox

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
#ifndef CVS_XIMAGING_H
#define CVS_XIMAGING_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <xtypes.h>
#include <ximage.h>
#include <xhistogram.h>

// Structure defining HSL components
typedef struct _hsl
{
    uint16_t Hue;        // [0, 359]
    float    Saturation; // [0, 1]
    float    Luminance;  // [0, 1]
}
xhsl;

// Structure defining HSV components
typedef struct _hsv
{
    uint16_t Hue;        // [0, 359]
    float    Saturation; // [0, 1]
    float    Value;      // [0, 1]
}
xhsv;

// ===== Image drawing functions =====

// Draw line between the specified points (all coordinates are inclusive)
XErrorCode XDrawingLine( ximage* image, int32_t x1, int32_t y1, int32_t x2, int32_t y2, xargb color );
// Draw rectangle (all coordinates are inclusive)
XErrorCode XDrawingRectangle( ximage* image, int32_t x1, int32_t y1, int32_t x2, int32_t y2, xargb color );
// Draw circle
XErrorCode XDrawingCircle( ximage* image, int32_t xc, int32_t yc, int32_t r, xargb color );
// Draw ellipse
XErrorCode XDrawingEllipse( ximage* image, int32_t xc, int32_t yc, int32_t rx, int32_t ry, xargb color );
// Draw one image on another at the specified location
XErrorCode XDrawingImage( ximage* image, const ximage* imageToDraw, int32_t x, int32_t y );
// Fill rectangle with alpha blending (all coordinates are inclusive)
XErrorCode XDrawingBlendRectangle( ximage* image, int32_t x1, int32_t y1, int32_t x2, int32_t y2, xargb color );
// Fill frame with alpha blending (all coordinates are inclusive)
XErrorCode XDrawingBlendFrame( ximage* image, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t xThickness, uint32_t yThickness, xargb color );
// Fill circle (with alpha blending) in the specified image with the specfied color
XErrorCode XDrawingBlendCircle( ximage* image, int32_t xc, int32_t yc, int32_t r, xargb color );
// Fill ring (with alpha blending) in the specified image with the specfied color
XErrorCode XDrawingBlendRing( ximage* image, int32_t xc, int32_t yc, int32_t r1, int32_t r2, xargb color );
// Fill pie (with alpha blending) between the specified start/end angles in the specified image
XErrorCode XDrawingBlendPie( ximage* image, int32_t xc, int32_t yc, int32_t r1, int32_t r2, float angleStart, float angleEnd,
                             xargb color, int32_t* buffer, uint32_t* bufferSize );
// Fill convex polygon (with alpha blending)
XErrorCode XDrawingBlendConvexPolygon( ximage* image, const xpoint* points, uint32_t pointsCount, xargb color );
// Fill ellipse (with alpha blending) in the specified image with the specfied color
XErrorCode XDrawingBlendEllipse( ximage* image, int32_t xc, int32_t yc, int32_t rx, int32_t ry, xargb color );
// Fill rectangle without alpha blending (all coordinates are inclusive)
XErrorCode XDrawingFillRectangle( ximage* image, int32_t x1, int32_t y1, int32_t x2, int32_t y2, xargb color );
// Fill entire image with the specfied color
XErrorCode XDrawingFillImage( ximage* image, xargb color );
// Fill image with the specified color according to the mask image
XErrorCode XDrawingMaskedFill( ximage* image, const ximage* mask, int32_t maskX, int32_t maskY, xargb fillColor );
// Fill image with the specified image according to the mask image (both images mush have same pixel format)
XErrorCode XDrawingMaskedImageFill( ximage* image, const ximage* fillImage, const ximage* mask, int32_t maskX, int32_t maskY );

// Draw the specified text on the image at the specified location
XErrorCode XDrawingText( ximage* image, xstring text, int32_t x, int32_t y, xargb color, xargb background, bool addBorder );

// ===== Image processing functions =====

// Removes alpha channel from 32/64 bpp images - copies only color data
XErrorCode RemoveAlphaChannel( const ximage* src, ximage* dst );
// Adds alpha channel to 24/48 bpp images - copies color data and   s alpha values to none transparent
XErrorCode AddAlphaChannel( const ximage* src, ximage* dst );
// Pre-multiply RGB values of 32 bpp image with alpha channel
XErrorCode PremultiplyAlphaChannel( ximage* image );
// Undo pre-multiplication of RGB values in 32 bpp image with alpha channel
XErrorCode UndoPremultiplyAlphaChannel( ximage* image );
// Converts source color image into grayscale
XErrorCode ColorToGrayscale( const ximage* src, ximage* dst );
// Converts source grayscale image to 24/32 bpp color image
XErrorCode GrayscaleToColor( const ximage* src, ximage* dst );
// Changes color image to grayscale but keeps its original pixel format (24/32 bpp RGB(A))
XErrorCode DesaturateColorImage( ximage* src );
// Converts source 1 bpp binary image into grayscale
XErrorCode BinaryToGrayscale( const ximage* src, ximage* dst );
// Converts source indexed image to color 32 bpp image
XErrorCode IndexedToColor( const ximage* src, ximage* dst );
// Applies threshold to the image's data (>= threshold)
XErrorCode ThresholdImage( ximage* src, uint16_t threshold );
// Inverts the specified image
XErrorCode InvertImage( ximage* src );
// Re-map values of RGB planes in 24/32 bpp color image
XErrorCode ColorRemapping( ximage* src, const uint8_t* redMap, const uint8_t* greenMap, const uint8_t* blueMap );
// Re-map values of 8bpp grayscale image
XErrorCode GrayscaleRemapping( ximage* src, const uint8_t* map );
// Re-map 8bpp grayscale color image to 24 bpp color image
XErrorCode GrayscaleRemappingToRGB( const ximage* src, ximage* dst, const uint8_t* redMap, const uint8_t* greenMap, const uint8_t* blueMap );
// Swap specified color channels in a RGB(A) image
XErrorCode SwapRGBChannels( ximage* src, uint32_t channel1, uint32_t channel2 );
// Swap Red and Blue components in RGB (RGBA) image
XErrorCode SwapRedBlue( ximage* src );
// Swap Red and Blue components in RGB (RGBA) image - put result to destination image
XErrorCode SwapRedBlueCopy( const ximage* src, ximage* dst );
// Performs gray world normalization filter. It calculate first mean values of each RGB channel and a global mean.
// The it updates all pixel value multiplying them with next coefficients: mean/redMean, mean/greenMean, mean/blueMean
XErrorCode GrayWorldNormalization( ximage* src );
// Contrast stretching image processing filter
XErrorCode ContrastStretching( ximage* src );
// Histogram equalization image processing filter
XErrorCode HistogramEqualization( ximage* src );
// Re-color 8bpp grayscale image into 24 bpp color image by mapping grayscale values to gradient between the specified two colors
XErrorCode GradientGrayscaleReColoring( const ximage* src, ximage* dst, xargb startColor, xargb endColor );
// Re-color 8bpp grayscale image into 24 bpp color image by mapping grayscale values to two gradients between the specified three colors
XErrorCode GradientGrayscaleReColoring2( const ximage* src, ximage* dst, xargb startColor, xargb middleColor, xargb endColor, uint8_t threshold );
// Re-color 8bpp grayscale image into 24 bpp color image by mapping grayscale values to four gradients between the specified five colors
XErrorCode GradientGrayscaleReColoring4( const ximage* src, ximage* dst,
                xargb startColor, xargb middleColor1, xargb middleColor2, xargb middleColor3, xargb endColor,
                uint8_t threshold1, uint8_t threshold2, uint8_t threshold3 );
// Convert grayscale image into color image by applying heat color map
XErrorCode GrayscaleToHeatGradient( const ximage* src, ximage* dst );

// Levels linear filter for 8 bpp grayscale images
XErrorCode LevelsLinearGrayscale( ximage* src, uint8_t inMin, uint8_t inMax, uint8_t outMin, uint8_t outMax );
// Levels linear filter for 24/32 bpp color images
XErrorCode LevelsLinear( ximage* src,
    uint8_t inRedMin,   uint8_t inRedMax,   uint8_t outRedMin,   uint8_t outRedMax,
    uint8_t inGreenMin, uint8_t inGreenMax, uint8_t outGreenMin, uint8_t outGreenMax,
    uint8_t inBlueMin,  uint8_t inBlueMax,  uint8_t outBlueMin,  uint8_t outBlueMax );

// Extracts specified ARGB channel from the source image
XErrorCode ExtractRGBChannel( const ximage* src, ximage* dst, uint32_t channelIndex );
// Replaces specified ARGB channel in the destination image
XErrorCode ReplaceRGBChannel( ximage* dst, const ximage* channel, uint32_t channelIndex );
// Extracts specified nRGB channel from the source image
XErrorCode ExtractNRGBChannel( const ximage* src, ximage* dst, uint32_t channelIndex );

// Remove colors outside/inside of the specified range
XErrorCode ColorFiltering( ximage* src, uint8_t minRed, uint8_t maxRed, uint8_t minGreen, uint8_t maxGreen,
                           uint8_t minBlue, uint8_t maxBlue, bool fillOutside, xargb fillColor );
// Filters colors outside/inside certain distance from a sample color (0 - Euclidean, 1 - Manhattan)
XErrorCode ColorFilteringByDistance( ximage* src, xargb sampleColor, uint16_t maxDistance, uint8_t distanceType, bool fillOutside, xargb fillColor );

// Remove colors outside/inside of the specified HSL range
XErrorCode HslColorFiltering( ximage* src, xhsl minValues, xhsl maxValues, bool fillOutside, xargb fillColor );
// Remove colors outside/inside of the specified HSV range
XErrorCode HsvColorFiltering( ximage* src, xhsv minValues, xhsv maxValues, bool fillOutside, xargb fillColor );

// ===== 2 source image processing routines =====

// Apply mask to an image by setting its pixels to fill color if corresponding pixels of the mask have 0 value
// and fillOnZero is set to xtrue. If fillOnZero is set to xfalse, then filling happens if mask has non zero value.
XErrorCode MaskImage( ximage* image, const ximage* mask, xargb fillColor, bool fillOnZero );
// Merge two images by applying MAX operator for every pair of pixels (result is put back to image1)
XErrorCode MergeImages( ximage* image1, const ximage* image2 );
// Intersect two images by applying MIN operator for every pair of pixels (result is put back to image1)
XErrorCode IntersectImages( ximage* image1, const ximage* image2 );
// Move image1 towards image2 by the specified step size
XErrorCode MoveTowardsImages( ximage* image1, const ximage* image2, uint8_t stepSize );
// Fade one image into another by calculating: image1 * factor + image2 * (1.0 - factor), where factor is in [0, 1] range (result is put back to image1)
XErrorCode FadeImages( ximage* image1, const ximage* image2, float factor );
// Add two images; image2 is multiplied by the specified factor which is in [0, 1] range (result is put back to image1)
XErrorCode AddImages( ximage* image1, const ximage* image2, float factor2 );
// Subtract two images; image2 is multiplied by the specified factor which is in [0, 1] range and then
// subtracted from image1 (result is put back to image1)
XErrorCode SubtractImages( ximage* image1, const ximage* image2, float factor2 );
// Callculates images difference: image1 = abs( image1 - image2 )
XErrorCode DiffImages( ximage* image1, const ximage* image2 );
// Callculates thresholded images difference: image1 = ( abs( image1 - image2 ) >= threshold ) ? hiColor : lowColor
XErrorCode DiffImagesThresholded( ximage* image1, const ximage* image2, int16_t threshold, uint32_t* diffPixels, xargb hiColor, xargb lowColor );

// Blend modes supported by the BlendImages() function =====
// as described on http://en.wikipedia.org/wiki/Blend_modes#Multiply_and_Screen
enum
{
    BlendMode_Multiply   = 0,
    BlendMode_Screen     = 1,
    BlendMode_Overlay    = 2,
    BlendMode_ColorDodge = 3,
    BlendMode_ColorBurn  = 4,
};
typedef uint8_t XBlendMode;

// Blends two image using the specified blend mode. image1 is treated as top layer and image2 is base as top layer.
XErrorCode BlendImages( ximage* image1, const ximage* image2, XBlendMode blendMode );

// ===== Mathematical morphology =====

// ===== Types of structuring element for some math morphology operators  =====
enum
{
    SEType_Square  = 0,
    SEType_Circle  = 1,
    SEType_Diamond = 2
};
typedef uint8_t XStructuringElementType;

// ===== Different modes of Hit-and-Miss morphological operator =====
enum
{
    HMMode_HitAndMiss = 0, /* on match pixel is set to white, otherwise to black    */
    HMMode_Thinning   = 1, /* on match pixel is set to black, otherwise not changed */
    HMMode_Thickening = 2  /* on match pixel is set to white, otherwise not changed */
};
typedef uint8_t XHitAndMissMode;

// 3x3 erosion filter for grayscale images containing only black (0) and white (255) pixels
XErrorCode BinaryErosion3x3( const ximage* src, ximage* dst );
// 3x3 erosion filter with square structuring element
XErrorCode Erosion3x3( const ximage* src, ximage* dst );
// 3x3 dilatation filter for grayscale images containing only black (0) and white (255) pixels
XErrorCode BinaryDilatation3x3( const ximage* src, ximage* dst );
// 3x3 dilatation filter with square structuring element
XErrorCode Dilatation3x3( const ximage* src, ximage* dst );

// Applies erosion morphological operator to the specified image.
//
// seSize is in [3, 51] range and must be odd value
// se (structuring element) is array of seSize*seSize (1 is for element to use, 0 - to ignore)
//
XErrorCode Erosion( const ximage* src, ximage* dst, int8_t* se, uint32_t seSize );

// Applies dilatation morphological operator to the specified image.
//
// seSize is in [3, 51] range and must be odd value
// se (structuring element) is array of seSize*seSize (1 is for element to use, 0 - to ignore)
//
XErrorCode Dilatation( const ximage* src, ximage* dst, int8_t* se, uint32_t seSize );

// Applies hit-and-miss morphological operator to the specified image.
//
// seSize is in [3, 51] range and must be odd value
// se (structuring element) is array of seSize*seSize (1 is for foreground/object, 0 - background, -1 - to ignore)
// mode is the Hit-and-Miss to use
//
XErrorCode HitAndMiss( const ximage* src, ximage* dst, int8_t* se, uint32_t seSize, XHitAndMissMode mode );

// Erode horizontal edges in grayscale images - pixels, which are not connected to 3 neighbours above or below
XErrorCode ErodeHorizontalEdges( const ximage* src, ximage* dst );

// Erode vertical edges in grayscale images - pixels, which are not connected to 3 neighbours on the left or right
XErrorCode ErodeVerticalEdges( const ximage* src, ximage* dst );

// Prepare structuring element of the specified type
XErrorCode FillMorphologicalStructuringElement( int8_t* se, uint32_t seSize, XStructuringElementType type );

// Calculates distance transformation of a binary image, which represents a distance map -
// shortest distance from non-background pixel to object's edge.
XErrorCode DistanceTransformation( const ximage* src, ximage* dst );

// Calculates distance transformation of background in a binary image, which represents a distance map -
// shortest distance from background pixel to an object.
XErrorCode BackgroundDistanceTransformation( const ximage* src, ximage* dst );

// Removes specified amount of objects' edges in a segmented grayscale image
XErrorCode ObjectsThinning( ximage* src, ximage* tempDistanceMap, uint16_t thinningAmount );

// Grow objects' edges by the specified amount in a segmented grayscale image
XErrorCode ObjectsThickening( ximage* src, ximage* tempDistanceMap, uint16_t growingAmount );

// Keeps only specified thickness of objects edges removing anything esle inside (works on segmented grayscale images)
XErrorCode ObjectsEdges( ximage* src, ximage* tempDistanceMap, uint16_t edgeThickness );

// Similar to object edges, but produces outline instead, which is around the object (not the edges of the object itself)
XErrorCode ObjectsOutline( ximage* src, ximage* tempDistanceMap, uint16_t outlineThickness, uint16_t outlineGap );


// === Convolution ===

// Perform convolution on the specified image.
//
// kernel       is an array of kernelSize*kernelSize size (pixel weights)
// kernelSize   is in [3, 51] range, must be odd value
// processAlpha specifies if alpha channel should be processed for 32 bpp images or not
//
XErrorCode Convolution( const ximage* src, ximage* dst, const float* kernel, uint32_t kernelSize, bool processAlpha );

// Perform separable convolution on the specified image (first it does processing with horizontal kernel keeping result
// in temporary image and then it does processing with vertical kernel)
//
// hKernel      horizontal kernel is an array of kernelSize elements
// hKernel      vertical kernel -/-
// kernelSize   is in [3, 51] range, must be odd value
//
XErrorCode SeparableConvolution( const ximage* src, ximage* dst, ximage* tempImage,
                                 const float* hKernel, const float* vKernel, uint32_t kernelSize );


// ===== Different modes of handling image borders while doing convolution =====
enum
{
    BHMode_Extend = 0,
    BHMode_Wrap   = 1,
    BHMode_Zeros  = 2,
    BHMode_Crop   = 3,
    BHMode_Invalid
};
typedef uint8_t XConvolutionBorderHandlingMode;

// Perform convolution on the specified image.
//
// kernel     is an array of kernelSize*kernelSize size (pixel weights)
// kernelSize is in [3, 51] range, must be odd value
// divisor    is the value used to divide weighted sum of pixels
// offset     is the value to add after dividing
// bhMode     is the mode of handling image borders
//
XErrorCode ConvolutionEx( const ximage* src, ximage* dst, const float* kernel, uint32_t kernelSize, float divisor, float offset, XConvolutionBorderHandlingMode bhMode );


// ===== Adaptive thresholding =====

// Calculate optimal threshold for grayscale image using Otsu algorithm
XErrorCode CalculateOtsuThreshold( const ximage* src, uint16_t* threshold );
// Apply Otsu thresholding to an image
XErrorCode OtsuThresholding( ximage* image );

// ===== Color reduction =====

// Note: All bellow binarization routines require destination image to be "clean" - black pixels only.

// Perform ordered dithering of the specified 8 bpp grayscale image putting result into the specified 1 bpp binary image
XErrorCode BinaryOrderedDithering( const ximage* src, ximage* dst, const uint8_t* thresholdMatrix, uint32_t matrixOrder );
// Perform ordered dithering of the specified 8 bpp grayscale image using 2x2 threshold matrix
XErrorCode BinaryOrderedDithering2( const ximage* src, ximage* dst );
// Perform ordered dithering of the specified 8 bpp grayscale image using 3x3 threshold matrix
XErrorCode BinaryOrderedDithering3( const ximage* src, ximage* dst );
// Perform ordered dithering of the specified 8 bpp grayscale image using 4x4 threshold matrix
XErrorCode BinaryOrderedDithering4( const ximage* src, ximage* dst );
// Perform ordered dithering of the specified 8 bpp grayscale image using 8x8 threshold matrix
XErrorCode BinaryOrderedDithering8( const ximage* src, ximage* dst );

// Perform error diffusion dithering of the specified 8 bpp grayscale image putting result into the specified 1 bpp binary image
XErrorCode BinaryErrorDiffusionDithering( const ximage* src, ximage* dst, ximage* tmp,
                                          int32_t diffusionLinesCount, const int32_t* diffusionLinesLength,
                                          const uint8_t** diffusionLinesCoefficients );
// Perform error diffusion dithering of the specified 8 bpp grayscale image using the Floyd-Steinberg algorithm
XErrorCode FloydSteinbergBinaryDithering( const ximage* src, ximage* dst, ximage* tmp );
// Perform error diffusion dithering of the specified 8 bpp grayscale image using the Burkes algorithm
XErrorCode BurkesBinaryDithering( const ximage* src, ximage* dst, ximage* tmp );
// Perform error diffusion dithering of the specified 8 bpp grayscale image using the Jarvis-Judice-Ninke algorithm
XErrorCode JarvisJudiceNinkeBinaryDithering( const ximage* src, ximage* dst, ximage* tmp );
// Perform error diffusion dithering of the specified 8 bpp grayscale image using the Sierra algorithm
XErrorCode SierraBinaryDithering( const ximage* src, ximage* dst, ximage* tmp );
// Perform error diffusion dithering of the specified 8 bpp grayscale image using the Stucki algorithm
XErrorCode StuckiBinaryDithering( const ximage* src, ximage* dst, ximage* tmp );

// ===== Image smoothing =====

// 3x3 mean filter
XErrorCode Mean3x3( const ximage* src, ximage* dst );
// Calculates mean value of window with size radius * 2 + 1. However, it takes only certain pixels
// for averaging - those, which color is withing the specified distance from the color of the
// window's ceter pixel.
XErrorCode MeanShift( const ximage* src, ximage* dst, uint16_t radius, uint16_t colorDistance );
// Blur image using 5x5 kernel
XErrorCode BlurImage( const ximage* src, ximage* dst );
// Perform Gaussian blur with the specified sigma value and blurring radius
XErrorCode GaussianBlur( const ximage* src, ximage* dst, float sigma, uint8_t radius );
// Perform Gaussian sharpening with the specified sigma value and sharpening radius
XErrorCode GaussianSharpen( const ximage* src, ximage* dst, float sigma, uint8_t radius );

// Calculates Gaussian blur 1D kernel of the specified size using the specified sigma value.
// Kernel must be an allocated array of size: radius * 2 + 1
XErrorCode CreateGaussianBlurKernel1D( float sigma, int radius, float* kernel );
// Calculates Gaussian blur 2D kernel of the specified size using the specified sigma value.
// Kernel must be an allocated array of size: (radius * 2 + 1) * (radius * 2 + 1)
XErrorCode CreateGaussianBlurKernel2D( float sigma, int radius, float* kernel );
// Calculates Gaussian sharpen 2D kernel of the specified size using the specified sigma value.
// Kernel must be an allocated array of size: (radius * 2 + 1) * (radius * 2 + 1)
XErrorCode CreateGaussianSharpenKernel2D( float sigma, int radius, float* kernel );

// ===== Edge detection =====

// ===== Enumeration of simple edge detection techniques =====
enum
{
    EdgeDetector_Difference  = 0,
    EdgeDetector_Homogeneity = 1,
    EdgeDetector_Sobel       = 2,
};
typedef uint8_t XSimpleEdgeDetector;

// Simple edge detector
XErrorCode EdgeDetector( const ximage* src, ximage* dst, XSimpleEdgeDetector type, bool scaleIntensity );

XErrorCode CannyEdgeDetector( const ximage* src, ximage* dst, ximage* tempBlurImage1, ximage* tempBlurImage2, ximage* tempEdgesImage,
                              ximage* gradients, ximage* orientations, const float* blurKernel, uint32_t kernelSize,
                              uint16_t lowThreshold, uint16_t highThreshold );

// ===== Image transformation =====

// Mirror the specified image over X and/or Y axis
XErrorCode MirrorImage( ximage* src, bool xMirror, bool yMirror );
// Resize image using nearest neighbour "interpolation"
XErrorCode ResizeImageNearestNeighbor( const ximage* src, ximage* dst );
// Resize image using bilinear interpolation
XErrorCode ResizeImageBilinear( const ximage* src, ximage* dst );
// Rotate image counter clockwise by 90 degrees
XErrorCode RotateImage90( const ximage* src, ximage* dst );
// Rotate image counter clockwise by 270 degrees
XErrorCode RotateImage270( const ximage* src, ximage* dst );

// Shift image in X/Y directions by the specified number of pixels
XErrorCode ShiftImage( ximage* src, int dx, int dy, bool fillOpenSpace, xargb fillColor );

// Rotate image by the specified angle using bilinear interpolation
XErrorCode RotateImageBilinear( const ximage* src, ximage* dst, float angle, xargb fillColor );

// Calculate image size for a rotated image, so it fit into the new size
XErrorCode CalculateRotatedImageSize( int32_t width, int32_t height, float angle, int32_t* newWidth, int32_t* newHeight );

// Embed source image into target using the specified 4 quadrilateral points
XErrorCode EmbedQuadrilateral( ximage* target, const ximage* source, const xpoint* targetQuadrilateral, bool interpolate );
// Extract specified quadrilateral from source image into target (the target's image size specifies the result size)
XErrorCode ExtractQuadrilateral( const ximage* source, ximage* target, const xpoint* sourceQuadrilateral, bool interpolate );


// ===== ???? =====

// Add uniform additive noise to the specified image
XErrorCode UniformAdditiveNoise( ximage* src, uint32_t seed, uint8_t amplitude );
// Add salt-and-pepper noise to the specified image
XErrorCode SaltAndPepperNoise( ximage* src, uint32_t seed, float noiseAmount, uint8_t pepperValue, uint8_t saltValue );
// Performs simple posterization on the given image - replaces ranges of colors with a single value
XErrorCode SimplePosterization( ximage* src, uint8_t interval, XRangePoint fillType );

// Fill horizontal gaps between objects in a thresholded grayscale image
XErrorCode HorizontalRunLengthSmoothing( ximage* src, uint16_t maxGap );
// Fill vertical gaps between objects in a thresholded grayscale image
XErrorCode VerticalRunLengthSmoothing( ximage* src, uint16_t maxGap );

// ===== Different color effects =====

// Turn RGB color image into sepia colors (brown colors which create effect of an old picture)
XErrorCode MakeSepiaImage( ximage* src );
// Set hue value of all pixels to same value
XErrorCode SetImageHue( ximage* src, uint16_t hue );
// Rotates RGB components, r <- g <- b <- r
XErrorCode RotateRGBChannels( ximage* src );

// ===== Different image effects =====

// Perform pixellation of the specified image
XErrorCode ImagePixellate( ximage* src, uint8_t pixelWidth, uint8_t pixelHeight );

// ===== Calculation of color maps =====

// Calculate linear map for changing pixels' values
XErrorCode CalculateLinearMap( uint8_t* map, uint8_t inMin, uint8_t inMax, uint8_t outMin, uint8_t outMax );
// Calculate linear map for changing pixels' values only within the specified input range
XErrorCode CalculateLinearMapInInputRange( uint8_t* map, uint8_t inMin, uint8_t inMax, uint8_t outMin, uint8_t outMax );
// Calculate linear map which increases/decreases brightness
XErrorCode CalculateBrightnessChangeMap( uint8_t* map, int16_t increaseBrightnessBy );
// Calculate linear map which increases/decreases contrast
XErrorCode CalculateContrastChangeMap( uint8_t* map, int16_t increaseContrastBy );
// Calculate linear map which increases/decreases high/low levels
XErrorCode CalculateColorLevelChangeMap( uint8_t* map, int16_t increaseLowLevelBy, int16_t increaseHighLevelBy );
// Calculate gamma correction map
XErrorCode CalculateGammaCorrectionMap( uint8_t* map, float gamma, bool inverse );
// Calculate "S-curve" map (contrast correction)
XErrorCode CalculateSCurveMap( uint8_t* map, float factor, bool inverse );
// Calculate filtering map - fill values outside/inside the range with the specified value
XErrorCode CalculateFilteringMap( uint8_t* map, uint8_t min, uint8_t max, uint8_t fillValue, bool fillOutsideRange );

// Calculate heat gradient color map
XErrorCode CalculateHeatGradientColorMap( uint8_t* redMap, uint8_t* greenMap, uint8_t* blueMap );

// ===== Image statistics functions =====

// Calculate RGB histogram for 24/32 bpp color image
XErrorCode GetColorImageHistograms( const ximage* image, xhistogram* redHistogram, xhistogram* greenHistogram, xhistogram* blueHistogram );
// Calculate intensity histogram for 8 bpp grayscale image
XErrorCode GetGrayscaleImageHistogram( const ximage* image, xhistogram* histogram );

// ===== Blob counting/processing functions =====

// Build map of disconnected objects and count them (temp label map can be set to NULL)
XErrorCode BcBuildObjectsMap( const ximage* image, ximage* map, uint32_t* objectsCountFound, uint32_t* tempLabelsMap, uint32_t tempLabelsMapSize );
// Build map of disconnected background areas and count them (temp label map can be set to NULL)
XErrorCode BcBuildBackgroundMap( const ximage* image, ximage* map, uint32_t* objectsCountFound, uint32_t* tempLabelsMap, uint32_t tempLabelsMapSize );
// Create colored map (colors are not unique) from the map of disconnected objects
XErrorCode BcDoColorLabeling( const ximage* map, ximage* image );
// Find bounding rectangles of all objects. Rectangles array must be preallocated for objectsCount items.
XErrorCode BcGetObjectsRectangles( const ximage* map, uint32_t objectsCount, xrect* rectangles );
// Find areas of all objects (number of pixels in every object). Areas array must be preallocated for objectsCount values.
XErrorCode BcGetObjectsArea( const ximage* map, uint32_t objectsCount, uint32_t* areas, uint32_t* totalArea );
// Find bounding rectangles of all objects and their area. Rectangles and areas arrays must be preallocated for objectsCount items.
XErrorCode BcGetObjectsRectanglesAndArea( const ximage* map, uint32_t objectsCount, xrect* rectangles, uint32_t* areas );
// Fill objects specified by the fill map. Fill map is array of size objectsCount+1, which contains 1 or 0 to indicate if object must be filled or not.
XErrorCode BcFillObjects( ximage* image, const ximage* map, const uint8_t* fillMap, xargb fillColor );
// Fill all object except the one with the specified ID
XErrorCode BcKeepObjectIDOnly( ximage* image, const ximage* map, uint32_t blobId, xargb fillColor );

// --- Fill map for the below functions must be preallocated for objectsCount+1 value

// Initialize fill map, which can be used to fill non edge objects
XErrorCode BcBuildFillMapNonEdgeObjects( int imageWidth, int imageHeight, uint32_t objectsCount, const xrect* rectangles, uint8_t* fillMap, uint32_t* objectsToFillCount );
// Initialize fill map, which can be used to fill non edge objects smaller than the specified size
XErrorCode BcBuildFillMapNonEdgeBySizeObjects( int imageWidth, int imageHeight, uint32_t minWidth, uint32_t minHeight, bool coupleFiltering,
                                               uint32_t objectsCount, const xrect* rectangles, uint8_t* fillMap,
                                               uint32_t* objectsToFillCount, uint32_t* objectsToStayCount );
// Initialize fill map, which can be used to fill non edge objects having area smaller than the specified limit
XErrorCode BcBuildFillMapNonEdgeByAreaObjects( int imageWidth, int imageHeight, uint32_t minArea,
                                               uint32_t objectsCount, const xrect* rectangles, const uint32_t* areas, uint8_t* fillMap,
                                               uint32_t* objectsToFillCount, uint32_t* objectsToStayCount );
// Initialize fill map, which can be used to fill edge objects
XErrorCode BcBuildFillMapEdgeObjects( int imageWidth, int imageHeight, uint32_t objectsCount, const xrect* rectangles, uint8_t* fillMap );
// Initialize fill map, which can be used to fill objects smaller than minimum size or bigger than maximum size.
// If coupled filtering is used, then both with and height must be smaller or bigger.
XErrorCode BcBuildFillMapOutOfSizeObjects( uint32_t minWidth, uint32_t minHeight, uint32_t maxWidth, uint32_t maxHeight, bool coupleFiltering,
                                           uint32_t objectsCount, const xrect* rectangles, uint8_t* fillMap, uint32_t* blobsLeft );
// Initialize fill map, which can be used to fill objects having smaller or bigger area than the specified limit
XErrorCode BcBuildFillMapOutOfAreaObjects( uint32_t minArea, uint32_t maxArea,
                                           uint32_t objectsCount, const uint32_t* areas, uint8_t* fillMap, uint32_t* blobsLeft );

// Tool functions which build map first and then do filling using above functions

// Fill objects not touching image's edge
XErrorCode BcFillNonEdgeObjects( ximage* image, const ximage* map, uint32_t objectsCount, const xrect* rectangles, xargb fillColor, uint32_t* objectsFilledCount );
// Fill objects touching image's edge
XErrorCode BcFillEdgeObjects( ximage* image, const ximage* map, uint32_t objectsCount, const xrect* rectangles, xargb fillColor );
// Fill objects smaller or bigger than the specified min/max sizes
XErrorCode BcFilterBySizeObjects( ximage* image, const ximage* map, uint32_t objectsCount, const xrect* rectangles, xargb fillColor,
                                  uint32_t minWidth, uint32_t minHeight, uint32_t maxWidth, uint32_t maxHeight, bool coupleFiltering, uint32_t* blobsLeft );

// Collect object's points on its left/right edges. Each edge array must be at least object's height in size.
XErrorCode BcGetObjectLeftRightEdges( const ximage* blobsMap, uint32_t blobId, xrect blobRect, uint32_t edgeArraysSize, xpoint* leftEdge, xpoint* rightEdge, uint32_t* avgThickness );
// Collect object's points on its top/bottom edges. Each edge array must be at least object's width in size.
XErrorCode BcGetObjectTopBottomEdges( const ximage* blobsMap, uint32_t blobId, xrect blobRect, uint32_t edgeArraysSize, xpoint* topEdge, xpoint* bottomEdge, uint32_t* avgThickness );
// Collect object's points. The edge array must be at least ( width + height ) * 2 in size. First left edge's points are put into the
// array, then right edge's, finally top/bottom edge points are mixed (to make sure they don't repeat left/right edges).
XErrorCode BcGetObjectEdgePoints( const ximage* blobsMap, uint32_t blobId, xrect blobRect, uint32_t edgeArraysSize, xpoint* edgePoints, uint32_t* edgePointsCount, uint32_t* avgVerticalThickness );

// ===== Shape checking/analyzing functions =====

// Find quadrilateral points of the specified point cloud
//
// Relative distortion limit is used to calculate distortion limit as: RelativeDistortionLimit * ( W * H ) / 2,
// where W and H are width and height of the bounding rectangle. Distortion limit is used to reject corners, which
// are too close to the main quadrilateral's line connecting the two furthest points.
//
XErrorCode FindQuadrilateralCorners( const xpoint* points, uint32_t pointsCount, xrect boundingRect, float relDistortionLimit, xpoint* quadPoints, bool* gotFourPoints );

// Check if points fit the convex shape specified by its corners
//
// Mean distance from the shape's sides should not exceed maximum allowed distance, which is calculated as:
// MAX( minAcceptableDistortion, RelativeDistortionLimit * ( RectWidth + RectHeight ) / 2 )
//
// The functions requires some pre-allocated buffers to operate. Work buffer 1 must have cornersCount * 3 number of elements.
// Work buffer 2 must have cornersCount number of elements.
//
XErrorCode CheckPointsFitShape( const xpoint* points, uint32_t pointsCount, xrect pointsRect, const xpoint* shapeCorners, uint32_t cornersCount,
                                float relDistortionLimit, float minAcceptableDistortion, float* workBuffer1, bool* workBuffer2 );

// Extended version: Check if points fit the convex shape specified by its corners
//
// Mean distance from the shape's sides should not exceed maximum allowed distance, which is calculated for each side as:
// MAX( minAcceptableDistortion, RelativeDistortionLimit * MAX ( sideLengthX, sideLengthY ) )
//
// The functions requires some pre-allocated buffers to operate. Work buffer 1 must have cornersCount * 4 number of elements.
// Work buffer 2 must have cornersCount number of elements. Work buffer 3 must have cornersCount * 2 number of elements.
//
XErrorCode CheckPointsFitShapeEx( const xpoint* points, uint32_t pointsCount, const xpoint* shapeCorners, uint32_t cornersCount,
                                  float relDistortionLimit, float minAcceptableDistortion, float* workBuffer1, bool* workBuffer2, uint32_t* workBuffer3 );

// Check if points fit a circle
XErrorCode CheckPointsFitCircle( const xpoint* points, uint32_t pointsCount, xrect pointsRect,
                                 float relDistortionLimit, float minAcceptableDistortion,
                                 xpointf* center, float* radius, float* meanDeviation );

// Check if the specified set of points form a quadrilateral
XErrorCode CheckPointsFitQuadrilateral( const xpoint* points, uint32_t pointsCount, xrect pointsRect,
                                        float relDistortionLimit, float minAcceptableDistortion, xpoint* quadPoints );

// Optimizes convex hull by removing obtuse angles (close to flat) from a shape
XErrorCode OptimizeFlatAngles( const xpoint* srcPoints, uint32_t srcPointsCount, xpoint* dstPoints, uint32_t* dstPointsCount, float maxAngleToKeep );

// ===== Color space conversion =====

// Convert RGB color to HSL
XErrorCode Rgb2Hsl( const xargb* rgb, xhsl* hsl );
// Convert RGB color to SL part only of HSL (hue is not calculated)
XErrorCode Rgb2Sl( const xargb* rgb, xhsl* hsl );
// Convert HSL color to RGB
XErrorCode Hsl2Rgb( const xhsl* hsl, xargb* rgb );

// Convert RGB color to HSV
XErrorCode Rgb2Hsv( const xargb* rgb, xhsv* hsv );
// Convert HSV color to RGB
XErrorCode Hsv2Rgb( const xhsv* hsv, xargb* rgb );


#ifdef __cplusplus
}
#endif

#endif // CVS_XIMAGING_H
