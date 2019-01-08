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

#include "xvision.h"
#include <ximaging.h>
#include <memory.h>

// Size of glyph's cells when extracting them from quadrilaterals
#define GLYPH_CELL_SIZE (14)
// Size of glyph's border to ignore - only the center square is scanned
#define GLYPH_CELL_BORDER_TO_IGNORE (2)
// Size of glyph's cells, which will be actually checked
#define GLYPH_CELL_SIZE_TO_CHECK  (GLYPH_CELL_SIZE - GLYPH_CELL_BORDER_TO_IGNORE - GLYPH_CELL_BORDER_TO_IGNORE)

// Maximum number of white pixels to tolerate on glyph's edge cell. Note: above settings affect this value.
#define GLYPH_EDGE_CELL_MAX_LEVEL (30)

// Minimum number of white pixels to accept glyph's cell as white
#define GLYPH_CELL_MIN_WHITE_LEVEL (75)
// Maximum number of black pixels to accept glyph's cell as black
#define GLYPH_CELL_MAX_BLACK_LEVEL (25)

// Threshold value to apply to the image containing objects' edges
#define EDGES_THRESHOLD (40)

// Minimum width/height of blobs to analyse for glyphs
#define MIN_BLOB_WIDTH  (32)
#define MIN_BLOB_HEIGHT (32)

// Minimum acceptable average thickness of blob to analyse for glyphs
#define MIN_BLOB_AVG_THICKNESS (10)

// Relative distortion limit used with FindQuadrilateralCorners() to find corners of quadrilaterals
#define QUADRILATERAL_REL_DISTORTION_LIMIT (0.1f)

// RelMinimum distortion limit used with CheckPointsToFitShape() to check if points fit quadrilateral's shape
#define SHAPE_REL_DISTORTION_LIMIT (0.025f)
#define SHAPE_MIN_DISTORTION_LIMIT (0.1f)

// Maximum allowed angle between sides of quadrilaterals - used with OptimizeFlatAngles()
#define QUADRILATERAL_MAX_ANGLE_TO_KEEP (160)

// Step size used for calculating average brightness difference along left/right edges
#define EDGE_STEP_SIZE (3)

// Minimum acceptable average difference between pixel values outside quadrilateral and inside
#define MIN_EDGE_CONTRAST (20)

// Local helper functions
static bool CheckPointsFitQuadrilateralHelper( const xpoint* points, uint32_t pointsCount, xrect pointsRect, xpoint* quadPoints );
static int  GetAverageBrightnessDiffOnLeftRightEdges( const xpoint* leftEdgePoints, const xpoint* rightEdgePoints, uint32_t pointsCount, const ximage* grayImage );
static bool RecognizeGlyph( const ximage* glyphImage, uint32_t glyphSize, uint32_t* glyphBuffer, char* glyphString );

// Structure used for sorting blobs by size
typedef struct _idAndSize
{
    uint32_t Id;
    uint32_t Size;
}
IdAndSize;

// Internal data for the glyph detection context
typedef struct _glyphDetectionData
{
    uint32_t   GlyphSize;
    uint32_t   MaxGlyphs;

    ximage*    GrayImage;
    ximage*    EdgeImage;
    ximage*    BlobsMapImage;
    ximage*    GlyphImage;

    uint32_t*  TempLabelsMap;
    uint32_t   MaxObjectsCount;
    xrect*     ObjectsRectangles;
    IdAndSize* ObjectsIdAndSize;
    uint32_t   AllocatedObjectsInfoCount;

    xpoint*    BlobEdgePoints;
    uint32_t   AllocatedEdgePointsCount;

    uint32_t*  GlyphBuffer;
    uint32_t   AllocatedGlyphBufferSize;
}
GlyphDetectionData;

// Helper function to compare elements of the ObjectsIdAndSize array so they could be sorted in descending order (bigger objects go first)
static int CompareObjectSizes( const void* p1, const void* p2 )
{
    return ( ( (IdAndSize*) p2 )->Size ) - ( ( (IdAndSize*) p1 )->Size );
}

void FreeDetectedGlyphsInfo( DetectedGlyphInfo** info, uint32_t count )
{
    if ( ( info != 0 ) && ( *info != 0 ) )
    {
        uint32_t i;

        for ( i = 0; i < count; i++ )
        {
            if ( ( *info )[i].Code != 0 )
            {
                free( ( *info )[i].Code );
            }
        }

        free( *info );
        *info = 0;
    }
}

