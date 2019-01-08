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
#include <stdio.h>
#include <memory.h>
#include <math.h>

// Preferred width/height for image to be processed
#define IMAGE_BASE_SIZE (800)

// Kernel size for Gaussian blur and sigma value
#define BLUR_SIZE  (3)
#define BLUR_SIGMA (1.4f)

// Low/High threshold for Canny edge detection
#define CANNY_LOW_THRESHOLD  (20)
#define CANNY_HIGH_THRESHOLD (100)

// Min/Max size for blob filtering
#define BLOB_MIN_WIDTH  (20)
#define BLOB_MIN_HEIGHT (20)
#define BLOB_MAX_WIDTH  (10000)
#define BLOB_MAX_HEIGHT (10000)

// Below are used to calculate maximum allowed line deviation from its ideal path.
// For vertical lines:
// maxDev = XINRANGE( (int) ( LINE_MUL_DELTA * blobHeight ), LINE_MIN_DELTA, LINE_MAX_DELTA );
// For horizontal lines:
// maxDev = XINRANGE( (int) ( LINE_MUL_DELTA * blobWidth ), LINE_MIN_DELTA, LINE_MAX_DELTA );
#define LINE_MIN_DELTA  (5)
#define LINE_MAX_DELTA  (15)
#define LINE_MUL_DELTA  (0.15)

// Below are used to calculate maximum allowed lines' mean thickness
// For vertical lines:
// maxMeanThickness = XMIN( (int) ( 0.25 * blobHeight ), 15 );
// For horizontal lines:
// maxMeanThickness = XMIN( (int) ( 0.25 * blobWidth ), 15 );
#define LINE_MAX_MEAN_THICKNESS (15)
#define LINE_MUL_MEAN_THICKNESS (0.25)

// Below are used to calculate maximum allowed mean deviation from mean thickness
// For vertical lines:
// maxMeanThickDev = XINRANGE( (int) ( LINE_MUL_MEAN_THICKNESS_DEV * blobHeight ), LINE_MIN_MEAN_THICKNESS_DEV, LINE_MAX_MEAN_THICKNESS_DEV );
// For horizontal lines:
// maxMeanThickDev = XINRANGE( (int) ( LINE_MUL_MEAN_THICKNESS_DEV * blobWidth ), LINE_MIN_MEAN_THICKNESS_DEV, LINE_MAX_MEAN_THICKNESS_DEV );
#define LINE_MIN_MEAN_THICKNESS_DEV (2)
#define LINE_MAX_MEAN_THICKNESS_DEV (5)
#define LINE_MUL_MEAN_THICKNESS_DEV (0.04)

// Maximum allowed distance between lines in bar code
#define LINES_MAX_DISTANCE (20)

// Minimum number of lines in a cluster to even consider it
#define LINES_MIN_CLUSTER_SIZE (10)
// Minimum number of lines in a cluster if something bigger was already found
#define LINES_MIN_SECOND_CLUSTER_SIZE (20)

// Maximum allowed angle difference between adjacent  line in a cluster
#define LINES_MAX_ANGLE_DIFF (24)
// Maximum allowed mean angle difference between adjacent line in a cluster
#define LINES_MAX_MEAN_ANGLE_DIFF (5)

// Data structure used for searching objects' clusters
typedef struct _objectsCluster
{
    uint32_t*               Objects;
    uint32_t                ObjectsCount;
    float                   AverageAngle;
    bool                    IsVertical;
    struct _objectsCluster* Next;

    xpoint                  Quad[4];
    xpoint                  Center;
    xpoint                  TopCenter;
    xpoint                  BottomCenter;
}
ObjectsCluster;

// Memory buffers for different information describing detected blobs
typedef struct _blobsInfo
{
    uint32_t        AllocatedSize;
    xrect*          Rectangles;
    xpoint*         LineStartPoints;
    xpoint*         LineEndPoints;
    float*          LineSlopes;
    bool*           LineVertical;
    float*          LineMeanThickness;
    uint8_t*        FillMap;
    ObjectsCluster* LinesClusters;

    uint32_t        AllocatedLineThicknessBufferSize;
    int32_t*        LineThicknessBuffer;
}
BlobsInfo;

// Internal data for the bar code detection context
typedef struct _barcodeDetectionData
{
    uint32_t   MaxBarcodes;
    bool       ImageResizeIsNeeded;
    float      ResizeFactor;

    ximage*    GrayImage;
    ximage*    ResizedImage;
    ximage*    EdgeImage;
    ximage*    VerticalLinesImage;
    ximage*    VerticalLinesTempImage;
    ximage*    HorizontalLinesImage;
    ximage*    HorizontalLinesTempImage;

    ximage*    TempBlurImage1;
    ximage*    TempBlurImage2;
    ximage*    TempEdgesImage;
    ximage*    EdgeGradientsImage;
    ximage*    EdgeOrientationsImage;

    ximage*    VerticalBlobsMap;
    ximage*    HorizontalBlobsMap;

    float      BlurKernel[BLUR_SIZE];

    uint32_t   VerticalObjectsCount;
    uint32_t   HorizontalObjectsCount;

    uint32_t   AllocatedObjectLabelsCount;
    uint32_t*  VerticalLabelsMap;
    uint32_t*  HorizontalLabelsMap;

    BlobsInfo  VerticalObjectInfo;
    BlobsInfo  HorizontalObjectInfo;

    uint8_t*   VerticalClustersWorkingMemory;
    uint32_t   VerticalClustersAllocatedWorkingMemory;

    uint8_t*   HorizontalClustersWorkingMemory;
    uint32_t   HorizontalClustersAllocatedWorkingMemory;
}
BarcodeDetectionData;

// Local helper functions
static XErrorCode ProcessVerticalLines( BarcodeDetectionData* data );
static XErrorCode ProcessHorizontalLines( BarcodeDetectionData* data );
static uint32_t KeepVLines( BarcodeDetectionData* data );
static uint32_t KeepHLines( BarcodeDetectionData* data );
static uint32_t FindLinesClusters( BlobsInfo* blobsInfo, uint32_t objectsCount, uint32_t objectsLeft, uint8_t* workingBuffer );
static void FilterOutliersFromVericalClusters( BlobsInfo* blobsInfo, uint8_t* workingBuffer );
static void FilterEdgeGapsFromVerticalClusters( BlobsInfo* blobsInfo, uint8_t* workingBuffer );
static void FilterClustersWithNoisyAngles( BlobsInfo* blobsInfo );
static void FindlClustersQuads( BlobsInfo* blobsInfo );

// Allocated memory to use for searching objects' clusters
static XErrorCode AllocateClustersWorkingMemory( uint32_t objectsCount, uint8_t** workingMemory, uint32_t* allocatedAmount )
{
    uint32_t   memoryNeeded = objectsCount * ( sizeof( ObjectsCluster ) + sizeof( uint32_t ) + sizeof( uint32_t ) ) + ( objectsCount * ( objectsCount + 1 ) / 2 ) * sizeof( uint32_t );
    XErrorCode ret          = SuccessCode;

    if ( *allocatedAmount < memoryNeeded )
    {
        if ( *workingMemory != 0 )
        {
            free( *workingMemory );
        }

        *workingMemory = (uint8_t*) malloc( memoryNeeded );

        if ( *workingMemory == 0 )
        {
            ret = ErrorOutOfMemory;
            *allocatedAmount = 0;
        }
        else
        {
            *allocatedAmount = memoryNeeded;
        }
    }

    return ret;
}

// Free buffers allocated for detected objects/blobs information
static void FreeObjectsInfo( BlobsInfo* blobsInfo )
{
    if ( blobsInfo->Rectangles != 0 )
    {
        free( blobsInfo->Rectangles );
        blobsInfo->Rectangles = 0;
    }
    if ( blobsInfo->LineStartPoints != 0 )
    {
        free( blobsInfo->LineStartPoints );
        blobsInfo->LineStartPoints = 0;
    }
    if ( blobsInfo->LineEndPoints != 0 )
    {
        free( blobsInfo->LineEndPoints );
        blobsInfo->LineEndPoints = 0;
    }
    if ( blobsInfo->LineSlopes != 0 )
    {
        free( blobsInfo->LineSlopes );
        blobsInfo->LineSlopes = 0;
    }
    if ( blobsInfo->LineVertical != 0 )
    {
        free( blobsInfo->LineVertical );
        blobsInfo->LineVertical = 0;
    }
    if ( blobsInfo->LineMeanThickness != 0 )
    {
        free( blobsInfo->LineMeanThickness );
        blobsInfo->LineMeanThickness = 0;
    }
    if ( blobsInfo->FillMap != 0 )
    {
        free( blobsInfo->FillMap );
        blobsInfo->FillMap = 0;
    }
    if ( blobsInfo->LineThicknessBuffer != 0 )
    {
        free( blobsInfo->LineThicknessBuffer );
        blobsInfo->LineThicknessBuffer = 0;
    }

    blobsInfo->AllocatedSize = 0;
    blobsInfo->AllocatedLineThicknessBufferSize = 0;
}

// Allocate buffers for different information about detected objects/blobs
static XErrorCode AllocateObjectsInfo( BlobsInfo* blobsInfo, uint32_t objectsCount, uint32_t maxLineLengh )
{
    XErrorCode ret = SuccessCode;

    if ( blobsInfo->AllocatedSize < objectsCount )
    {
        if ( blobsInfo->Rectangles != 0 )
        {
            free( blobsInfo->Rectangles );
        }
        if ( blobsInfo->LineStartPoints != 0 )
        {
            free( blobsInfo->LineStartPoints );
        }
        if ( blobsInfo->LineEndPoints != 0 )
        {
            free( blobsInfo->LineEndPoints );
        }
        if ( blobsInfo->LineSlopes != 0 )
        {
            free( blobsInfo->LineSlopes );
        }
        if ( blobsInfo->LineVertical != 0 )
        {
            free( blobsInfo->LineVertical );
        }
        if ( blobsInfo->LineMeanThickness != 0 )
        {
            free( blobsInfo->LineMeanThickness );
        }
        if ( blobsInfo->FillMap != 0 )
        {
            free( blobsInfo->FillMap );
        }

        blobsInfo->Rectangles        = (xrect*)   malloc( objectsCount * sizeof( xrect ) );
        blobsInfo->LineStartPoints   = (xpoint*)  malloc( objectsCount * sizeof( xpoint ) );
        blobsInfo->LineEndPoints     = (xpoint*)  malloc( objectsCount * sizeof( xpoint ) );
        blobsInfo->LineSlopes        = (float*)   malloc( objectsCount * sizeof( float) );
        blobsInfo->LineVertical      = (bool*)    malloc( objectsCount * sizeof( bool ) );
        blobsInfo->LineMeanThickness = (float*)   malloc( objectsCount * sizeof( float ) );
        blobsInfo->FillMap           = (uint8_t*) malloc( ( objectsCount + 1 ) * sizeof( uint8_t ) );

        if ( ( blobsInfo->Rectangles == 0 ) || ( blobsInfo->FillMap == 0 ) ||
             ( blobsInfo->LineStartPoints == 0 ) || ( blobsInfo->LineEndPoints == 0 ) ||
             ( blobsInfo->LineSlopes == 0 ) || ( blobsInfo->LineVertical == 0 ) ||
             ( blobsInfo->LineMeanThickness == 0 ) )
        {
            blobsInfo->AllocatedSize = 0;
            ret = ErrorOutOfMemory;
        }
        else
        {
            blobsInfo->AllocatedSize = objectsCount;
        }
    }

    // allocate temporary buffer for lines' thickness check
    if ( ( ret == SuccessCode ) && ( blobsInfo->AllocatedLineThicknessBufferSize < maxLineLengh ) )
    {
        if ( blobsInfo->LineThicknessBuffer != 0 )
        {
            free( blobsInfo->LineThicknessBuffer );
        }

        blobsInfo->LineThicknessBuffer = (int32_t*) malloc( maxLineLengh * sizeof( int32_t ) );

        if ( blobsInfo->LineThicknessBuffer == 0 )
        {
            blobsInfo->AllocatedLineThicknessBufferSize = 0;
            ret = ErrorOutOfMemory;
        }
        else
        {
            blobsInfo->AllocatedLineThicknessBufferSize = maxLineLengh;
        }
    }

    blobsInfo->LinesClusters = 0;

    return ret;
}

