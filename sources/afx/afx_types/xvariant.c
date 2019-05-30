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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "xtypes.h"
#include "ximage.h"

// some string constants
static const char* STR_NULL  = "null";
static const char* STR_EMPTY = "empty";
static const char* STR_TRUE  = "true";
static const char* STR_FALSE = "false";
// some format specifiers used with sprintf()
static const char* STR_INT_FORMAT        = "%d";
static const char* STR_UINT_FORMAT       = "%u";
static const char* STR_FLOAT_FORMAT_OUT  = "%.17G";
static const char* STR_DOUBLE_FORMAT_OUT = "%.17lG";
static const char* STR_ARGB_FORMAT32     = "%08X";
static const char* STR_ARGB_FORMAT24     = "%06X";
static const char* STR_RANGE_FORMAT      = "[%d, %d]";
static const char* STR_RANGEF_FORMAT_OUT = "[%.17G, %.17G]";
static const char* STR_RANGEF_FORMAT_IN  = "[%G, %G]";
static const char* STR_POINT_FORMAT      = "[%d, %d]";
static const char* STR_POINTF_FORMAT_OUT = "[%.17G, %.17G]";
static const char* STR_POINTF_FORMAT_IN  = "[%G, %G]";
static const char* STR_SIZE_FORMAT       = "[%d, %d]";

#define LONG_STRING_LENGTH (10000)

// Helpers to convert arrays to/from strings
static XErrorCode XStringToArray( const xstring str, XVarType dstType, xarray** parray );
static XErrorCode XArrayToString( const xarray* array, XVarType srcType, xstring* pstr );

// check if given double value is Infinity
static bool xisinf( double x )
{
   volatile double temp = x;
   return ( ( temp == x ) && ( ( temp - x ) != 0.0 ) ) ? true : false;
}

// Initialiaze variant variable setting it to empty
void XVariantInit( xvariant* var )
{
    if ( var != 0 )
    {
        var->type = XVT_Empty;
        var->value.ptrVal = 0;
    }
}

// Clear variant variable to empty and free any taken memory
void XVariantClear( xvariant* var )
{
    if ( var != 0 )
    {
        if ( ( var->type & XVT_Array ) == XVT_Array )
        {
            XArrayFree( &( var->value.arrayVal ) );
        }
        else if ( ( var->type & XVT_Array2d ) == XVT_Array2d )
        {
            XArrayFree2d( &( var->value.array2Val ) );
        }
        else if ( ( var->type & XVT_ArrayJagged ) == XVT_ArrayJagged )
        {
            XArrayFreeJagged( &( var->value.arrayJaggedVal ) );
        }
        else if ( var->type == XVT_String )
        {
            XStringFree( &var->value.strVal );
        }
        else if ( var->type == XVT_Image )
        {
            XImageFree( &var->value.imageVal );
        }
        var->type = XVT_Empty;
        var->value.ptrVal = 0;
    }
}

// Make a copy of the variant variable
XErrorCode XVariantCopy( const xvariant* src, xvariant* dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        xvariant temp;

        XVariantInit( &temp );

        temp.type = src->type;

        if ( ( src->type & XVT_Array ) == XVT_Array )
        {
            XArrayCopy( src->value.arrayVal, &( temp.value.arrayVal ) );
        }
        else if ( ( src->type & XVT_Array2d ) == XVT_Array2d )
        {
            XArrayCopy2d( src->value.array2Val, &( temp.value.array2Val ) );
        }
        else if ( ( src->type & XVT_ArrayJagged ) == XVT_ArrayJagged )
        {
            XArrayCopyJagged( src->value.arrayJaggedVal, &( temp.value.arrayJaggedVal ) );
        }
        else if ( src->type == XVT_String )
        {
            // allocate copy of the string
            temp.value.strVal = XStringAlloc( src->value.strVal );

            if ( temp.value.strVal == 0 )
            {
                ret = ErrorOutOfMemory;
            }
        }
        else if ( src->type == XVT_Image )
        {
            ximage* srcImage = src->value.imageVal;

            ret = XImageCreate( srcImage->data, srcImage->width, srcImage->height, srcImage->stride, srcImage->format, &temp.value.imageVal );
        }
        else
        {
            // copy the rest as is
            temp.value = src->value;
        }

        if ( ret == SuccessCode )
        {
            XVariantClear( dst );

            dst->type  = temp.type;
            dst->value = temp.value;
        }
        else
        {
            XVariantClear( &temp );
        }
    }

    return ret;
}