// Free glyph detection context and all associated data with it
void FreeGlyphDetectionContext( GlyphDetectionContext** pContext )
{
    if ( ( pContext != 0 ) && ( *pContext != 0 ) )
    {
        GlyphDetectionContext* context = *pContext;

        if ( context->Data != 0 )
        {
            GlyphDetectionData* data = (GlyphDetectionData*) context->Data;

            XImageFree( &data->GrayImage );
            XImageFree( &data->EdgeImage );
            XImageFree( &data->BlobsMapImage );
            XImageFree( &data->GlyphImage );

            if ( data->TempLabelsMap != 0 )
            {
                free( data->TempLabelsMap );
            }

            if ( data->ObjectsRectangles != 0 )
            {
                free( data->ObjectsRectangles );
            }

            if ( data->ObjectsIdAndSize != 0 )
            {
                free( data->ObjectsIdAndSize );
            }

            if ( data->BlobEdgePoints != 0 )
            {
                free( data->BlobEdgePoints );
            }

            if ( data->GlyphBuffer != 0 )
            {
                free( data->GlyphBuffer );
            }

            FreeDetectedGlyphsInfo( &context->DetectedGlyphs, data->MaxGlyphs );

            free( context->Data );
        }

        XFree( (void**) pContext );
    }
}

