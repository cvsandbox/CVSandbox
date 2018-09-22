/*
    Plug-ins' interface library of Computer Vision Sandbox

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
#include <assert.h>
#include "iplugintypes.h"

// Helper function which provides default implementation for the "GetPixelFormatTranslations" method
// of image processing filter plug-in
//
XErrorCode GetPixelFormatTranslationsImpl( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count,
    const XPixelFormat* supportedInputFormats, const XPixelFormat* supportedOutputFormats, int32_t supportedFormatsCount )
{
    XErrorCode ret = SuccessCode;

    assert( supportedInputFormats );
    assert( supportedOutputFormats );

    if ( ( count == 0 ) ||
       ( ( *count != 0 ) && ( ( inputFormats == 0 ) || ( outputFormats == 0 ) ) ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        if ( *count < supportedFormatsCount )
        {
            ret = ErrorTooSmallBuffer;
        }
        else
        {
            memcpy( inputFormats,  supportedInputFormats,  sizeof( XPixelFormat ) * supportedFormatsCount );
            memcpy( outputFormats, supportedOutputFormats, sizeof( XPixelFormat ) * supportedFormatsCount );
        }

        *count = supportedFormatsCount;
    }

    return ret;
}

// Helper function to check if the specified pixel format is in the array of supported formats
bool IsPixelFormatSupportedImpl( const XPixelFormat* supportedInputFormats, int32_t count, XPixelFormat formatToCheck )
{
    int32_t index;
    bool    ret = false;

    if ( supportedInputFormats != 0 )
    {
        for ( index = 0; index < count; index++ )
        {
            if ( supportedInputFormats[index] == formatToCheck )
            {
                ret = true;
                break;
            }
        }
    }

    return ret;
}


// Helper function which provides default implementation for the "GetSupportedExtensions" method
// of image importing/exporting plug-in
//
XErrorCode GetSupportedExtensionsImpl( const char** supportedExtensionsIn, int32_t inCount, xstring* supportedExtensionsOut, int32_t* outCount )
{
    XErrorCode ret = SuccessCode;

    assert( supportedExtensionsIn );

    if ( ( outCount == 0 ) ||
         ( ( *outCount != 0 ) && ( supportedExtensionsOut == 0 ) ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        if ( *outCount < inCount )
        {
            ret = ErrorTooSmallBuffer;
            *outCount = inCount;
        }
        else
        {
            int i;

            for ( i = 0; i < inCount; i++ )
            {
                supportedExtensionsOut[i] = XStringAlloc( supportedExtensionsIn[i] );

                if ( supportedExtensionsOut[i] == 0 )
                {
                    ret = ErrorOutOfMemory;
                    break;
                }
            }

            if ( i != inCount )
            {
                // clen-up on error
                while ( --i >= 0 )
                {
                    XStringFree( &supportedExtensionsOut[i] );
                }
            }
            else
            {
                *outCount = inCount;
            }
        }
    }

    return ret;
}

// Helper function which provides default implementation for the "GetSupportedImageFormats" method
// of image exporting plug-in
//
XErrorCode GetSupportedPixelFormatsImpl( const XPixelFormat* supportedPixelFormatsIn, int32_t inCount,
                                         XPixelFormat* supportedPixelFormatsOut, int32_t* outCount )
{
    XErrorCode ret = SuccessCode;

    assert( supportedPixelFormatsIn );

    if ( ( outCount == 0 ) ||
         ( ( *outCount != 0 ) && ( supportedPixelFormatsOut == 0 ) ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        if ( *outCount < inCount )
        {
            ret = ErrorTooSmallBuffer;
            *outCount = inCount;
        }
        else
        {
            int i;

            for ( i = 0; i < inCount; i++ )
            {
                supportedPixelFormatsOut[i] = supportedPixelFormatsIn[i];
            }

            *outCount = inCount;
        }
    }

    return ret;
}