// Check if two variants have same values (including type)
XErrorCode XVariantAreEqual( const xvariant* var1, const xvariant* var2 )
{
    XErrorCode ret = ErrorFailed;

    if ( ( var1 == 0 ) || ( var2 == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( var1->type != var2->type )
    {
        ret = ErrorIncompatibleTypes;
    }
    else
    {
        if (
            (   var1->type == XVT_Null )   ||
            (   var1->type == XVT_Empty )  ||
            ( ( var1->type == XVT_Bool )   && ( var1->value.boolVal == var2->value.boolVal ) ) ||
            ( ( var1->type == XVT_I1 )     && ( var1->value.bVal == var2->value.bVal ) ) ||
            ( ( var1->type == XVT_I2 )     && ( var1->value.sVal == var2->value.sVal ) ) ||
            ( ( var1->type == XVT_I4 )     && ( var1->value.iVal == var2->value.iVal ) ) ||
            ( ( var1->type == XVT_U1 )     && ( var1->value.ubVal == var2->value.ubVal ) ) ||
            ( ( var1->type == XVT_U2 )     && ( var1->value.usVal == var2->value.usVal ) ) ||
            ( ( var1->type == XVT_U4 )     && ( var1->value.uiVal == var2->value.uiVal ) ) ||
            ( ( var1->type == XVT_R4 )     && ( var1->value.fVal == var2->value.fVal ) ) ||
            ( ( var1->type == XVT_R8 )     && ( var1->value.dVal == var2->value.dVal ) ) ||
            ( ( var1->type == XVT_String ) && ( strcmp( var1->value.strVal, var2->value.strVal ) == 0 ) ) ||
            ( ( var1->type == XVT_ARGB )   && ( var1->value.argbVal.argb == var2->value.argbVal.argb ) ) ||
            ( ( var1->type == XVT_Range )  && ( var1->value.rangeVal.min == var2->value.rangeVal.min ) &&
                                              ( var1->value.rangeVal.max == var2->value.rangeVal.max ) ) ||
            ( ( var1->type == XVT_RangeF ) && ( var1->value.frangeVal.min == var2->value.frangeVal.min ) &&
                                              ( var1->value.frangeVal.max == var2->value.frangeVal.max ) ) ||
            ( ( var1->type == XVT_Point )  && ( var1->value.pointVal.x == var2->value.pointVal.x ) &&
                                              ( var1->value.pointVal.y == var2->value.pointVal.y ) ) ||
            ( ( var1->type == XVT_PointF ) && ( var1->value.fpointVal.x == var2->value.fpointVal.x ) &&
                                              ( var1->value.fpointVal.y == var2->value.fpointVal.y ) ) ||
            ( ( var1->type == XVT_Size )   && ( var1->value.sizeVal.width  == var2->value.sizeVal.width ) &&
                                              ( var1->value.sizeVal.height == var2->value.sizeVal.height ) ) ||
            ( ( var1->type == XVT_Image )  && ( var1->value.imageVal != 0 ) && ( var2->value.imageVal != 0 ) &&
                                              ( var1->value.imageVal->data != var2->value.imageVal->data ) )
           )
        {
            ret = SuccessCode;
        }
        else if ( ( ( var1->type & XVT_Array ) != 0 ) && ( ( var2->type & XVT_Array ) != 0 ) )
        {
            ret = XArrayAreEqual( var1->value.arrayVal, var2->value.arrayVal );
        }
        else if ( ( ( var1->type & XVT_Array2d ) != 0 ) && ( ( var2->type & XVT_Array2d ) != 0 ) )
        {
            ret = XArrayAreEqual2d( var1->value.array2Val, var2->value.array2Val );
        }
        else if ( ( ( var1->type & XVT_ArrayJagged ) != 0 ) && ( ( var2->type & XVT_ArrayJagged ) != 0 ) )
        {
            ret = XArrayAreEqualJagged( var1->value.arrayJaggedVal, var2->value.arrayJaggedVal );
        }
    }

    return ret;
}

// Converts variant type to a string value
XErrorCode XVariantToString( const xvariant* src, xstring* dst )
{
    XErrorCode ret = SuccessCode;
    xstring str = 0;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        if ( IS_ARRAY_TYPE( src->type ) )
        {
            ret = XArrayToString( src->value.arrayVal, src->type, &str );
        }
        else if ( src->type == XVT_Empty )
        {
            str = XStringAlloc( STR_EMPTY );
        }
        else if ( src->type == XVT_Null )
        {
            str = XStringAlloc( STR_NULL );
        }
        else if ( src->type == XVT_Bool )
        {
            str = XStringAlloc( ( src->value.boolVal == false ) ? STR_FALSE : STR_TRUE );
        }
        else if ( src->type == XVT_String )
        {
            str = XStringAlloc( src->value.strVal );
        }
        else
        {
            char buffer[128];

            switch ( src->type )
            {
                case XVT_I1:
                    sprintf( buffer, STR_INT_FORMAT, src->value.bVal );
                    break;
                case XVT_I2:
                    sprintf( buffer, STR_INT_FORMAT, src->value.sVal );
                    break;
                case XVT_I4:
                    sprintf( buffer, STR_INT_FORMAT, src->value.iVal );
                    break;
                case XVT_U1:
                    sprintf( buffer, STR_UINT_FORMAT, src->value.ubVal );
                    break;
                case XVT_U2:
                    sprintf( buffer, STR_UINT_FORMAT, src->value.usVal );
                    break;
                case XVT_U4:
                    sprintf( buffer, STR_UINT_FORMAT, src->value.uiVal );
                    break;
                case XVT_R4:
                    sprintf( buffer, STR_FLOAT_FORMAT_OUT, src->value.fVal );
                    break;
                case XVT_R8:
                    sprintf( buffer, STR_DOUBLE_FORMAT_OUT, src->value.dVal );
                    break;
                case XVT_ARGB:
                    if ( src->value.argbVal.components.a != 0xFF )
                    {
                        sprintf( buffer, STR_ARGB_FORMAT32, src->value.argbVal.argb );
                    }
                    else
                    {
                        xargb temp = src->value.argbVal;

                        temp.components.a = 0;
                        sprintf( buffer, STR_ARGB_FORMAT24, temp );
                    }
                    break;
                case XVT_Range:
                    sprintf( buffer, STR_RANGE_FORMAT, src->value.rangeVal.min, src->value.rangeVal.max );
                    break;
                case XVT_RangeF:
                    sprintf( buffer, STR_RANGEF_FORMAT_OUT, src->value.frangeVal.min, src->value.frangeVal.max );
                    break;
                case XVT_Point:
                    sprintf( buffer, STR_POINT_FORMAT, src->value.pointVal.x, src->value.pointVal.y );
                    break;
                case XVT_PointF:
                    sprintf( buffer, STR_POINTF_FORMAT_OUT, src->value.fpointVal.x, src->value.fpointVal.y );
                    break;
                case XVT_Size:
                    sprintf( buffer, STR_SIZE_FORMAT, src->value.sizeVal.width, src->value.sizeVal.height );
                    break;

                case XVT_Image:
                    {
                        xstring strFormat = XImageGetPixelFormatShortName( src->value.imageVal->format );

                        sprintf( buffer, "image: %dx%d@%s [%p]", src->value.imageVal->width,
                                                                 src->value.imageVal->height,
                                                                 strFormat,
                                                                 src->value.imageVal->data );

                        XStringFree( &strFormat );
                    }
                    break;
                default:
                    return ErrorIncompatibleTypes;
            }

            str = XStringAlloc( buffer );
        }

        if ( ret == SuccessCode )
        {
            if ( str == 0 )
            {
                ret = ErrorOutOfMemory;
            }
            else
            {
                *dst = str;
            }
        }
    }

    return ret;
}

// Converts variant type to a double value
XErrorCode XVariantToDouble( const xvariant* src, double* dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        switch ( src->type )
        {
            case XVT_Bool:
                *dst = ( src->value.boolVal == false ) ? 0 : 1;
                break;
            case XVT_I1:
                *dst = src->value.bVal;
                break;
            case XVT_I2:
                *dst = src->value.sVal;
                break;
            case XVT_I4:
                *dst = src->value.iVal;
                break;
            case XVT_U1:
                *dst = src->value.ubVal;
                break;
            case XVT_U2:
                *dst = src->value.usVal;
                break;
            case XVT_U4:
                *dst = src->value.uiVal;
                break;
            case XVT_R4:
                *dst = src->value.fVal;
                break;
            case XVT_R8:
                *dst = src->value.dVal;
                break;
            case XVT_String:
                {
                    char* endPtr;
                    double temp;

                    temp = strtod( src->value.strVal, &endPtr );

                    if ( xisinf( temp ) == true )
                    {
                        ret = ErrorLimitsExceeded;
                    }
                    else if ( *endPtr != 0 )
                    {
                        ret = ErrorInvalidFormat;
                    }
                    else
                    {
                        *dst = temp;
                    }
                }
                break;
            default:
                ret = ErrorIncompatibleTypes;
                break;
        }
    }

    return ret;
}

