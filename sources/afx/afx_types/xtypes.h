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
#ifndef CVS_XTYPES_H
#define CVS_XTYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "xerrors.h"

// Handy macro to bypass MSVC level 4 warning regarding unreferenced parameter
#define XUNREFERENCED_PARAMETER(param) (void)param;

// PI values
#define XPI     (3.14159265)
#define XPI2    (6.28318530)
#define XPIHalf (1.570796325)

// Min/Max macros. NOTE: never use with increments and anything else changing value
#define XMIN(a, b) (((a)<(b))?(a):(b))
#define XMAX(a, b) (((a)>(b))?(a):(b))
#define XMIN3(a, b, c) (((a)<(b))?(((a)<(c))?(a):(c)):(((b)<(c))?(b):(c)))
#define XMAX3(a, b, c) ((((a)>=(b))&&((a)>=(c)))?(a):(((b)>=(c))?(b):(c)))
// Macro to make sure a value is in certain range
#define XINRANGE(a, min, max) (((a)<(max))?(((a)>(min))?(a):(min)):(max))

// Macro to get array's size in bytes
#define XARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

// ===== Definition of version structure =====
typedef struct _version
{
    uint8_t  major;
    uint8_t  minor;
    uint16_t revision;
}
xversion;

// Check if two version are equal or not
bool XVersionAreEqual( const xversion* version1, const xversion* version2 );
// Copy source version into destination
void XVersionCopy( const xversion* src, xversion* dst );
// Get string representation of the specified version. Returns number of characters (including 0 terminator) put into the buffer.
uint32_t XVersionToString( const xversion* version, char* buffer, uint32_t bufferLen );
// Convert given string into version
XErrorCode XStringToVersion( const char* buffer, xversion* version );

// ===== Define common memory management functions =====

// Allocate memory block of required size - malloc() replacement
void* XMAlloc( size_t size );
// Allocate memory block of required size and zero initialize it - calloc() replacement
void* XCAlloc( size_t count, size_t size );
// Free specified block - free() replacement
void XFree( void** memblock );

// ===== Definition of string type =====

// Basically it is just a char pointer. The type is mostly required
// to define string allocation/de-allocation rules between caller
// and callee.
typedef const char* xstring;

// Allocate x-string
xstring XStringAlloc( const char* src );
// Free memory taken by x-string
void XStringFree( xstring* str );

// ===== Definition of GUID =====

// 128 bit GUID structure
typedef struct _xguid
{
    uint32_t part1;
    uint32_t part2;
    uint32_t part3;
    uint32_t part4;
}
xguid;

// Check if two guids are equal or not
bool XGuidAreEqual( const xguid* guid1, const xguid* guid2 );
// Copy source GUID into destination
void XGuidCopy( const xguid* src, xguid* dst );
// Get string representation of the specified GUID. Returns number of characters (including 0 terminator) put into the buffer.
uint32_t XGuidToString( const xguid* guid, char* buffer, uint32_t bufferLen );
// Convert given string into GUID
XErrorCode XStringToGuid( const char* buffer, xguid* guid );

#ifdef __cplusplus
// provide xguid comparer for STL maps
struct xguid_cmp
{
    bool operator() ( const xguid& lhs, const xguid& rhs ) const
    {
        return ( ( lhs.part1  < rhs.part1 ) ||
               ( ( lhs.part1 == rhs.part1 ) && ( lhs.part2  < rhs.part2 ) ) ||
               ( ( lhs.part1 == rhs.part1 ) && ( lhs.part2 == rhs.part2 ) && ( lhs.part3  < rhs.part3 ) ) ||
               ( ( lhs.part1 == rhs.part1 ) && ( lhs.part2 == rhs.part2 ) && ( lhs.part3 == rhs.part3 ) && ( lhs.part4 < rhs.part4 ) ) );
    }
};
#endif

// ===== Definition of ARGB color type =====
typedef union
{
    uint32_t argb;
    struct
    {
        uint8_t b;
        uint8_t g;
        uint8_t r;
        uint8_t a;
    }
    components;
}
xargb;

// ===== Definition of integer range structure =====
typedef struct _xrange
{
    int32_t min;
    int32_t max;
}
xrange;

// Get intersection of two ranges - make sure subject range is within specified bounds
void XRangeIntersect( xrange* subject, const xrange* bounds );

// ===== Definition of floating point range structure =====
typedef struct _xrangef
{
    float min;
    float max;
}
xrangef;

// Get intersection of two ranges - make sure subject range is within specified bounds
void XRangeIntersect ( xrange*  subject, const xrange*  bounds );
void XRangeIntersectF( xrangef* subject, const xrangef* bounds );

// ===== Enumeration defining some key points of a range =====
typedef enum XRangePointTag
{
    RangeMin    = 0,
    RangeMax    = 1,
    RangeMiddle = 2
}
XRangePoint;

// ===== Definition of integer 2D point structure =====
typedef struct _xpoint
{
    int32_t x;
    int32_t y;
}
xpoint;