// Free barcode detection context allocated by barcode detection functions
void FreeBarcodeDetectionContext( BarcodeDetectionContext** pContext )
{
    if ( ( pContext != 0 ) && ( *pContext != 0 ) )
    {
        BarcodeDetectionContext* context = *pContext;

        if ( context->Data != 0 )
        {
            BarcodeDetectionData* data = (BarcodeDetectionData*) context->Data;

            XImageFree( &data->GrayImage );
            XImageFree( &data->ResizedImage );
            XImageFree( &data->EdgeImage );
            XImageFree( &data->VerticalLinesImage );
            XImageFree( &data->HorizontalLinesImage );
            // shared with other images, so not freed
            // XImageFree( &data->VerticalLinesTempImage );
            // XImageFree( &data->HorizontalLinesTempImage );

            XImageFree( &data->TempBlurImage1 );
            XImageFree( &data->TempBlurImage2 );
            XImageFree( &data->TempEdgesImage );
            XImageFree( &data->EdgeGradientsImage );
            XImageFree( &data->EdgeOrientationsImage );

            XImageFree( &data->VerticalBlobsMap );
            XImageFree( &data->HorizontalBlobsMap );

            if ( data->VerticalLabelsMap != 0 )
            {
                free( data->VerticalLabelsMap );
            }
            if ( data->HorizontalLabelsMap != 0 )
            {
                free( data->HorizontalLabelsMap );
            }

            FreeObjectsInfo( &data->VerticalObjectInfo );
            FreeObjectsInfo( &data->HorizontalObjectInfo );

            if ( data->VerticalClustersWorkingMemory != 0 )
            {
                free( data->VerticalClustersWorkingMemory );
            }
            if ( data->HorizontalClustersWorkingMemory != 0 )
            {
                free( data->HorizontalClustersWorkingMemory );
            }

            free( context->Data );
        }

        if ( context->DetectedBarcodes != 0 )
        {
            free( context->DetectedBarcodes );
        }

        XFree( (void**) pContext );
    }
}

// Allocate bar code detection context and any required internal data structures
static XErrorCode AllocateContext( int32_t imageWidth, int32_t imageHeight, XPixelFormat imageFormat, uint32_t maxBarcodes, BarcodeDetectionContext** pContext )
{
    XErrorCode               ret     = SuccessCode;
    BarcodeDetectionContext* context = *pContext;

    if ( context == 0 )
    {
        context = XCAlloc( 1, sizeof( BarcodeDetectionContext ) );

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
        BarcodeDetectionData* data         = (BarcodeDetectionData*) context->Data;
        int32_t               imageMaxSize = XMAX( imageWidth, imageHeight );
        uint32_t              maxObjects;

        context->DetectedBarcodesCount = 0;

        if ( data == 0 )
        {
            data = (BarcodeDetectionData*) calloc( 1, sizeof( BarcodeDetectionData ) );

            if ( data == 0 )
            {
                ret = ErrorOutOfMemory;
            }
            else
            {
                context->Data = data;
                data->ImageResizeIsNeeded = false;
            }
        }

        if ( ret == SuccessCode )
        {
            CreateGaussianBlurKernel1D( BLUR_SIGMA, BLUR_SIZE >> 1, data->BlurKernel );
        }

        // allocate temporary image for grayscale conversion
        if ( ( ret == SuccessCode ) && ( imageFormat != XPixelFormatGrayscale8 ) )
        {
            ret = XImageAllocateRaw( imageWidth, imageHeight, XPixelFormatGrayscale8, &data->GrayImage );
        }

        // check size of the source image and decide if it needs to be resized
        if ( ( ret == SuccessCode ) && ( imageMaxSize > IMAGE_BASE_SIZE ) )
        {
            data->ImageResizeIsNeeded = true;
            data->ResizeFactor        = (float) imageMaxSize / IMAGE_BASE_SIZE;

            imageWidth  = (int32_t) ( imageWidth  / data->ResizeFactor );
            imageHeight = (int32_t) ( imageHeight / data->ResizeFactor );

            ret = XImageAllocateRaw( imageWidth, imageHeight, XPixelFormatGrayscale8, &data->ResizedImage );
        }

        // all other images/buffers are allocated for the resized image
        maxObjects = ( ( imageWidth / 2 ) + 1 ) * ( ( imageHeight / 2 ) + 1 ) + 1;

        // allocate temporary image for edges
        if ( ret == SuccessCode )
        {
            ret = XImageAllocateRaw( imageWidth, imageHeight, XPixelFormatGrayscale8, &data->EdgeImage );
        }

        // allocate temporary images for image blurring, which is done as part of canny edge finding
        if ( ret == SuccessCode )
        {
            ret = XImageAllocateRaw( imageWidth, imageHeight, XPixelFormatGrayscaleR4, &data->TempBlurImage1 );
        }
        if ( ret == SuccessCode )
        {
            ret = XImageAllocateRaw( imageWidth, imageHeight, XPixelFormatGrayscaleR4, &data->TempBlurImage2 );
        }

        // allocate temporary edge image (not final result)
        if ( ret == SuccessCode )
        {
            ret = XImageAllocateRaw( imageWidth, imageHeight, XPixelFormatGrayscale8, &data->TempEdgesImage );
        }

        // allocate temporary images for edge's gradients and orientations
        if ( ret == SuccessCode )
        {
            ret = XImageAllocateRaw( imageWidth, imageHeight, XPixelFormatGrayscaleR4, &data->EdgeGradientsImage );
        }
        if ( ret == SuccessCode )
        {
            ret = XImageAllocateRaw( imageWidth, imageHeight, XPixelFormatGrayscale8, &data->EdgeOrientationsImage );
        }

        // allocate images for vertical/horizontal lines
        if ( ret == SuccessCode )
        {
            ret = XImageAllocateRaw( imageWidth, imageHeight, XPixelFormatGrayscale8, &data->VerticalLinesImage );
        }
        if ( ret == SuccessCode )
        {
            // ret = XImageAllocateRaw( imageWidth, imageHeight, XPixelFormatGrayscale8, &data->VerticalLinesTempImage );
            // since vertical lines splitting happens after edge detection, temporary images left from there can be reused
            data->VerticalLinesTempImage = data->EdgeOrientationsImage;
        }
        if ( ret == SuccessCode )
        {
            ret = XImageAllocateRaw( imageWidth, imageHeight, XPixelFormatGrayscale8, &data->HorizontalLinesImage);
        }
        if ( ret == SuccessCode )
        {
            // ret = XImageAllocateRaw( imageWidth, imageHeight, XPixelFormatGrayscale8, &data->HorizontalLinesTempImage );
            // since horizontal lines splitting happens after edge detection, temporary images left from there can be reused
            data->HorizontalLinesTempImage = data->TempEdgesImage;
        }

        // allocate images for vertical/horizontal blobs' maps
        if ( ret == SuccessCode )
        {
            ret = XImageAllocateRaw( imageWidth, imageHeight, XPixelFormatGrayscale32, &data->VerticalBlobsMap );
        }
        if ( ret == SuccessCode )
        {
            ret = XImageAllocateRaw( imageWidth, imageHeight, XPixelFormatGrayscale32, &data->HorizontalBlobsMap );
        }

        // allocate buffers for labels' maps
        if ( ( ret == SuccessCode ) && ( data->AllocatedObjectLabelsCount < maxObjects ) )
        {
            if ( data->VerticalLabelsMap != 0 )
            {
                free( data->VerticalLabelsMap );
            }

            if ( data->HorizontalLabelsMap != 0 )
            {
                free( data->HorizontalLabelsMap );
            }

            data->VerticalLabelsMap   = (uint32_t*) malloc( maxObjects * sizeof( uint32_t ) );
            data->HorizontalLabelsMap = (uint32_t*) malloc( maxObjects * sizeof( uint32_t ) );

            if ( ( data->VerticalLabelsMap == 0 ) || ( data->HorizontalLabelsMap == 0 ) )
            {
                data->AllocatedObjectLabelsCount = 0;
                ret = ErrorOutOfMemory;
            }
            else
            {
                data->AllocatedObjectLabelsCount = maxObjects;
            }
        }

        // allocate initial buffers for objects' information
        if ( ret == SuccessCode )
        {
            ret = AllocateObjectsInfo( &data->VerticalObjectInfo, 1000, imageHeight );
        }
        if ( ret == SuccessCode )
        {
            ret = AllocateObjectsInfo( &data->HorizontalObjectInfo, 1000, imageWidth );
        }

        // allocate memory for detected bar codes' information
        if ( ret == SuccessCode )
        {
            if ( data->MaxBarcodes < maxBarcodes )
            {
                if ( context->DetectedBarcodes != 0 )
                {
                    free( context->DetectedBarcodes );
                }

                context->DetectedBarcodes = (DetectedBarcodeInfo*) calloc( maxBarcodes, sizeof( DetectedBarcodeInfo ) );

                if ( context->DetectedBarcodes == 0 )
                {
                    ret = ErrorOutOfMemory;
                    data->MaxBarcodes = 0;
                }
                else
                {
                    data->MaxBarcodes = maxBarcodes;
                }
            }
        }
    }

    return ret;
}

// Get bounding rectangle and quadrilateral for the specified cluster
static void GetClusterRectAndQuad( const ObjectsCluster* cluster, const BlobsInfo* blobsInfo, xrect* rect, xpoint* quad )
{
    uint32_t* objects     = cluster->Objects;
    xrect*    rectangles  = blobsInfo->Rectangles;
    uint32_t  i, n;
    xrect     r;

    *rect  = rectangles[objects[0]];

    for ( i = 1, n = cluster->ObjectsCount; i < n; i++ )
    {
        r = rectangles[objects[i]];

        if ( r.x1 < rect->x1 ) rect->x1 = r.x1;
        if ( r.x2 > rect->x2 ) rect->x2 = r.x2;
        if ( r.y1 < rect->y1 ) rect->y1 = r.y1;
        if ( r.y2 > rect->y2 ) rect->y2 = r.y2;
    }

    quad[0] = cluster->Quad[0];
    quad[1] = cluster->Quad[1];
    quad[2] = cluster->Quad[2];
    quad[3] = cluster->Quad[3];
}

// Sort clusters by number of objects in descending order
static void SortClusters( ObjectsCluster** pClusters )
{
    if ( ( pClusters != 0 ) && ( *pClusters != 0 ) )
    {
        ObjectsCluster* head    = *pClusters;
        ObjectsCluster* current = head;
        ObjectsCluster* prev    = 0;

        // not the most efficient sorting algorithm, however there will be 1-2 clusters only in most cases
        while ( current->Next != 0 )
        {
            ObjectsCluster* maxFromTheRest     = current->Next;
            ObjectsCluster* maxFromTheRestPrev = current;

            ObjectsCluster* candidate          = maxFromTheRest->Next;
            ObjectsCluster* candidatePrev      = maxFromTheRest;

            while ( candidate != 0 )
            {
                if ( candidate->ObjectsCount > maxFromTheRest->ObjectsCount )
                {
                    maxFromTheRest     = candidate;
                    maxFromTheRestPrev = candidatePrev;
                }

                candidatePrev = candidate;
                candidate     = candidate->Next;
            }

            if ( maxFromTheRest->ObjectsCount > current->ObjectsCount )
            {
                // get the element with more objects out of the list
                maxFromTheRestPrev->Next = maxFromTheRest->Next;

                // insert it in front of the current
                maxFromTheRest->Next = current;
                if ( prev != 0 )
                {
                    prev->Next = maxFromTheRest;
                }
                else
                {
                    head = maxFromTheRest;
                }

                // keep current as is to see if there is anything bigger than that, but change previous pointer
                prev = maxFromTheRest;
            }
            else
            {
                prev    = current;
                current = current->Next;
            }
        }

        *pClusters = head;
    }
}