// Converts variant type to a float value
XErrorCode XVariantToFloat( const xvariant* src, float* dst )
{
    XErrorCode ret = SuccessCode;

    if ( dst == 0 )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        double dVal = 0;
        // first try converting to larger type - double
        ret = XVariantToDouble( src, &dVal );

        if ( ret == SuccessCode )
        {
            // check upper limits only
            if ( ( dVal > FLT_MAX ) || ( dVal < -FLT_MAX ) )
            {
                ret = ErrorLimitsExceeded;
            }
            else
            {
                *dst = (float) dVal;
            }
        }
    }

    return ret;
}

// Converts variant type to a 32 bit uint value
XErrorCode XVariantToUInt( const xvariant* src, uint32_t* dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        switch ( src->type )
        {
            case XVT_Bool:
                *dst = ( src->value.boolVal != false ) ? 1 : 0;
                break;
            case XVT_I1:
                if ( src->value.bVal >= 0 )
                {
                    *dst = src->value.bVal;
                }
                else
                {
                    ret = ErrorLimitsExceeded;
                }
                break;
            case XVT_I2:
                if ( src->value.sVal >= 0 )
                {
                    *dst = src->value.sVal;
                }
                else
                {
                    ret = ErrorLimitsExceeded;
                }
                break;
            case XVT_I4:
                if ( src->value.iVal >= 0 )
                {
                    *dst = src->value.iVal;
                }
                else
                {
                    ret = ErrorLimitsExceeded;
                }
                break;
            case XVT_U1:
                *dst = src->value.ubVal;
                break;
            case XVT_U2:
                *dst = src->value.usVal;
                break;
            case XVT_U4:
                *dst = src->value.uiVal;
                break;
            case XVT_R4:
                if ( ( src->value.fVal > 0 ) && ( src->value.fVal <= UINT32_MAX ) )
                {
                    *dst = (uint32_t) src->value.fVal;
                }
                else
                {
                    ret = ErrorLimitsExceeded;
                }
                break;
            case XVT_R8:
                if ( ( src->value.dVal > 0 ) && ( src->value.dVal <= UINT32_MAX ) )
                {
                    *dst = (uint32_t) src->value.dVal;
                }
                else
                {
                    ret = ErrorLimitsExceeded;
                }
                break;
            case XVT_String:
                {
                    uint32_t temp;

                    // TODO: improve checking of UINT, since sscanf does not guarantee too much
                    if ( sscanf( src->value.strVal, STR_UINT_FORMAT, &temp ) == 1 )
                    {
                        *dst = temp;
                    }
                    else
                    {
                        ret = ErrorInvalidFormat;
                    }
                }
                break;
            default:
                ret = ErrorIncompatibleTypes;
                break;
        }
    }

    return ret;
}

// Converts variant type to a 16 bit uint value
XErrorCode XVariantToUShort( const xvariant* src, uint16_t* dst )
{
    XErrorCode ret = SuccessCode;

    if ( dst == 0 )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        uint32_t val32 = 0;
        // first try converting to larger type
        ret = XVariantToUInt( src, &val32 );

        if ( ret == SuccessCode )
        {
            // check upper limits only
            if ( val32 > UINT16_MAX )
            {
                ret = ErrorLimitsExceeded;
            }
            else
            {
                *dst = (uint16_t) val32;
            }
        }
    }

    return ret;
}

// Converts variant type to a 8 bit uint value
XErrorCode XVariantToUByte( const xvariant* src, uint8_t* dst )
{
    XErrorCode ret = SuccessCode;

    if ( dst == 0 )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        uint32_t val32 = 0;
        // first try converting to larger type
        ret = XVariantToUInt( src, &val32 );

        if ( ret == SuccessCode )
        {
            // check upper limits only
            if ( val32 > UINT8_MAX )
            {
                ret = ErrorLimitsExceeded;
            }
            else
            {
                *dst = (uint8_t) val32;
            }
        }
    }

    return ret;
}

// Converts variant type to a 32 bit int value
XErrorCode XVariantToInt( const xvariant* src, int32_t* dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        switch ( src->type )
        {
            case XVT_Bool:
                *dst = ( src->value.boolVal != false ) ? 1 : 0;
                break;
            case XVT_I1:
                *dst = src->value.bVal;
                break;
            case XVT_I2:
                *dst = src->value.sVal;
                break;
            case XVT_I4:
                *dst = src->value.iVal;
                break;
            case XVT_U1:
                *dst = (int32_t) src->value.ubVal;
                break;
            case XVT_U2:
                *dst = (int32_t) src->value.usVal;
                break;
            case XVT_U4:
                if ( src->value.uiVal <= INT32_MAX )
                {
                    *dst = (int32_t) src->value.uiVal;
                }
                else
                {
                    ret = ErrorLimitsExceeded;
                }
                break;
            case XVT_R4:
                if ( ( src->value.fVal >= INT32_MIN ) && ( src->value.fVal <= INT32_MAX ) )
                {
                    *dst = (int32_t) src->value.fVal;
                }
                else
                {
                    ret = ErrorLimitsExceeded;
                }
                break;
            case XVT_R8:
                if ( ( src->value.dVal >= INT32_MIN ) && ( src->value.dVal <= INT32_MAX ) )
                {
                    *dst = (int32_t) src->value.dVal;
                }
                else
                {
                    ret = ErrorLimitsExceeded;
                }
                break;
            case XVT_String:
                {
                    int32_t temp;

                    // TODO: improve checking of INT, since sscanf does not guarantee too much
                    if ( sscanf( src->value.strVal, STR_INT_FORMAT, &temp ) == 1 )
                    {
                        *dst = temp;
                    }
                    else
                    {
                        ret = ErrorInvalidFormat;
                    }
                }
                break;
            default:
                ret = ErrorIncompatibleTypes;
                break;
        }
    }

    return ret;
}

