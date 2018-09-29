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

#pragma once
#ifndef CVS_XFUNCTION_DESCRIPTOR_HPP
#define CVS_XFUNCTION_DESCRIPTOR_HPP

#include <string>
#include <memory>
#include <XInterfaces.hpp>
#include <ifunction.h>

// Class which wraps description of a plug-in function
class XFunctionDescriptor : private CVSandbox::Uncopyable
{
private:
    XFunctionDescriptor( int32_t id, FunctionDescriptor* desc );

public:
    ~XFunctionDescriptor( );

    static const std::shared_ptr<XFunctionDescriptor> Create( int32_t id, FunctionDescriptor* desc );

    // Property ID
    inline int32_t ID( ) const { return mID; }

    // Return type of the function
    XVarType ReturnType( ) const;

    // Description of the property
    const std::string Name( ) const;
    const std::string Description( ) const;

    // Arguments count of the function
    int32_t ArgumentsCount( ) const;

    // Get argument's info
    void GetArgumentInfo( int32_t argument, std::string& name, std::string& description, XVarType* type ) const;
    XVarType GetArgumentType( int32_t argument ) const;

private:
    int32_t             mID;
    FunctionDescriptor* mDescriptor;
};

#endif // CVS_XFUNCTION_DESCRIPTOR_HPP
