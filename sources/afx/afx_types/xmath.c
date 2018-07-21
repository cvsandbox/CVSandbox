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

#include "xmath.h"

#define Det2( a, b, c, d ) ( (a) * (d) - (b) * (c) )

// Calculate determinant of a matrix
XErrorCode MatrixDeterminant( const xmatrix3* src, float* det )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( det == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        *det = src->m11 * src->m22 * src->m33 +
               src->m12 * src->m23 * src->m31 +
               src->m13 * src->m21 * src->m32 -
               src->m11 * src->m23 * src->m32 -
               src->m12 * src->m21 * src->m33 -
               src->m13 * src->m22 * src->m31;
    }

    return ret;
}

// Multiply matrix A by B and put result into C
XErrorCode MatrixMultiply( const xmatrix3* a, const xmatrix3* b, xmatrix3* c )
{
    XErrorCode ret = SuccessCode;

    if ( ( a == 0 ) || ( b == 0 ) || ( c == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        c->m11 = a->m11 * b->m11 + a->m12 * b->m21 + a->m13 * b->m31;
        c->m12 = a->m11 * b->m12 + a->m12 * b->m22 + a->m13 * b->m32;
        c->m13 = a->m11 * b->m13 + a->m12 * b->m23 + a->m13 * b->m33;

        c->m21 = a->m21 * b->m11 + a->m22 * b->m21 + a->m23 * b->m31;
        c->m22 = a->m21 * b->m12 + a->m22 * b->m22 + a->m23 * b->m32;
        c->m23 = a->m21 * b->m13 + a->m22 * b->m23 + a->m23 * b->m33;

        c->m31 = a->m31 * b->m11 + a->m32 * b->m21 + a->m33 * b->m31;
        c->m32 = a->m31 * b->m12 + a->m32 * b->m22 + a->m33 * b->m32;
        c->m33 = a->m31 * b->m13 + a->m32 * b->m23 + a->m33 * b->m33;
    }

    return ret;
}

// Calculate adjugate of the source matrix
XErrorCode MatrixAdjugate( const xmatrix3* src, xmatrix3* dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        dst->m11 =  Det2( src->m22, src->m23, src->m32, src->m33 );
        dst->m12 = -Det2( src->m12, src->m13, src->m32, src->m33 );
        dst->m13 =  Det2( src->m12, src->m13, src->m22, src->m23 );

        dst->m21 = -Det2( src->m21, src->m23, src->m31, src->m33 );
        dst->m22 =  Det2( src->m11, src->m13, src->m31, src->m33 );
        dst->m23 = -Det2( src->m11, src->m13, src->m21, src->m23 );

        dst->m31 =  Det2( src->m21, src->m22, src->m31, src->m32 );
        dst->m32 = -Det2( src->m11, src->m12, src->m31, src->m32 );
        dst->m33 =  Det2( src->m11, src->m12, src->m21, src->m22 );
    }

    return ret;
}

// Calculate inverse of the source matrix
XErrorCode MatrixInverse( const xmatrix3* src, xmatrix3* dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        float det;

        MatrixDeterminant( src, &det );

        if ( det == 0 )
        {
            ret = ErrorFailed;
        }
        else
        {
            float inv = 1.0f / det;

            MatrixAdjugate( src, dst );

            dst->m11 *= inv;
            dst->m12 *= inv;
            dst->m13 *= inv;

            dst->m21 *= inv;
            dst->m22 *= inv;
            dst->m23 *= inv;

            dst->m31 *= inv;
            dst->m32 *= inv;
            dst->m33 *= inv;
        }
    }

    return ret;
}
