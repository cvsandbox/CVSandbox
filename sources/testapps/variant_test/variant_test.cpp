/*
    XVariant data type test application

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

#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <stdio.h>

#include "xtypes.h"
#include "XVariantArray.hpp"
#include "XVariantArrayJagged.hpp"

using namespace CVSandbox;

static void Test1( );
static void Test2( );
static void Test3( );
static void Test4( );
static void Test5( );

int main( int /* argc */, char** /* argv */ )
{
    // _CrtSetBreakAlloc( 81 );

    printf( "Testing variant types ... \n" );

    {
        Test1( );
        Test2( );
        Test3( );
        Test4( );
        Test5( );
    }

    printf( "\n\ndone\n" );

    #ifdef _DEBUG
    getchar( );
    #endif

    #ifdef _MSC_VER
    _CrtDumpMemoryLeaks( );
    #endif

    return 0;
}

static void PrintIntegeArrayFromVariant( xvariant* var )
{
    if ( var != nullptr )
    {
        if ( var->type == ( XVT_Array | XVT_I4 ) )
        {
            xarray* array = var->value.arrayVal;

            printf( "len = %u, [ ", array->length );

            for ( uint32_t i = 0; i < array->length; i++ )
            {
                printf( "%d ", array->elements[i].value.iVal );
            }

            printf( "]\n" );
        }
        else if ( var->type == ( XVT_Array2d | XVT_I4 ) )
        {
            xarray2d* array = var->value.array2Val;

            printf( "row = %u, cols = %u, [ \n", array->rows, array->cols );

            for ( uint32_t i = 0; i < array->rows; i++ )
            {
                printf( "[ " );
                for ( uint32_t j = 0; j < array->cols; j++ )
                {
                    printf( "%d ", array->elements[i * array->cols + j].value.iVal );
                }
                printf( "]\n" );
            }

            printf( "]\n" );
        }
    }
}

static void PrintFloatArrayFromVariant( xvariant* var )
{
    if ( ( var != nullptr ) && ( var->type == ( XVT_Array | XVT_R4 ) ) )
    {
        xarray* array = var->value.arrayVal;

        printf( "len = %u, [", array->length );

        for ( uint32_t i = 0; i < array->length; i++ )
        {
            printf( "%.1f ", array->elements[i].value.fVal );
        }

        printf( "]\n" );
    }
}

static void GetIntegerArray( const xarray* array, uint32_t index )
{
    XErrorCode ecode;
    xvariant   xvar;

    XVariantInit( &xvar );

    ecode = XArrayGet( array, index, &xvar );
    if ( ecode != SuccessCode )
    {
        printf( "ERROR: Failed getting sub array of jagged array at %u: %d \n", index, ecode );
    }
    else
    {
        printf( "got element, type = %04X \n", xvar.type );

        if ( xvar.type != ( XVT_Array | XVT_I4 ) )
        {
            printf( "ERROR: Unexpected element type at index %u \n", index );
        }
        else
        {
            PrintIntegeArrayFromVariant( &xvar );
        }

        XVariantClear( &xvar );
    }
}

static void GetFloatArray( const xarray* array, uint32_t index )
{
    XErrorCode ecode;
    xvariant   xvar;

    XVariantInit( &xvar );

    ecode = XArrayGet( array, index, &xvar );
    if ( ecode != SuccessCode )
    {
        printf( "ERROR: Failed getting sub array of jagged array at %u: %d \n", index, ecode );
    }
    else
    {
        printf( "got element, type = %04X \n", xvar.type );

        if ( xvar.type != ( XVT_Array | XVT_R4 ) )
        {
            printf( "ERROR: Unexpected element type at index %u \n", index );
        }
        else
        {
            PrintFloatArrayFromVariant( &xvar );
        }

        XVariantClear( &xvar );
    }
}

static void SetIntegerSubArray( xarrayJagged* pArray, uint32_t index, uint32_t subArrayLength, int32_t value )
{
    XErrorCode ecode;
    xvariant   xvar;

    XVariantInit( &xvar );

    xvar.type = XVT_I4;
    xvar.value.iVal = value;

    ecode = XArrayAllocateJaggedSub( pArray, index, subArrayLength );
    if ( ecode != SuccessCode )
    {
        printf( "ERROR: Failed allocating sub array at %u: %d \n", index, ecode );
    }
    else
    {
        ecode = XArraySetAllJaggedSub( pArray, index, &xvar );
        if ( ecode != SuccessCode )
        {
            printf( "ERROR: Failed setting all element of sub array at %u \n", index );
        }
    }
}

