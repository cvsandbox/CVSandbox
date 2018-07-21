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

#include <string.h>
#include <math.h>
#include "xhistogram.h"

// Allocate empty histogram of the specified size
XErrorCode XHistogramCreate( uint32_t length, xhistogram** histogram )
{
    XErrorCode ret = SuccessCode;

    if ( histogram == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( length == 0 )
    {
        ret = ErrorInvalidArgument;
    }
    else
    {
        *histogram = (xhistogram*) XCAlloc( 1, sizeof( xhistogram ) );

        if ( *histogram == 0 )
        {
            ret = ErrorOutOfMemory;
        }
        else
        {
            (*histogram)->values = (uint32_t*) XCAlloc( length, sizeof( uint32_t ) );

            if ( (*histogram)->values == 0 )
            {
                ret = ErrorOutOfMemory;
                XFree( (void**) histogram );
            }
            else
            {
                (*histogram)->length = length;
            }
        }
    }

    return ret;
}

// Clone the histogram
XErrorCode XHistogramClone( const xhistogram* src, xhistogram** dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        ret = XHistogramCreate( src->length, dst );

        if ( ret == SuccessCode )
        {
            (*dst)->length = src->length;

            ret = XHistogramCopy( src, *dst );

            if ( ret != SuccessCode )
            {
                XHistogramFree( dst );
            }
        }
    }

    return ret;
}

// Copy the histogram
XErrorCode XHistogramCopy( const xhistogram* src, xhistogram* dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( src->length != dst->length )
    {
        ret = ErrorInvalidArgument;
    }
    else
    {
        memcpy( dst->values, src->values, sizeof( uint32_t ) * src->length );
        dst->min       = src->min;
        dst->minEx0    = src->minEx0;
        dst->max       = src->max;
        dst->total     = src->total;
        dst->totalEx0  = src->totalEx0;
        dst->mean      = src->mean;
        dst->stddev    = src->stddev;
        dst->meanEx0   = src->meanEx0;
        dst->stddevEx0 = src->stddevEx0;
        dst->minHits   = src->minHits;
        dst->maxHits   = src->maxHits;
    }

    return ret;
}

// Clear the histogram
XErrorCode XHistogramClear( xhistogram* histogram )
{
    XErrorCode ret = SuccessCode;

    if ( histogram == 0 )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        memset( histogram->values, 0, sizeof( uint32_t ) * histogram->length );
        histogram->min       = 0;
        histogram->minEx0    = 0;
        histogram->max       = 0;
        histogram->total     = 0;
        histogram->totalEx0  = 0;
        histogram->mean      = 0;
        histogram->stddev    = 0;
        histogram->meanEx0   = 0;
        histogram->stddevEx0 = 0;
        histogram->minHits   = 0;
        histogram->maxHits   = 0;
    }

    return ret;
}

// Update histogram by recalculating its statistics values
XErrorCode XHistogramUpdate( xhistogram* histogram )
{
    XErrorCode ret = SuccessCode;

    if ( histogram == 0 )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        uint32_t  i;
        uint32_t  value;
        double    diff;
        double    mean = 0;

        histogram->min       = histogram->length - 1;
        histogram->minEx0    = histogram->min;
        histogram->max       = 0;
        histogram->total     = 0;
        histogram->totalEx0  = 0;
        histogram->mean      = 0;
        histogram->stddev    = 0;
        histogram->meanEx0   = 0;
        histogram->stddevEx0 = 0;
        histogram->minHits   = UINT32_MAX;
        histogram->maxHits   = 0;

        for ( i = 0; i < histogram->length; i++ )
        {
            value = histogram->values[i];

            // find min/max hits value
            if ( value < histogram->minHits )
            {
                histogram->minHits = value;
            }
            if ( value > histogram->maxHits )
            {
                histogram->maxHits = value;
            }

            if ( value != 0 )
            {
                // find min/max value of the random variable represented in the histogram
                if ( i < histogram->min )
                {
                    histogram->min = i;
                }
                if ( i > histogram->max )
                {
                    histogram->max = i;
                }

                histogram->total += value;
                mean  += (double) value * i;

                if ( ( i != 0 ) && ( i < histogram->minEx0 ) )
                {
                    histogram->minEx0 = i;
                }
            }
        }

        if ( histogram->total != 0 )
        {
            double stddev    = 0;
            double stddevEx0 = 0;
            double meanEx0   = mean;

            mean /= histogram->total;

            histogram->totalEx0 = histogram->total - histogram->values[0];

            if ( histogram->totalEx0 != 0 )
            {
                meanEx0 /= histogram->totalEx0;
            }

            // for 0
            stddev += mean * mean * histogram->values[0];

            // for the rest
            for ( i = 1; i < histogram->length; i++ )
            {
                value   = histogram->values[i];
                diff    = (double) i - mean;
                stddev += diff * diff * value;

                diff       = (double) i - meanEx0;
                stddevEx0 += diff * diff * value;
            }

            histogram->mean   = (float) mean;
            histogram->stddev = (float) sqrt( stddev / histogram->total );

            if ( histogram->totalEx0 != 0 )
            {
                histogram->meanEx0   = (float) meanEx0;
                histogram->stddevEx0 = (float) sqrt( stddevEx0 / histogram->totalEx0 );
            }
        }
    }

    return ret;
}

// Frees the specified histogram
void XHistogramFree( xhistogram** histogram )
{
    if ( ( histogram != 0 ) && ( *histogram != 0 ) )
    {
        XFree( (void**) &(*histogram)->values );
        XFree( (void**) histogram );
    }
}
