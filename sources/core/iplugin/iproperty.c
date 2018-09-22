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

#include <assert.h>
#include "iproperty.h"

// Helper function to convert input value of a property to the required type
XErrorCode PropertyChangeTypeHelper( int32_t id, const xvariant* value, const PropertyDescriptor** descriptors,
                                     int32_t count, xvariant* convertedValue )
{
    XErrorCode ret = ErrorFailed;

    if ( ( id < 0 ) || ( id >= count ) )
    {
        ret = ErrorInvalidProperty;
    }
    else
    {
        if ( descriptors == 0 )
        {
            assert( descriptors );
            // just make a copy of the value if property descriptors are not set
            // ret = XVariantCopy( value, convertedValue );
        }
        else if ( ( descriptors[id]->Flags & PropertyFlag_ReadOnly ) == PropertyFlag_ReadOnly )
        {
            ret = ErrorReadOnlyProperty;
        }
        else
        {
            // change type to the type of the property
            ret = XVariantChangeType( value, convertedValue, descriptors[id]->Type );

            if ( ret == SuccessCode )
            {
                XVariantCheckInRange( convertedValue, &descriptors[id]->MinValue, &descriptors[id]->MaxValue );
            }
        }
    }

    return ret;
}

// Helper function to convert input value of an indexed property to the required type
XErrorCode PropertyChangeTypeIndexedHelper( int32_t id, const xvariant* value, const PropertyDescriptor** descriptors,
                                            int32_t count, xvariant* convertedValue )
{
    XErrorCode ret = ErrorFailed;

    if ( ( id < 0 ) || ( id >= count ) )
    {
        ret = ErrorInvalidProperty;
    }
    else
    {
        if ( descriptors == 0 )
        {
            assert( descriptors );
        }
        else if ( ( descriptors[id]->Flags & PropertyFlag_ReadOnly ) == PropertyFlag_ReadOnly )
        {
            ret = ErrorReadOnlyProperty;
        }
        else if ( ( descriptors[id]->Type & XVT_Array ) != XVT_Array )
        {
            ret = ErrorNotIndexedProperty;
        }
        else
        {
            // change type to the type of the property
            ret = XVariantChangeType( value, convertedValue, descriptors[id]->Type & XVT_Any );
        }
    }

    return ret;
}

// Helper function to initialize selection by value property with string value
XErrorCode InitSelectionProperty( PropertyDescriptor* descriptor, const char** values, uint8_t count, uint8_t defaultValue )
{
    XErrorCode ret = SuccessCode;

    if ( ( descriptor == 0 ) || ( values == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        descriptor->DefaultValue.type = XVT_U1;
        descriptor->DefaultValue.value.ubVal = ( defaultValue < count ) ? defaultValue : 0;

        descriptor->MinValue.type = XVT_U1;
        descriptor->MinValue.value.ubVal = 0;

        descriptor->MaxValue.type = XVT_U1;
        descriptor->MaxValue.value.ubVal = count - 1;

        descriptor->ChoicesCount = count;
        descriptor->Choices = (xvariant*) malloc( count * sizeof( xvariant ) );

        if ( descriptor->Choices == 0 )
        {
            ret = ErrorOutOfMemory;
        }
        else
        {
            int i;

            for ( i = 0; i < count; i++ )
            {
                descriptor->Choices[i].type = XVT_String;
                descriptor->Choices[i].value.strVal = XStringAlloc( values[i] );

                if ( descriptor->Choices[i].value.strVal == 0 )
                {
                    break;
                }
            }

            if ( i != count )
            {
                ret = ErrorOutOfMemory;
                CleanSelectionProperty( descriptor );
            }
        }
    }

    return ret;
}

// Helper function to clean selection property
void CleanSelectionProperty( PropertyDescriptor* descriptor )
{
    if ( descriptor != 0 )
    {
        if ( descriptor->Choices != 0 )
        {
            int i;

            for ( i = 0; i < descriptor->ChoicesCount; i++ )
            {
                XVariantClear( &( descriptor->Choices[i] ) );
            }

            free( descriptor->Choices );
            descriptor->Choices = 0;
        }

        descriptor->ChoicesCount = 0;
    }
}