static bool StringToArray( xstring str, xvariant* arrayVar, XVarType type )
{
    bool ret = false;

    if ( arrayVar != nullptr )
    {
        XErrorCode ecode;
        xvariant   strVar;

        XVariantInit( &strVar );
        XVariantInit( arrayVar );

        strVar.type = XVT_String;
        strVar.value.strVal = str;

        ecode = XVariantChangeType( &strVar, arrayVar, type );
        if ( ecode != SuccessCode )
        {
            printf( "ERROR: Failed converting string to requested type: %d \n", ecode );
        }
        else
        {
            ret = true;
        }
    }

    return ret;
}

void PrintArray( const XVariantArray& array )
{
    printf( "array, length: %u, type: %u \n", array.Length( ), array.Type( ) );
    printf( "{ " );

    for ( uint32_t i = 0; i < array.Length( ); i++ )
    {
        XVariant var;

        if ( array.Get( i, var ) == SuccessCode )
        {
            printf( "%s ", var.ToString( ).c_str( ) );
        }
        else
        {
            printf( "ERROR: Failed getting element (%u) \n", i );
        }
    }

    printf( "} \n" );

}

void PrintJaggedArray( const XVariantArrayJagged& array )
{
    printf( "jagged array, length: %u, type: %u \n", array.Length( ), array.Type( ) );

    for ( uint32_t i = 0; i < array.Length( ); i++ )
    {
        printf( "{ " );

        for ( uint32_t j = 0; j < array.Length( i ); j++ )
        {
            XVariant var;

            if ( array.Get( i, j, var ) == SuccessCode )
            {
                printf( "%s ", var.ToString( ).c_str( ) );
            }
            else
            {
                printf( "ERROR: Failed getting element (%u, %u) \n", i, j );
            }
        }

        printf( "} \n" );
    }

}