// Check if the specified point is inside or on the edge of the specified quadrilateral
static bool CheckPointIsInQuad( const xpoint* quad, xpoint point )
{
    bool  ret       = true;
    int   i         = 0;
    int   firstSign = 0;

    for ( i = 0; i < 4; i++ )
    {
        xpoint nextPoint     = ( i == 3 ) ? quad[0] : quad[i + 1];
        int    directionSign = 0;

        if ( nextPoint.x == quad[i].x )
        {
            if ( nextPoint.y >= quad[i].y )
            {
                directionSign = ( point.x >= nextPoint.x ) ? 1 : -1;
            }
            else
            {
                directionSign = ( point.x >= nextPoint.x ) ? -1 : 1;
            }
        }
        else
        {
            float  lineK = (float) ( nextPoint.y - quad[i].y ) / ( nextPoint.x - quad[i].x );
            float  lineB = quad[i].y - lineK * quad[i].x;
            float  lineS = ( nextPoint.x > quad[i].x ) ? -1.0f : 1.0f;

            // calculate only numerator part of the distance between line and point
            float distanceNum = lineS * ( lineB + lineK * point.x - point.y );

            directionSign = ( distanceNum >= 0 ) ? 1 : -1;
        }

        if ( i == 0 )
        {
            firstSign = directionSign;
        }
        else
        {
            if ( firstSign != directionSign )
            {
                ret = false;
                break;
            }
        }
    }

    return ret;
}

/*
static void FindBiggestCluster( const ObjectsCluster* cluster, const xrect* rectangles, xrect* rect, uint32_t* objectsCount )
{
    const ObjectsCluster* biggestCluster = cluster;

    if ( biggestCluster == 0 )
    {
        *objectsCount = 0;
    }
    else
    {
        const ObjectsCluster* nextCluster = biggestCluster->Next;
        uint32_t i;
        xrect    r;

        while ( nextCluster != 0 )
        {
            if ( nextCluster->ObjectsCount > biggestCluster->ObjectsCount )
            {
                biggestCluster = nextCluster;
            }

            nextCluster = nextCluster->Next;
        }

        *objectsCount = biggestCluster->ObjectsCount;

        *rect = rectangles[biggestCluster->Objects[0]];
        for ( i = 1; i < biggestCluster->ObjectsCount; i++ )
        {
            r = rectangles[biggestCluster->Objects[i]];

            if ( r.x1 < rect->x1 ) rect->x1 = r.x1;
            if ( r.x2 > rect->x2 ) rect->x2 = r.x2;
            if ( r.y1 < rect->y1 ) rect->y1 = r.y1;
            if ( r.y2 > rect->y2 ) rect->y2 = r.y2;
        }
    }
}

static uint32_t CountClusters( const ObjectsCluster* cluster )
{
    uint32_t counter = 0;

    while ( cluster != 0 )
    {
        counter++;
        cluster = cluster->Next;
    }

    return counter;
}
*/

// Find bar codes in the specified image. Context is allocated and can be reused by subsequent call.
XErrorCode FindBarcodes( const ximage* image, uint32_t maxBarcodes, BarcodeDetectionContext** pContext )
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
        BarcodeDetectionContext* context   = 0;
        BarcodeDetectionData*    data      = 0;
        const ximage*            grayImage = image;
        XErrorCode               ecode1    = SuccessCode, ecode2 = SuccessCode;
        int                      i;

        maxBarcodes = XINRANGE( maxBarcodes, 1, 10 );

        ret = AllocateContext( image->width, image->height, image->format, maxBarcodes, pContext );

        if ( ret == SuccessCode )
        {
            context = *pContext;
            data    = (BarcodeDetectionData*) context->Data;
        }

        // 1 - get grayscale image, if the source is color
        if ( ( ret == SuccessCode ) && ( image->format != XPixelFormatGrayscale8 ) )
        {
            ret = ColorToGrayscale( image, data->GrayImage );
            if ( ret == SuccessCode )
            {
                grayImage = data->GrayImage;
            }
        }

        // 2 - resize image if needed
        if ( ( ret == SuccessCode ) && ( data->ImageResizeIsNeeded ) )
        {
            ret = ResizeImageBilinear( grayImage, data->ResizedImage );
            if ( ret == SuccessCode )
            {
                grayImage = data->ResizedImage;
            }
        }

        // 3 - get edges of the objects
        if ( ret == SuccessCode )
        {
            ret = CannyEdgeDetector( grayImage, data->EdgeImage, data->TempBlurImage1, data->TempBlurImage2,
                                     data->TempEdgesImage, data->EdgeGradientsImage, data->EdgeOrientationsImage,
                                     data->BlurKernel, BLUR_SIZE, CANNY_LOW_THRESHOLD, CANNY_HIGH_THRESHOLD );
        }

        // 4 - process vertical and horizontal lines in parallel
        if ( ret == SuccessCode )
        {
            #pragma omp parallel for schedule(static) shared( data )
            for ( i = 0; i < 2; i++ )
            {
                if ( i == 0 )
                {
                    ecode1 = ProcessVerticalLines( data );
                }
                else
                {
                    ecode2 = ProcessHorizontalLines( data );
                }
            }

            if ( ecode1 != SuccessCode )
            {
                ret = ecode1;
            }
            else if ( ecode2 != SuccessCode )
            {
                ret = ecode2;
            }
        }

        /*
        // debug only
        if ( ( ret == SuccessCode ) && ( data->ImageResizeIsNeeded == false ) )
        {
            if ( image->format == XPixelFormatRGB24 )
            {
                int width  = image->width;
                int height = image->height;
                int stride = image->stride;
                int x, y;

                uint8_t* ptr = (uint8_t*) image->data;

                for ( y = 0; y < height; y++ )
                {
                    uint8_t*  row  = ptr + y * stride;
                    uint32_t* rowV = (uint32_t*) ( data->VerticalBlobsMap->data + y * data->VerticalBlobsMap->stride );
                    uint32_t* rowH = (uint32_t*) ( data->HorizontalBlobsMap->data + y * data->HorizontalBlobsMap->stride );

                    uint8_t* erow = data->VerticalLinesImage->data + y * data->VerticalLinesImage->stride;
                    //uint8_t* erow = data->EdgeImage->data + y * data->EdgeImage->stride;

                    for ( x = 0; x < width; x++, row += 3, rowV++, rowH++, erow++ )
                    {
                        if ( ( ( *rowV != 0 ) && ( data->VerticalObjectInfo.FillMap[*rowV] == 0 ) ) ||
                             ( ( *rowH != 0 ) && ( data->HorizontalObjectInfo.FillMap[*rowH] == 0 ) ) )
                        {
                            row[0] = 255;
                            row[1] = 0;
                            row[2] = 0;
                        }
                        else if ( ( ( *rowV != 0 ) && ( data->VerticalObjectInfo.FillMap[*rowV] == 0xFF ) ) ||
                                  ( ( *rowH != 0 ) && ( data->HorizontalObjectInfo.FillMap[*rowH] == 0xFF ) ) )
                        {
                            row[0] = 0;
                            row[1] = 0;
                            row[2] = 255;
                        }
                        else if ( ( ( *rowV != 0 ) && ( data->VerticalObjectInfo.FillMap[*rowV] == 0xFE ) ) ||
                                  ( ( *rowH != 0 ) && ( data->HorizontalObjectInfo.FillMap[*rowH] == 0xFE ) ) )
                        {
                            row[0] = 0;
                            row[1] = 255;
                            row[2] = 255;
                        }
                        else if ( ( ( *rowV != 0 ) && ( data->VerticalObjectInfo.FillMap[*rowV] == 0xFD ) ) ||
                                  ( ( *rowH != 0 ) && ( data->HorizontalObjectInfo.FillMap[*rowH] == 0xFD ) ) )
                        {
                            row[0] = 255;
                            row[1] = 255;
                            row[2] = 0;
                        }
                        else if ( ( ( *rowV != 0 ) && ( data->VerticalObjectInfo.FillMap[*rowV] == 0xFC ) ) ||
                                  ( ( *rowH != 0 ) && ( data->HorizontalObjectInfo.FillMap[*rowH] == 0xFC ) ) )
                        {
                            row[0] = 255;
                            row[1] = 0;
                            row[2] = 255;
                        }
                    }
                }
            }
        }*/
        /// <<

        // 5 - collect found clusters of vertical/horizontal lines
        if ( ret == SuccessCode )
        {
            ObjectsCluster* barcodeClusters = data->VerticalObjectInfo.LinesClusters;
            ObjectsCluster* currentCluster;
            uint32_t        clustersCounter = 0;

            /*
            uint32_t verticalClusters   = 0;
            uint32_t horizontalClusters = 0;

            char     buffer[32];
            xargb    c, c2, b;

            c.argb = 0xFFFF0000;
            b.argb = 0XFF000000;
            c2.argb = 0xFF00FF00;
            */

            // merge two lists of clusters
            if ( data->HorizontalObjectInfo.LinesClusters != 0 )
            {
                if ( barcodeClusters == 0 )
                {
                    // no vertical clusters at all
                    barcodeClusters = data->HorizontalObjectInfo.LinesClusters;
                }
                else
                {
                    ObjectsCluster* currentCluster = barcodeClusters;

                    // append horizontal clusters to the end of vertical clusters
                    while ( currentCluster->Next != 0 )
                    {
                        currentCluster = currentCluster->Next;
                    }

                    currentCluster->Next = data->HorizontalObjectInfo.LinesClusters;
                }
            }

            // sort clusters in descending order by number of lines in them
            SortClusters( &barcodeClusters );

            currentCluster = barcodeClusters;
            while ( ( currentCluster != 0 ) && ( clustersCounter < maxBarcodes ) )
            {
                xrect    rect;
                xpoint   quad[4];

                // reject anything small, if we already good big enough cluster
                if ( ( clustersCounter > 0 ) && ( currentCluster->ObjectsCount < LINES_MIN_SECOND_CLUSTER_SIZE ) )
                {
                    break;
                }

                GetClusterRectAndQuad( currentCluster,
                    ( currentCluster->IsVertical ) ? &data->VerticalObjectInfo : &data->HorizontalObjectInfo,
                    &rect, & (quad[0]) );

                // make sure current cluster's center is not inside of any already found quads
                if ( clustersCounter > 0 )
                {
                    xpoint   clusterCenter;
                    uint32_t i;

                    clusterCenter.x = ( rect.x1 + rect.x2 ) / 2;
                    clusterCenter.y = ( rect.y1 + rect.y2 ) / 2;

                    for ( i = 0; i < clustersCounter; i++ )
                    {
                        if ( CheckPointIsInQuad( context->DetectedBarcodes[i].Quadrilateral, clusterCenter ) )
                        {
                            break;
                        }
                    }

                    if ( i != clustersCounter )
                    {
                        // don't include this cluster
                        currentCluster = currentCluster->Next;
                        continue;
                    }
                }

                if ( data->ImageResizeIsNeeded )
                {
                    int i;

                    rect.x1 = (int32_t) ( data->ResizeFactor * rect.x1 );
                    rect.y1 = (int32_t) ( data->ResizeFactor * rect.y1 );
                    rect.x2 = (int32_t) ( data->ResizeFactor * rect.x2 );
                    rect.y2 = (int32_t) ( data->ResizeFactor * rect.y2 );

                    for ( i = 0; i < 4; i++ )
                    {
                        quad[i].x = (int32_t) ( data->ResizeFactor * quad[i].x );
                        quad[i].y = (int32_t) ( data->ResizeFactor * quad[i].y );
                    }
                }

                context->DetectedBarcodes[clustersCounter].BoundingRect     = rect;
                context->DetectedBarcodes[clustersCounter].Quadrilateral[0] = quad[0];
                context->DetectedBarcodes[clustersCounter].Quadrilateral[1] = quad[1];
                context->DetectedBarcodes[clustersCounter].Quadrilateral[2] = quad[2];
                context->DetectedBarcodes[clustersCounter].Quadrilateral[3] = quad[3];
                context->DetectedBarcodes[clustersCounter].IsVertical       = currentCluster->IsVertical;

                /*
                if ( currentCluster->IsVertical )
                {
                    verticalClusters++;
                }
                else
                {
                    horizontalClusters++;
                }
                */

                clustersCounter++;
                currentCluster = currentCluster->Next;

                /*
                {
                    rect.x1 -= 4;
                    rect.y1 -= 4;
                    rect.x2 += 4;
                    rect.y2 += 4;

                    XDrawingBlendFrame( (ximage*) image, rect.x1, rect.y1, rect.x2, rect.y2, 3, 3, c );

                    XDrawingLine( (ximage*) image, quad[0].x, quad[0].y, quad[1].x, quad[1].y, c2 );
                    XDrawingLine( (ximage*) image, quad[1].x, quad[1].y, quad[2].x, quad[2].y, c2 );
                    XDrawingLine( (ximage*) image, quad[2].x, quad[2].y, quad[3].x, quad[3].y, c2 );
                    XDrawingLine( (ximage*) image, quad[3].x, quad[3].y, quad[0].x, quad[0].y, c2 );
                }
                */
            }

            context->DetectedBarcodesCount = clustersCounter;

            /*
            sprintf( buffer, "Vertical  : %u", verticalClusters );
            XDrawingText( (ximage*) image, buffer, 3, 3, c, b, true );
            sprintf( buffer, "Horizontal: %u", horizontalClusters);
            XDrawingText( (ximage*) image, buffer, 3, 13, c, b, true );
            */
        }
    }

    return ret;
}