// Converts variant type to a 16 bit int value
XErrorCode XVariantToShort( const xvariant* src, int16_t* dst )
{
    XErrorCode ret = SuccessCode;

    if ( dst == 0 )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        int32_t val32 = 0;
        // first try converting to larger type
        ret = XVariantToInt( src, &val32 );

        if ( ret == SuccessCode )
        {
            // check limits
            if ( ( val32 > INT16_MAX ) || ( val32 < INT16_MIN ) )
            {
                ret = ErrorLimitsExceeded;
            }
            else
            {
                *dst = (int16_t) val32;
            }
        }
    }

    return ret;
}

// Converts variant type to a 8 bit int value
XErrorCode XVariantToByte( const xvariant* src, int8_t* dst )
{
    XErrorCode ret = SuccessCode;

    if ( dst == 0 )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        int32_t val32 = 0;
        // first try converting to larger type
        ret = XVariantToInt( src, &val32 );

        if ( ret == SuccessCode )
        {
            // check limits
            if ( ( val32 > INT8_MAX ) || ( val32 < INT8_MIN ) )
            {
                ret = ErrorLimitsExceeded;
            }
            else
            {
                *dst = (int8_t) val32;
            }
        }
    }

    return ret;
}

// Converts variant type to a bool value
XErrorCode XVariantToBool( const xvariant* src, bool* dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        switch ( src->type )
        {
            case XVT_Bool:
                *dst = src->value.boolVal;
                break;
            case XVT_String:
                if ( strcmp( src->value.strVal, STR_TRUE ) == 0 )
                {
                    *dst = true;
                }
                else if ( strcmp( src->value.strVal, STR_FALSE ) == 0 )
                {
                    *dst = false;
                }
                else
                {
                    ret = ErrorInvalidFormat;
                }
                break;

            case XVT_I1:
            case XVT_I2:
            case XVT_I4:
            case XVT_U1:
            case XVT_U2:
            case XVT_U4:
            case XVT_R4:
            case XVT_R8:
                {
                    double dVal;

                    ret = XVariantToDouble( src, &dVal );

                    if ( ret == SuccessCode )
                    {
                        if ( dVal == 1.0 )
                        {
                            *dst = true;
                        }
                        else if ( dVal == 0.0 )
                        {
                            *dst = false;
                        }
                        else
                        {
                            ret = ErrorLimitsExceeded;
                        }
                    }
                }
                break;
            default:
                ret = ErrorIncompatibleTypes;
                break;
        }
    }

    return ret;
}

// Converts variant type to a bool value
XErrorCode XVariantToArgb( const xvariant* src, xargb* dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        switch ( src->type )
        {
            case XVT_ARGB:
                *dst = src->value.argbVal;
                break;

            case XVT_String:
                {
                    size_t len = strlen( src->value.strVal );

                    if ( len == 8 )
                    {
                        if ( sscanf( src->value.strVal, STR_ARGB_FORMAT32, &dst->argb ) != 1 )
                            ret = ErrorInvalidFormat;
                    }
                    else if ( len == 6 )
                    {
                        // in the case if only 6 hex numbers present, suppose it is RGB value with alpha set to FF
                        if ( sscanf( src->value.strVal, STR_ARGB_FORMAT24, &dst->argb ) != 1 )
                        {
                            ret = ErrorInvalidFormat;
                        }
                        else
                        {
                            dst->components.a = 0xFF;
                        }
                    }
                    else
                    {
                        ret = ErrorInvalidFormat;
                    }
                }
                break;

            case XVT_U4:
                dst->argb = src->value.uiVal;
                break;

            default:
                ret = ErrorIncompatibleTypes;
                break;
        }
    }

    return ret;
}

// Converts variant type to a range value
XErrorCode XVariantToRange( const xvariant* src, xrange* dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        switch ( src->type )
        {
            case XVT_Range:
                *dst = src->value.rangeVal;
                break;

            case XVT_RangeF:
                dst->min = (int32_t) src->value.frangeVal.min;
                dst->max = (int32_t) src->value.frangeVal.max;
                break;

            case XVT_String:
                if ( sscanf( src->value.strVal, STR_RANGE_FORMAT, &dst->min, &dst->max ) != 2 )
                {
                    ret = ErrorInvalidFormat;
                }
                break;

            default:
                ret = ErrorIncompatibleTypes;
                break;
        }
    }

    return ret;
}

// Converts variant type to a floating range value
XErrorCode XVariantToRangeF( const xvariant* src, xrangef* dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        switch ( src->type )
        {
        case XVT_RangeF:
            *dst = src->value.frangeVal;
            break;

        case XVT_Range:
            dst->min = (float) src->value.rangeVal.min;
            dst->max = (float) src->value.rangeVal.max;
            break;

        case XVT_String:
            if ( sscanf( src->value.strVal, STR_RANGEF_FORMAT_IN, &dst->min, &dst->max ) != 2 )
            {
                ret = ErrorInvalidFormat;
            }
            break;

        default:
            ret = ErrorIncompatibleTypes;
            break;
        }
    }

    return ret;
}

// Converts variant type to a point value
XErrorCode XVariantToPoint( const xvariant* src, xpoint* dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        switch ( src->type )
        {
        case XVT_Point:
            *dst = src->value.pointVal;
            break;

        case XVT_PointF:
            dst->x = (int32_t) src->value.fpointVal.x;
            dst->y = (int32_t) src->value.fpointVal.y;
            break;

        case XVT_String:
            if ( sscanf( src->value.strVal, STR_POINT_FORMAT, &dst->x, &dst->y ) != 2 )
            {
                ret = ErrorInvalidFormat;
            }
            break;

        default:
            ret = ErrorIncompatibleTypes;
            break;
        }
    }

    return ret;
}

