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

#include "XScriptingEnginePlugin.hpp"

using namespace std;

XScriptingEnginePlugin::XScriptingEnginePlugin( void* plugin, bool ownIt ) :
    XPlugin( plugin, PluginType_ScriptingEngine, ownIt )
{
}

XScriptingEnginePlugin::~XScriptingEnginePlugin( )
{
}

// Create plug-in wrapper
const shared_ptr<XScriptingEnginePlugin> XScriptingEnginePlugin::Create( void* plugin, bool ownIt )
{
    return shared_ptr<XScriptingEnginePlugin>( new XScriptingEnginePlugin( plugin, ownIt ) );
}

// Set callback to interface with the calling host
void XScriptingEnginePlugin::SetCallbacks( const ScriptingEnginePluginCallbacks* callbacks, void* userParam )
{
    SScriptingEnginePlugin* sep = static_cast<SScriptingEnginePlugin*>( mPlugin );
    return sep->SetCallbacks( sep, callbacks, userParam );
}

// Get default file name extension for the engine's scripts
string XScriptingEnginePlugin::GetDefaultExtension( )
{
    SScriptingEnginePlugin* sep  = static_cast<SScriptingEnginePlugin*>( mPlugin );
    xstring                 xext = sep->GetDefaultExtension( sep );
    string                  ext;

    if ( xext != nullptr )
    {
        ext = string( xext );
        XStringFree( &xext );
    }

    return ext;
}

// Perform any necessary initialization of the scripting engine
XErrorCode XScriptingEnginePlugin::Init( )
{
    SScriptingEnginePlugin* sep = static_cast<SScriptingEnginePlugin*>( mPlugin );
    return sep->Init( sep );
}

// Set script file to be loaded (this might be also exposed as plug-ins property,
// but duplicated for clients who would like to rely on the set interface)
void XScriptingEnginePlugin::SetScriptFile( const string& fileName )
{
    SScriptingEnginePlugin* sep = static_cast<SScriptingEnginePlugin*>( mPlugin );
    return sep->SetScriptFile( sep, fileName.c_str( ) );
}

// Load previously set script from file
XErrorCode XScriptingEnginePlugin::LoadScript( )
{
    SScriptingEnginePlugin* sep = static_cast<SScriptingEnginePlugin*>( mPlugin );
    return sep->LoadScript( sep );
}

// Run initialization part of the loaded script (any code outside of Main)
XErrorCode XScriptingEnginePlugin::InitScript( )
{
    SScriptingEnginePlugin* sep = static_cast<SScriptingEnginePlugin*>( mPlugin );
    return sep->InitScript( sep );
}

// Run the Main function of the script
XErrorCode XScriptingEnginePlugin::RunScript( )
{
    SScriptingEnginePlugin* sep = static_cast<SScriptingEnginePlugin*>( mPlugin );
    return sep->RunScript( sep );
}

// Get error message which scripting engine may generate if LoadScript() returns ErrorFailedLoadingScript or InitScript()/RunScript() return ErrorFailedRunningScript
string XScriptingEnginePlugin::GetLastErrorMessage( )
{
    SScriptingEnginePlugin* sep  = static_cast<SScriptingEnginePlugin*>( mPlugin );
    xstring                 xmsg = sep->GetLastErrorMessage( sep );
    string                  msg;

    if ( xmsg != nullptr )
    {
        msg = string( xmsg );
        XStringFree( &xmsg );
    }

    return msg;
}