// Testing plain jagged arrays
void Test1( )
{
    XErrorCode    ecode;
    xarrayJagged* arrayJagged  = nullptr;
    xarrayJagged* arrayJagged2 = nullptr;
    xarrayJagged* arrayJagged3 = nullptr;
    xarray*       subArray     = nullptr;
    xvariant      xvar;

    printf( "=== Test 1 === (basic jagged array functions) \n" );

    XVariantInit( &xvar );

    ecode = XArrayAllocateJagged( &arrayJagged, XVT_I4, 5 );

    if ( ecode != SuccessCode )
    {
        printf( "ERROR: Failed allocating jagged array: %d \n", ecode );
    }
    else
    {
        xarray* array = (xarray*) arrayJagged;

        printf( "Array length: %u \n", XArrayLength( array ) );
        printf( "Elements type: %04X \n", XArrayType( array ) );

        ecode = XArrayGet( array, 2, &xvar );
        if ( ecode != SuccessCode )
        {
            printf( "ERROR: Failed getting sub array of jagged array \n" );
        }
        else
        {
            printf( "got element, type = %04X \n", xvar.type );
            XVariantClear( &xvar );
        }

        // set all elements of the jagged array
        ecode = XArrayAllocate( &subArray, XVT_I4, 1 );
        if ( ecode != SuccessCode )
        {
            printf( "ERROR: Failed allocating 1D array for all: %d \n", ecode );
        }
        else
        {
            xvar.type = XVT_I4;
            xvar.value.iVal = 9;

            XArraySet( subArray, 0, &xvar );

            xvar.type = XVT_Array | XVT_I4;
            xvar.value.arrayVal = subArray;

            ecode = XArraySetAll( array, &xvar );
            if ( ecode != SuccessCode )
            {
                printf( "ERROR: Failed setting sub array for all: %d \n", ecode );
            }
            else
            {
                printf( "Set sub array for all \n" );
            }

            if ( xvar.type != ( XVT_Array | XVT_I4 ) )
            {
                printf( "ERROR: Source element was affected \n" );
            }

            XArrayFree( &subArray );
            XVariantInit( &xvar );
        }

        // allocate 1D array and set few of its elements
        ecode = XArrayAllocate( &subArray, XVT_I4, 2 );
        if ( ecode != SuccessCode )
        {
            printf( "ERROR: Failed allocating 1D array for 2: %d \n", ecode );
        }
        else
        {
            xvar.type       = XVT_I4;
            xvar.value.iVal = 7;

            XArraySet( subArray, 0, &xvar );

            xvar.value.iVal = 13;
            XArraySet( subArray, 1, &xvar );

            xvar.type = XVT_Array | XVT_I4;
            xvar.value.arrayVal = subArray;

            ecode = XArraySet( array, 2, &xvar );
            if ( ecode != SuccessCode )
            {
                printf( "ERROR: Failed setting sub array at 2: %d \n", ecode );
            }
            else
            {
                printf( "Set sub array at 2 \n" );
            }

            if ( xvar.type != ( XVT_Array | XVT_I4 ) )
            {
                printf( "ERROR: Source element was affected \n" );
            }

            XArrayFree( &subArray );
            XVariantInit( &xvar );
        }

        // allocate another 1D array and move it into the jagged one
        ecode = XArrayAllocate( &subArray, XVT_I4, 3 );
        if ( ecode != SuccessCode )
        {
            printf( "ERROR: Failed allocating 1D array for 3: %d \n", ecode );
        }
        else
        {
            xvar.type = XVT_I4;
            xvar.value.iVal = 42;

            XArraySet( subArray, 0, &xvar );

            xvar.value.iVal = 77;
            XArraySet( subArray, 1, &xvar );

            xvar.value.iVal = 99;
            XArraySet( subArray, 2, &xvar );

            xvar.type = XVT_Array | XVT_I4;
            xvar.value.arrayVal = subArray;

            ecode = XArrayMove( array, 3, &xvar );
            if ( ecode != SuccessCode )
            {
                printf( "ERROR: Failed moving sub array at 3: %d \n", ecode );
            }
            else
            {
                printf( "Moved sub array at 3 \n" );
            }

            if ( xvar.type != XVT_Empty )
            {
                printf( "Source element must be set to empty \n" );
                XVariantClear( &xvar );
            }
            else
            {
                subArray = nullptr;
            }

            XVariantInit( &xvar );
        }

        GetIntegerArray( array, 0 );
        GetIntegerArray( array, 1 );
        GetIntegerArray( array, 2 );
        GetIntegerArray( array, 3 );
        GetIntegerArray( array, 4 );

        // try setting value of different type
        xvar.type = XVT_I4;
        xvar.value.iVal = 3;

        ecode = XArraySet( array, 4, &xvar );
        if ( ecode == SuccessCode )
        {
            printf( "ERROR: Should not be able to set value of different type \n" );
        }
        else
        {
            printf( "Setting value of different type, %d, %s \n", ecode, ( ecode == ErrorIncompatibleTypes ) ? "ok" : "bad" );
        }

        // copy jagged array
        ecode = XArrayCopyJagged( arrayJagged, &arrayJagged2 );
        if ( ecode != SuccessCode )
        {
            printf( "ERROR: Failed making copy of jagged array: %d \n", ecode );
        }
        else
        {
            xarray* array2 = (xarray*) arrayJagged2;

            printf( "Made copy of jagged array \n" );

            printf( "Array length: %u \n", XArrayLength( array2 ) );
            printf( "Elements type: %04X \n", XArrayType( array2 ) );

            GetIntegerArray( array2, 0 );
            GetIntegerArray( array2, 1 );
            GetIntegerArray( array2, 2 );
            GetIntegerArray( array2, 3 );
            GetIntegerArray( array2, 4 );
        }

        // check it two arrays are equal
        ecode = XArrayAreEqualJagged( arrayJagged, arrayJagged2 );
        if ( ecode != SuccessCode )
        {
            printf( "ERROR: Two arrays are not equal: %d \n", ecode );
        }
        else
        {
            printf( "Arrays are equal \n" );
        }

        // change type of the jagged array
        ecode = XArrayChangeTypeJagged( arrayJagged, &arrayJagged3, XVT_R4 );
        if ( ecode != SuccessCode )
        {
            printf( "ERROR: Failed changing type of jagged array: %d \n", ecode );
        }
        else
        {
            xarray* array3 = (xarray*) arrayJagged3;

            printf( "Cahnged type of jagged array \n" );

            printf( "Array length: %u \n", XArrayLength( array3 ) );
            printf( "Elements type: %04X \n", XArrayType( array3 ) );

            GetFloatArray( array3, 0 );
            GetFloatArray( array3, 1 );
            GetFloatArray( array3, 2 );
            GetFloatArray( array3, 3 );
            GetFloatArray( array3, 4 );
        }

        // Make sure two arrays are not equal
        ecode = XArrayAreEqualJagged( arrayJagged, arrayJagged3 );
        if ( ecode == SuccessCode )
        {
            printf( "ERROR: Two arrays should not be equal \n" );
        }
        else
        {
            printf( "Arrays are not equal: %d (%s) \n", ecode, ( ecode == ErrorIncompatibleTypes ) ? "ok" : "bad" );
        }

        array = nullptr;
    }

    XArrayFreeJagged( &arrayJagged );
    XArrayFreeJagged( &arrayJagged2 );
    XArrayFreeJagged( &arrayJagged3 );
    XArrayFree( &subArray );

    printf( "\n" );
}

