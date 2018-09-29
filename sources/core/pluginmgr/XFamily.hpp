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
#ifndef CVS_XFAMILY_HPP
#define CVS_XFAMILY_HPP

#include <memory>
#include <string>
#include <XInterfaces.hpp>
#include <XGuid.hpp>
#include <XImage.hpp>
#include <ifamily.h>

// Class which wraps information about plug-in family
class XFamily : private CVSandbox::Uncopyable
{
private:
    XFamily( const FamilyDescriptor* desc );

public:
    ~XFamily( );

    static const std::shared_ptr<XFamily> Create( const FamilyDescriptor* desc );
    // Get default family of plug-ins'
    static const std::shared_ptr<XFamily> DefaultFamily( );

    // ID, name, description and icon of plug-in family
    const CVSandbox::XGuid ID( ) const;
    const std::string Name( ) const;
    const std::string Description( ) const;
    const std::shared_ptr<const CVSandbox::XImage> Icon( bool getSmall = true ) const;

private:
    const FamilyDescriptor* mDescriptor;
};

#endif // CVS_XFAMILY_HPP
