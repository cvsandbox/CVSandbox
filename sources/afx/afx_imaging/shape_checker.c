/*
    Imaging library of Computer Vision Sandbox

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

#include "ximaging.h"
#include <math.h>
#include <float.h>
#include <memory.h>

// Find the furthest point from the specified reference point
static XErrorCode GetFurthestPoint( const xpoint* points, uint32_t pointsCount, xpoint refPoint, xpoint* pFurthestPoint )
{
    XErrorCode ret = SuccessCode;

    if ( ( points == 0 ) || ( pFurthestPoint == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( pointsCount == 0 )
    {
        ret = ErrorInvalidArgument;
    }
    else
    {
        xpoint   furthestPoint = refPoint;
        int32_t  maxDistance = -1;
        int32_t  dx, dy, distance;
        int32_t  rx = refPoint.x;
        int32_t  ry = refPoint.y;
        uint32_t i;

        for ( i = 0; i < pointsCount; i++ )
        {
            dx       = rx - points[i].x;
            dy       = ry - points[i].y;
            distance = dx * dx + dy * dy;

            if ( distance > maxDistance )
            {
                maxDistance   = distance;
                furthestPoint = points[i];
            }
        }

        *pFurthestPoint = furthestPoint;
    }

    return ret;
}

// Find two furthest points from the line specified by two points - one point on one side of the line and the second point on the other side
static XErrorCode GetFurthestPointsFromLine( const xpoint* points, uint32_t pointsCount, xpoint lineStart, xpoint lineEnd,
                                             xpoint* pFurthestPoint1, float* pDistace1, xpoint* pFurthestPoint2, float* pDistace2 )
{
    XErrorCode ret = SuccessCode;

    if ( ( points == 0 ) || ( pFurthestPoint1 == 0 ) || ( pFurthestPoint2 == 0 ) || ( pDistace1 == 0 ) || ( pDistace2 == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( pointsCount == 0 )
    {
        ret = ErrorInvalidArgument;
    }
    else
    {
        float distance1 = 0.0f, distance2 = 0.0f;

        *pFurthestPoint1 = points[0];
        *pFurthestPoint2 = points[0];

        if ( lineStart.x != lineEnd.x )
        {
            // line's equation y(x) = k * x + b
            float    lineK    = (float) ( lineEnd.y - lineStart.y ) / ( lineEnd.x - lineStart.x );
            float    lineB    = lineStart.y - lineK * lineStart.x;
            float    div      = (float) sqrt( lineK * lineK + 1 );
            float    distance = 0;
            uint32_t i;

            for ( i = 0; i < pointsCount; i++ )
            {
                distance = ( lineK * points[i].x + lineB - points[i].y ) / div;

                if ( distance > distance1 )
                {
                    distance1 = distance;
                    *pFurthestPoint1 = points[i];
                }
                if ( distance < distance2 )
                {
                    distance2 = distance;
                    *pFurthestPoint2 = points[i];
                }
            }
        }
        else
        {
            int      lineX    = lineStart.x;
            float    distance = 0;
            uint32_t i;

            for ( i = 0; i < pointsCount; i++ )
            {
                distance = (float) ( lineX - points[i].x );

                if ( distance > distance1 )
                {
                    distance1 = distance;
                    *pFurthestPoint1 = points[i];
                }
                if ( distance < distance2 )
                {
                    distance2 = distance;
                    *pFurthestPoint2 = points[i];
                }
            }
        }

        *pDistace1 =  distance1;
        *pDistace2 = -distance2;
    }

    return ret;
}

// Get angle between the specified vectors
static float GetAngleBetweenVectors( xpoint startPoint, xpoint vector1end, xpoint vector2end )
{
    float dx1 = (float) ( vector1end.x - startPoint.x );
    float dy1 = (float) ( vector1end.y - startPoint.y );

    float dx2 = (float) ( vector2end.x - startPoint.x );
    float dy2 = (float) ( vector2end.y - startPoint.y );

    return (float) ( acos( ( dx1 * dx2 + dy1 * dy2 ) / ( sqrt( dx1 * dx1 + dy1 * dy1 ) * sqrt( dx2 * dx2 + dy2 * dy2 ) ) ) * 180.0f / XPI );
}

// Find quadrilateral points of the specified point cloud
XErrorCode FindQuadrilateralCorners( const xpoint* points, uint32_t pointsCount, xrect boundingRect, float relDistortionLimit,
                                     xpoint* quadPoints, bool* gotFourPoints )
{
    XErrorCode ret = SuccessCode;

    if ( ( points == 0 ) || ( quadPoints == 0 ) || ( gotFourPoints == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( pointsCount < 3 )
    {
        ret = ErrorInvalidArgument;
    }
    else
    {
        xpoint center;
        xpoint point3, point4;
        float  distance3, distance4;
        float  distortionLimit = relDistortionLimit * ( boundingRect.x2 - boundingRect.x1 + boundingRect.y2 - boundingRect.y1 + 2 ) / 2;
        int    cornersFound    = 2;

        center.x = ( boundingRect.x1 + boundingRect.x2 ) / 2;
        center.y = ( boundingRect.y1 + boundingRect.y2 ) / 2;

        // find first two corners
        GetFurthestPoint( points, pointsCount, center, &( quadPoints[0] ) );
        GetFurthestPoint( points, pointsCount, quadPoints[0], &( quadPoints[1] ) );

        // get two furthest points from the line formed by the two just found points
        GetFurthestPointsFromLine( points, pointsCount, quadPoints[0], quadPoints[1],
                                   &point3, &distance3, &point4, &distance4 );

        // Ideally points 1 and 2 form them main diagonal of the
        // quadrilateral and points 3 and 4 form another diagonal.

        // But if one of the points (3 or 4) is very close to the line
        // connecting points 1 and 2, then it is on the same line.
        // Which means corner was not found.
        // In this case we deal with a trapezoid or triangle, where
        // (1-2) line is one of its sides.

        // Another interesting case is when both points (3) and (4) are
        // very close to the (1-2) line. In this case we may have just a flat
        // quadrilateral.

        if ( ( ( distance3 >= distortionLimit ) && ( distance4 >= distortionLimit ) ) ||
             ( ( distance3 < distortionLimit ) && ( distance3 != 0 ) &&
               ( distance4 < distortionLimit ) && ( distance4 != 0 ) ) )
        {
            // don't add one of the corners, if the point is already in the corners list
            // (this may happen when both #3 and #4 points are very close to the line
            // connecting #1 and #2)
            if ( ( ( point3.x != quadPoints[0].x ) || ( point3.y != quadPoints[0].y ) ) &&
                 ( ( point3.x != quadPoints[1].x ) || ( point3.y != quadPoints[1].y ) ) )
            {
                quadPoints[cornersFound] = point3;
                cornersFound++;
            }

            if ( ( ( point4.x != quadPoints[0].x ) || ( point4.y != quadPoints[0].y ) ) &&
                 ( ( point4.x != quadPoints[1].x ) || ( point4.y != quadPoints[1].y ) ) &&
                 ( ( point4.x != point3.x ) || ( point4.y != point3.y ) ) )
            {
                quadPoints[cornersFound] = point4;
                cornersFound++;
            }
        }
        else
        {
            // it seems that we deal with kind of trapezoid,
            // where point 1 and 2 are on the same edge
            xpoint tempPoint = ( distance3 > distance4 ) ? point3 : point4;

            // try finding 3rd point - furthest from line between 1st corner and the temp point
            GetFurthestPointsFromLine( points, pointsCount, quadPoints[0], tempPoint,
                                       &point3, &distance3, &point4, &distance4 );

            bool thirdPointIsFound = false;

            if ( ( distance3 >= distortionLimit ) && ( distance4 >= distortionLimit ) )
            {
                int dx42 = point4.x - quadPoints[1].x;
                int dy42 = point4.y - quadPoints[1].y;
                int dx32 = point3.x - quadPoints[1].x;
                int dy32 = point3.y - quadPoints[1].y;

                // take the point furthest away from the 2nd corner
                if ( dx42 * dx42 + dy42 * dy42 > dx32 * dx32 + dy32 * dy32 )
                {
                    point3 = point4;
                }

                thirdPointIsFound = true;
            }
            else
            {
                // try finding 3rd point again - furthest from line between 2nd corner and the temp point
                GetFurthestPointsFromLine( points, pointsCount, quadPoints[1], tempPoint,
                                           &point3, &distance3, &point4, &distance4 );

                if ( ( distance3 >= distortionLimit ) && ( distance4 >= distortionLimit ) )
                {
                    int dx41 = point4.x - quadPoints[0].x;
                    int dy41 = point4.y - quadPoints[0].y;
                    int dx31 = point3.x - quadPoints[0].x;
                    int dy31 = point3.y - quadPoints[0].y;

                    // take the point furthest away from the 1st corner
                    if ( dx41 * dx41 + dy41 * dy41 > dx31 * dx31 + dy31 * dy31 )
                    {
                        point3 = point4;
                    }

                    thirdPointIsFound = true;
                }
            }

            if ( thirdPointIsFound == false)
            {
                // failed to find 3rd edge point, which is away enough from the temp point.
                // this means that the clound looks more like triangle
                quadPoints[cornersFound] = tempPoint;
                cornersFound++;
            }
            else
            {
                float tempDistance;

                quadPoints[cornersFound] = point3;
                cornersFound++;

                // try finding 4th point - furthest from line between 1st corner and 3rd
                GetFurthestPointsFromLine( points, pointsCount, quadPoints[0], point3,
                                           &tempPoint, &tempDistance, &point4, &distance4 );

                if ( ( distance4 >= distortionLimit ) && ( tempDistance >= distortionLimit ) )
                {
                    int dxT2 = tempPoint.x - quadPoints[1].x;
                    int dyT2 = tempPoint.y - quadPoints[1].y;
                    int dx42 = point4.x    - quadPoints[1].x;
                    int dy42 = point4.y    - quadPoints[1].y;

                    if ( dxT2 * dxT2 + dyT2 * dyT2 > dx42 * dx42 + dy42 * dy42 )
                    {
                        point4 = tempPoint;
                    }
                }
                else
                {
                    // try finding 4th point again - furthest from line between 2nd corner and 3rd
                    GetFurthestPointsFromLine( points, pointsCount, quadPoints[1], point3,
                                               &tempPoint, &tempDistance, &point4, &distance4 );

                    {
                        int dxT1 = tempPoint.x - quadPoints[0].x;
                        int dyT1 = tempPoint.y - quadPoints[0].y;
                        int dx41 = point4.x    - quadPoints[0].x;
                        int dy41 = point4.y    - quadPoints[0].y;

                        if ( ( dxT1 * dxT1 + dyT1 * dyT1 > dx41 * dx41 + dy41 * dy41 ) &&
                             ( ( tempPoint.x != quadPoints[1].x ) || ( tempPoint.y != quadPoints[1].y ) ) &&
                             ( ( tempPoint.x != point3.x ) || ( tempPoint.y != point3.y ) ) )
                        {
                            point4 = tempPoint;
                        }
                    }
                }

                if ( ( ( point4.x != quadPoints[0].x ) || ( point4.y != quadPoints[0].y ) ) &&
                     ( ( point4.x != quadPoints[1].x ) || ( point4.y != quadPoints[1].y ) ) &&
                     ( ( point4.x != point3.x )        || ( point4.y != point3.y        ) ) )
                {
                    quadPoints[cornersFound] = point3;
                    cornersFound++;
                }
            }
        }

        if ( cornersFound == 2 )
        {
            ret = ErrorInvalidArgument;
        }
        else
        {
            int   i;
            float k1, k2;

            // put the point with lowest X as the first
            for ( i = 1; i < cornersFound; i++ )
            {
                if ( ( quadPoints[i].x < quadPoints[0].x ) ||
                     ( ( quadPoints[i].x == quadPoints[0].x ) && ( quadPoints[i].y < quadPoints[0].y ) ) )
                {
                    xpoint temp   = quadPoints[i];
                    quadPoints[i] = quadPoints[0];
                    quadPoints[0] = temp;
                }
            }

            // sort other points in counter clockwise order
            k1 = ( quadPoints[1].x != quadPoints[0].x ) ?
                        ( (float) ( quadPoints[1].y - quadPoints[0].y ) / ( quadPoints[1].x - quadPoints[0].x ) ) :
                        ( ( quadPoints[1].y > quadPoints[0].y ) ? FLT_MAX : -FLT_MAX );

            k2 = ( quadPoints[2].x != quadPoints[0].x ) ?
                        ( (float) ( quadPoints[2].y - quadPoints[0].y ) / ( quadPoints[2].x - quadPoints[0].x ) ) :
                        ( ( quadPoints[2].y > quadPoints[0].y ) ? FLT_MAX : -FLT_MAX );

            if ( k2 < k1 )
            {
                xpoint temp = quadPoints[1];
                float  tk   = k1;

                quadPoints[1] = quadPoints[2];
                quadPoints[2] = temp;

                k1 = k2;
                k2 = tk;
            }

            if ( cornersFound == 4 )
            {
                float k3 = ( quadPoints[3].x != quadPoints[0].x ) ?
                                ( (float) ( quadPoints[3].y - quadPoints[0].y ) / ( quadPoints[3].x - quadPoints[0].x ) ) :
                                ( ( quadPoints[3].y > quadPoints[0].y ) ? FLT_MAX : -FLT_MAX );

                if ( k3 < k1 )
                {
                    xpoint temp = quadPoints[1];
                    float  tk   = k1;

                    quadPoints[1] = quadPoints[3];
                    quadPoints[3] = temp;

                    k1 = k3;
                    k3 = tk;
                }

                if ( k3 < k2 )
                {
                    xpoint temp = quadPoints[2];
                    float  tk   = k2;

                    quadPoints[2] = quadPoints[3];
                    quadPoints[3] = temp;

                    k2 = k3;
                    k3 = tk;
                }
            }

            *gotFourPoints = ( cornersFound == 4 ) ? true : false;
        }
    }

    return ret;
}

// Check if points fit the convex shape specified by its corners
XErrorCode CheckPointsFitShape( const xpoint* points, uint32_t pointsCount, xrect pointsRect, const xpoint* shapeCorners, uint32_t cornersCount,
                                float relDistortionLimit, float minAcceptableDistortion, float* workBuffer1, bool* workBuffer2 )
{
    XErrorCode ret = SuccessCode;

    if ( ( points == 0 ) || ( shapeCorners == 0 ) || ( workBuffer1 == 0 ) || ( workBuffer2 == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( pointsCount == 0 ) || ( cornersCount < 3 ) )
    {
        ret = ErrorInvalidArgument;
    }
    else
    {
        // lines coefficients (for representation as y(x)=k*x+b)
        float*   lineK  = workBuffer1;
        float*   lineB  = workBuffer1 + cornersCount;
        float*   div    = workBuffer1 + cornersCount * 2; // precalculated divisor
        bool*   isVert = workBuffer2;
        uint32_t i, j, next;
        float    meanDistance    = 0;
        float    distortionLimit = relDistortionLimit * ( pointsRect.x2 - pointsRect.x1 + pointsRect.y2 - pointsRect.y1 + 2 ) / 2;
        float    maxDitance      = XMAX( minAcceptableDistortion, distortionLimit );

        for ( i = 0, next = 1; i < cornersCount; i++, next++ )
        {
            xpoint currentPoint = shapeCorners[i];
            xpoint nextPoint    = shapeCorners[( next == cornersCount) ? 0 : next];

            isVert[i] = ( nextPoint.x == currentPoint.x );

            if ( !isVert[i] )
            {
                lineK[i] = (float) ( nextPoint.y - currentPoint.y ) / ( nextPoint.x - currentPoint.x );
                lineB[i] = currentPoint.y - lineK[i] * currentPoint.x;
                div[i]   = (float) sqrt( lineK[i] * lineK[i] + 1 );
            }
        }

        for ( i = 0; i < pointsCount; i++ )
        {
            float minDistance = FLT_MAX;

            for ( j = 0; j < cornersCount; j++ )
            {
                float distance = 0;

                if ( !isVert[j] )
                {
                    distance = ( lineK[j] * points[i].x + lineB[j] - points[i].y ) / div[j];
                }
                else
                {
                    distance = (float) ( points[i].x - shapeCorners[j].x );
                }

                if ( distance < 0 )
                {
                    distance = -distance;
                }
                if ( distance < minDistance )
                {
                    minDistance = distance;
                }
            }

            meanDistance += minDistance;
        }
        meanDistance /= pointsCount;

        if ( meanDistance > maxDitance )
        {
            ret = ErrorFailed;
        }
    }

    return ret;
}

// Extended version: Check if points fit the convex shape specified by its corners
XErrorCode CheckPointsFitShapeEx( const xpoint* points, uint32_t pointsCount, const xpoint* shapeCorners, uint32_t cornersCount,
                                  float relDistortionLimit, float minAcceptableDistortion, float* workBuffer1, bool* workBuffer2, uint32_t* workBuffer3 )
{
    XErrorCode ret = SuccessCode;

    if ( ( points == 0 ) || ( shapeCorners == 0 ) || ( workBuffer1 == 0 ) || ( workBuffer2 == 0 ) || ( workBuffer3 == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( pointsCount == 0 ) || ( cornersCount < 3 ) )
    {
        ret = ErrorInvalidArgument;
    }
    else
    {
        // lines coefficients (for representation as y(x)=k*x+b)
        float*    lineK           = workBuffer1;
        float*    lineB           = workBuffer1 + cornersCount;
        float*    div             = workBuffer1 + cornersCount * 2; // precalculated divisor
        float*    meanDistances   = workBuffer1 + cornersCount * 3;
        bool*    isVert          = workBuffer2;
        uint32_t* maxSidePoints   = workBuffer3;
        uint32_t* sidePoints      = workBuffer3 + cornersCount;

        uint32_t  i, j, next;

        for ( i = 0, next = 1; i < cornersCount; i++, next++ )
        {
            xpoint currentPoint = shapeCorners[i];
            xpoint nextPoint    = shapeCorners[( next == cornersCount) ? 0 : next];
            int    dx, dy;

            isVert[i] = ( nextPoint.x == currentPoint.x );

            if ( !isVert[i] )
            {
                lineK[i] = (float) ( nextPoint.y - currentPoint.y ) / ( nextPoint.x - currentPoint.x );
                lineB[i] = currentPoint.y - lineK[i] * currentPoint.x;
                div[i]   = (float) sqrt( lineK[i] * lineK[i] + 1 );
            }

            dx = nextPoint.x - currentPoint.x;
            dy = nextPoint.y - currentPoint.y;

            if ( dx < 0 ) { dx = -dx; }
            if ( dy < 0 ) { dy = -dy; }

            dx++;
            dy++;

            maxSidePoints[i] = XMAX( dx, dy );
            meanDistances[i] = 0.0f;
            sidePoints[i]    = 0;
        }

        for ( i = 0; i < pointsCount; i++ )
        {
            float minDistance = FLT_MAX;
            int   winnerLine  = 0;

            for ( j = 0; j < cornersCount; j++ )
            {
                float distance = 0;

                if ( !isVert[j] )
                {
                    distance = ( lineK[j] * points[i].x + lineB[j] - points[i].y ) / div[j];
                }
                else
                {
                    distance = (float) ( points[i].x - shapeCorners[j].x );
                }

                if ( distance < 0 )
                {
                    distance = -distance;
                }
                if ( distance < minDistance )
                {
                    minDistance = distance;
                    winnerLine  = j;
                }
            }

            meanDistances[winnerLine] += minDistance;
            sidePoints[winnerLine]++;
        }

        for ( i = 0; i < cornersCount; i++ )
        {
            if ( ( sidePoints[i] == 0 ) || ( sidePoints[i] < ( maxSidePoints[i] >> 1 ) ) )
            {
                ret = ErrorFailed;
                break;
            }
            else
            {
                float meanDistance    = meanDistances[i] / sidePoints[i];
                float distortionLimit = relDistortionLimit * maxSidePoints[i];
                float maxDitance      = XMAX( minAcceptableDistortion, distortionLimit );

                if ( meanDistance > maxDitance )
                {
                    ret = ErrorFailed;
                    break;
                }
            }
        }
    }

    return ret;
}

// Check if points fit a circle
XErrorCode CheckPointsFitCircle( const xpoint* points, uint32_t pointsCount, xrect pointsRect,
                                 float relDistortionLimit, float minAcceptableDistortion,
                                 xpointf* pCenter, float* pRadius, float* pMeanDeviation )
{
    XErrorCode ret = SuccessCode;

    if ( ( points == 0 ) || ( pCenter == 0 ) || ( pRadius == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( pointsCount == 0 )
    {
        ret = ErrorInvalidArgument;
    }
    else if ( ( pointsCount < 8 ) || ( pointsRect.x2 - pointsRect.x1 <= 1 ) || ( pointsRect.y2 - pointsRect.y1 <= 1 ) )
    {
        // lets suppose anything with less than 8 points is not really a circle,
        // same as if too small rectangle
        ret = ErrorFailed;
    }
    else
    {
        int32_t  widthM1  = pointsRect.x2 - pointsRect.x1;
        int32_t  heightM1 = pointsRect.y2 - pointsRect.y1;
        float    xRadius  = (float) widthM1  / 2.0f;
        float    yRadius  = (float) heightM1 / 2.0f;
        float    radius   = ( xRadius + yRadius ) / 2.0f;
        float    cx       = xRadius + pointsRect.x1;
        float    cy       = yRadius + pointsRect.y1;
        float    relDev   = relDistortionLimit * (float) ( widthM1 + heightM1 + 1 + 1 ) / 2.0f;
        float    maxDev   = XMAX( minAcceptableDistortion,  relDev );
        float    meanDev  = 0.0f, dx, dy, dev;
        uint32_t i;

        for ( i = 0; i < pointsCount; i++ )
        {
            dx = cx - points[i].x;
            dy = cy - points[i].y;

            dev = (float) sqrt( dx * dx + dy * dy ) - radius;
            if ( dev < 0 )
            {
                dev = -dev;
            }

            meanDev += dev;
        }

        meanDev /= pointsCount;

        if ( meanDev > maxDev )
        {
            ret = ErrorFailed;
        }
        else
        {
            pCenter->x = cx;
            pCenter->y = cy;
            *pRadius   = radius;

            if ( pMeanDeviation != 0 )
            {
                *pMeanDeviation = meanDev;
            }
        }
    }

    return ret;
}

// Relative distortion limit used with FindQuadrilateralCorners() to find corners of quadrilaterals
#define QUADRILATERAL_REL_DISTORTION_LIMIT (0.1f)

// Maximum allowed angle between sides of quadrilaterals - used with OptimizeFlatAngles()
#define QUADRILATERAL_MAX_ANGLE_TO_KEEP (160)

// Check if the specified set of points form a quadrilateral
XErrorCode CheckPointsFitQuadrilateral( const xpoint* points, uint32_t pointsCount, xrect pointsRect,
                                        float relDistortionLimit, float minAcceptableDistortion, xpoint* quadPoints )
{
    XErrorCode ret = SuccessCode;

    if ( ( points == 0 ) || ( quadPoints == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( pointsCount == 0 )
    {
        ret = ErrorInvalidArgument;
    }
    else if ( ( pointsCount < 8 ) || ( pointsRect.x2 - pointsRect.x1 <= 1 ) || ( pointsRect.y2 - pointsRect.y1 <= 1 ) )
    {
        // lets suppose anything with less than 8 points is not really a quadrilateral,
        // same as if too small rectangle
        ret = ErrorFailed;
    }
    else
    {
        xpoint   tempQuadPoints[4];
        float    workBuffer1[16];
        bool    workBuffer2[4];
        uint32_t workBuffer3[8];
        uint32_t finalPointsCount;
        bool    gotFourPoints;

        ret = ErrorFailed;

        if ( ( FindQuadrilateralCorners( points, pointsCount, pointsRect, QUADRILATERAL_REL_DISTORTION_LIMIT, tempQuadPoints, &gotFourPoints ) == SuccessCode ) &&
             ( gotFourPoints == true ) )
        {
            if ( ( OptimizeFlatAngles( tempQuadPoints, 4, quadPoints, &finalPointsCount, QUADRILATERAL_MAX_ANGLE_TO_KEEP ) == SuccessCode ) &&
                 ( finalPointsCount == 4 ) )
            {
                ret = CheckPointsFitShapeEx( points, pointsCount, quadPoints, 4, relDistortionLimit, minAcceptableDistortion,
                                             workBuffer1, workBuffer2, workBuffer3 );
            }
        }
    }

    return ret;
}

// Optimizes convex hull by removing obtuse angles (close to flat) from a shape
XErrorCode OptimizeFlatAngles( const xpoint* srcPoints, uint32_t srcPointsCount, xpoint* dstPoints, uint32_t* dstPointsCount, float maxAngleToKeep )
{
    XErrorCode ret = SuccessCode;

    if ( ( srcPoints == 0 ) || ( dstPoints == 0 ) || ( dstPointsCount == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        if ( srcPointsCount <= 3 )
        {
            memcpy( dstPoints, srcPoints, srcPointsCount * sizeof( xpoint ) );
            *dstPointsCount = srcPointsCount;
        }
        else
        {
            float    angle;
            uint32_t optimizedPointsCount = 2;
            uint32_t i;

            dstPoints[0] = srcPoints[0];
            dstPoints[1] = srcPoints[1];

            for ( i = 2; i < srcPointsCount; i++ )
            {
                // add new point
                dstPoints[optimizedPointsCount] = srcPoints[i];
                optimizedPointsCount++;

                // get angle between 2 vectors, which start from the next to last point
                angle = GetAngleBetweenVectors( dstPoints[optimizedPointsCount - 2],
                    dstPoints[optimizedPointsCount - 3], dstPoints[optimizedPointsCount - 1] );

                if ( ( angle > maxAngleToKeep ) &&
                     ( ( optimizedPointsCount > 3 ) || ( i < srcPointsCount - 1 ) ) )
                {
                    // remove the next to last point - override the last one and decrease counter
                    dstPoints[optimizedPointsCount - 2] = srcPoints[i];
                    optimizedPointsCount--;
                }
            }

            if ( optimizedPointsCount > 3 )
            {
                // check the last point
                angle = GetAngleBetweenVectors( dstPoints[optimizedPointsCount - 1], dstPoints[optimizedPointsCount - 2], dstPoints[0] );

                if ( angle > maxAngleToKeep )
                {
                    // remove last point by decreasing counter
                    optimizedPointsCount--;
                }

                if ( optimizedPointsCount > 3 )
                {
                    // check the first point
                    angle = GetAngleBetweenVectors( dstPoints[0], dstPoints[optimizedPointsCount - 1], dstPoints[1] );

                    if ( angle > maxAngleToKeep )
                    {
                        optimizedPointsCount--;
                        memcpy( dstPoints, &( dstPoints[1] ), optimizedPointsCount * sizeof( xpoint ) );
                    }
                }
            }

            *dstPointsCount = optimizedPointsCount;
        }
    }

    return ret;
}