// Testing jagged array in xvariant
void Test2( )
{
    XErrorCode    ecode;
    xarrayJagged* arrayJagged = nullptr;
    xarray*       subArray    = nullptr;
    xvariant      xvar;
    xvariant      xvarArray1;
    xvariant      xvarArray2;
    xvariant      xvarArray3;
    xvariant      xvarArray4;

    printf( "=== Test 2 === (jagged array within variant) \n" );

    XVariantInit( &xvar );
    XVariantInit( &xvarArray1 );
    XVariantInit( &xvarArray2 );
    XVariantInit( &xvarArray3 );
    XVariantInit( &xvarArray4 );

    ecode = XArrayAllocateJagged( &arrayJagged, XVT_I4, 3 );

    if ( ecode != SuccessCode )
    {
        printf( "ERROR: Failed allocating jagged array: %d \n", ecode );
    }
    else
    {
        xarray* array = (xarray*) arrayJagged;

        printf( "Array length: %u \n", XArrayLength( array ) );
        printf( "Elements type: %04X \n", XArrayType( array ) );

        // set all elements of the jagged array
        ecode = XArrayAllocate( &subArray, XVT_I4, 2 );
        if ( ecode != SuccessCode )
        {
            printf( "ERROR: Failed allocating 1D array for all: %d \n", ecode );
        }
        else
        {
            xvar.type = XVT_I4;
            xvar.value.iVal = 9;

            XArraySet( subArray, 0, &xvar );

            xvar.value.iVal = 13;

            XArraySet( subArray, 1, &xvar );

            xvar.type = XVT_Array | XVT_I4;
            xvar.value.arrayVal = subArray;

            ecode = XArraySetAll( array, &xvar );
            if ( ecode != SuccessCode )
            {
                printf( "ERROR: Failed setting sub array for all: %d \n", ecode );
            }
            else
            {
                printf( "Set sub array for all \n" );
            }

            if ( xvar.type != ( XVT_Array | XVT_I4 ) )
            {
                printf( "ERROR: Source element was affected \n" );
            }

            XArrayFree( &subArray );
            XVariantInit( &xvar );
        }

        GetIntegerArray( array, 0 );
        GetIntegerArray( array, 1 );
        GetIntegerArray( array, 2 );

        // copy jagged array into xvariant
        ecode = XArrayCopyJagged( arrayJagged, &xvarArray1.value.arrayJaggedVal );
        if ( ecode != SuccessCode )
        {
            printf( "ERROR: Failed copying jagged array into variant: %d \n", ecode );
        }
        else
        {
            xvarArray1.type = XVT_ArrayJagged | XVT_I4;

            printf( "Copied jagged array int variant \n" );

            printf( "Variant type: %04X \n", xvarArray1.type );

            printf( "Array length: %u \n", XArrayLength( xvarArray1.value.arrayVal ) );
            printf( "Elements type: %04X \n", XArrayType( xvarArray1.value.arrayVal ) );

            GetIntegerArray( xvarArray1.value.arrayVal, 0 );
            GetIntegerArray( xvarArray1.value.arrayVal, 1 );
            GetIntegerArray( xvarArray1.value.arrayVal, 2 );
        }

        // make a copy of variant, which contains jagged array
        ecode = XVariantCopy( &xvarArray1, &xvarArray2 );
        if ( ecode != SuccessCode )
        {
            printf( "ERROR: Failed copying variant with jagged array in it: %d \n", ecode );
        }
        else
        {
            printf( "Copied variant with jagged array inside \n" );

            printf( "Variant type: %04X \n", xvarArray2.type );

            printf( "Array length: %u \n", XArrayLength( xvarArray2.value.arrayVal ) );
            printf( "Elements type: %04X \n", XArrayType( xvarArray2.value.arrayVal ) );

            GetIntegerArray( xvarArray2.value.arrayVal, 0 );
            GetIntegerArray( xvarArray2.value.arrayVal, 1 );
            GetIntegerArray( xvarArray2.value.arrayVal, 2 );
        }

        // change type of the variant with jagged array
        ecode = XVariantChangeType( &xvarArray1, &xvarArray3, XVT_ArrayJagged | XVT_R4 );
        if ( ecode != SuccessCode )
        {
            printf( "ERROR: Failed changing type of variant with jagged array: %d \n", ecode );
        }
        else
        {
            printf( "Changed type of variant with jagged array inside \n" );

            printf( "Variant type: %04X \n", xvarArray3.type );

            printf( "Array length: %u \n", XArrayLength( xvarArray3.value.arrayVal ) );
            printf( "Elements type: %04X \n", XArrayType( xvarArray3.value.arrayVal ) );

            GetFloatArray( xvarArray3.value.arrayVal, 0 );
            GetFloatArray( xvarArray3.value.arrayVal, 1 );
            GetFloatArray( xvarArray3.value.arrayVal, 2 );
        }

        // make sure two variants are equal
        ecode = XVariantAreEqual( &xvarArray1, &xvarArray2 );
        if ( ecode != SuccessCode )
        {
            printf( "ERROR: Two variants (1 and 2) are not equal: %d \n", ecode );
        }
        else
        {
            printf( "Variants 1 and 2 are equal \n" );
        }

        // make sure two variants are not equal
        ecode = XVariantAreEqual( &xvarArray1, &xvarArray3 );
        if ( ecode == SuccessCode )
        {
            printf( "ERROR: Variants 1 and 3 should not be equal \n" );
        }
        else
        {
            printf( "Variants 1 and 3 are not equal: %d (%s) \n", ecode, ( ecode == ErrorIncompatibleTypes ) ? "ok" : "bad" );
        }

        // get copy of jagged array as 1D array
        ecode = XVariantChangeType( &xvarArray1, &xvarArray4, XVT_Array | XVT_I4 );
        if ( ecode != SuccessCode )
        {
            printf( "ERROR: Failed changing type of variant with jagged array to variant with 1D array: %d \n", ecode );
        }
        else
        {
            printf( "Changed type of jagged array variant to 1D array variant \n" );

            printf( "Variant type: %04X \n", xvarArray4.type );

            printf( "Array length: %u \n", XArrayLength( xvarArray4.value.arrayVal ) );
            printf( "Elements type: %04X \n", XArrayType( xvarArray4.value.arrayVal ) );
        }
    }

    XArrayFreeJagged( &arrayJagged );
    XArrayFree( &subArray );

    XVariantClear( &xvarArray1 );
    XVariantClear( &xvarArray2 );
    XVariantClear( &xvarArray3 );
    XVariantClear( &xvarArray4 );

    printf( "\n" );
}

