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

#include "xtypes.h"

// Get intersection of two ranges - make sure subject range is within specified bounds
void XRangeIntersect( xrange* subject, const xrange* bounds )
{
    if ( subject->min < bounds->min )
    {
        subject->min = bounds->min;
    }
    if ( subject->min > bounds->max )
    {
        subject->min = bounds->max;
    }

    if ( subject->max < bounds->min )
    {
        subject->max = bounds->min;
    }
    if ( subject->max > bounds->max )
    {
        subject->max = bounds->max;
    }
}

void XRangeIntersectF( xrangef* subject, const xrangef* bounds )
{
    if ( subject->min < bounds->min )
    {
        subject->min = bounds->min;
    }
    if ( subject->min > bounds->max )
    {
        subject->min = bounds->max;
    }

    if ( subject->max < bounds->min )
    {
        subject->max = bounds->min;
    }
    if ( subject->max > bounds->max )
    {
        subject->max = bounds->max;
    }
}