// Try finding vertically positioned bar code areas
static XErrorCode ProcessVerticalLines( BarcodeDetectionData* data )
{
    XErrorCode ret          = SuccessCode;
    BlobsInfo* blobsInfo    = &data->VerticalObjectInfo;
    uint32_t   objectsCount = 0;
    uint32_t   objectsLeft  = 0;

    // 1 - remove horizontal lines
    ret = ErodeHorizontalEdges( data->EdgeImage, data->VerticalLinesImage );
    if ( ret == SuccessCode )
    {
        ret = ErodeHorizontalEdges( data->VerticalLinesImage, data->VerticalLinesTempImage );
    }
    if ( ret == SuccessCode )
    {
        ret = ErodeHorizontalEdges( data->VerticalLinesTempImage, data->VerticalLinesImage );
    }

    // 2 - find vertical blobs
    if ( ret == SuccessCode )
    {
        ret = BcBuildObjectsMap( data->VerticalLinesImage, data->VerticalBlobsMap, &data->VerticalObjectsCount,
                                 data->VerticalLabelsMap, data->AllocatedObjectLabelsCount );
        if ( ret == SuccessCode )
        {
            objectsCount = data->VerticalObjectsCount;
        }
    }

    if ( objectsCount > 0 )
    {
        // 3 - make sure we have enough memory for blobs' information
        ret = AllocateObjectsInfo( blobsInfo, objectsCount, data->EdgeImage->height );

        // 4 - collect blobs' bounding rectangles and areas
        if ( ret == SuccessCode )
        {
            ret = BcGetObjectsRectangles( data->VerticalBlobsMap, objectsCount, blobsInfo->Rectangles );
        }

        // 5 - filter blobs by size
        if ( ret == SuccessCode )
        {
            ret = BcBuildFillMapOutOfSizeObjects( BLOB_MIN_WIDTH, BLOB_MIN_HEIGHT, BLOB_MAX_WIDTH, BLOB_MAX_HEIGHT, true,
                                                  objectsCount, blobsInfo->Rectangles, blobsInfo->FillMap, &objectsLeft );
        }

        if ( objectsLeft > 0 )
        {
            // 6 - remove vertical lines which are not so straight or have uneven thickness
            if ( ret == SuccessCode )
            {
                objectsLeft -= KeepVLines( data );

                ret = AllocateClustersWorkingMemory( objectsLeft, &data->VerticalClustersWorkingMemory, &data->VerticalClustersAllocatedWorkingMemory );
            }

            // 7 - process clusters of vertical lines
            if ( ret == SuccessCode )
            {
                ObjectsCluster* currentCluster;

                objectsLeft -= FindLinesClusters( blobsInfo, objectsCount, objectsLeft, data->VerticalClustersWorkingMemory );

                // remove outliers from clusters
                FilterOutliersFromVericalClusters( blobsInfo, data->VerticalClustersWorkingMemory );

                // remove distant edge lines
                FilterEdgeGapsFromVerticalClusters( blobsInfo, data->VerticalClustersWorkingMemory );

                // remove clusters with rapid change of angle between lines
                FilterClustersWithNoisyAngles( blobsInfo );

                // Find quadrilaterals of the line clusters
                FindlClustersQuads( blobsInfo );

                // mark all preserved clusters as vertical
                currentCluster = blobsInfo->LinesClusters;
                while ( currentCluster != 0 )
                {
                    currentCluster->IsVertical = true;
                    currentCluster = currentCluster->Next;
                }
            }
        }
    }

    return ret;
}

// Try finding horizontally positioned bar code areas
static XErrorCode ProcessHorizontalLines( BarcodeDetectionData* data )
{
    XErrorCode ret           = SuccessCode;
    BlobsInfo* blobsInfo     = &data->HorizontalObjectInfo;
    uint32_t   objectsCount  = 0;
    uint32_t   objectsLeft   = 0;
    int32_t    imageHeightM1 = data->EdgeImage->height - 1;

    // 1 - remove vertical lines
    ret = ErodeVerticalEdges( data->EdgeImage, data->HorizontalLinesImage );
    if ( ret == SuccessCode )
    {
        ret = ErodeVerticalEdges( data->HorizontalLinesImage, data->HorizontalLinesTempImage );
    }
    if ( ret == SuccessCode )
    {
        ret = ErodeVerticalEdges( data->HorizontalLinesTempImage, data->HorizontalLinesImage );
    }

    // 2 - find horizontal blobs
    if ( ret == SuccessCode )
    {
        ret = BcBuildObjectsMap( data->HorizontalLinesImage, data->HorizontalBlobsMap, &data->HorizontalObjectsCount,
                                 data->HorizontalLabelsMap, data->AllocatedObjectLabelsCount );
        objectsCount = data->HorizontalObjectsCount;
    }

    if ( objectsCount > 0 )
    {
        // 3 - make sure we have enough memory for blobs' information
        ret = AllocateObjectsInfo( blobsInfo, objectsCount, data->EdgeImage->width );

        // 4 - collect blobs' bounding rectangles and areas
        if ( ret == SuccessCode )
        {
            ret = BcGetObjectsRectangles( data->HorizontalBlobsMap, objectsCount, blobsInfo->Rectangles );
        }

        // 5 - filter blobs by size
        if ( ret == SuccessCode )
        {
            ret = BcBuildFillMapOutOfSizeObjects( BLOB_MIN_WIDTH, BLOB_MIN_HEIGHT, BLOB_MAX_WIDTH, BLOB_MAX_HEIGHT, true,
                                                  objectsCount, blobsInfo->Rectangles, blobsInfo->FillMap, &objectsLeft );
        }

        if ( objectsLeft > 0 )
        {
            // 6 - remove horizontal lines which are not so straight or have uneven thickness
            if ( ret == SuccessCode )
            {
                objectsLeft -= KeepHLines( data );

                ret = AllocateClustersWorkingMemory( objectsLeft, &data->HorizontalClustersWorkingMemory, &data->HorizontalClustersAllocatedWorkingMemory );
            }

            // 7 - process clusters of horizontal lines
            if ( ret == SuccessCode )
            {
                ObjectsCluster* currentCluster;

                objectsLeft -= FindLinesClusters( blobsInfo, objectsCount, objectsLeft, data->HorizontalClustersWorkingMemory );

                // remove outliers from clusters
                FilterOutliersFromVericalClusters( blobsInfo, data->HorizontalClustersWorkingMemory );

                // remove distant edge lines
                FilterEdgeGapsFromVerticalClusters( blobsInfo, data->HorizontalClustersWorkingMemory );

                // remove clusters with rapid change of angle between lines
                FilterClustersWithNoisyAngles( blobsInfo );

                // Find quadrilaterals of the line clusters
                FindlClustersQuads( blobsInfo );

                // mark all preserved clusters as horizontal and restore lines' coordinates
                currentCluster = blobsInfo->LinesClusters;
                while ( currentCluster != 0 )
                {
                    uint32_t i, n = currentCluster->ObjectsCount;
                    uint32_t blobId;
                    xpoint*  startPoints = blobsInfo->LineStartPoints;
                    xpoint*  endPoints   = blobsInfo->LineEndPoints;
                    int32_t  x, y;

                    for ( i = 0; i < n; i++ )
                    {
                        blobId = currentCluster->Objects[i];

                        x = startPoints[blobId].y;
                        y = imageHeightM1 - startPoints[blobId].x;

                        startPoints[blobId].x = x;
                        startPoints[blobId].y = y;

                        x = endPoints[blobId].y;
                        y = imageHeightM1 - endPoints[blobId].x;

                        endPoints[blobId].x = x;
                        endPoints[blobId].y = y;
                    }

                    for ( i = 0; i < 4; i++ )
                    {
                        x = currentCluster->Quad[i].y;
                        y = imageHeightM1 - currentCluster->Quad[i].x;

                        currentCluster->Quad[i].x = x;
                        currentCluster->Quad[i].y = y;
                    }

                    currentCluster->IsVertical = false;
                    currentCluster = currentCluster->Next;
                }
            }
        }
    }
    return ret;
}

// Check if the blob looks like a "vertical" line line (slope values >= 1)
static bool CheckVLine( const ximage* blobMap, uint32_t blobId, xrect rect, int32_t* thicknessBuffer, float* meanThick, float* slope, bool* isVertical, int32_t* topX, int32_t* bottomX )
{
    bool ret   = true;

    int stride = blobMap->stride;
    int startX = rect.x1;
    int endX   = rect.x2;
    int startY = rect.y1;
    int endY   = rect.y2;
    int y, i;

    int blobHeightM1 = endY - startY;
    int blobHeight   = blobHeightM1 + 1;

    int32_t left, right;
    int32_t topMidX, bottomMidX, midX;

    uint8_t*  ptr = blobMap->data;
    uint32_t* row;

    float meanThickness = 0.0f, meanThicknessDev = 0.0f, diff;
    int   delta, maxDelta = XINRANGE( (int) ( LINE_MUL_DELTA * blobHeight ), LINE_MIN_DELTA, LINE_MAX_DELTA );

    // last row of the object
    {
        row = (uint32_t*) ( ptr + endY * stride + startX * 4 );

        left = startX;
        while ( *row != blobId )
        {
            row++;
            left++;
        }

        row = (uint32_t*) ( ptr + endY * stride + endX * 4 );

        right = endX;

        while ( *row != blobId )
        {
            row--;
            right--;
        }

        thicknessBuffer[blobHeightM1] = right - left + 1;
        meanThickness += thicknessBuffer[blobHeightM1];
        topMidX = bottomMidX = ( left + right ) / 2;
    }

    // the rest of rows
    for ( y = startY, i = 0; y < endY; y++, i++ )
    {
        row = (uint32_t*) ( ptr + y * stride + startX * 4 );

        left = startX;
        while ( *row != blobId )
        {
            row++;
            left++;
        }

        row = (uint32_t*) ( ptr + y * stride + endX * 4 );

        right = endX;

        while ( *row != blobId )
        {
            row--;
            right--;
        }

        thicknessBuffer[i] = right - left + 1;
        meanThickness += thicknessBuffer[i];
        midX = left + ( right - left ) / 2;

        if ( i == 0 )
        {
            topMidX = midX;
        }
        else
        {
            // check deviation from the line connecting top and bottom points
            delta = ( topMidX + ( bottomMidX - topMidX ) * i / blobHeightM1 ) - midX;
            if ( delta < 0 ) delta = -delta;

            if ( delta >= maxDelta )
            {
                ret = false;
            }
        }
    }

    // mean thickness of the object in horizontal direction
    meanThickness /= blobHeight;

    // calculate mean deviation from mean thickness
    for ( i = 0; i < blobHeight; i++ )
    {
        diff = meanThickness - thicknessBuffer[i];
        if ( diff < 0 ) diff = -diff;

        meanThicknessDev += diff;
    }
    meanThicknessDev /= blobHeight;

    // check if mean tickness and its deviation fall into limits
    int maxMeanThickness = XMIN( (int) ( LINE_MUL_MEAN_THICKNESS * blobHeight ), LINE_MAX_MEAN_THICKNESS );
    int maxMeanThickDev  = XINRANGE( (int) ( LINE_MUL_MEAN_THICKNESS_DEV * blobHeight ), LINE_MIN_MEAN_THICKNESS_DEV, LINE_MAX_MEAN_THICKNESS_DEV );

    if ( ( meanThicknessDev > maxMeanThickDev ) || ( meanThickness > maxMeanThickness ) )
    {
        ret = false;
    }

    // provide slope of the object
    if ( topMidX == bottomMidX )
    {
        *isVertical = true;
        *slope      = 0;
    }
    else
    {
        *isVertical = false;
        *slope      = (float) ( endY - startY ) / ( bottomMidX - topMidX );
    }

    // provide middle X coordinates at the top and bottom rows
    *topX      = topMidX;
    *bottomX   = bottomMidX;

    *meanThick = meanThickness;

    return ret;
}

