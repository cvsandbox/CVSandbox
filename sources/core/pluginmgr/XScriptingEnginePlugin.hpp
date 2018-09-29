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
#ifndef CVS_XSCRIPTING_ENGINE_PLUGIN_HPP
#define CVS_XSCRIPTING_ENGINE_PLUGIN_HPP

#include "XPlugin.hpp"

class XScriptingEnginePlugin : public XPlugin
{
private:
    XScriptingEnginePlugin( void* plugin, bool ownIt );

public:
    virtual ~XScriptingEnginePlugin( );

    // Create plug-in wrapper
    static const std::shared_ptr<XScriptingEnginePlugin> Create( void* plugin, bool ownIt = true );

    // Set callback to interface with the calling host
    void SetCallbacks( const ScriptingEnginePluginCallbacks* callbacks, void* userParam );
    // Get default file name extension for the engine's scripts
    std::string GetDefaultExtension( );
    // Perform any necessary initialization of the scripting engine
    XErrorCode Init( );
    // Set script file to be loaded (this might be also exposed as plug-ins property,
    // but duplicated for clients who would like to rely on the set interface)
    void SetScriptFile( const std::string& fileName );
    // Load previously set script from file
    XErrorCode LoadScript( );
    // Run initialization part of the loaded script (any code outside of Main)
    XErrorCode InitScript( );
    // Run the Main function of the script
    XErrorCode RunScript( );
    // Get error message which scripting engine may generate if LoadScript() returns ErrorFailedLoadingScript or InitScript()/RunScript() return ErrorFailedRunningScript
    std::string GetLastErrorMessage( );
};

#endif // CVS_XSCRIPTING_ENGINE_PLUGIN_HPP
