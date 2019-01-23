/*
    Common tools for Computer Vision Sandbox

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
#ifndef CVS_FAVOURITE_PLUGINS_MANAGER_HPP
#define CVS_FAVOURITE_PLUGINS_MANAGER_HPP

#include <XPluginsCollection.hpp>
#include <XModulesCollection.hpp>
#include "cvsandboxtools_global.h"

class CVS_SHARED_EXPORT FavouritePluginsManager
{
public:
    FavouritePluginsManager( );

    void Load( const std::shared_ptr<const XModulesCollection>& modulesCollection );
    void Save( );

    std::shared_ptr<XPluginsCollection> PluginsCollection( ) const
    {
        return mPlugins;
    }

private:
    const std::shared_ptr<XPluginsCollection>   mPlugins;
};

#endif // CVS_FAVOURITE_PLUGINS_MANAGER_HPP