// Check if the blob looks like a "horizontal" line line (slope values <= 1)
static bool CheckHLine( const ximage* blobMap, uint32_t blobId, xrect rect, int32_t* thicknessBuffer, float* meanThick, float* slope, int32_t* leftY, int32_t* rightY )
{
    bool ret = true;

    int stride = blobMap->stride;
    int startX = rect.x1;
    int endX   = rect.x2;
    int startY = rect.y1;
    int endY   = rect.y2;
    int x, i;

    int blobWidthM1 = endX - startX;
    int blobWidth   = blobWidthM1 + 1;

    int32_t top, bottom;
    int32_t leftMidY, rightMidY, midY;

    uint8_t* ptr = blobMap->data;
    uint8_t* col;

    float meanThickness = 0.0f, meanThicknessDev = 0.0f, diff;
    int   delta, maxDelta = XINRANGE( (int) ( LINE_MUL_DELTA * blobWidth ), LINE_MIN_DELTA, LINE_MAX_DELTA );

    // last column of the object
    {
        col = ptr + startY * stride + endX * 4;

        top = startY;
        while ( *( (uint32_t*) col ) != blobId )
        {
            col += stride;
            top++;
        }

        col = ptr + endY * stride + endX * 4;

        bottom = endY;
        while ( *( (uint32_t*) col ) != blobId )
        {
            col -= stride;
            bottom--;
        }

        thicknessBuffer[blobWidthM1] = bottom - top + 1;
        meanThickness += thicknessBuffer[blobWidthM1];
        leftMidY = rightMidY = ( top + bottom ) / 2;
    }

    // the rest of columns
    for ( x = startX, i = 0; x < endX; x++, i++ )
    {
        col = ptr + startY * stride + x * 4;

        top = startY;
        while ( *( (uint32_t*) col ) != blobId )
        {
            col += stride;
            top++;
        }

        col = ptr + endY * stride + x * 4;

        bottom = endY;
        while ( *( (uint32_t*) col ) != blobId )
        {
            col -= stride;
            bottom--;
        }

        thicknessBuffer[i] = bottom - top + 1;
        meanThickness += thicknessBuffer[i];
        midY = ( top + bottom ) / 2;

        if ( i == 0 )
        {
            leftMidY = midY;
        }
        else
        {
            delta = ( leftMidY + ( rightMidY - leftMidY ) * i / ( blobWidthM1 ) ) - midY;
            if ( delta < 0 ) delta = -delta;

            if ( delta >= maxDelta )
            {
                ret = false;
            }
        }
    }

    // mean thickness of the object in vertical direction
    meanThickness /= blobWidth;

    // calculate mean deviation from mean thickness
    for ( i = 0; i < blobWidth; i++ )
    {
        diff = meanThickness - thicknessBuffer[i];
        if ( diff < 0 ) diff = -diff;

        meanThicknessDev += diff;
    }
    meanThicknessDev /= blobWidth;

    // check if mean thickness and its deviation fall into limits
    int maxMeanThickness = XMIN( (int) ( LINE_MUL_MEAN_THICKNESS * blobWidth ), LINE_MAX_MEAN_THICKNESS );
    int maxMeanThickDev  = XINRANGE( (int) ( LINE_MUL_MEAN_THICKNESS_DEV * blobWidth ), LINE_MIN_MEAN_THICKNESS_DEV, LINE_MAX_MEAN_THICKNESS_DEV );

    if ( ( meanThicknessDev > maxMeanThickDev ) || ( meanThickness > maxMeanThickness ) )
    {
        ret = false;
    }

    // provide slope of the object
    if ( startX != endX )
    {
        *slope = (float) ( rightMidY - leftMidY ) / ( endX - startX );
    }
    else
    {
        *slope = 0;
    }

    // provide middle Y coordinates at the left and right columns
    *leftY     = leftMidY;
    *rightY    = rightMidY;

    *meanThick = meanThickness;

    return ret;
}

// Keep more or less straight vertical lines, removing curves or objects with uneven thickness
uint32_t KeepVLines( BarcodeDetectionData* data )
{
    BlobsInfo* blobsInfo    = &data->VerticalObjectInfo;
    uint8_t*   fillMap      = blobsInfo->FillMap;
    uint32_t   objectsCount = data->VerticalObjectsCount;
    uint32_t   i, j;
    float      slope, meanThickness;
    bool       straightV;
    int32_t    topX, bottomX;
    uint32_t   counter = 0;

    for ( i = 0, j = 1; i < objectsCount; i++, j++ )
    {
        if ( fillMap[j] == 0 )
        {
            if ( CheckVLine( data->VerticalBlobsMap, j, blobsInfo->Rectangles[i], blobsInfo->LineThicknessBuffer,
                             &meanThickness, &slope, &straightV, &topX, &bottomX ) == false )
            {
                fillMap[j] = 0xFF; // TODO: to be set 1
                counter++;
            }
            else
            {
                blobsInfo->LineStartPoints[i].x = topX;
                blobsInfo->LineStartPoints[i].y = blobsInfo->Rectangles[i].y1;

                blobsInfo->LineEndPoints[i].x = bottomX;
                blobsInfo->LineEndPoints[i].y = blobsInfo->Rectangles[i].y2;

                blobsInfo->LineSlopes[i]   = slope;
                blobsInfo->LineVertical[i] = straightV;

                blobsInfo->LineMeanThickness[i] = meanThickness;
            }
        }
    }

    return counter;
}

// Keep more or less straight horizontal lines, removing curves or objects with uneven thickness
uint32_t KeepHLines( BarcodeDetectionData* data )
{
    BlobsInfo* blobsInfo     = &data->HorizontalObjectInfo;
    uint8_t*   fillMap       = blobsInfo->FillMap;
    uint32_t   objectsCount  = data->HorizontalObjectsCount;
    uint32_t   i, j;
    float      slope, meanThickness;
    int32_t    leftY, rightY;
    int32_t    imageHeightM1 = data->EdgeImage->height - 1;
    uint32_t   counter = 0;

    for ( i = 0, j = 1; i < objectsCount; i++, j++ )
    {
        if ( fillMap[j] == 0 )
        {
            if ( CheckHLine( data->HorizontalBlobsMap, j, blobsInfo->Rectangles[i], blobsInfo->LineThicknessBuffer,
                             &meanThickness, &slope, &leftY, &rightY ) == false )
            {
                fillMap[j] = 0xFF; // TODO: to be set 1
                counter++;
            }
            else
            {
                // rotate 90 degrees and invert slope - this way the line can be processed as vertical
                blobsInfo->LineStartPoints[i].y = blobsInfo->Rectangles[i].x1;
                blobsInfo->LineStartPoints[i].x = imageHeightM1 - leftY;

                blobsInfo->LineEndPoints[i].y = blobsInfo->Rectangles[i].x2;
                blobsInfo->LineEndPoints[i].x = imageHeightM1 - rightY;

                if ( slope != 0.0f )
                {
                    blobsInfo->LineSlopes[i]   = -1.0f / slope;
                    blobsInfo->LineVertical[i] = false;
                }
                else
                {
                    blobsInfo->LineSlopes[i]   = 0.0f;
                    blobsInfo->LineVertical[i] = true;
                }

                blobsInfo->LineMeanThickness[i] = meanThickness;
            }
        }
    }

    return counter;
}

// Intersect two clusters and put result into the first one (which is supposed to have more allocated memory)
static bool IntersectOverlappingClusters( ObjectsCluster* cluster1, ObjectsCluster* cluster2, uint32_t* intersectionBuffer )
{
    bool      foundIntersection = false;
    uint32_t* objects1 = cluster1->Objects;
    uint32_t* objects2 = cluster2->Objects;
    uint32_t  counter1 = 0, counter2 = 0, counterI = 0;
    uint32_t  objectsCount1 = cluster1->ObjectsCount;
    uint32_t  objectsCount2 = cluster2->ObjectsCount;

    while ( ( counter1 != objectsCount1 ) && ( counter2 != objectsCount2 ) )
    {
        if ( objects1[counter1] < objects2[counter2] )
        {
            intersectionBuffer[counterI] = objects1[counter1];
            counterI++;
            counter1++;
            continue;
        }

        if ( objects2[counter2] < objects1[counter1] )
        {
            intersectionBuffer[counterI] = objects2[counter2];
            counterI++;
            counter2++;
            continue;
        }

        intersectionBuffer[counterI] = objects1[counter1];
        counterI++;
        counter1++;
        counter2++;

        foundIntersection = true;
    }

    if ( foundIntersection )
    {
        // copy remaining
        while ( counter1 != objectsCount1 )
        {
            intersectionBuffer[counterI] = objects1[counter1];
            counterI++;
            counter1++;
        }
        while ( counter2 != objectsCount2 )
        {
            intersectionBuffer[counterI] = objects2[counter2];
            counterI++;
            counter2++;
        }

        // move it into the first cluster, which should have enough pre-allocated memory to fit the intersection
        memcpy( objects1, intersectionBuffer, counterI * sizeof( uint32_t ) );
        cluster1->ObjectsCount = counterI;
    }

    return foundIntersection;
}

// Remove clusters with too few elements
static void FilterClustersBySize( BlobsInfo* blobsInfo )
{
    ObjectsCluster* clustersList   = blobsInfo->LinesClusters;
    ObjectsCluster* currentCluster = clustersList;
    ObjectsCluster* prevCluster    = 0;
    uint8_t*        fillMap        = blobsInfo->FillMap;
    uint32_t        i;

    while ( currentCluster != 0 )
    {
        if ( currentCluster->ObjectsCount < LINES_MIN_CLUSTER_SIZE )
        {
            // >> TODO: to remove
            for ( i = 0; i < currentCluster->ObjectsCount; i++ )
            {
                fillMap[currentCluster->Objects[i] + 1] = 0xFE;
            }
            // <<

            currentCluster = currentCluster->Next;

            if ( prevCluster == 0 )
            {
                clustersList = currentCluster;
            }
            else
            {
                prevCluster->Next = currentCluster;
            }
        }
        else
        {
            prevCluster    = currentCluster;
            currentCluster = currentCluster->Next;
        }
    }

    blobsInfo->LinesClusters = clustersList;
}

