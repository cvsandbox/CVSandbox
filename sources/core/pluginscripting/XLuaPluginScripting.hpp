/*
    Plug-ins' scripting library of Computer Vision Sandbox

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
#ifndef CVS_XLUA_PLUGIN_SCRIPTING_HPP
#define CVS_XLUA_PLUGIN_SCRIPTING_HPP

#include <XInterfaces.hpp>
#include "IPluginScripting.hpp"
#include "IScriptingHost.hpp"

namespace Private
{
    class XLuaPluginScriptingData;
}

class XLuaPluginScripting : public IPluginScripting, private CVSandbox::Uncopyable
{
public:
    XLuaPluginScripting( IScriptingHost* host );
    ~XLuaPluginScripting( );

public:
    virtual XErrorCode Init( );

    virtual XErrorCode LoadScriptFromFile( const std::string& fileName );
    virtual XErrorCode IsMainDefined( );
    virtual XErrorCode InitScript( );
    virtual XErrorCode RunScript( );

    virtual const std::string GetLastErrorMessage( ) const;

    // Set/Get how often to force GC run (after every Nth run of Main(), where N = period. if 0 - let Lua do it)
    void SetGcPeriod( uint32_t period );
    uint32_t GcPeriod( );

private:
    Private::XLuaPluginScriptingData* mData;
};

#endif // CVS_XLUA_PLUGIN_SCRIPTING_HPP