// Converts variant type to a floating point value
XErrorCode XVariantToPointF( const xvariant* src, xpointf* dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        switch ( src->type )
        {
        case XVT_PointF:
            *dst = src->value.fpointVal;
            break;

        case XVT_Point:
            dst->x = (float) src->value.pointVal.x;
            dst->y = (float) src->value.pointVal.y;
            break;

        case XVT_String:
            if ( sscanf( src->value.strVal, STR_POINTF_FORMAT_IN, &dst->x, &dst->y ) != 2 )
            {
                ret = ErrorInvalidFormat;
            }
            break;

        default:
            ret = ErrorIncompatibleTypes;
            break;
        }
    }

    return ret;
}

// Converts variant type to a size value
XErrorCode XVariantToSize( const xvariant* src, xsize* dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        switch ( src->type )
        {
        case XVT_Size:
            *dst = src->value.sizeVal;
            break;

        case XVT_String:
            if ( sscanf( src->value.strVal, STR_SIZE_FORMAT, &dst->width, &dst->height ) != 2 )
            {
                ret = ErrorInvalidFormat;
            }
            break;

        default:
            ret = ErrorIncompatibleTypes;
            break;
        }
    }

    return ret;
}

// Change type of the variant variable to the specified one
XErrorCode XVariantChangeType( const xvariant* src, xvariant* dst, XVarType dstType )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        xvariant temp;

        // do type conversion into temporary to make sure destination element is not affected on failure
        XVariantInit( &temp );

        if ( src->type == dstType )
        {
            ret = XVariantCopy( src, &temp );
        }
        else if ( IS_ARRAY_TYPE( src->type ) )
        {
            // 1D array to 1D array
            if ( ( ( src->type & XVT_Array ) == XVT_Array ) && ( ( dstType & XVT_Array ) == XVT_Array ) )
            {
                if ( ( dstType & XVT_Any ) == XVT_Any )
                {
                    // create a copy of the source array without changing its type
                    ret = XArrayCopy( src->value.arrayVal, &temp.value.arrayVal );
                }
                else
                {
                    // create copy of the array with elements changed to the target type
                    ret = XArrayChangeType( src->value.arrayVal, &temp.value.arrayVal, dstType & XVT_Any );
                }
            }
            // 2D array to 2D array
            else if ( ( ( src->type & XVT_Array2d ) == XVT_Array2d ) && ( ( dstType & XVT_Array2d ) == XVT_Array2d ) )
            {
                if ( ( dstType & XVT_Any ) == XVT_Any )
                {
                    // create a copy of the 2D source array without changing its type
                    ret = XArrayCopy2d( src->value.array2Val, &temp.value.array2Val );
                }
                else
                {
                    // create copy of the 2D array with elements changed to the target type
                    ret = XArrayChangeType2d( src->value.array2Val, &temp.value.array2Val, dstType & XVT_Any );
                }
            }
            // jagged array to jagged
            else if ( ( ( src->type & XVT_ArrayJagged ) == XVT_ArrayJagged ) && ( ( dstType & XVT_ArrayJagged ) == XVT_ArrayJagged ) )
            {
                if ( ( dstType & XVT_Any ) == XVT_Any )
                {
                    // create a copy of the source jagged array
                    ret = XArrayCopyJagged( src->value.arrayJaggedVal, &temp.value.arrayJaggedVal );
                }
                else
                {
                    // create copy of the jagged array with elements changed to the target type
                    ret = XArrayChangeTypeJagged( src->value.arrayJaggedVal, &temp.value.arrayJaggedVal, dstType & XVT_Any );
                }
            }
            // 2D array to 1D array
            else if ( ( ( src->type & XVT_Array2d ) == XVT_Array2d ) && ( ( dstType & XVT_Array ) == XVT_Array ) )
            {
                if ( ( dstType & XVT_Any ) == XVT_Any )
                {
                    // create a copy of the 2D source array without changing its type
                    ret = XArrayCopy( (const xarray*) src->value.array2Val, &temp.value.arrayVal );
                }
                else
                {
                    // create copy of the 2D array with elements changed to the target type
                    ret = XArrayChangeType( (const xarray*) src->value.array2Val, &temp.value.arrayVal, dstType & XVT_Any );
                }
            }
            // jagged array to 1D array (only copying, no elements' type conversion)
            else if ( ( ( src->type & XVT_ArrayJagged ) == XVT_ArrayJagged ) &&
                      ( ( dstType & XVT_Array ) == XVT_Array ) &&
                      ( ( src->type & XVT_Any ) == ( dstType & XVT_Any ) ) )
            {
                ret = XArrayCopyJaggedTo1D( src->value.arrayJaggedVal, &temp.value.arrayVal );
            }
            // 1D/2D/jagged array to string
            else if ( dstType == XVT_String )
            {
                ret = XVariantToString( src, &temp.value.strVal );
                if ( ret == SuccessCode )
                {
                    temp.type = XVT_String;
                }
            }
            // can not convert array to none array type or 1d array to 2d
            else
            {
                ret = ErrorIncompatibleTypes;
            }

            if ( ret == SuccessCode )
            {
                temp.type = dstType;
            }
        }
        else if ( ( src->type == XVT_String ) && ( IS_ARRAY_TYPE( dstType ) ) )
        {
            // string to array conversion
            ret = XStringToArray( src->value.strVal, dstType, &temp.value.arrayVal );
            if ( ret == SuccessCode )
            {
                temp.type = dstType;
            }
        }
        else
        {
            switch ( dstType )
            {
                case XVT_String:
                    ret = XVariantToString( src, &temp.value.strVal );
                    break;
                case XVT_R8:
                    ret = XVariantToDouble( src, &temp.value.dVal );
                    break;
                case XVT_R4:
                    ret = XVariantToFloat( src, &temp.value.fVal );
                    break;

                case XVT_U4:
                    ret = XVariantToUInt( src, &temp.value.uiVal );
                    break;
                case XVT_U2:
                    ret = XVariantToUShort( src, &temp.value.usVal );
                    break;
                case XVT_U1:
                    ret = XVariantToUByte( src, &temp.value.ubVal );
                    break;

                case XVT_I4:
                    ret = XVariantToInt( src, &temp.value.iVal );
                    break;
                case XVT_I2:
                    ret = XVariantToShort( src, &temp.value.sVal );
                    break;
                case XVT_I1:
                    ret = XVariantToByte( src, &temp.value.bVal );
                    break;

                case XVT_Bool:
                    ret = XVariantToBool( src, &temp.value.boolVal );
                    break;

                case XVT_ARGB:
                    ret = XVariantToArgb( src, &temp.value.argbVal );
                    break;

                case XVT_Range:
                    ret = XVariantToRange( src, &temp.value.rangeVal );
                    break;
                case XVT_RangeF:
                    ret = XVariantToRangeF( src, &temp.value.frangeVal );
                    break;

                case XVT_Point:
                    ret = XVariantToPoint( src, &temp.value.pointVal );
                    break;
                case XVT_PointF:
                    ret = XVariantToPointF( src, &temp.value.fpointVal );
                    break;

                case XVT_Size:
                    ret = XVariantToSize( src, &temp.value.sizeVal );
                    break;

                default:
                    ret = ErrorIncompatibleTypes;
                    break;
            }

            if ( ret == SuccessCode )
            {
                temp.type = dstType;
            }
        }

        if ( ret == SuccessCode )
        {
            XVariantClear( dst );

            dst->type  = temp.type;
            dst->value = temp.value;
        }
        else
        {
            XVariantClear( &temp );
        }
    }

    return ret;
}