// Allocate glyph detection context and any required internal data structures
static XErrorCode AllocateContext( int32_t imageWidth, int32_t imageHeight, XPixelFormat imageFormat, uint32_t glyphSize, uint32_t maxGlyphs, GlyphDetectionContext** pContext )
{
    XErrorCode             ret     = SuccessCode;
    GlyphDetectionContext* context = *pContext;

    if ( context == 0 )
    {
        context = XCAlloc( 1, sizeof( GlyphDetectionContext ) );

        if ( context == 0 )
        {
            ret = ErrorOutOfMemory;
        }
        else
        {
            *pContext = context;
        }
    }

    if ( ret == SuccessCode )
    {
        GlyphDetectionData* data = (GlyphDetectionData*) context->Data;

        context->DetectedGlyphsCount = 0;

        if ( data == 0 )
        {
            data = (GlyphDetectionData*) calloc( 1, sizeof( GlyphDetectionData ) );

            if ( data == 0 )
            {
                ret = ErrorOutOfMemory;
            }
            else
            {
                context->Data = data;
            }
        }

        // allocate temporary image for grayscale conversion
        if ( ( ret == SuccessCode ) && ( imageFormat != XPixelFormatGrayscale8 ) )
        {
            ret = XImageAllocateRaw( imageWidth, imageHeight, XPixelFormatGrayscale8, &data->GrayImage );
        }

        // allocate temporary image for edges
        if ( ret == SuccessCode )
        {
            ret = XImageAllocateRaw( imageWidth, imageHeight, XPixelFormatGrayscale8, &data->EdgeImage );
        }

        // allocate temporary image for blob counting map
        if ( ret == SuccessCode )
        {
            ret = XImageAllocateRaw( imageWidth, imageHeight, XPixelFormatGrayscale32, &data->BlobsMapImage );
        }

        // allocate temporary image for extracting glyph images
        if ( ret == SuccessCode )
        {
            ret = XImageAllocateRaw( ( glyphSize + 2 ) * GLYPH_CELL_SIZE, ( glyphSize + 2 ) * GLYPH_CELL_SIZE, XPixelFormatGrayscale8, &data->GlyphImage );
        }

        // allocate temporary buffer used by blob counter for labelling objects
        if ( ret == SuccessCode )
        {
            uint32_t maxObjectsCount = ( ( imageWidth / 2 + 1 ) * ( imageHeight / 2 + 1 ) ) + 1;

            if ( maxObjectsCount > data->MaxObjectsCount )
            {
                if ( data->TempLabelsMap != 0 )
                {
                    free( data->TempLabelsMap );
                    data->TempLabelsMap   = 0;
                    data->MaxObjectsCount = 0;
                }

                data->TempLabelsMap = (uint32_t*) malloc( maxObjectsCount * sizeof( uint32_t ) );

                if ( data->TempLabelsMap == 0 )
                {
                    ret = ErrorOutOfMemory;
                }
                else
                {
                    data->MaxObjectsCount = maxObjectsCount;
                }
            }
        }

        // allocate initial memory for information about detected objects
        if ( ( ret == SuccessCode ) && ( data->AllocatedObjectsInfoCount == 0 ) )
        {
            data->AllocatedObjectsInfoCount = 1000;
            data->ObjectsRectangles = (xrect*) malloc( data->AllocatedObjectsInfoCount * sizeof( xrect ) );
            data->ObjectsIdAndSize  = (IdAndSize*) malloc( data->AllocatedObjectsInfoCount * sizeof( IdAndSize ) );

            if ( ( data->ObjectsRectangles == 0 ) || ( data->ObjectsIdAndSize == 0 ) )
            {
                ret = ErrorOutOfMemory;
                data->AllocatedObjectsInfoCount = 0;

                if ( data->ObjectsRectangles != 0 )
                {
                    free( data->ObjectsRectangles );
                }
                if ( data->ObjectsIdAndSize != 0 )
                {
                    free( data->ObjectsIdAndSize );
                }
            }
        }

        // allocated buffer for blobs' edge points
        if ( ret == SuccessCode )
        {
            uint32_t maxEdgePoints = ( imageHeight + imageWidth ) * 2;

            if ( data->AllocatedEdgePointsCount < maxEdgePoints )
            {
                if ( data->BlobEdgePoints != 0 )
                {
                    free( data->BlobEdgePoints );
                    data->BlobEdgePoints = 0;
                    data->AllocatedEdgePointsCount = 0;
                }

                data->BlobEdgePoints = (xpoint*) malloc( maxEdgePoints * sizeof( xpoint ) );

                if ( data->BlobEdgePoints == 0 )
                {
                    ret = ErrorOutOfMemory;
                }
                else
                {
                    data->AllocatedEdgePointsCount = maxEdgePoints;
                }
            }
        }

        // allocate buffer for glyph extraction/recognition
        if ( ret == SuccessCode )
        {
            uint32_t requiredGlyphBufferSize = ( glyphSize + 2 ) * ( glyphSize + 2 );

            if ( data->AllocatedGlyphBufferSize < requiredGlyphBufferSize )
            {
                if ( data->GlyphBuffer != 0 )
                {
                    free( data->GlyphBuffer );
                    data->GlyphBuffer = 0;
                    data->AllocatedGlyphBufferSize = 0;
                }

                data->GlyphBuffer = (uint32_t*) malloc( requiredGlyphBufferSize * sizeof( uint32_t ) );

                if ( data->GlyphBuffer == 0 )
                {
                    ret = ErrorOutOfMemory;
                }
                else
                {
                    data->AllocatedGlyphBufferSize = requiredGlyphBufferSize;
                }
            }
        }

        // allocate memory for recognized glyphs' information
        if ( ret == SuccessCode )
        {
            if ( ( data->MaxGlyphs < maxGlyphs ) || ( data->GlyphSize < glyphSize ) )
            {
                FreeDetectedGlyphsInfo( &context->DetectedGlyphs, data->MaxGlyphs );

                context->DetectedGlyphs = (DetectedGlyphInfo*) calloc( maxGlyphs, sizeof( DetectedGlyphInfo ) );

                if ( context->DetectedGlyphs == 0 )
                {
                    ret = ErrorOutOfMemory;
                    data->MaxGlyphs = 0;
                }
                else
                {
                    uint32_t i;

                    // set allocated size even something below fails - needed for correct clean-up
                    data->MaxGlyphs = maxGlyphs;
                    data->GlyphSize = glyphSize;

                    for ( i = 0; i < maxGlyphs; i++ )
                    {
                        context->DetectedGlyphs[i].Code = (char*) calloc( 1, glyphSize * glyphSize + 1 );

                        if ( context->DetectedGlyphs[i].Code == 0 )
                        {
                            ret = ErrorOutOfMemory;
                            break;
                        }
                    }

                    if ( i != maxGlyphs )
                    {
                        // discard partially allocated structure
                        FreeDetectedGlyphsInfo( &context->DetectedGlyphs, data->MaxGlyphs );
                        data->MaxGlyphs = 0;
                    }
                }
            }
        }
    }

    return ret;
}

