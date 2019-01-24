/*
    Computer Vision Sandbox

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

#pragma once
#ifndef CVS_IPROJECTTREEUI_HPP
#define CVS_IPROJECTTREEUI_HPP

#include <string>
#include <XGuid.hpp>
#include "ProjectObject.hpp"

// Interface to be exposed by widget showing project's tree
class IProjectTreeUI
{
public:
    virtual ~IProjectTreeUI( ) { }

    // Check if root item is selected
    virtual bool IsRootSelected( ) const = 0;

    // Get ID of the selected item
    virtual CVSandbox::XGuid GetSelectedItem( ) const = 0;

    // Get ID of the selected folder or the folder containing selected item
    virtual CVSandbox::XGuid GetSelectedOrParentFolder( ) const = 0;

    // Set selected item ID
    virtual void SetSelectedItem( const CVSandbox::XGuid& id ) = 0;

    // Get type of the selected item
    virtual ProjectObjectType GetSelectedItemType( ) const = 0;
};

#endif // CVS_IPROJECTTREEUI_HPP
