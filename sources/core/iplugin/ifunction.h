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

#pragma once
#ifndef CVS_IFUNCTION_H
#define CVS_IFUNCTION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <xtypes.h>

// Structure providing description of a function's argument
typedef struct _ArgumentDescriptor
{
    XVarType      Type;
    xstring       Name;
    xstring       Description;
}
ArgumentDescriptor;

// Structure providing description of a plug-in's function
typedef struct _FunctionDescriptor
{
    XVarType      ReturnType;
    xstring       Name;
    xstring       Description;
    int32_t       ArgumentsCount;
    ArgumentDescriptor** Arguments;
}
FunctionDescriptor;

// Create a copy of the specified function descriptor
FunctionDescriptor* CopyFunctionDescriptor( const FunctionDescriptor* src );
// Free memory taken by the specified function descriptor
void FreeFunctionDescriptor( FunctionDescriptor** pDescriptor );

// Create a copy of the specified argument descriptor
ArgumentDescriptor* CopyArgumentDescriptor( const ArgumentDescriptor* src );
// Free memory taken by the specified argument descriptor
void FreeArgumentDescriptor( ArgumentDescriptor** pDescriptor );

// Helper function to validate expected types of arguments are passed to a plug-ins "CallFunction" method
XErrorCode ValidateFunctionArguments( int32_t id, const xarray* arguments, const FunctionDescriptor** descriptors, int32_t count );

#ifdef __cplusplus
}
#endif

#endif // CVS_IFUNCTION_H