// Allocate buffers required to gather information about detected blobs
static XErrorCode AllocateObjectsInfoBuffers( GlyphDetectionData* data, uint32_t foundObjectsCount )
{
    XErrorCode ret = SuccessCode;

    if ( foundObjectsCount > data->AllocatedObjectsInfoCount )
    {
        if ( data->ObjectsRectangles != 0 )
        {
            free( data->ObjectsRectangles );
        }
        if ( data->ObjectsIdAndSize != 0 )
        {
            free( data->ObjectsIdAndSize );
        }

        data->ObjectsRectangles = (xrect*) malloc( foundObjectsCount * sizeof( xrect ) );
        data->ObjectsIdAndSize  = (IdAndSize*) malloc( data->AllocatedObjectsInfoCount * sizeof( IdAndSize ) );

        if ( ( data->ObjectsRectangles == 0 ) || ( data->ObjectsIdAndSize == 0 ) )
        {
            ret = ErrorOutOfMemory;
            data->AllocatedObjectsInfoCount = 0;

            if ( data->ObjectsRectangles != 0 )
            {
                free( data->ObjectsRectangles );
            }
            if ( data->ObjectsIdAndSize != 0 )
            {
                free( data->ObjectsIdAndSize );
            }
        }
        else
        {
            data->AllocatedObjectsInfoCount = foundObjectsCount;
        }
    }

    return ret;
}