// ===== Definition of point structure with floats =====
typedef struct _xpointf
{
    float x;
    float y;
}
xpointf;

// ===== Definition of integer 2D size structure =====
typedef struct _xsize
{
    int32_t width;
    int32_t height;
}
xsize;

// ===== Definition of integer rectangle structure =====

typedef struct _xrect
{
    int32_t x1;
    int32_t y1;
    int32_t x2;
    int32_t y2;
}
xrect;

// ===== Definition of variant type =====

typedef uint16_t XVarType;

// List of data types supported by the variant type
enum
{
    XVT_Empty = 0,
    XVT_Null,
    XVT_Bool,
    XVT_I1,
    XVT_I2,
    XVT_I4,
    XVT_U1,
    XVT_U2,
    XVT_U4,
    XVT_R4,
    XVT_R8,
    XVT_String,
    XVT_ARGB,
    XVT_Range,
    XVT_RangeF,
    XVT_Point,
    XVT_PointF,
    XVT_Size,

    // Images must be used as variant types with extra care. XVariantCopy() does not make deep copy of an image.
    // Only an image structure is allocated/deallocated by XVaraintXXX functions.
    // And so the original image must remain alive while any copied variant is alive.
    XVT_Image,

    // all types are put before the Any
    XVT_Any = 0x00FF,

    // reserved values start from 0x0100

    XVT_ArrayJagged = 0x2000,
    XVT_Array2d     = 0x4000,
    XVT_Array       = 0x8000,
};

// Macro to check if the specified variant type represnts an array
#define IS_ARRAY_TYPE(type)     ( ( type & ( XVT_Array | XVT_Array2d | XVT_ArrayJagged ) ) != 0 )

// Forward declaration of variant's arrays
struct _XVARIANT_ARRAY;
struct _XVARIANT_ARRAY_2D;
struct _XVARIANT_ARRAY_JAGGED;

// Forward declaration of ximage (which lives in ximage.h)
struct _ximage;

// Union of variant data types
union xvariant_value
{
    bool     boolVal;
    int8_t   bVal;
    uint8_t  ubVal;
    int16_t  sVal;
    uint16_t usVal;
    int32_t  iVal;
    uint32_t uiVal;
    float    fVal;
    double   dVal;
    xstring  strVal;
    xargb    argbVal;
    xrange   rangeVal;
    xrangef  frangeVal;
    xpoint   pointVal;
    xpointf  fpointVal;
    xsize    sizeVal;
    void*    ptrVal;
    struct _XVARIANT_ARRAY*        arrayVal;
    struct _XVARIANT_ARRAY_2D*     array2Val;
    struct _XVARIANT_ARRAY_JAGGED* arrayJaggedVal;
    struct _ximage*                imageVal;
};

// Variant type's structure
typedef struct _XVARIANT
{
    union xvariant_value value;
    XVarType type;
}
xvariant;

// Initialize variant variable setting it to empty
void XVariantInit( xvariant* var );
// Clear variant variable to empty and free any taken memory
void XVariantClear( xvariant* var );
// Make a copy of the variant variable
XErrorCode XVariantCopy( const xvariant* src, xvariant* dst );
// Check if two variants have same values (including type)
XErrorCode XVariantAreEqual( const xvariant* var1, const xvariant* var2 );
// Change type of the variant variable to the specified one
XErrorCode XVariantChangeType( const xvariant* src, xvariant* dst, XVarType dstType );
// Set of conversion functions
XErrorCode XVariantToString( const xvariant* src, xstring* dst );
XErrorCode XVariantToDouble( const xvariant* src, double* dst );
XErrorCode XVariantToFloat( const xvariant* src, float* dst );
XErrorCode XVariantToUInt( const xvariant* src, uint32_t* dst );
XErrorCode XVariantToUShort( const xvariant* src, uint16_t* dst );
XErrorCode XVariantToUByte( const xvariant* src, uint8_t* dst );
XErrorCode XVariantToInt( const xvariant* src, int32_t* dst );
XErrorCode XVariantToShort( const xvariant* src, int16_t* dst );
XErrorCode XVariantToByte( const xvariant* src, int8_t* dst );
XErrorCode XVariantToBool( const xvariant* src, bool* dst );
XErrorCode XVariantToRange( const xvariant* src, xrange* dst );
XErrorCode XVariantToRangeF( const xvariant* src, xrangef* dst );
XErrorCode XVariantToPoint( const xvariant* src, xpoint* dst );
XErrorCode XVariantToPointF( const xvariant* src, xpointf* dst );
XErrorCode XVariantToSize( const xvariant* src, xsize* dst );
XErrorCode XVariantToArgb( const xvariant* src, xargb* dst );

// Make sure numeric variant value is in the specified range
void XVariantCheckInRange( xvariant* src, const xvariant* min, const xvariant* max );

// Variant array functions
typedef struct _XVARIANT_ARRAY
{
    XVarType  type;
    xvariant* elements;
    uint32_t  length;
}
xarray;

