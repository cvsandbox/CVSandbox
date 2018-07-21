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

#include <stdlib.h>
#include "xtypes.h"

// Free specified array of variants
void XArrayFree( xarray** pArray )
{
    if ( pArray != 0 )
    {
        xarray* array = *pArray;

        if ( array != 0 )
        {
            if ( array->elements != 0 )
            {
                uint32_t counter = array->length;

                while ( counter != 0 )
                {
                    counter--;
                    XVariantClear( &array->elements[counter] );
                }

                XFree( (void**) &array->elements );
            }

            XFree( (void**) pArray );
        }
    }
}
void XArrayFree2d( xarray2d** pArray )
{
    XArrayFree( (xarray**) pArray );
}
void XArrayFreeJagged( xarrayJagged** pArray )
{
    XArrayFree( (xarray**) pArray );
}

// Allocate array of the specified size and type
XErrorCode XArrayAllocate( xarray** pArray, XVarType type, uint32_t length )
{
    XErrorCode ret = SuccessCode;

    if ( pArray == 0 )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        if ( ( type == XVT_Empty ) || ( type == XVT_Null ) || ( type > XVT_Any ) )
        {
            ret = ErrorInvalidType;
        }
        else
        {
            XArrayFree( pArray );

            *pArray = (xarray*) XMAlloc( sizeof( xarray ) );

            if ( *pArray == 0 )
            {
                ret = ErrorOutOfMemory;
            }
            else
            {
                xarray* array = *pArray;

                array->elements = (xvariant*) XCAlloc( length, sizeof( xvariant ) );

                if ( array->elements == 0 )
                {
                    ret = ErrorOutOfMemory;
                    XArrayFree( pArray );
                }
                else
                {
                    array->type   = type;
                    array->length = length;
                }
            }
        }
    }

    return ret;
}

// Allocate 2D array of the specified size and type
XErrorCode XArrayAllocate2d( xarray2d** pArray, XVarType type, uint32_t rows, uint32_t cols )
{
    XErrorCode ret = SuccessCode;

    if ( pArray == 0 )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        if ( ( type == XVT_Empty ) || ( type == XVT_Null ) || ( type > XVT_Any ) )
        {
            ret = ErrorInvalidType;
        }
        else
        {
            XArrayFree2d( pArray );

            *pArray = (xarray2d*) XMAlloc( sizeof( xarray2d ) );

            if ( *pArray == 0 )
            {
                ret = ErrorOutOfMemory;
            }
            else
            {
                xarray2d* array = *pArray;

                array->elements = (xvariant*) XCAlloc( rows * cols, sizeof( xvariant ) );

                if ( array->elements == 0 )
                {
                    ret = ErrorOutOfMemory;
                    XArrayFree2d( pArray );
                }
                else
                {
                    array->type   = type;
                    array->length = rows * cols;
                    array->rows   = rows;
                    array->cols   = cols;
                }
            }
        }
    }

    return ret;
}

// Allocate jagged array of the specified size and type
XErrorCode XArrayAllocateJagged( xarrayJagged** pArray, XVarType type, uint32_t length )
{
    XErrorCode ret = SuccessCode;

    if ( pArray == 0 )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        if ( ( type == XVT_Empty ) || ( type == XVT_Null ) || ( type > XVT_Any ) )
        {
            ret = ErrorInvalidType;
        }
        else
        {
            XArrayFreeJagged( pArray );

            *pArray = (xarrayJagged*) XMAlloc( sizeof( xarrayJagged ) );

            if ( *pArray == 0 )
            {
                ret = ErrorOutOfMemory;
            }
            else
            {
                xarrayJagged* array = *pArray;

                array->elements = (xvariant*) XCAlloc( length, sizeof( xvariant ) );

                if ( array->elements == 0 )
                {
                    ret = ErrorOutOfMemory;
                    XArrayFreeJagged( pArray );
                }
                else
                {
                    array->type   = type | XVT_Array;
                    array->length = length;
                }
            }
        }
    }

    return ret;
}