// Find lines' clusters - groups of lines staying close to each other
uint32_t FindLinesClusters( BlobsInfo* blobsInfo, uint32_t objectsCount, uint32_t objectsLeft, uint8_t* workingBuffer )
{
    uint32_t    objectsCountM1 = objectsCount - 1;
    xpoint*     startPoints    = blobsInfo->LineStartPoints;
    xpoint*     endPoints      = blobsInfo->LineEndPoints;
    uint8_t*    fillMap        = blobsInfo->FillMap;
    xpoint      start1, end1, start2, end2;
    uint32_t    i, j, i1, j1, processedObjects = 0, distantObjects = 0;
    int         diffX, diffY, maxDistance = LINES_MAX_DISTANCE * LINES_MAX_DISTANCE;
    bool        looksNear;

    ObjectsCluster* clustersList   = 0;
    ObjectsCluster* currentCluster = 0;
    ObjectsCluster* prevCluster    = 0;
    ObjectsCluster* nextCluster    = 0;
    ObjectsCluster* lastCluster    = 0;

    // point to available memory
    uint8_t* memPtr = workingBuffer;

    // buffer to build intersection in
    uint32_t* intersectionBuffer = (uint32_t*) memPtr;
    memPtr += sizeof( uint32_t ) * objectsLeft * 2;

    for ( i = 0, i1 = 1; i < objectsCountM1; i++, i1++ )
    {
        // skip filtered objects
        if ( ( fillMap[i1] == 1 ) ||
             ( fillMap[i1] > 0xF0 ) ) // TODO: to be removed
        {
            continue;
        }

        start1 = startPoints[i];
        end1   = endPoints[i];

        // get memory for the cluster structure
        currentCluster = (ObjectsCluster*) memPtr;
        memPtr += sizeof( ObjectsCluster );

        // get memory for the cluster members
        currentCluster->Objects = (uint32_t*) memPtr;
        memPtr += sizeof( uint32_t ) * ( objectsLeft - processedObjects );

        // initialize the cluster put itself as the first member
        currentCluster->Next         = 0;
        currentCluster->Objects[0]   = i;
        currentCluster->ObjectsCount = 1;

        // increment number of processed objects
        processedObjects++;

        for ( j = i + 1, j1 = j + 1; j < objectsCount; j++, j1++ )
        {
            // skip filtered objects
            if ( ( fillMap[j1] == 1 ) ||
                 ( fillMap[j1] > 0xF0 ) ) // TODO: to be removed
            {
                continue;
            }

            start2    = startPoints[j];
            end2      = endPoints[j];
            looksNear = false;

            // check start points first
            diffX = start1.x - start2.x;
            diffY = start1.y - start2.y;

            if ( diffX * diffX + diffY * diffY <= maxDistance )
            {
                looksNear = true;
            }
            else
            {
                // check end points next
                diffX = end1.x - end2.x;
                diffY = end1.y - end2.y;

                if ( diffX * diffX + diffY * diffY <= maxDistance )
                {
                    looksNear = true;
                }
                else
                {
                    // finally check mid-points
                    diffX = ( ( start1.x + end1.x ) - ( start2.x + end2.x ) ) / 2;
                    diffY = ( ( start1.y + end1.y ) - ( start2.y + end2.y ) ) / 2;

                    if ( diffX * diffX + diffY * diffY <= maxDistance )
                    {
                        looksNear = true;
                    }
                }
            }

            // the objects seem to be close to each other
            if ( looksNear )
            {
                fillMap[j1] = 2;
                currentCluster->Objects[currentCluster->ObjectsCount] = j;
                currentCluster->ObjectsCount++;
            }
        }

        // check if there is anyone else in the cluster
        if ( currentCluster->ObjectsCount == 1 )
        {
            if ( fillMap[i1] == 0 )
            {
                distantObjects++;
                fillMap[i1] = 0xFE;
            }
        }
        else
        {
            if ( clustersList == 0 )
            {
                clustersList = currentCluster;
            }

            if ( lastCluster != 0 )
            {
                lastCluster->Next = currentCluster;
            }

            lastCluster = currentCluster;
        }

        // repair map
        if ( fillMap[i1] == 2 )
        {
            fillMap[i1] = 0;
        }
    }

    // repair the last object
    if ( fillMap[objectsCount] == 2 )
    {
        fillMap[objectsCount] = 0;
    }
    else
    {
        // TODO: to remove
        fillMap[objectsCount] = 0xFE;
    }

    // merge overlapping sub-clusters
    if ( clustersList != 0 )
    {
        bool foundIntersection;

        currentCluster = clustersList;

        while ( currentCluster != 0 )
        {
            foundIntersection = false;
            prevCluster       = currentCluster;
            nextCluster       = currentCluster->Next;

            while ( nextCluster != 0 )
            {
                if ( IntersectOverlappingClusters( currentCluster, nextCluster, intersectionBuffer ) )
                {
                    foundIntersection = true;
                    nextCluster = nextCluster->Next;
                    // remove just processed cluster, since it got merged into current cluster
                    prevCluster->Next = nextCluster;
                }
                else
                {
                    prevCluster = nextCluster;
                    nextCluster = nextCluster->Next;
                }
            }

            if ( foundIntersection == false )
            {
                currentCluster = currentCluster->Next;
            }
        }
    }

    blobsInfo->LinesClusters = clustersList;

    // don't bother about clusters with too few elements
    FilterClustersBySize( blobsInfo );

    return distantObjects;
}

// Remove specified objects from the cluster
static void PruneObjectsFromCluster( ObjectsCluster* cluster, const uint32_t* pruningBuffer, uint32_t objectsToRemove )
{
    uint32_t* objects1 = cluster->Objects;
    uint32_t  counter1 = 0, counter2 = 0, counterR = 0;
    uint32_t  objectsCount1 = cluster->ObjectsCount;

    while ( ( counter1 != objectsCount1 ) && ( counter2 != objectsToRemove ) )
    {
        if ( objects1[counter1] < pruningBuffer[counter2] )
        {
            objects1[counterR] = objects1[counter1];
            counterR++;
            counter1++;
            continue;
        }

        if ( pruningBuffer[counter2] < objects1[counter1] )
        {
            // prunning buffer contains something, which in not in the cluster - ignore it
            counter2++;
            continue;
        }

        // found something to remove
        counter1++;
        counter2++;
    }

    // copy remainig if anything left
    while ( counter1 != objectsCount1 )
    {
        objects1[counterR] = objects1[counter1];
        counterR++;
        counter1++;
    }

    cluster->ObjectsCount = counterR;
}

// A helper function for sorting floating point numbers
static int CompareFloats( const void* item1, const void* item2 )
{
    if ( *(float*) item1 <  *(float*) item2 ) return -1;
    if ( *(float*) item1 == *(float*) item2 ) return  0;

    return  1;
}

// Check clusters of vertical objects and remove outliers/noisy objects
void FilterOutliersFromVericalClusters( BlobsInfo* blobsInfo, uint8_t* workingBuffer )
{
    ObjectsCluster* currentCluster = blobsInfo->LinesClusters;
    uint32_t*       pruningBuffer  = (uint32_t*) workingBuffer;
    bool            firstPass      = true;

    while ( currentCluster != 0 )
    {
        int32_t topX      = 0, topY      = 0;
        int32_t bottomX   = 0, bottomY   = 0;
        int32_t blobCX    = 0, blobCY    = 0;
        int32_t clusterCX = 0, clusterCY = 0;
        int32_t avgHeight = 0;

        bool     verticalLines = false;
        float    avgSlope      = 0.0f;
        float    avgAngle      = 0.0f;
        uint32_t angleCounter  = 0;
        float    scanSlope     = 0.0f;
        float    topB = 0.0f, bottomB = 0.0f;
        float    angle;

        uint32_t* objects = currentCluster->Objects;
        uint32_t  i, n = currentCluster->ObjectsCount;
        uint32_t  blobId;
        uint32_t  removeCounter = 0;
        xpoint    lineStart;
        xpoint    lineEnd;

        // find average lines' slope first and some other average values
        for ( i = 0; i < n; i++ )
        {
            blobId     = objects[i];
            lineStart  = blobsInfo->LineStartPoints[blobId];
            lineEnd    = blobsInfo->LineEndPoints[blobId];

            topX      += lineStart.x;
            topY      += lineStart.y;

            bottomX   += lineEnd.x;
            bottomY   += lineEnd.y;

            avgHeight += ( lineEnd.y - lineStart.y + 1 );

            if ( blobsInfo->LineVertical[blobId] == false )
            {
                if ( firstPass )
                {
                    if ( blobsInfo->LineSlopes[blobId] > 0 )
                    {
                        angle = (float) atan( blobsInfo->LineSlopes[blobId] );
                    }
                    else
                    {
                        angle = (float) ( XPI + atan( blobsInfo->LineSlopes[blobId] ) );
                    }

                    blobsInfo->LineSlopes[blobId] = angle;
                }
                else
                {
                    angle = blobsInfo->LineSlopes[blobId];
                }

                avgAngle += angle;
                angleCounter++;
            }
            else
            {
                if ( firstPass )
                {
                    blobsInfo->LineSlopes[blobId] = (float) XPIHalf;
                }
            }
        }

        topX      /= n;
        topY      /= n;
        bottomX   /= n;
        bottomY   /= n;
        avgHeight /= n;

        clusterCX = ( topX + bottomX ) / 2;
        clusterCY = ( topY + bottomY ) / 2;

        if ( angleCounter != 0 )
        {
            avgAngle /= angleCounter;
            avgSlope  = (float) tan( avgAngle );
            scanSlope = -1.0f / avgSlope;

            topB      = (float) topY    - scanSlope * topX;
            bottomB   = (float) bottomY - scanSlope * bottomX;

            currentCluster->AverageAngle = avgAngle;
        }
        else
        {
            verticalLines = true;
            currentCluster->AverageAngle = (float) ( XPIHalf );
        }

        // Top/bottom X/Y points are not very accurate at this point, since calculation was based
        // on averaging coordinates. Some correction will be done based on median top/bottom distances
        // from the middle line.
        {
            float* distToStart = (float*) workingBuffer;
            float* distToEnd   = (float*) ( workingBuffer + sizeof( float ) * n );

            if ( verticalLines )
            {
                // find Y difference between center of the cluster and start/end points of lines in it
                for ( i = 0; i < n; i++ )
                {
                    blobId    = objects[i];
                    lineStart = blobsInfo->LineStartPoints[blobId];
                    lineEnd   = blobsInfo->LineEndPoints[blobId];

                    distToStart[i] = (float) ( lineStart.y - clusterCY );
                    distToEnd[i]   = (float) ( lineEnd.y   - clusterCY );
                }

                // sort to get median
                qsort( distToStart, n, sizeof( float ), CompareFloats );
                qsort( distToEnd,   n, sizeof( float ), CompareFloats );

                topY    = (int32_t) ( clusterCY + distToStart[n / 2] );
                bottomY = (int32_t) ( clusterCY + distToEnd[n / 2] );
            }
            else
            {
                float midB        = (float) clusterCY - scanSlope * clusterCX;
                float distanceDiv = (float) sqrt( 1 + scanSlope * scanSlope );

                // calculate unit vector pointing from middle line towards bottom line
                float slopeB = (float) clusterCY - avgSlope * clusterCX;
                float y2     = (float) topY;
                float x2     = ( y2 - slopeB ) / avgSlope;
                float unitX  = clusterCX - x2;
                float unitY  = clusterCY - y2;
                float norm   = (float) sqrt( unitX * unitX + unitY * unitY );
                float distance, dx, dy;

                unitX /= norm;
                unitY /= norm;

                // find distances between the line going through the center of the cluster and start/end points of lines in it
                for ( i = 0; i < n; i++ )
                {
                    blobId    = objects[i];
                    lineStart = blobsInfo->LineStartPoints[blobId];
                    lineEnd   = blobsInfo->LineEndPoints[blobId];

                    distToStart[i] = -( midB + scanSlope * lineStart.x - lineStart.y ) / distanceDiv;
                    distToEnd[i]   = -( midB + scanSlope * lineEnd.x   - lineEnd.y ) / distanceDiv;
                }

                // sort to get median
                qsort( distToStart, n, sizeof( float ), CompareFloats );
                qsort( distToEnd,   n, sizeof( float ), CompareFloats );

                // update topX in the case it gives wider area
                distance = distToStart[n / 2];
                dx = (float) ( topX - clusterCX );
                dy = (float) ( topY - clusterCY );

                if ( dx * dx + dy * dy < distance * distance )
                {
                    topX = (int32_t) ( (float) clusterCX + unitX * distance );
                    topY = (int32_t) ( (float) clusterCY + unitY * distance );
                    topB = (float) topY - scanSlope * topX;
                }

                // update bottomX in the case it gives wider area
                distance = distToEnd[n / 2];
                dx = (float) ( bottomX - clusterCX );
                dy = (float) ( bottomY - clusterCY );

                if ( dx * dx + dy * dy < distance * distance )
                {
                    bottomX = (int32_t) ( (float) clusterCX + unitX * distance );
                    bottomY = (int32_t) ( (float) clusterCY + unitY * distance );
                    bottomB = (float) bottomY - scanSlope * bottomX;
                }
            }
        }

        currentCluster->Center.x = clusterCX;
        currentCluster->Center.y = clusterCY;

        currentCluster->TopCenter.x = topX;
        currentCluster->TopCenter.y = topY;

        currentCluster->BottomCenter.x = bottomX;
        currentCluster->BottomCenter.y = bottomY;

        // Now it is time to correct average lines angle and, as the result, the slope for
        // top/mid/bottom scan lines.
        {
            float    distanceDiv = (float) sqrt( 1 + scanSlope * scanSlope );
            float    dStart, dEnd;

            float    leftMedianDist, rightMedianDist;
            uint32_t leftCounter = 0, rightCounter = 0;
            float    leftCX = 0.0f, rightCX = 0.0f;

            float*   leftDist  = (float*) workingBuffer;
            float*   rightDist = (float*) ( workingBuffer + sizeof( float ) * n );

            uint32_t leftCounter2 = 0, rightCounter2 = 0;

            for ( i = 0; i < n; i++ )
            {
                blobId    = objects[i];
                lineStart = blobsInfo->LineStartPoints[blobId];
                lineEnd   = blobsInfo->LineEndPoints[blobId];

                blobCX = ( lineStart.x + lineEnd.x ) / 2;
                blobCY = ( lineStart.y + lineEnd.y ) / 2;

                dStart = -( topB    + scanSlope * lineStart.x - lineStart.y ) / distanceDiv;
                dEnd   = -( bottomB + scanSlope * lineEnd.x   - lineEnd.y   ) / distanceDiv;

                if ( blobCX < clusterCX )
                {
                    leftCX += (float) blobCX;
                    leftCounter++;

                    if ( leftCounter2 < n - 1 )
                    {
                        leftDist[leftCounter2]     = dStart;
                        leftDist[leftCounter2 + 1] = dEnd;
                        leftCounter2 += 2;
                    }
                }
                else
                {
                    rightCX += (float) blobCX;
                    rightCounter++;

                    if ( rightCounter2 < n - 1 )
                    {
                        rightDist[rightCounter2]     = dStart;
                        rightDist[rightCounter2 + 1] = dEnd;
                        rightCounter2 += 2;
                    }
                }
            }

            // average X coordinate for objects on the left and right from the mid line
            leftCX  /= leftCounter;
            rightCX /= rightCounter;

            // sort distances to get median value
            qsort( leftDist,  leftCounter2,  sizeof( float ), CompareFloats );
            qsort( rightDist, rightCounter2, sizeof( float ), CompareFloats );

            leftMedianDist  = leftDist [leftCounter2  / 2];
            rightMedianDist = rightDist[rightCounter2 / 2];

            {
                float diffSlope = ( rightMedianDist - leftMedianDist ) / ( rightCX - leftCX );

                if ( diffSlope != 0.0f )
                {
                    float diffAngle = (float) atan( diffSlope );

                    // correct average lines' angle and update scan slopes
                    avgAngle += diffAngle;
                    currentCluster->AverageAngle = avgAngle;

                    if ( avgAngle != (float) XPIHalf )
                    {
                        avgSlope  = (float) tan( avgAngle );
                        scanSlope = -1.0f / avgSlope;

                        topB      = (float) topY - scanSlope * topX;
                        bottomB   = (float) bottomY - scanSlope * bottomX;

                        verticalLines = false;
                    }
                    else
                    {
                        verticalLines = true;
                    }
                }
            }
        }

        // collect list of objects, which look like noise
        for ( i = 0; i < n; i++ )
        {
            int32_t topYlimit    = topY;
            int32_t bottomYlimit = bottomY;;

            blobId    = objects[i];
            lineStart = blobsInfo->LineStartPoints[blobId];
            lineEnd   = blobsInfo->LineEndPoints[blobId];
            blobCX    = ( lineStart.x + lineEnd.x ) / 2;
            blobCY    = ( lineStart.y + lineEnd.y ) / 2;

            if ( verticalLines == false )
            {
                topYlimit    = (int32_t) ( scanSlope * blobCX + topB );
                bottomYlimit = (int32_t) ( scanSlope * blobCX + bottomB );
            }

            if ( ( blobCY < topYlimit ) || ( blobCY > bottomYlimit ) ||
                 ( lineStart.y + avgHeight / 2 < topYlimit ) ||
                 ( lineEnd.y   - avgHeight / 2 > bottomYlimit ) )
            {
                pruningBuffer[removeCounter] = blobId;

                removeCounter++;

                blobsInfo->FillMap[blobId + 1] = 0xFD;
            }
        }

        // remove noisy objects from the cluster
        if ( removeCounter != 0 )
        {
            PruneObjectsFromCluster( currentCluster, pruningBuffer, removeCounter );
        }

        // keep cleaning the cluster until there is something to clean
        if ( ( removeCounter == 0 ) || ( currentCluster->ObjectsCount < LINES_MIN_CLUSTER_SIZE ) )
        {
            currentCluster = currentCluster->Next;
            firstPass      = true;
        }
        else
        {
            // continue with the same cluster
            firstPass = false;
        }
    }

    // don't bother about clusters with too few elements
    FilterClustersBySize( blobsInfo );
}