// Testing conversion of array in a variant to string
void Test3( )
{
    XErrorCode    ecode       = SuccessCode;
    xarray*       array1d     = nullptr;
    xarray2d*     array2d     = nullptr;
    xarrayJagged* arrayJagged = nullptr;
    xvariant      xvarElement;
    xvariant      xvarArray1d;
    xvariant      xvarArray2d;
    xvariant      xvarArrayJagged;

    printf( "=== Test 3 (arrays to string and back) === \n\n" );

    XVariantInit( &xvarElement );
    XVariantInit( &xvarArray1d );
    XVariantInit( &xvarArray2d );
    XVariantInit( &xvarArrayJagged );

    xvarElement.type = XVT_I4;

    // 1D array test
    ecode = XArrayAllocate( &array1d, XVT_I4, 4 );

    if ( ecode != SuccessCode )
    {
        printf( "ERROR: Failed allocating 1D array: %d \n", ecode );
    }
    else
    {
        xvarArray1d.type = XVT_Array | XVT_I4;
        xvarArray1d.value.arrayVal = array1d;

        xvarElement.value.iVal = 13;
        ecode = XArraySetAll( array1d, &xvarElement );
        if ( ecode != SuccessCode )
        {
            printf( "ERROR: Failed setting all elements of 1D array: %d \n", ecode );
        }
        else
        {
            xstring strArray = nullptr;

            ecode = XVariantToString( &xvarArray1d, &strArray );
            if ( ecode != SuccessCode )
            {
                printf( "ERROR: Failed converting 1D array to string \n" );
            }
            else
            {
                xvariant arrayVar;

                printf( "1D array as string: %s \n", strArray );

                if ( !StringToArray( strArray, &arrayVar, XVT_Array | XVT_I4 ) )
                {
                    printf( "ERROR: Failed getting 1D array from string \n" );
                }
                else
                {
                    printf( "Got 1D array from string \n" );

                    printf( "Variant type: %04X \n", arrayVar.type );
                    printf( "Elements type: %04X \n", XArrayType( arrayVar.value.arrayVal ) );
                    printf( "Array length: %u \n", XArrayLength( arrayVar.value.arrayVal ) );

                    PrintIntegeArrayFromVariant( &arrayVar );

                    XVariantClear( &arrayVar );
                }

                XStringFree( &strArray );
            }
        }

        // array now lives in xvariant
        array1d = nullptr;
        printf( "\n" );
    }

    // 2D array test
    ecode = XArrayAllocate2d( &array2d, XVT_I4, 2, 3 );

    if ( ecode != SuccessCode )
    {
        printf( "ERROR: Failed allocating 2D array: %d \n", ecode );
    }
    else
    {
        xvarArray2d.type = XVT_Array2d | XVT_I4;
        xvarArray2d.value.array2Val = array2d;

        xvarElement.value.iVal = 42;
        ecode = XArraySetAll2d( array2d, &xvarElement );
        if ( ecode != SuccessCode )
        {
            printf( "ERROR: Failed setting all elements of 2D array: %d \n", ecode );
        }
        else
        {
            xstring strArray = nullptr;

            ecode = XVariantToString( &xvarArray2d, &strArray );
            if ( ecode != SuccessCode )
            {
                printf( "ERROR: Failed converting 2D array to string \n" );
            }
            else
            {
                xvariant arrayVar;

                printf( "2D array as string: %s \n", strArray );

                if ( !StringToArray( strArray, &arrayVar, XVT_Array2d | XVT_I4 ) )
                {
                    printf( "ERROR: Failed getting 2D array from string \n" );
                }
                else
                {
                    printf( "Got 2D array from string \n" );

                    printf( "Variant type: %04X \n", arrayVar.type );
                    printf( "Elements type: %04X \n", XArrayType( arrayVar.value.arrayVal ) );
                    printf( "Array length: %u \n", XArrayLength( arrayVar.value.arrayVal ) );

                    PrintIntegeArrayFromVariant( &arrayVar );

                    XVariantClear( &arrayVar );
                }

                XStringFree( &strArray );
            }
        }

        // array now lives in xvariant
        array2d = nullptr;
        printf( "\n" );
    }

    // jagged array test
    ecode = XArrayAllocateJagged( &arrayJagged, XVT_I4, 3 );

    if ( ecode != SuccessCode )
    {
        printf( "ERROR: Failed allocating jagged array: %d \n", ecode );
    }
    else
    {
        xvarArrayJagged.type = XVT_ArrayJagged | XVT_I4;
        xvarArrayJagged.value.arrayJaggedVal = arrayJagged;

        // sub array 1
        SetIntegerSubArray( arrayJagged, 0, 2, 1 );
        SetIntegerSubArray( arrayJagged, 1, 3, 3 );
        SetIntegerSubArray( arrayJagged, 2, 4, 7 );

        xvarElement.value.iVal = 2;
        ecode = XArraySetJagged( arrayJagged, 0, 1, &xvarElement );
        if ( ecode != SuccessCode )
        {
            printf( "ERROR: Failed setting element of jagged array: %d \n", ecode );
        }

        xvarElement.value.iVal = 4;
        ecode = XArraySetJagged( arrayJagged, 1, 2, &xvarElement );
        if ( ecode != SuccessCode )
        {
            printf( "ERROR: Failed setting element of jagged array: %d \n", ecode );
        }

        xvarElement.value.iVal = 8;
        ecode = XArraySetJagged( arrayJagged, 2, 3, &xvarElement );
        if ( ecode != SuccessCode )
        {
            printf( "ERROR: Failed setting element of jagged array: %d \n", ecode );
        }

        xstring strArray = nullptr;

        ecode = XVariantToString( &xvarArrayJagged, &strArray );
        if ( ecode != SuccessCode )
        {
            printf( "ERROR: Failed converting jagged array to string \n" );
        }
        else
        {
            xvariant arrayVar;

            printf( "Jagged array as string: %s \n", strArray );

            if ( !StringToArray( strArray, &arrayVar, XVT_ArrayJagged | XVT_I4 ) )
            {
                printf( "ERROR: Failed getting jagged array from string \n" );
            }
            else
            {
                printf( "Got jagged array from string \n" );

                printf( "Variant type: %04X \n", arrayVar.type );
                printf( "Elements type: %04X \n", XArrayType( arrayVar.value.arrayVal ) );
                printf( "Array length: %u \n", XArrayLength( arrayVar.value.arrayVal ) );

                GetIntegerArray( arrayVar.value.arrayVal, 0 );
                GetIntegerArray( arrayVar.value.arrayVal, 1 );
                GetIntegerArray( arrayVar.value.arrayVal, 2 );

                xvariant arrayElement;

                XVariantInit( &arrayElement );

                ecode = XArrayGetJagged( arrayVar.value.arrayJaggedVal, 2, 3, &arrayElement );
                if ( ecode != SuccessCode )
                {
                    printf( "ERROR: Failed getting element of jagged array: %d \n", ecode );
                }
                else
                {
                    printf( "Got element of jagged array \n" );

                    printf( "Element type: %04X \n", arrayElement.type );
                    printf( "Element value: %d \n", arrayElement.value.iVal );
                }

                XVariantClear( &arrayVar );
            }

            XStringFree( &strArray );
        }

        // array now lives in xvariant
        arrayJagged = nullptr;
        printf( "\n" );
    }

    XVariantClear( &xvarElement );
    XVariantClear( &xvarArray1d );
    XVariantClear( &xvarArray2d );
    XVariantClear( &xvarArrayJagged );

    printf( "\n" );
}

