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
#include "ifunction.h"

// Helper function to validate expected types of arguments are passed to a plug-ins "CallFunction" method
XErrorCode ValidateFunctionArguments( int32_t id, const xarray* arguments, const FunctionDescriptor** descriptors, int32_t count )
{
    XErrorCode ret = SuccessCode;

    assert( descriptors );

    if ( arguments == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( id < 0 ) || ( id >= count ) )
    {
        ret = ErrorInvalidFunction;
    }
    else
    {
        const FunctionDescriptor* descriptor = descriptors[id];

        if ( (int32_t) arguments->length != descriptor->ArgumentsCount )
        {
            ret = ErrorInvalidArgumentCount;
        }
        else
        {
            int32_t i;

            for ( i = 0; i < descriptor->ArgumentsCount; i++ )
            {
                if ( arguments->elements[i].type != descriptor->Arguments[i]->Type )
                {
                    ret = ErrorIncompatibleTypes;
                    break;
                }
            }
        }
    }

    return ret;
}
