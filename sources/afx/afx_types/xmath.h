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
#ifndef CVS_XMATH_H
#define CVS_XMATH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xerrors.h"

// 3x3 matrix with float value
typedef struct _xmatrix3
{
    // row 1
    float m11; float m12; float m13;
    // row 3
    float m21; float m22; float m23;
    // row 3
    float m31; float m32; float m33;
}
xmatrix3;

// Calculate determinant of a matrix
XErrorCode MatrixDeterminant( const xmatrix3* src, float* det );
// Multiply matrix A by B and put result into C
XErrorCode MatrixMultiply( const xmatrix3* a, const xmatrix3* b, xmatrix3* c );
// Calculate adjugate of the source matrix
XErrorCode MatrixAdjugate( const xmatrix3* src, xmatrix3* dst );
// Calculate inverse of the source matrix
XErrorCode MatrixInverse( const xmatrix3* src, xmatrix3* dst );

#ifdef __cplusplus
}
#endif

#endif // CVS_XMATH_H
