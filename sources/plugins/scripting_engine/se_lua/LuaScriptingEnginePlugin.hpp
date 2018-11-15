/*
    Lua Scripting Engine plug-in of Computer Vision Sandbox

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
#ifndef LUA_SCRIPTING_ENGINE_PLUGIN_HPP
#define LUA_SCRIPTING_ENGINE_PLUGIN_HPP

#include <memory>
#include <iplugintypescpp.hpp>

namespace Private
{
    class LuaScriptingEnginePluginData;
}

class LuaScriptingEnginePlugin : public IScriptingEnginePlugin
{
public:
    LuaScriptingEnginePlugin( );
    ~LuaScriptingEnginePlugin( );

    // IPluginBase interface
    virtual void Dispose( );

    virtual XErrorCode GetProperty( int32_t id, xvariant* value ) const;
    virtual XErrorCode SetProperty( int32_t id, const xvariant* value );

    // IScriptingEnginePlugin interface

    // Set callback to interface with the calling host
    void SetCallbacks( const ScriptingEnginePluginCallbacks* callbacks, void* userParam );
    // Get default file name extension for the engine's scripts
    virtual xstring GetDefaultExtension( );
    // Perform any necessary initialization of the scripting engine
    virtual XErrorCode Init( );
    // Set script file to be loaded (this might be also exposed as plug-ins property,
    // but duplicated for clients who would like to rely on the set interface)
    virtual void SetScriptFile( xstring fileName );
    // Load previously set script from file
    virtual XErrorCode LoadScript( );
    // Run initialization part of the loaded script (any code outside of Main)
    virtual XErrorCode InitScript( );
    // Run the Main function of the script
    virtual XErrorCode RunScript( );
    // Get error message which scripting engine may generate if LoadScript() returns ErrorFailedLoadingScript or InitScript()/RunScript() return ErrorFailedRunningScript
    virtual xstring GetLastErrorMessage( );

private:
    static const PropertyDescriptor**        propertiesDescription;
    ::Private::LuaScriptingEnginePluginData* mData;
};

#endif // LUA_SCRIPTING_ENGINE_PLUGIN_HPP