// Make sure numeric variant value is in the specified range
void XVariantCheckInRange( xvariant* src, const xvariant* min, const xvariant* max )
{
    if ( src != 0 )
    {
        if ( src->type == XVT_Range )
        {
            // check range value
            if ( ( min != 0 ) && ( min->type == XVT_I4 ) )
            {
                if ( src->value.rangeVal.min < min->value.iVal )
                {
                    src->value.rangeVal.min = min->value.iVal;
                }
                if ( src->value.rangeVal.max < min->value.iVal )
                {
                    src->value.rangeVal.max = min->value.iVal;
                }
            }
            if ( ( max != 0 ) && ( max->type == XVT_I4 ) )
            {
                if ( src->value.rangeVal.min > max->value.iVal )
                {
                    src->value.rangeVal.min = max->value.iVal;
                }
                if ( src->value.rangeVal.max > max->value.iVal )
                {
                    src->value.rangeVal.max = max->value.iVal;
                }
            }
        }
        else if ( src->type == XVT_RangeF )
        {
            if ( ( min != 0 ) && ( min->type == XVT_R4 ) )
            {
                if ( src->value.frangeVal.min < min->value.fVal )
                {
                    src->value.frangeVal.min = min->value.fVal;
                }
                if ( src->value.frangeVal.max < min->value.fVal )
                {
                    src->value.frangeVal.max = min->value.fVal;
                }
            }
            if ( ( max != 0 ) && ( max->type == XVT_R4 ) )
            {
                if ( src->value.frangeVal.min > max->value.fVal )
                {
                    src->value.frangeVal.min = max->value.fVal;
                }
                if ( src->value.frangeVal.max > max->value.fVal )
                {
                    src->value.frangeVal.max = max->value.fVal;
                }
            }
        }
        else if ( src->type == XVT_Point )
        {
            // check point value

            if ( min != 0 )
            {
                int32_t minX = INT32_MIN;
                int32_t minY = INT32_MIN;

                if ( min->type == XVT_I4 )
                {
                    minX = min->value.iVal;
                    minY = min->value.iVal;
                }
                else if ( min->type == XVT_Point )
                {
                    minX = min->value.pointVal.x;
                    minY = min->value.pointVal.y;
                }

                if ( src->value.pointVal.x < minX )
                {
                    src->value.pointVal.x = minX;
                }
                if ( src->value.pointVal.y < minY )
                {
                    src->value.pointVal.y = minY;
                }
            }

            if ( max != 0 )
            {
                int32_t maxX = INT32_MAX;
                int32_t maxY = INT32_MAX;

                if ( max->type == XVT_I4 )
                {
                    maxX = max->value.iVal;
                    maxY = max->value.iVal;
                }
                else if ( max->type == XVT_Point )
                {
                    maxX = max->value.pointVal.x;
                    maxY = max->value.pointVal.y;
                }

                if ( src->value.pointVal.x > maxX )
                {
                    src->value.pointVal.x = maxX;
                }
                if ( src->value.pointVal.y > maxY )
                {
                    src->value.pointVal.y = maxY;
                }
            }
        }
        else if ( src->type == XVT_PointF )
        {
            // check floating point value
            if ( min != 0 )
            {
                float minX = -FLT_MAX;
                float minY = -FLT_MAX;

                if ( min->type == XVT_R4 )
                {
                    minX = min->value.fVal;
                    minY = min->value.fVal;
                }
                else if ( min->type == XVT_PointF )
                {
                    minX = min->value.fpointVal.x;
                    minY = min->value.fpointVal.y;
                }

                if ( src->value.fpointVal.x < minX )
                {
                    src->value.fpointVal.x = minX;
                }
                if ( src->value.fpointVal.y < minY )
                {
                    src->value.fpointVal.y = minY;
                }
            }

            if ( max != 0 )
            {
                float maxX = FLT_MAX;
                float maxY = FLT_MAX;

                if ( max->type == XVT_R4 )
                {
                    maxX = max->value.fVal;
                    maxY = max->value.fVal;
                }
                else if ( max->type == XVT_PointF )
                {
                    maxX = max->value.fpointVal.x;
                    maxY = max->value.fpointVal.y;
                }

                if ( src->value.fpointVal.x > maxX )
                {
                    src->value.fpointVal.x = maxX;
                }
                if ( src->value.fpointVal.y > maxY )
                {
                    src->value.fpointVal.y = maxY;
                }
            }
        }
        else if ( src->type == XVT_Size )
        {
            // check size value

            if ( min != 0 )
            {
                int32_t minW = INT32_MIN;
                int32_t minH = INT32_MIN;

                if ( min->type == XVT_I4 )
                {
                    minW = min->value.uiVal;
                    minH = min->value.uiVal;
                }
                else if ( min->type == XVT_Size )
                {
                    minW = min->value.sizeVal.width;
                    minH = min->value.sizeVal.height;
                }

                if ( src->value.sizeVal.width < minW )
                {
                    src->value.sizeVal.width = minW;
                }
                if ( src->value.sizeVal.height < minH )
                {
                    src->value.sizeVal.height = minH;
                }
            }

            if ( max != 0 )
            {
                int32_t minW = INT32_MAX;
                int32_t minH = INT32_MAX;

                if ( max->type == XVT_I4 )
                {
                    minW = max->value.uiVal;
                    minH = max->value.uiVal;
                }
                else if ( max->type == XVT_Size )
                {
                    minW = max->value.sizeVal.width;
                    minH = max->value.sizeVal.height;
                }

                if ( src->value.sizeVal.width > minW )
                {
                    src->value.sizeVal.width = minW;
                }
                if ( src->value.sizeVal.height > minH )
                {
                    src->value.sizeVal.height = minH;
                }
            }
        }
        else
        {
            // check numeric values

            if ( ( min != 0 ) && ( min->type == src->type ) )
            {
                switch ( src->type )
                {
                case XVT_I1:
                    if ( src->value.bVal < min->value.bVal )
                    {
                        src->value.bVal = min->value.bVal;
                    }
                    break;
                case XVT_I2:
                    if ( src->value.sVal < min->value.sVal )
                    {
                        src->value.sVal = min->value.sVal;
                    }
                    break;
                case XVT_I4:
                    if ( src->value.iVal < min->value.iVal )
                    {
                        src->value.iVal = min->value.iVal;
                    }
                    break;
                case XVT_U1:
                    if ( src->value.ubVal < min->value.ubVal )
                    {
                        src->value.ubVal = min->value.ubVal;
                    }
                    break;
                case XVT_U2:
                    if ( src->value.usVal < min->value.usVal )
                    {
                        src->value.usVal = min->value.usVal;
                    }
                    break;
                case XVT_U4:
                    if ( src->value.uiVal < min->value.uiVal )
                    {
                        src->value.uiVal = min->value.uiVal;
                    }
                    break;
                case XVT_R4:
                    if ( src->value.fVal < min->value.fVal )
                    {
                        src->value.fVal = min->value.fVal;
                    }
                    break;
                case XVT_R8:
                    if ( src->value.dVal < min->value.dVal )
                    {
                        src->value.dVal = min->value.dVal;
                    }
                    break;
                }
            }

            if ( ( max != 0 ) && ( max->type == src->type ) )
            {
                switch ( src->type )
                {
                case XVT_I1:
                    if ( src->value.bVal > max->value.bVal )
                    {
                        src->value.bVal = max->value.bVal;
                    }
                    break;
                case XVT_I2:
                    if ( src->value.sVal > max->value.sVal )
                    {
                        src->value.sVal = max->value.sVal;
                    }
                    break;
                case XVT_I4:
                    if ( src->value.iVal > max->value.iVal )
                    {
                        src->value.iVal = max->value.iVal;
                    }
                    break;
                case XVT_U1:
                    if ( src->value.ubVal > max->value.ubVal )
                    {
                        src->value.ubVal = max->value.ubVal;
                    }
                    break;
                case XVT_U2:
                    if ( src->value.usVal > max->value.usVal )
                    {
                        src->value.usVal = max->value.usVal;
                    }
                    break;
                case XVT_U4:
                    if ( src->value.uiVal > max->value.uiVal )
                    {
                        src->value.uiVal = max->value.uiVal;
                    }
                    break;
                case XVT_R4:
                    if ( src->value.fVal > max->value.fVal )
                    {
                        src->value.fVal = max->value.fVal;
                    }
                    break;
                case XVT_R8:
                    if ( src->value.dVal > max->value.dVal )
                    {
                        src->value.dVal = max->value.dVal;
                    }
                    break;
                }
            }
        }
    }
}