// Filter distant line from left/rigth edges of vertical clusters
void FilterEdgeGapsFromVerticalClusters( BlobsInfo* blobsInfo, uint8_t* workingBuffer )
{
    ObjectsCluster* currentCluster = blobsInfo->LinesClusters;
    uint8_t*        fillMap        = blobsInfo->FillMap;

    while ( currentCluster != 0 )
    {
        uint32_t* objects = currentCluster->Objects;
        uint32_t  i, n    = currentCluster->ObjectsCount;
        uint32_t  blobId;
        xpoint    lineStart;
        xpoint    lineEnd;

        uint32_t  clusterCx = 0, clusterCy = 0;
        uint32_t  blobCx    = 0, blobCy    = 0;
        uint32_t  leftLineCx, leftLineCy, rightLineCx, rightLineCy;
        uint32_t  clusterWidth;
        float     avgAngle = 0.0f;

        float     midLineK = 0.0f, midLineB = 0.0f, midLineDistanceDiv = 1.0f, midLineSignMul = 1;
        bool      vertical = false;

        float     distanceToCenter, distBetweenLines, meanDistBetweenLines = 0.0f;

        float*    distancesToCenter       = (float*) workingBuffer;
        float*    distancesToCenterSorted = (float*) ( workingBuffer + sizeof( float ) * n );
        uint32_t* pruningBuffer           = (uint32_t*) distancesToCenterSorted;

        blobId    = objects[0];
        lineStart = blobsInfo->LineStartPoints[blobId];
        lineEnd   = blobsInfo->LineEndPoints[blobId];

        leftLineCx = rightLineCx = ( lineStart.x + lineEnd.x ) / 2;
        leftLineCy = rightLineCy = ( lineStart.y + lineEnd.y ) / 2;

        // calculate center of of the cluster, most left/right lines and average angle of lines
        for ( i = 0; i < n; i++ )
        {
            blobId     = objects[i];
            lineStart  = blobsInfo->LineStartPoints[blobId];
            lineEnd    = blobsInfo->LineEndPoints[blobId];

            blobCx     = ( lineStart.x + lineEnd.x ) / 2;
            blobCy     = ( lineStart.y + lineEnd.y ) / 2;

            avgAngle  += blobsInfo->LineSlopes[blobId];

            clusterCx += blobCx;
            clusterCy += blobCy;

            if ( blobCx < leftLineCx )
            {
                leftLineCx = blobCx;
                leftLineCy = blobCy;
            }

            if ( blobCx > rightLineCx )
            {
                rightLineCx = blobCx;
                rightLineCy = blobCy;
            }
        }

        clusterCx /= n;
        clusterCy /= n;
        avgAngle  /= n;

        if ( avgAngle == (float) XPIHalf )
        {
            clusterWidth = rightLineCx - leftLineCx + 1;
            vertical     = true;
        }
        else
        {
            midLineK = (float) tan( avgAngle );
            midLineB = (float) clusterCy - midLineK * clusterCx;

            // divider used in calculation of distance to the middle line
            midLineDistanceDiv = (float) sqrt( 1 + midLineK * midLineK );
            // sign multiplier to make sure left distances are negative for any slope of the middle line
            midLineSignMul     = ( avgAngle > (float) XPIHalf ) ? -1.0f : 1.0f;

            // width of the cluster is the distance between left most and the right most lines
            clusterWidth = (uint32_t) ( fabs( midLineB + midLineK * leftLineCx  - leftLineCy  ) / midLineDistanceDiv +
                                        fabs( midLineB + midLineK * rightLineCx - rightLineCy ) / midLineDistanceDiv );
        }

        // for each line in the cluster, calculate its distance to the middle line
        for ( i = 0; i < n; i++ )
        {
            blobId    = objects[i];
            lineStart = blobsInfo->LineStartPoints[blobId];
            lineEnd   = blobsInfo->LineEndPoints[blobId];

            blobCx    = ( lineStart.x + lineEnd.x ) / 2;
            blobCy    = ( lineStart.y + lineEnd.y ) / 2;

            distanceToCenter = (float) ( ( vertical ) ? (float) blobCx - clusterCx :
                               midLineSignMul * ( midLineB + midLineK * blobCx - blobCy ) / midLineDistanceDiv );

            distancesToCenter[i]       = distanceToCenter;
            distancesToCenterSorted[i] = distanceToCenter;
        }

        // sort lines' distances to center
        qsort( distancesToCenterSorted, n, sizeof( float ), CompareFloats );

        // find mean distance between lines
        for ( i = 1; i < n; i++ )
        {
            distBetweenLines      = distancesToCenterSorted[i] - distancesToCenterSorted[i - 1];
            meanDistBetweenLines += distBetweenLines;
        }

        meanDistBetweenLines /= ( n - 1 );

        // search for gaps which are 3 times more than the mean distance
        {
            int      leftGapStart       = -1,   rightGapStart       = -1;
            float    leftGapCutDistance = 0.0f, rightGapCutDistance = 0.0f;
            float    maxEdgeSizeToCheck = 0.07f * clusterWidth;
            uint32_t k;

            // check the left 25%
            for ( i = 0, k = n / 4; i < k; i++ )
            {
                if ( distancesToCenterSorted[i + 1] - distancesToCenterSorted[i] > meanDistBetweenLines * 3 )
                {
                    // only check certain part at the left edge of the cluster
                    if ( distancesToCenterSorted[i] - distancesToCenterSorted[0] < maxEdgeSizeToCheck )
                    {
                        leftGapStart       = i;
                        leftGapCutDistance = distancesToCenterSorted[i];
                    }
                    break;
                }
            }

            // check the right 25%
            for ( i = n - 1, k = n - n / 4; i > k; i-- )
            {
                if ( distancesToCenterSorted[i] - distancesToCenterSorted[i - 1] > meanDistBetweenLines * 3 )
                {
                    // only check certain part at the right edge of the cluster
                    if ( distancesToCenter[n - 1] - distancesToCenterSorted[i] < maxEdgeSizeToCheck )
                    {
                        rightGapStart       = i;
                        rightGapCutDistance = distancesToCenterSorted[i];
                    }
                    break;
                }
            }

            // in the case gaps are found on the left/right edges, check their average height
            if ( ( leftGapStart != -1 ) || ( rightGapStart != -1 ) )
            {
                uint32_t leftCounter = 0, rightCounter = 0, midCounter = 0;
                float    leftAvgHeight = 0.0f, rightAvgHeight = 0.0f, midAvgHeight = 0.0f;
                float    leftMeanThickness = 0.0f, rightMeanThickness = 0.0f;
                float    height;

                for ( i = 0; i < n; i++ )
                {
                    blobId    = objects[i];
                    lineStart = blobsInfo->LineStartPoints[blobId];
                    lineEnd   = blobsInfo->LineEndPoints[blobId];

                    blobCx    = ( lineStart.x + lineEnd.x ) / 2;
                    blobCy    = ( lineStart.y + lineEnd.y ) / 2;

                    height    = (float) ( lineEnd.y - lineStart.y + 1 );

                    distanceToCenter = distancesToCenter[i];

                    if ( ( leftGapStart != -1 ) && ( distanceToCenter <= leftGapCutDistance ) )
                    {
                        leftAvgHeight += height;
                        leftCounter++;
                        leftMeanThickness += blobsInfo->LineMeanThickness[blobId];
                    }
                    else if ( ( rightGapStart != -1 ) && ( distanceToCenter >= rightGapCutDistance ) )
                    {
                        rightAvgHeight += height;
                        rightCounter++;
                        rightMeanThickness += blobsInfo->LineMeanThickness[blobId];
                    }
                    else
                    {
                        midAvgHeight += height;
                        midCounter++;
                    }
                }

                if ( midCounter != 0 )
                {
                    midAvgHeight /= midCounter;
                }

                if ( leftCounter != 0 )
                {
                    leftAvgHeight /= leftCounter;

                    if ( ( ( leftCounter > 1 ) || ( leftMeanThickness >= 3 ) ) && ( midAvgHeight / leftAvgHeight < 2 ) )
                    {
                        leftGapStart = -1;
                    }
                }

                if ( rightCounter != 0 )
                {
                    rightAvgHeight /= rightCounter;

                    if ( ( ( rightCounter > 1 ) || ( rightMeanThickness >= 3 ) ) && ( midAvgHeight / rightAvgHeight < 2 ) )
                    {
                        rightGapStart = -1;
                    }
                }
            }

            // collect IDs of lines to remove from the cluster
            if ( ( leftGapStart != -1 ) || ( rightGapStart != -1 ) )
            {
                uint32_t removeCounter = 0;

                for ( i = 0; i < n; i++ )
                {
                    blobId    = objects[i];
                    lineStart = blobsInfo->LineStartPoints[blobId];
                    lineEnd   = blobsInfo->LineEndPoints[blobId];

                    blobCx = ( lineStart.x + lineEnd.x ) / 2;
                    blobCy = ( lineStart.y + lineEnd.y ) / 2;

                    distanceToCenter = distancesToCenter[i];

                    if ( ( ( leftGapStart  != -1 ) && ( distanceToCenter <= leftGapCutDistance ) ) ||
                         ( ( rightGapStart != -1 ) && ( distanceToCenter >= rightGapCutDistance ) ) )
                    {
                        pruningBuffer[removeCounter] = blobId;

                        fillMap[blobId + 1] = 0xFC; // TODO: to change
                        removeCounter++;
                    }
                }

                PruneObjectsFromCluster( currentCluster, pruningBuffer, removeCounter );
            }
        }

        currentCluster = currentCluster->Next;
    }

    // don't bother about clusters with too few elements
    FilterClustersBySize( blobsInfo );
}

