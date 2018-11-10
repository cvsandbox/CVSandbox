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
#ifndef CVS_XLUA_PLUGIN_SCRIPTING_USER_TYPES_HPP
#define CVS_XLUA_PLUGIN_SCRIPTING_USER_TYPES_HPP

#include <memory>
#include <XPluginsEngine.hpp>

extern "C"
{
    #include <lua.h>
}

// Create Lua user data for the specified plug-in leaving it on Lua's stack
void CreateLuaUserDataForPlugin( lua_State* luaState,
                                 const std::shared_ptr<const XPluginDescriptor>& descriptor,
                                 const std::shared_ptr<XPlugin>& plugin );

// Register user data types for Lua plug-ins scripting
void RegisterLuaUserDataTypes( lua_State* luaState );

// Put image on Lua stack
void PutImageOnLuaStack( lua_State* luaState, const std::shared_ptr<CVSandbox::XImage>& image );
// Get image from Lua stack
const std::shared_ptr<CVSandbox::XImage> GetImageFromLuaStack( lua_State* luaState, int stackIndex );
// Check if Lua stack contains image at the specified index
bool IsImageOnLuaStack( lua_State* luaState, int stackIndex );

// Report an error for the specified XErrorCode
void ReportXError( lua_State* luaState, XErrorCode errorCode );

// Report argument error through Lua engine
void ReportArgError( lua_State* luaState, int stackIndex, const char* strErrorMessage );

// Helper function to check C function got expected number of arguments from Lua
void CheckArgumentsCount( lua_State* luaState, int expectedCount );
void CheckArgumentsCount( lua_State* luaState, int expectedMinCount, int expectedMaxCount );

// Convert XVariant to something which Lua would understand and push it to stack
void PushXVariantToLuaStack( lua_State* luaState, const CVSandbox::XVariant& variant );
// Get XVariant value from Lua's stack
const CVSandbox::XVariant GetXVariantFromLuaStack( lua_State* luaState, int stackIndex );

#endif // CVS_XLUA_PLUGIN_SCRIPTING_USER_TYPES_HPP