// Convert array to string
XErrorCode XArrayToString( const xarray* array, XVarType srcType, xstring* pstr )
{
    XErrorCode ret = SuccessCode;

    if ( ( array == 0 ) || ( pstr == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( !IS_ARRAY_TYPE( srcType ) )
    {
        ret = ErrorIncompatibleTypes;
    }
    else
    {
        XVarType elementType = srcType & XVT_Any;

        if ( ( elementType >= XVT_Bool ) && ( elementType <= XVT_Size ) && ( elementType != XVT_String ) )
        {
            char* tempStr = (char*) malloc( LONG_STRING_LENGTH );

            if ( tempStr == 0 )
            {
                ret = ErrorOutOfMemory;
            }
            else
            {
                size_t   allocated = LONG_STRING_LENGTH;
                size_t   left      = LONG_STRING_LENGTH - 1;
                char*    ptr       = tempStr;
                uint32_t i;

                if ( ( srcType & XVT_Array2d ) == XVT_Array2d )
                {
                    ptr += sprintf( tempStr, "[[%u,%u]]", ( (xarray2d*) array )->rows, ( (xarray2d*) array )->cols );
                }
                else
                {
                    ptr += sprintf( tempStr, "[%u]", array->length );
                }

                left -= ( ptr - tempStr );

                for ( i = 0; ( i < array->length ) && ( ret == SuccessCode ); i++ )
                {
                    xstring strdst = 0;

                    ret = XVariantToString( &( array->elements[i] ), &strdst );

                    if ( ret == SuccessCode )
                    {
                        size_t len = strlen( strdst );

                        if ( left < len + 1 )
                        {
                            // need more memory for the temporary string
                            size_t toAlloc    = XMAX( LONG_STRING_LENGTH, len + 1 );
                            char*  tempBuffer = malloc( allocated + toAlloc );

                            if ( tempBuffer != 0 )
                            {
                                strcpy( tempBuffer, tempStr );

                                allocated += toAlloc;
                                left      += toAlloc;

                                ptr = tempBuffer + ( allocated - left - 1 ); // extra 1 byte for null termination

                                free( tempStr );
                                tempStr = tempBuffer;
                            }
                        }

                        if ( left >= len + 1 )
                        {
                            // append delimiter
                            if ( i != 0 )
                            {
                                *ptr = '|';
                                ptr++;
                                *ptr = '\0';
                                left--;
                            }

                            // append string element
                            strcpy( ptr, strdst );
                            ptr  += len;
                            left -= len;
                        }
                        else
                        {
                            ret = ErrorOutOfMemory;
                        }

                        XStringFree( &strdst );
                    }
                }

                if ( ret == SuccessCode )
                {
                    *pstr = XStringAlloc( tempStr );
                }

                free( tempStr );
            }
        }
        else
        {
            ret = ErrorIncompatibleTypes;
        }
    }

    return ret;
}

// Helper function to parse element from string representation of array
static XErrorCode ParseArrayElement( char** strPtr, XVarType elementType, xvariant* var )
{
    char*      ptr          = *strPtr;
    char*      ptrDelimiter = strchr( ptr, '|' );
    char       buffer[64];
    xvariant   tempVar;
    size_t     toCopy;
    XErrorCode ret = SuccessCode;

    tempVar.type = XVT_String;

    if ( ptrDelimiter == 0 )
    {
        // delimiter was not found - last value maybe
        tempVar.value.strVal = ptr;
    }
    else
    {
        toCopy = (size_t) ( ptrDelimiter - ptr );
        if ( toCopy >= sizeof( buffer ) )
        {
            // since we support limited range of types, we don't expect too long strings
            ret = ErrorInvalidArgument;
        }

        else
        {
            // copy the string between delimiters, so we have string value only
            strncpy( buffer, ptr, toCopy );
            buffer[toCopy] = 0;

            tempVar.value.strVal = buffer;
        }
    }

    if ( ret == SuccessCode )
    {
        ret = XVariantChangeType( &tempVar, var, elementType );
    }

    ptr = ptrDelimiter;
    if ( ptr )
    {
        ptr++;
    }

    if ( ret == SuccessCode )
    {
        *strPtr = ptr;
    }

    return ret;
}

// Convert string to array of the given type
XErrorCode XStringToArray( const xstring str, XVarType dstType, xarray** parray )
{
    XErrorCode ret = SuccessCode;

    if ( ( str == 0 ) || ( parray == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( !IS_ARRAY_TYPE( dstType ) )
    {
        ret = ErrorIncompatibleTypes;
    }
    else
    {
        XVarType elementType = dstType & XVT_Any;

        if ( ( elementType >= XVT_Bool ) && ( elementType <= XVT_Size ) && ( elementType != XVT_String ) )
        {
            xarray*  array       = 0;
            uint32_t arrayLength = 0;
            size_t   strLength   = strlen( str );
            char*    ptr         = 0;

            if ( ( ( dstType & XVT_Array ) != 0 ) || ( ( dstType & XVT_ArrayJagged ) != 0 ) )
            {
                if ( ( strLength < 3 ) || ( str[0] != '[' ) || ( str[1] < '0' ) || ( str[1] > '9' ) ||
                     ( ( ptr = strchr( str, ']' ) ) == 0 ) )
                {
                    ret = ErrorInvalidArgument;
                }
                else
                {
                    ptr++;

                    if ( sscanf( &str[1], "%u", &arrayLength ) != 1 )
                    {
                        ret = ErrorInvalidArgument;
                    }
                    else
                    {
                        if ( ( dstType & XVT_Array ) != 0 )
                        {
                            ret = XArrayAllocate( &array, elementType, arrayLength );
                        }
                        else
                        {
                            ret = XArrayAllocateJagged( (xarrayJagged**) &array, elementType, arrayLength );
                        }
                    }
                }
            }
            else
            {
                if ( ( strLength < 7 ) || ( str[0] != '[' ) || ( str[1] != '[' ) || ( str[2] < '0' ) || ( str[2] > '9' ) ||
                     ( ( ptr = strstr( str, "]]" ) ) == 0 ) )
                {
                    ret = ErrorInvalidArgument;
                }
                else
                {
                    int rows, cols;

                    ptr += 2;

                    if ( sscanf( &str[2], "%u,%u", &rows, &cols ) != 2 )
                    {
                        ret = ErrorInvalidArgument;
                    }
                    else
                    {
                        arrayLength = rows * cols;

                        ret = XArrayAllocate2d( (xarray2d**) &array, elementType, rows, cols );
                    }
                }
            }

            if ( ret == SuccessCode )
            {
                uint32_t i;

                if ( ( ( dstType & XVT_Array ) != 0 ) || ( ( dstType & XVT_Array2d ) != 0 ) )
                {
                    for ( i = 0; ( i < arrayLength ) && ( ret == SuccessCode ) && ( ptr != 0 ); i++ )
                    {
                        ret = ParseArrayElement( &ptr, elementType, &( array->elements[i] ) );
                    }
                }
                else
                {
                    xarray*  subAarray      = 0;
                    uint32_t subArrayLength = 0;
                    uint32_t j;

                    for ( i = 0; ( i < arrayLength ) && ( ret == SuccessCode ) && ( ptr != 0 ); i++ )
                    {
                        if ( ( *ptr != '[' ) || ( sscanf( &ptr[1], "%u", &subArrayLength ) != 1 ) ||
                             ( ( ptr = strchr( ptr, ']' ) ) == 0 ) )
                        {
                            ret = ErrorInvalidArgument;
                        }
                        else
                        {
                            ret = XArrayAllocate( &subAarray, elementType, subArrayLength );

                            if ( ret == SuccessCode )
                            {
                                array->elements[i].type           = elementType | XVT_Array;
                                array->elements[i].value.arrayVal = subAarray;

                                ptr++;

                                for ( j = 0; ( j < subArrayLength ) && ( ret == SuccessCode ) && ( ptr != 0 ); j++ )
                                {
                                    ret = ParseArrayElement( &ptr, elementType, &( subAarray->elements[j] ) );
                                }

                                if ( ( ret == SuccessCode ) && ( j != subArrayLength ) )
                                {
                                    ret = ErrorInvalidArgument;
                                }

                                // sub array is now part of jagged array
                                subAarray = 0;
                            }
                        }
                    }
                }

                if ( ( ret == SuccessCode ) && ( i != arrayLength ) )
                {
                    ret = ErrorInvalidArgument;
                }

                if ( ret == SuccessCode )
                {
                    *parray = array;
                }
                else
                {
                    XArrayFree( &array );
                }
            }
        }
        else
        {
            ret = ErrorIncompatibleTypes;
        }
    }

    return ret;
}