// Remove clusters where angle between lines changes too rapidly
void FilterClustersWithNoisyAngles( BlobsInfo* blobsInfo )
{
    ObjectsCluster* clustersList   = blobsInfo->LinesClusters;
    ObjectsCluster* currentCluster = clustersList;
    ObjectsCluster* prevCluster    = 0;
    uint8_t*        fillMap        = blobsInfo->FillMap;

    float   angle, prevAngle;
    float   diff, meanAngleDiff, maxAngleDiff;

    while ( currentCluster != 0 )
    {
        uint32_t* objects = currentCluster->Objects;
        uint32_t  i, n = currentCluster->ObjectsCount;

        meanAngleDiff = 0.0f;
        maxAngleDiff  = 0.0f;
        prevAngle     = blobsInfo->LineSlopes[objects[0]];

        for ( i = 1; i < n; i++ )
        {
            angle = blobsInfo->LineSlopes[objects[i]];

            diff = angle - prevAngle;
            if ( diff < 0 ) { diff = -diff; }
            if ( diff > maxAngleDiff ) { maxAngleDiff = diff; }
            meanAngleDiff += diff;

            prevAngle = angle;
        }

        meanAngleDiff /= n;
        meanAngleDiff *= (float) ( 180.0f / XPI );
        maxAngleDiff  *= (float) ( 180.0f / XPI );

        if ( ( meanAngleDiff > LINES_MAX_MEAN_ANGLE_DIFF ) || ( maxAngleDiff > LINES_MAX_ANGLE_DIFF ) )
        {
            // >> TODO: to remove
            for ( i = 0; i < currentCluster->ObjectsCount; i++ )
            {
                fillMap[currentCluster->Objects[i] + 1] = 0xFC;
            }
            // <<

            currentCluster = currentCluster->Next;

            if ( prevCluster == 0 )
            {
                clustersList = currentCluster;
            }
            else
            {
                prevCluster->Next = currentCluster;
            }
        }
        else
        {
            prevCluster    = currentCluster;
            currentCluster = currentCluster->Next;
        }
    }

    blobsInfo->LinesClusters = clustersList;
}

// Find quadrilaterals for the clusters in this group
void FindlClustersQuads( BlobsInfo* blobsInfo )
{
    ObjectsCluster* currentCluster = blobsInfo->LinesClusters;

    while ( currentCluster != 0 )
    {
        uint32_t* objects = currentCluster->Objects;
        uint32_t  i, n    = currentCluster->ObjectsCount;
        uint32_t  blobId;
        xpoint    lineStart;
        xpoint    lineEnd;

        int32_t   clusterCx  = currentCluster->Center.x;
        int32_t   clusterCy  = currentCluster->Center.y;
        int32_t   topCx      = currentCluster->TopCenter.x;
        int32_t   topCy      = currentCluster->TopCenter.y;
        int32_t   bottomCx   = currentCluster->BottomCenter.x;
        int32_t   bottomCy   = currentCluster->BottomCenter.y;
        float     avgAngle   = currentCluster->AverageAngle;
        float     leftAngle  = 0.0f;
        float     rightAngle = 0.0f;

        float     midLineK = 0.0f, midLineB = 0.0f, midLineDistanceDiv = 1.0f, midLineSignMul = 1;
        bool      vertical = false, isLeftVertical = false, isRightVertical = false;

        float     distanceToCenter, leftMostDistance = 0.0f, rightMostDistance = 0.0f;
        uint32_t  blobCx, blobCy, rightCx = clusterCx;

        if ( avgAngle == (float) XPIHalf )
        {
            vertical = true;
        }
        else
        {
            midLineK = (float) tan( avgAngle );
            midLineB = (float) clusterCy - midLineK * clusterCx;

            // divider used in calculation of distance to the middle line
            midLineDistanceDiv = (float) sqrt( 1 + midLineK * midLineK );
            // sign multiplier to make sure left distances are negative for any slope of the middle line
            midLineSignMul = ( avgAngle > (float) XPIHalf ) ? -1.0f : 1.0f;
        }

        // for each line in the cluster, calculate its distance to the middle line
        for ( i = 0; i < n; i++ )
        {
            blobId    = objects[i];
            lineStart = blobsInfo->LineStartPoints[blobId];
            lineEnd   = blobsInfo->LineEndPoints[blobId];

            blobCx    = ( lineStart.x + lineEnd.x ) / 2;
            blobCy    = ( lineStart.y + lineEnd.y ) / 2;

            distanceToCenter = (float) ( ( vertical ) ? (float) blobCx - clusterCx :
                                           midLineSignMul * ( midLineB + midLineK * blobCx - blobCy ) / midLineDistanceDiv );

            if ( distanceToCenter < leftMostDistance )
            {
                leftMostDistance = distanceToCenter;
                leftAngle        = blobsInfo->LineSlopes[blobId];
                isLeftVertical   = blobsInfo->LineVertical[blobId];
            }
            if ( distanceToCenter > rightMostDistance )
            {
                rightMostDistance = distanceToCenter;
                rightAngle        = blobsInfo->LineSlopes[blobId];
                isRightVertical   = blobsInfo->LineVertical[blobId];
                rightCx           = blobCx;
            }
        }

        // make quadrilateral a bit wider
        leftMostDistance  -= 7;
        rightMostDistance += 7;

        if ( vertical )
        {
            currentCluster->Quad[0].x = (int32_t) ( clusterCx + leftMostDistance );
            currentCluster->Quad[0].y = topCy;

            currentCluster->Quad[1].x = (int32_t) ( clusterCx + rightMostDistance );
            currentCluster->Quad[1].y = topCy;

            currentCluster->Quad[2].x = (int32_t) ( clusterCx + rightMostDistance );
            currentCluster->Quad[2].y = bottomCy;

            currentCluster->Quad[3].x = (int32_t) ( clusterCx + leftMostDistance );
            currentCluster->Quad[3].y = bottomCy;
        }
        else
        {
            // need to find unit vector going from center of the cluster to the right
            float lineK  = -1.0f / midLineK;
            float lineB  = (float) clusterCy - lineK * clusterCx;
            float rightX = (float) rightCx;
            float rightY = lineK * rightX + lineB;
            float unitX  = rightX - clusterCx;
            float unitY  = rightY - clusterCy;
            float norm   = (float) sqrt( unitX * unitX + unitY * unitY );

            float leftEdgePointX, rightEdgePointX;
            float leftEdgePointY, rightEdgePointY;

            // coordinates of quadrilateral's corners
            float x0, y0, x1, y1, x2, y2, x3, y3;

            // equation for the line on the top of the cluster
            float topLineK = lineK;
            float topLineB = (float) topCy - topLineK * topCx;

            // equation for the line at the bottom of the cluster
            float bottomLineK = lineK;
            float bottomLineB = (float) bottomCy - bottomLineK * bottomCx;

            // get the unit vector point right side from center
            unitX /= norm;
            unitY /= norm;

            // get left/right points on the mid line
            leftEdgePointX = (float) clusterCx + unitX * leftMostDistance;
            leftEdgePointY = (float) clusterCy + unitY * leftMostDistance;

            rightEdgePointX = (float) clusterCx + unitX * rightMostDistance;
            rightEdgePointY = (float) clusterCy + unitY * rightMostDistance;

            // find corners on the left
            if ( isLeftVertical == false )
            {
                // equation for the line on the left of the cluster
                float leftLineK = (float) tan( leftAngle );
                float leftLineB = leftEdgePointY - leftLineK * leftEdgePointX;

                // intersect left and top lines
                x0 = ( topLineB - leftLineB ) / ( leftLineK - topLineK );
                y0 = x0 * leftLineK + leftLineB;

                // intersect left and bottom lines
                x3 = ( bottomLineB - leftLineB ) / ( leftLineK - bottomLineK );
                y3 = x3 * leftLineK + leftLineB;
            }
            else
            {
                x0 = (float) clusterCx + leftMostDistance;
                y0 = (float) topCy;

                x3 = x0;
                y3 = (float) bottomCy;
            }

            // find corners on the right
            if ( isRightVertical == false )
            {
                // equation for the line on the right of the cluster
                float rightLineK = (float) tan( rightAngle );
                float rightLineB = rightEdgePointY - rightLineK * rightEdgePointX;

                // intersect right and top lines
                x1 = ( topLineB - rightLineB ) / ( rightLineK - topLineK );
                y1 = x1 * rightLineK + rightLineB;

                // intersect right and bottom lines
                x2 = ( bottomLineB - rightLineB ) / ( rightLineK - bottomLineK );
                y2 = x2 * rightLineK + rightLineB;
            }
            else
            {
                x1 = (float) clusterCx + rightMostDistance;
                y1 = (float) topCy;

                x2 = x1;
                y2 = (float) bottomCy;
            }

            currentCluster->Quad[0].x = (int32_t) x0;
            currentCluster->Quad[0].y = (int32_t) y0;

            currentCluster->Quad[1].x = (int32_t) x1;
            currentCluster->Quad[1].y = (int32_t) y1;

            currentCluster->Quad[2].x = (int32_t) x2;
            currentCluster->Quad[2].y = (int32_t) y2;

            currentCluster->Quad[3].x = (int32_t) x3;
            currentCluster->Quad[3].y = (int32_t) y3;
        }

        currentCluster = currentCluster->Next;
    }
}
