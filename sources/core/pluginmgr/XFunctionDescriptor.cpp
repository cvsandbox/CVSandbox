/*
    Plug-ins' management library of Computer Vision Sandbox

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
#include "XFunctionDescriptor.hpp"

using namespace std;
using namespace CVSandbox;

XFunctionDescriptor::XFunctionDescriptor( int32_t id, FunctionDescriptor* desc ) :
    mID( id ),
    mDescriptor( desc )
{
}

XFunctionDescriptor::~XFunctionDescriptor( )
{
}

const shared_ptr<XFunctionDescriptor> XFunctionDescriptor::Create( int32_t id, FunctionDescriptor* desc )
{
    assert( desc );
    return shared_ptr<XFunctionDescriptor>( ( desc == 0 ) ? 0 : new XFunctionDescriptor( id, desc ) );
}

XVarType XFunctionDescriptor::ReturnType( ) const
{
    return mDescriptor->ReturnType;
}

const string XFunctionDescriptor::Name( ) const
{
    return string( mDescriptor->Name );
}

const string XFunctionDescriptor::Description( ) const
{
    return string( mDescriptor->Description );
}

int32_t XFunctionDescriptor::ArgumentsCount( ) const
{
    return mDescriptor->ArgumentsCount;
}

// Get argument's info
void XFunctionDescriptor::GetArgumentInfo( int32_t argument, std::string& name, std::string& description, XVarType* type ) const
{
    if ( ( argument >= 0 ) && ( argument < mDescriptor->ArgumentsCount ) )
    {
        name        = mDescriptor->Arguments[argument]->Name;
        description = mDescriptor->Arguments[argument]->Description;

        if ( type != nullptr )
        {
            *type = mDescriptor->Arguments[argument]->Type;
        }
    }
}
XVarType XFunctionDescriptor::GetArgumentType( int32_t argument ) const
{
    XVarType ret = XVT_Empty;

    if ( ( argument >= 0 ) && ( argument < mDescriptor->ArgumentsCount ) )
    {
        ret = mDescriptor->Arguments[argument]->Type;
    }

    return ret;
}