// Testing XVariantArrayJagged
void Test4( )
{
    XErrorCode    ecode;

    printf( "=== Test 4 === (XVariantArrayJagged) \n" );

    XVariantArrayJagged array( XVT_U4, 3 );

    if ( array.IsAllocated( ) )
    {
        printf( "Allocated array, length: %u, type: %u \n", array.Length( ), array.Type( ) );
    }
    else
    {
        printf( "ERROR: Failed allocating array \n" );
    }

    if ( ( array.Length( 0 ) == 0 ) && ( array.Length( 1 ) == 0 ) && ( array.Length( 2 ) == 0 ) )
    {
        printf( "Sub arrays are all empty - OK \n" );
    }
    else
    {
        printf( "ERROR: Some sub arrays are not empty \n" );
    }

    if ( ( array.AllocateSubArray( 0, 2 ) == SuccessCode ) &&
         ( array.AllocateSubArray( 1, 3 ) == SuccessCode ) &&
         ( array.AllocateSubArray( 2, 4 ) == SuccessCode ) &&
         ( array.Length( 0 ) == 2 ) &&
         ( array.Length( 1 ) == 3 ) &&
         ( array.Length( 2 ) == 4 ) )
    {
        printf( "Allocated all sub arrays \n" );
    }
    else
    {
        printf( "ERROR: Failed allocating some sub arrays \n" );
    }

    // set some specific elements
    array.Set( 0, 0, XVariant( 1 ) );
    array.Set( 0, 1, XVariant( 2 ) );
    array.Set( 1, 0, XVariant( 3 ) );

    // get elements and check their values
    PrintJaggedArray( array );

    // copy array
    XVariantArrayJagged array2( array );
    XVariantArrayJagged array3( XVT_U1, 1 );

    array3 = array;

    if ( array2 == array )
    {
        printf( "array2 == aray, OK \n" );
    }
    else
    {
        printf( "ERROR: array2 != array \n" );
        PrintJaggedArray( array2 );
    }

    if ( array3 == array )
    {
        printf( "array3 == aray, OK \n" );
    }
    else
    {
        printf( "ERROR: array3 != array \n" );
        PrintJaggedArray( array3 );
    }

    array2.Set( 1, 1, XVariant( 4 ) );
    printf( "Changed original array \n" );

    if ( array2 != array )
    {
        printf( "array2 != aray, OK \n" );
    }
    else
    {
        printf( "ERROR: array3 == array \n" );
    }

    // set entire row of jagged array
    XVariantArray row( XVT_U2, 5 );

    row.SetAll( XVariant( 7 ) );
    row.Set( 2, XVariant( 42 ) );

    ecode = array.Set( 2, row );
    if ( ecode == SuccessCode )
    {
        printf( "Set row of the jagged array \n" );

        PrintJaggedArray( array );
    }
    else
    {
        printf( "ERROR: Failed setting row of jagged array, %u \n", ecode );
    }

    ecode = array.Set( 3, row );
    if ( ecode != SuccessCode )
    {
        printf( "Failed setting non existing row, OK, %u \n", ecode );
    }
    else
    {
        printf( "ERROR: Set non existing row \n" );
    }

    // settting all element of the jagged array
    ecode = array.SetAll( XVariant( 13 ) );
    if ( ecode == SuccessCode )
    {
        printf( "Set all elements of jagged array to uint 13 \n" );

        PrintJaggedArray( array );
    }
    else
    {
        printf( "ERROR: Failed setting all elements of jagged array to uint, %u \n", ecode );
    }

    ecode = array.SetAll( XVariant( "7" ) );
    if ( ecode == SuccessCode )
    {
        printf( "Set all elements of jagged array to string \"7\" \n" );

        PrintJaggedArray( array );
    }
    else
    {
        printf( "ERROR: Failed setting all elements of jagged array to string, %u \n", ecode );
    }

    // set all rows of jagged array
    ecode = array.SetAll( XVariant( row ) );
    if ( ecode == SuccessCode )
    {
        printf( "Set all rows of jagged array (from array in variant) \n" );

        PrintJaggedArray( array );
    }
    else
    {
        printf( "ERROR: Failed setting all rows of jagged array (from array in variant), %u \n", ecode );
    }

    // another way to set all rows of jagged array
    ecode = array2.SetAll( row );
    if ( ecode == SuccessCode )
    {
        printf( "Set all rows of jagged array (from array) \n" );

        PrintJaggedArray( array2 );
    }
    else
    {
        printf( "ERROR: Failed setting all rows of jagged array (from array), %u \n", ecode );
    }

    // prepare array with one un-allocated row
    array3 = XVariantArrayJagged( XVT_U4, 3 );

    PrintJaggedArray( array3 );

    ecode = array3.Set( 0, row );
    if ( ecode == SuccessCode )
    {
        printf( "Set un-allocated row 0 \n" );
    }
    else
    {
        printf( "ERROR: Failed setting un-allocated row 0, %u \n", ecode );
    }

    ecode = array3.Set( 2, row );
    if ( ecode == SuccessCode )
    {
        printf( "Set un-allocated row 2 \n" );
    }
    else
    {
        printf( "ERROR: Failed setting un-allocated row 2, %u \n", ecode );
    }

    PrintJaggedArray( array3 );

    ecode = array3.SetAll( XVariant( 9 ) );
    if ( ecode == SuccessCode )
    {
        printf( "Set all elements, except un-allocated rows \n" );
    }
    else
    {
        printf( "ERROR: Failed setting all elements in array with un-allocated rows, %u \n", ecode );
    }

    PrintJaggedArray( array3 );

    // get row from jagged array
    ecode = array3.Get( 2, row );
    if ( ecode == SuccessCode )
    {
        printf( "Got row from jagged array \n" );
        PrintArray( row );
    }
    else
    {
        printf( "ERROR: Failed getting row from jagged array, %u \n", ecode );
    }

    ecode = array3.Get( 1, row );
    if ( ecode == SuccessCode )
    {
        printf( "Got empty row from jagged array \n" );
        PrintArray( row );
    }
    else
    {
        printf( "ERROR: Failed getting empty row from jagged array, %u \n", ecode );
    }
}