typedef struct _XVARIANT_ARRAY_2D
{
    XVarType  type;
    xvariant* elements;
    uint32_t  length;
    uint32_t  rows;
    uint32_t  cols;
}
xarray2d;

// Make jagged array look same as 1D array (1D array can contain arrays as elements anyway).
// The main structure will have its type set to ( XVT_Array | elementType ), while all
// inner arrays will have it set to elementType.
typedef struct _XVARIANT_ARRAY_JAGGED
{
    XVarType  type;
    xvariant* elements;
    uint32_t  length;
}
xarrayJagged;

// Free specified array of variants
void XArrayFree( xarray** pArray );
void XArrayFree2d( xarray2d** pArray );
void XArrayFreeJagged( xarrayJagged** pArray );
// Allocate array of the specified size and type
XErrorCode XArrayAllocate( xarray** pArray, XVarType type, uint32_t length );
// Allocate 2D array of the specified size and type
XErrorCode XArrayAllocate2d( xarray2d** pArray, XVarType type, uint32_t rows, uint32_t cols );
// Allocate jagged array of the specified size and type (each element is set to Empty, so needs to be allocated separately)
XErrorCode XArrayAllocateJagged( xarrayJagged** pArray, XVarType type, uint32_t length );
// Allocate sub array of a jagged array at the specified index
XErrorCode XArrayAllocateJaggedSub( xarrayJagged* pArray, uint32_t index, uint32_t subArrayLength );
// Get element's type of the array (XVT_Null for null pointer)
XVarType XArrayType( const xarray* array );
// Get length of the array
uint32_t XArrayLength( const xarray* array );
// Get size of the of the 2D array
XErrorCode XArraySize( const xarray2d* array, uint32_t* rows, uint32_t* cols );
// Set specified element of the array to the specified value - value is copied.
// If value type and array element's type are different, then type change is applied.
XErrorCode XArraySet( xarray* array, uint32_t index, const xvariant* element );
// Set specified element of the 2D array to the specified value
XErrorCode XArraySet2d( xarray2d* array, uint32_t row, uint32_t col, const xvariant* element );
// Set element of a jagged array
XErrorCode XArraySetJagged( xarrayJagged* array, uint32_t outerIndex, uint32_t innerIndex, const xvariant* element );
// Set specified element of the array to the specified value - value is moved into array.
// If value type and array element's type are different, an error is returned.
XErrorCode XArrayMove( xarray* array, uint32_t index, xvariant* element );
// Set all elements of the array to the specified value
XErrorCode XArraySetAll( xarray* array, const xvariant* element );
// Set all elements of the 2D array to the specified value
XErrorCode XArraySetAll2d( xarray2d* array, const xvariant* element );
// Set all elements of the jagged array to the specified value
XErrorCode XArraySetAllJagged( xarrayJagged* array, const xvariant* element );
// Set all elements of jagged array's sub-array
XErrorCode XArraySetAllJaggedSub( xarrayJagged* array, uint32_t index, const xvariant* element );
// Get array's element at the specified index
XErrorCode XArrayGet( const xarray* array, uint32_t index, xvariant* element );
// Get 2D array's element at the specified row/column
XErrorCode XArrayGet2d( const xarray2d* array, uint32_t row, uint32_t col, xvariant* element );
// Get element of a jagged array
XErrorCode XArrayGetJagged( xarrayJagged* array, uint32_t outerIndex, uint32_t innerIndex, xvariant* element );
// Copy/clone specified variant array
XErrorCode XArrayCopy( const xarray* src, xarray** dst );
XErrorCode XArrayCopy2d( const xarray2d* src, xarray2d** dst );
XErrorCode XArrayCopyJagged( const xarrayJagged* src, xarrayJagged** dst );
XErrorCode XArrayCopyJaggedTo1D( const xarrayJagged* src, xarray** dst );
// Copy specified variant array and change type of elements to the target type
XErrorCode XArrayChangeType( const xarray* src, xarray** dst, XVarType dstType );
XErrorCode XArrayChangeType2d( const xarray2d* src, xarray2d** dst, XVarType dstType );
XErrorCode XArrayChangeTypeJagged( const xarrayJagged* src, xarrayJagged** dst, XVarType dstType );
// Check if two arrays have same elements (including type)
XErrorCode XArrayAreEqual( const xarray* array1, const xarray* array2 );
XErrorCode XArrayAreEqual2d( const xarray2d* array1, const xarray2d* array2 );
XErrorCode XArrayAreEqualJagged( const xarrayJagged* array1, const xarrayJagged* array2 );

// ===== Tool functions =====

// Get description for the specified error code
xstring XErrorGetDescription( XErrorCode error );

// ===== Some functions related to bits' management =====

// Get number of os bits set in the given integer
uint32_t XBitsCount( uint32_t bits );
// Get the lowest set bit in the given integer
uint32_t XBitsGetLowestSet( uint32_t bits );
// Get the highest set bit in the given integer
uint32_t XBitsGetHighestSet( uint32_t bits );

#ifdef __cplusplus
}
#endif

#endif // CVS_XTYPES_H
