/*
    Computer Vision Sandbox Script Runnner application

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

#include <stdio.h>
#include "ScriptingHost.hpp"

using namespace std;
using namespace CVSandbox;

extern const char*  APP_NAME;
extern xversion     APP_VERSION;

ScriptingHost::ScriptingHost( const map<string, string>& scriptArguments ) :
    XDefaultScriptingHost( scriptArguments, vector<string>( { "./cvsplugins/", "./cvsplugins-extra/" } ) )
{
}

const string ScriptingHost::Name( ) const
{
    return string( APP_NAME );
}

const XVersion ScriptingHost::Version( ) const
{
    return XVersion( APP_VERSION );
}

void ScriptingHost::PrintString( const string& message ) const
{
    printf( "%s\n", message.c_str( ) );
}