// Testing XVariantArrayJagged
void Test5( )
{
    XErrorCode ecode;

    printf( "=== Test 5 === (XVariantArrayJagged in XVariant) \n" );

    XVariantArrayJagged array( XVT_U4, 3 );
    XVariantArray       row( XVT_U2, 5 );

    row.SetAll( XVariant( 7 ) );
    row.Set( 2, XVariant( 42 ) );

    array.SetAll( row );

    PrintJaggedArray( array );

    if ( ( !array.IsAllocated( ) ) || ( array.Length( ) != 3 ) ||
         ( !array.IsAllocated( 0 ) ) || ( array.Length( 0 ) != 5 ) ||
         ( !array.IsAllocated( 1 ) ) || ( array.Length( 1 ) != 5 ) ||
         ( !array.IsAllocated( 2 ) ) || ( array.Length( 2 ) != 5 ) )
    {
        printf( "ERROR: Failed initializing array \n" );
    }

    XVariant varArray( array );

    printf( "Variant type: %04X, is jagged: %u \n", varArray.Type( ), varArray.IsArrayJagged( ) );

    printf( "As string: \n%s\n", varArray.ToString( ).c_str( ) );

    XVariantArrayJagged array2 = varArray.ToArrayJagged( );

    PrintJaggedArray( array2 );

    XVariant varString( varArray.ToString( ) );

    printf( "Variant type: %04X \n", varString.Type( ) );

    XVariantArrayJagged array3 = varString.ToArrayJagged( &ecode );
    if ( ecode != SuccessCode )
    {
        printf( "OK, can not get jagged from string variant, %u \n", ecode );
    }

    PrintJaggedArray( array3 );

    XVariant varArray2 = varString.ChangeType( XVT_ArrayJagged | XVT_U4, &ecode );
    if ( ecode == SuccessCode )
    {
        printf( "Got jagged array from string \n" );
    }
    else
    {
        printf( "ERROR: Failed getting jagged array from string, %u \n", ecode );
    }

    array2 = varArray2.ToArrayJagged( );

    PrintJaggedArray( array2 );
}