// Allocate sub array of a jagged array at the specified index
XErrorCode XArrayAllocateJaggedSub( xarrayJagged* pArray, uint32_t index, uint32_t subArrayLength )
{
    XErrorCode ret = SuccessCode;

    if ( pArray == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( index >= pArray->length )
    {
        ret = ErrorIndexOutOfBounds;
    }
    else
    {
        XVarType elementType = ( pArray->type & XVT_Any );
        xarray*  array1d     = 0;

        XVariantClear( &( pArray->elements[index] ) );

        array1d = (xarray*) XMAlloc( sizeof( xarray ) );

        if ( array1d == 0 )
        {
            ret = ErrorOutOfMemory;
        }
        else
        {
            array1d->elements = (xvariant*) XCAlloc( subArrayLength, sizeof( xvariant ) );

            if ( array1d->elements == 0 )
            {
                ret = ErrorOutOfMemory;
                XArrayFree( &array1d );
            }
            else
            {
                array1d->type   = elementType;
                array1d->length = subArrayLength;

                pArray->elements[index].type = elementType | XVT_Array;
                pArray->elements[index].value.arrayVal = array1d;
            }
        }
    }

    return ret;
}

// Get element's type of the array (XVT_Null for null pointer)
XVarType XArrayType( const xarray* array )
{
    return ( array == 0 ) ? XVT_Null : array->type;
}

// Get length of the array
uint32_t XArrayLength( const xarray* array )
{
    return ( array == 0 ) ? 0 : array->length;
}

// Get size of the 2D array
XErrorCode XArraySize( const xarray2d* array, uint32_t* rows, uint32_t* cols )
{
    XErrorCode ret = SuccessCode;

    if ( ( array == 0 ) || ( rows == 0 ) || ( cols == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        *rows = array->rows;
        *cols = array->cols;
    }

    return ret;
}

// Set specified element of the array to the specified value - value is copied
XErrorCode XArraySet( xarray* array, uint32_t index, const xvariant* element )
{
    XErrorCode ret = SuccessCode;

    if ( ( array == 0 ) || ( element == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( index >= array->length )
    {
        ret = ErrorIndexOutOfBounds;
    }
    else
    {
        if ( ( array->type == XVT_Any ) || ( array->type == element->type ) )
        {
            ret = XVariantCopy( element, &array->elements[index] );
        }
        else
        {
            ret = XVariantChangeType( element, &array->elements[index], array->type );
        }
    }

    return ret;
}

// Set specified element of the array to the specified value - value is moved into array
XErrorCode XArrayMove( xarray* array, uint32_t index, xvariant* element )
{
    XErrorCode ret = SuccessCode;

    if ( ( array == 0 ) || ( element == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( index >= array->length )
    {
        ret = ErrorIndexOutOfBounds;
    }
    else
    {
        if ( ( array->type == XVT_Any ) || ( array->type == element->type ) )
        {
            // clear old value of the array
            XVariantClear( &array->elements[index] );

            // set new value
            array->elements[index] = *element;

            // empty the source element
            element->type  = XVT_Empty;
        }
        else
        {
            ret = ErrorIncompatibleTypes;
        }
    }

    return ret;
}

// Set all elements of the array to the specified value
XErrorCode XArraySetAll( xarray* array, const xvariant* element )
{
    XErrorCode ret = SuccessCode;

    if ( ( array == 0 ) || ( element == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        const xvariant* srcPtr = element;
        xvariant        temp;
        uint32_t        i;

        XVariantInit( &temp );

        // do type conversion once if needed
        if ( ( array->type != XVT_Any ) && ( array->type != element->type ) )
        {
            ret = XVariantChangeType( element, &temp, array->type );
            srcPtr = &temp;
        }

        for ( i = 0; ( i < array->length ) && ( ret == SuccessCode ); i++ )
        {
            ret = XVariantCopy( srcPtr, &array->elements[i] );
        }

        XVariantClear( &temp );
    }

    return ret;
}

// Set specified element of the 2D array to the specified value
XErrorCode XArraySet2d( xarray2d* array, uint32_t row, uint32_t col, const xvariant* element )
{
    XErrorCode ret = SuccessCode;

    if ( ( array == 0 ) || ( element == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( row >= array->rows ) || ( col >= array->cols ) )
    {
        ret = ErrorIndexOutOfBounds;
    }
    else
    {
        if ( ( array->type == XVT_Any ) || ( array->type == element->type ) )
        {
            ret = XVariantCopy( element, &array->elements[row * array->cols + col] );
        }
        else
        {
            ret = XVariantChangeType( element, &array->elements[row * array->cols + col], array->type );
        }
    }

    return ret;
}

// Set element of a jagged array
XErrorCode XArraySetJagged( xarrayJagged* array, uint32_t outerIndex, uint32_t innerIndex, const xvariant* element )
{
    XErrorCode ret = SuccessCode;

    if ( ( array == 0 ) || ( element == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( outerIndex >= array->length )
    {
        ret = ErrorIndexOutOfBounds;
    }
    else if ( ( array->elements[outerIndex].type == XVT_Empty ) ||
              ( array->elements[outerIndex].type == XVT_Null ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( array->elements[outerIndex].type & XVT_Array ) == 0 )
    {
        ret = ErrorInvalidArgument;
    }
    else if ( innerIndex >= array->elements[outerIndex].value.arrayVal->length )
    {
        ret = ErrorIndexOutOfBounds;
    }
    else
    {
        XVarType elementType = array->elements[outerIndex].value.arrayVal->type;

        if ( ( elementType == XVT_Any ) || ( elementType == element->type ) )
        {
            ret = XVariantCopy( element, &( array->elements[outerIndex].value.arrayVal->elements[innerIndex] ) );
        }
        else
        {
            ret = XVariantChangeType( element, &( array->elements[outerIndex].value.arrayVal->elements[innerIndex] ), elementType );
        }
    }

    return ret;
}

// Set all elements of the 2D array to the specified value
XErrorCode XArraySetAll2d( xarray2d* array, const xvariant* element )
{
    return XArraySetAll( (xarray*) array, element );
}

// Set all elements of the jagged array to the specified value
XErrorCode XArraySetAllJagged( xarrayJagged* array, const xvariant* element )
{
    XErrorCode ret = SuccessCode;

    if ( ( array == 0 ) || ( element == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        uint32_t i;

        for ( i = 0; ( i < array->length ) && ( ret == SuccessCode ); i++ )
        {
            if ( array->elements[i].type != XVT_Empty )
            {
                ret = XArraySetAll( array->elements[i].value.arrayVal, element );
            }
        }
    }

    return ret;
}

// Set all elements of jagged array's sub-array
XErrorCode XArraySetAllJaggedSub( xarrayJagged* array, uint32_t index, const xvariant* element )
{
    XErrorCode ret = SuccessCode;

    if ( ( array == 0 ) || ( element == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( index >= array->length )
    {
        ret = ErrorIndexOutOfBounds;
    }
    else
    {
        ret = XArraySetAll( array->elements[index].value.arrayVal, element );
    }

    return ret;
}

// Get array's element at the specified index
XErrorCode XArrayGet( const xarray* array, uint32_t index, xvariant* element )
{
    XErrorCode ret = SuccessCode;

    if ( ( array == 0 ) || ( element == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        if ( index >= array->length )
        {
            ret = ErrorIndexOutOfBounds;
        }
        else
        {
            ret = XVariantCopy( &array->elements[index], element );
        }
    }

    return ret;
}

// Get 2D array's element at the specified row/column
XErrorCode XArrayGet2d( const xarray2d* array, uint32_t row, uint32_t col, xvariant* element )
{
    XErrorCode ret = SuccessCode;

    if ( ( array == 0 ) || ( element == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        if ( ( row >= array->rows ) || ( col >= array->cols ) )
        {
            ret = ErrorIndexOutOfBounds;
        }
        else
        {
            ret = XVariantCopy( &array->elements[row * array->cols + col], element );
        }
    }

    return ret;
}

// Get element of a jagged array
XErrorCode XArrayGetJagged( xarrayJagged* array, uint32_t outerIndex, uint32_t innerIndex, xvariant* element )
{
    XErrorCode ret = SuccessCode;

    if ( ( array == 0 ) || ( element == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( outerIndex >= array->length )
    {
        ret = ErrorIndexOutOfBounds;
    }
    else if ( ( array->elements[outerIndex].type == XVT_Empty ) ||
              ( array->elements[outerIndex].type == XVT_Null ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( array->elements[outerIndex].type & XVT_Array ) == 0 )
    {
        ret = ErrorInvalidArgument;
    }
    else if ( innerIndex >= array->elements[outerIndex].value.arrayVal->length )
    {
        ret = ErrorIndexOutOfBounds;
    }
    else
    {
        ret = XVariantCopy( &( array->elements[outerIndex].value.arrayVal->elements[innerIndex] ), element );
    }

    return ret;
}

// Helper function to copy elements of an array
static XErrorCode XArrayCopyElements( const xarray* src, xarray* dst )
{
    XErrorCode ret     = SuccessCode;
    uint32_t   counter = src->length;

    if ( src->type == dst->type )
    {
        while ( ( counter != 0 ) && ( ret == SuccessCode ) )
        {
            counter--;
            ret = XVariantCopy( &src->elements[counter], &dst->elements[counter] );
        }
    }
    else
    {
        XVarType dstType = dst->type;

        while ( ( counter != 0 ) && ( ret == SuccessCode ) )
        {
            counter--;
            ret = XVariantChangeType( &src->elements[counter], &dst->elements[counter], dstType );
        }

    }

    return ret;
}

// Copy specified variant array
XErrorCode XArrayCopy( const xarray* src, xarray** dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        // clean old array
        XArrayFree( dst );

        // allocate the new one
        ret = XArrayAllocate( dst, src->type, src->length );

        if ( ret == SuccessCode )
        {
            ret = XArrayCopyElements( src, *dst );

            if ( ret != SuccessCode )
            {
                // if failed copying any of the elements, free entire array then
                XArrayFree( dst );
            }
        }
    }

    return ret;
}

// Copy/clone specified variant 2D array
XErrorCode XArrayCopy2d( const xarray2d* src, xarray2d** dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        // clean old array
        XArrayFree2d( dst );

        // allocate the new one
        ret = XArrayAllocate2d( dst, src->type, src->rows, src->cols );

        if ( ret == SuccessCode )
        {
            ret = XArrayCopyElements( (const xarray*) src, (xarray*) *dst );

            if ( ret != SuccessCode )
            {
                // if failed copying any of the elements, free entire array then
                XArrayFree2d( dst );
            }
        }
    }

    return ret;
}

// Copy/clone specified variant jagged array
XErrorCode XArrayCopyJagged( const xarrayJagged* src, xarrayJagged** dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        // clean old array
        XArrayFreeJagged( dst );

        // allocate the new one (removing array specifier to get only elements type)
        ret = XArrayAllocateJagged( dst, src->type & XVT_Any, src->length );

        if ( ret == SuccessCode )
        {
            ret = XArrayCopyElements( (const xarray*) src, (xarray*) *dst );

            if ( ret != SuccessCode )
            {
                // if failed copying any of the elements, free entire array then
                XArrayFreeJagged( dst );
            }
        }
    }

    return ret;
}

// Copy/clone specified variant jagged array into 1D array (allocated)
XErrorCode XArrayCopyJaggedTo1D( const xarrayJagged* src, xarray** dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        XVarType elementsType  = src->type & XVT_Any;
        uint32_t elementsCount = 0;
        uint32_t i, j, counter = 0;

        // clean old array
        XArrayFree( dst );

        for ( i = 0; i < src->length; i++ )
        {
            if ( src->elements[i].type == ( elementsType | XVT_Array ) )
            {
                elementsCount += src->elements[i].value.arrayVal->length;
            }
        }

        // allocate the new one
        ret = XArrayAllocate( dst, elementsType, elementsCount );

        if ( ( ret == SuccessCode ) && ( elementsCount != 0 ) )
        {
            xarray* tempDst = *dst;

            for ( i = 0; ( i < src->length ) && ( ret == SuccessCode ); i++ )
            {
                if ( src->elements[i].type == ( elementsType | XVT_Array ) )
                {
                    xarray* tempSrc = src->elements[i].value.arrayVal;

                    for ( j = 0; ( j < tempSrc->length ) && ( ret == SuccessCode ); j++ )
                    {
                        ret = XVariantCopy( &( tempSrc->elements[j] ), &( tempDst->elements[counter] ) );
                        counter++;
                    }
                }
            }

            if ( ret != SuccessCode )
            {
                XArrayFree( dst );
            }
        }
    }

    return ret;
}

// Copy specified variant array and change type of elements to the target type
XErrorCode XArrayChangeType( const xarray* src, xarray** dst, XVarType dstType )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        // clean old array
        XArrayFree( dst );

        // allocate the new one
        ret = XArrayAllocate( dst, dstType, src->length );

        if ( ret == SuccessCode )
        {
            ret = XArrayCopyElements( src, *dst );

            if ( ret != SuccessCode )
            {
                // if failed copying any of the elements, free entire array then
                XArrayFree( dst );
            }
        }
    }

    return ret;
}

// Copy specified variant 2d array and change type of elements to the target type
XErrorCode XArrayChangeType2d( const xarray2d* src, xarray2d** dst, XVarType dstType )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        // clean old array
        XArrayFree2d( dst );

        // allocate the new one
        ret = XArrayAllocate2d( dst, dstType, src->rows, src->cols );

        if ( ret == SuccessCode )
        {
            ret = XArrayCopyElements( (const xarray*) src, (xarray*) *dst );

            if ( ret != SuccessCode )
            {
                // if failed copying any of the elements, free entire array then
                XArrayFree2d( dst );
            }
        }
    }

    return ret;
}

// Copy specified variant jagged array and change type of elements to the target type
XErrorCode XArrayChangeTypeJagged( const xarrayJagged* src, xarrayJagged** dst, XVarType dstType )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        // clean old array
        XArrayFreeJagged( dst );

        // allocate the new one
        ret = XArrayAllocateJagged( dst, dstType, src->length );

        if ( ret == SuccessCode )
        {
            ret = XArrayCopyElements( (const xarray*) src, (xarray*) *dst );

            if ( ret != SuccessCode )
            {
                // if failed copying any of the elements, free entire array then
                XArrayFreeJagged( dst );
            }
        }
    }

    return ret;
}

// Helper function which dows actual comparing of array elements
static XErrorCode XArrayAreEqualHelper( const xarray* array1, const xarray* array2 )
{
    XErrorCode ret  = SuccessCode;
    uint32_t i, len = array1->length;

    for ( i = 0; ( i < len ) && ( ret == SuccessCode ); i++ )
    {
        ret = XVariantAreEqual( &( array1->elements[i] ), &( array2->elements[i] ) );
    }

    return ret;
}

// Check if two arrays have same elements (including type)
XErrorCode XArrayAreEqual( const xarray* array1, const xarray* array2 )
{
    XErrorCode ret = ErrorFailed;

    if ( ( array1 == 0 ) || ( array2 == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( array1->type != array2->type )
    {
        ret = ErrorIncompatibleTypes;
    }
    else
    {
        if ( array1->length == array2->length )
        {
            ret = XArrayAreEqualHelper( array1, array2 );
        }
    }

    return ret;
}
XErrorCode XArrayAreEqual2d( const xarray2d* array1, const xarray2d* array2 )
{
    XErrorCode ret = ErrorFailed;

    if ( ( array1 == 0 ) || ( array2 == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( array1->type != array2->type )
    {
        ret = ErrorIncompatibleTypes;
    }
    else
    {
        if ( ( array1->rows == array2->rows ) && ( array1->cols == array2->cols ) )
        {
            ret = XArrayAreEqualHelper( (const xarray*) array1, (const xarray*) array2 );
        }
    }

    return ret;
}
XErrorCode XArrayAreEqualJagged( const xarrayJagged* array1, const xarrayJagged* array2 )
{
    return XArrayAreEqual( (const xarray*) array1, (const xarray*) array2 );
}
