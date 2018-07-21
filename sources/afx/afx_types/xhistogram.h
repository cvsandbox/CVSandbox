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
#ifndef CVS_XHISTOGRAM_H
#define CVS_XHISTOGRAM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "xtypes.h"

// ===== Color palette structure =====
typedef struct _xhistogram
{
    uint32_t* values;
    uint32_t  length;
    uint32_t  min;
    uint32_t  max;
    uint32_t  minEx0;
    uint32_t  minHits;
    uint32_t  maxHits;
    uint32_t  total;
    uint32_t  totalEx0;
    float     mean;
    float     stddev;
    float     meanEx0;
    float     stddevEx0;
}
xhistogram;

// Allocate empty histogram of the specified size
XErrorCode XHistogramCreate( uint32_t length, xhistogram** histogram );
// Clone the histogram
XErrorCode XHistogramClone( const xhistogram* src, xhistogram** dst );
// Copy the histogram
XErrorCode XHistogramCopy( const xhistogram* src, xhistogram* dst );
// Clear the histogram
XErrorCode XHistogramClear( xhistogram* histogram );
// Update histogram by recalculating its statistics values
XErrorCode XHistogramUpdate( xhistogram* histogram );
// Frees the specified histogram
void XHistogramFree( xhistogram** histogram );

#ifdef __cplusplus
}
#endif

#endif // CVS_XHISTOGRAM_H