// Find square binary glyphs in the specified image
XErrorCode FindGlyphs( const ximage* image, uint32_t glyphSize, uint32_t maxGlyphs, GlyphDetectionContext** pContext )
{
    XErrorCode ret = SuccessCode;

    if ( ( image == 0 ) || ( pContext == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( image->format != XPixelFormatGrayscale8 ) &&
              ( image->format != XPixelFormatRGB24 ) &&
              ( image->format != XPixelFormatRGBA32 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        GlyphDetectionContext* context           = 0;
        GlyphDetectionData*    data              = 0;
        uint32_t               foundObjectsCount = 0;
        const ximage*          grayImage         = image;

        glyphSize = XINRANGE( glyphSize, 2, 20 );
        maxGlyphs = XINRANGE( maxGlyphs, 1, 10 );

        ret = AllocateContext( image->width, image->height, image->format, glyphSize, maxGlyphs, pContext );

        if ( ret == SuccessCode )
        {
            context = *pContext;
            data    = ( GlyphDetectionData*) context->Data;
        }

        // 1 - get grayscale image, if the source is color
        if ( ( ret == SuccessCode ) && ( image->format != XPixelFormatGrayscale8 ) )
        {
            ret = ColorToGrayscale( image, data->GrayImage );
        }

        // 2 - get edges of the objects
        if ( ret == SuccessCode )
        {
            grayImage = data->GrayImage;

            ret = EdgeDetector( ( image->format == XPixelFormatGrayscale8 ) ? image : data->GrayImage,  data->EdgeImage, EdgeDetector_Difference, false );
        }

        // 3 - threshold edges
        if ( ret == SuccessCode )
        {
            ret = ThresholdImage( data->EdgeImage, EDGES_THRESHOLD );
        }

        // 4 - find individual blobs
        if ( ret == SuccessCode )
        {
            ret = BcBuildObjectsMap( data->EdgeImage, data->BlobsMapImage, &foundObjectsCount, data->TempLabelsMap, data->MaxObjectsCount );
        }

        // make sure there is enough memory for objests' information
        if ( ret == SuccessCode )
        {
            ret = AllocateObjectsInfoBuffers( data, foundObjectsCount );
        }

        // 5 - get rectangles of each individual blob
        if ( ret == SuccessCode )
        {
            ret = BcGetObjectsRectangles( data->BlobsMapImage, foundObjectsCount, data->ObjectsRectangles );
        }

        // 6 - go through the list of blobs and see if they look promising for glyph analysis
        if ( ret == SuccessCode )
        {
            uint32_t blobIndex, blobId;
            uint32_t blobsOfGoodSize = 0;
            xrect    blobRect;
            int32_t  blobWidth, blobHeight;

            for ( blobIndex = 0; blobIndex < foundObjectsCount; blobIndex++ )
            {
                blobRect   = data->ObjectsRectangles[blobIndex];
                blobWidth  = blobRect.x2 - blobRect.x1 + 1;
                blobHeight = blobRect.y2 - blobRect.y1 + 1;

                if ( ( blobWidth >= MIN_BLOB_WIDTH ) && ( blobHeight >= MIN_BLOB_HEIGHT ) )
                {
                    // hope we'll not get huge image resolution
                    data->ObjectsIdAndSize[blobsOfGoodSize].Id   = blobIndex;
                    data->ObjectsIdAndSize[blobsOfGoodSize].Size = (uint32_t) ( blobWidth * blobHeight );
                    blobsOfGoodSize++;
                }
            }

            if ( blobsOfGoodSize != 0 )
            {
                qsort( data->ObjectsIdAndSize, blobsOfGoodSize, sizeof( uint64_t ), CompareObjectSizes );
            }

            // go through the list of blobs satisfying minimum width/height; starting with the biggest blobs first
            for ( blobIndex = 0; ( blobIndex < blobsOfGoodSize ) && ( context->DetectedGlyphsCount < maxGlyphs ); blobIndex++ )
            {
                uint32_t edgePointsCount = 0;
                uint32_t avgVerticalThickness;

                blobId   = data->ObjectsIdAndSize[blobIndex].Id;
                blobRect = data->ObjectsRectangles[blobId];

                // get edge points of the object
                BcGetObjectEdgePoints( data->BlobsMapImage, blobId + 1, blobRect, data->AllocatedEdgePointsCount, data->BlobEdgePoints, &edgePointsCount, &avgVerticalThickness );

                // ignore all blobs, which are not thick enough
                if ( avgVerticalThickness >= MIN_BLOB_AVG_THICKNESS )
                {
                    xpoint* quadPoints = context->DetectedGlyphs[context->DetectedGlyphsCount].Quadrilateral;

                    if ( CheckPointsFitQuadrilateralHelper( data->BlobEdgePoints, edgePointsCount, blobRect, quadPoints ) )
                    {
                        uint32_t blobHeight  = blobRect.y2 - blobRect.y1 + 1;
                        xpoint*  leftPoints  = data->BlobEdgePoints;
                        xpoint*  rightPoints = data->BlobEdgePoints + blobHeight;

                        if ( GetAverageBrightnessDiffOnLeftRightEdges( leftPoints, rightPoints, blobHeight, grayImage ) > MIN_EDGE_CONTRAST )
                        {
                            // extract glyph image and apply OTSU thresholding to it
                            if ( ( ExtractQuadrilateral( grayImage, data->GlyphImage, quadPoints, true ) == SuccessCode ) &&
                                 ( OtsuThresholding( data->GlyphImage ) == SuccessCode ) )
                            {
                                // try recognizing glyph from its image
                                if ( RecognizeGlyph( data->GlyphImage, glyphSize, data->GlyphBuffer, context->DetectedGlyphs[context->DetectedGlyphsCount].Code ) )
                                {
                                    context->DetectedGlyphs[context->DetectedGlyphsCount].BoundingRect = blobRect;
                                    context->DetectedGlyphsCount++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return ret;
}

// Check if the specified set of points form a quadrilateral
bool CheckPointsFitQuadrilateralHelper( const xpoint* points, uint32_t pointsCount, xrect pointsRect, xpoint* quadPoints )
{
    xpoint   tempQuadPoints[4];
    float    workBuffer1[12];
    bool     workBuffer2[4];
    uint32_t finalPointsCount;
    bool     gotFourPoints;
    bool     ret = false;

    if ( ( FindQuadrilateralCorners( points, pointsCount, pointsRect, QUADRILATERAL_REL_DISTORTION_LIMIT, tempQuadPoints, &gotFourPoints ) == SuccessCode ) &&
         ( gotFourPoints ) )
    {
        if ( ( OptimizeFlatAngles( tempQuadPoints, 4, quadPoints, &finalPointsCount, QUADRILATERAL_MAX_ANGLE_TO_KEEP ) == SuccessCode ) &&
             ( finalPointsCount == 4 ) )
        {
            if ( CheckPointsFitShape( points, pointsCount, pointsRect, quadPoints, 4,
                                      SHAPE_REL_DISTORTION_LIMIT, SHAPE_MIN_DISTORTION_LIMIT,
                                      workBuffer1, workBuffer2 ) == SuccessCode )
            {
                ret = true;
            }
        }
    }

    return ret;
}

// Calculate average difference between pixels outside of an object and pixels inside it
int GetAverageBrightnessDiffOnLeftRightEdges( const xpoint* leftEdgePoints, const xpoint* rightEdgePoints, uint32_t pointsCount, const ximage* grayImage )
{
    int width   = grayImage->width;
    int widthM1 = width - 1;
    int stride  = grayImage->stride;
    int startY  = leftEdgePoints[0].y;
    int stopY   = leftEdgePoints[pointsCount - 1].y;
    int avgDiff = 0;
    int counter = 0;
    int step2   = EDGE_STEP_SIZE * 2;
    int i, y, x1, x2, x3, x4;

    uint8_t* ptr = grayImage->data;
    uint8_t* row;

    for ( y = startY, i = 0; y <= stopY; y++, i++ )
    {
        if ( rightEdgePoints[i].x - leftEdgePoints[i].x > step2 )
        {
            row = ptr + y * stride;

            x1 =  leftEdgePoints[i].x - EDGE_STEP_SIZE;
            x2 =  leftEdgePoints[i].x + EDGE_STEP_SIZE;
            x3 = rightEdgePoints[i].x - EDGE_STEP_SIZE;
            x4 = rightEdgePoints[i].x + EDGE_STEP_SIZE;

            if ( x1 < 0 )       { x1 = 0; }
            if ( x2 > widthM1 ) { x2 = widthM1; }
            if ( x3 < 0 )       { x3 = 0; }
            if ( x4 > widthM1 ) { x4 = widthM1; }

            avgDiff += row[x1] - row[x2];
            avgDiff += row[x4] - row[x3];
            counter += 2;
        }
    }

    if ( counter != 0 )
    {
        avgDiff /= counter;
    }

    return avgDiff;
}

// Recognize glyph from the specified image
static bool RecognizeGlyph( const ximage* glyphImage, uint32_t glyphSize, uint32_t* glyphBuffer, char* glyphString )
{
    bool  ret        = true;
    int   glyphCells = (int) glyphSize + 2;
    int   stride     = glyphImage->stride;
    int   i, j, ii, jj, counter = 0, whiteCellCounter = 0;

    uint8_t*  ptr = glyphImage->data;
    uint8_t*  row;
    uint8_t*  col;
    uint32_t* glyphRow;

    memset( glyphBuffer, 0, glyphCells * glyphCells * sizeof( uint32_t ) );

    // scan all glyph's rows
    for ( i = 0; i < glyphCells; i++ )
    {
        row      = ptr + stride * ( i * GLYPH_CELL_SIZE + GLYPH_CELL_BORDER_TO_IGNORE );
        glyphRow = &glyphBuffer[glyphCells * i];

        for ( ii = 0; ii < GLYPH_CELL_SIZE_TO_CHECK; ii++, row += stride )
        {
            // scan all glyph's columns
            for ( j = 0; j < glyphCells; j++ )
            {
                col = row + ( j * GLYPH_CELL_SIZE + GLYPH_CELL_BORDER_TO_IGNORE );

                for ( jj = 0; jj < GLYPH_CELL_SIZE_TO_CHECK; jj++, col++ )
                {
                    glyphRow[j] += *col >> 7;
                }
            }
        }
    }

    // scan all glyph's rows
    for ( i = 0; i < glyphCells; i++ )
    {
        // scan all glyph's columns
        for ( j = 0; j < glyphCells; j++ )
        {
            int value = glyphBuffer[i * glyphCells + j];

            if ( ( i == 0 ) || ( j == 0 ) || ( i == glyphCells - 1 ) || ( j == glyphCells - 1 ) )
            {
                if ( value > GLYPH_EDGE_CELL_MAX_LEVEL )
                {
                    ret = false;
                }
            }
            else
            {
                if ( value >= GLYPH_CELL_MIN_WHITE_LEVEL )
                {
                    glyphString[counter] = '1';
                    whiteCellCounter++;
                }
                else if ( value <= GLYPH_CELL_MAX_BLACK_LEVEL )
                {
                    glyphString[counter] = '0';
                }
                else
                {
                    ret = false;
                }

                counter++;
            }
        }
    }

    glyphString[counter] = '\0';

    // discard it, if there are no any white cells
    if ( whiteCellCounter == 0 )
    {
        ret = false;
    }

    return ret;
}
