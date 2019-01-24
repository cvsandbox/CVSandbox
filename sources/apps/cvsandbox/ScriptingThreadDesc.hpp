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
#ifndef CVS_SCRIPTING_THREAD_DESC_HPP
#define CVS_SCRIPTING_THREAD_DESC_HPP

#include <string>
#include <map>
#include <XGuid.hpp>
#include <XVariant.hpp>

class ScriptingThreadDesc
{
public:
    ScriptingThreadDesc( )
    { }

    ScriptingThreadDesc( const CVSandbox::XGuid& threadId, const std::string& name, uint32_t interval,
                         const CVSandbox::XGuid& pluginId, const std::string& description = "" ) :
        mThreadId( threadId ), mName( name ), mDescription( description ),
        mInterval( interval ), mPluginId( pluginId ), mPluginConfig( )
    {

    }

    // Check if two threads configuration is equal
    bool operator==( const ScriptingThreadDesc& rhs ) const
    {
        return ( ( mThreadId     == rhs.mThreadId     ) &&
                 ( mName         == rhs.mName         ) &&
                 ( mDescription  == rhs.mDescription  ) &&
                 ( mInterval     == rhs.mInterval     ) &&
                 ( mPluginId     == rhs.mPluginId     ) &&
                 ( mPluginConfig == rhs.mPluginConfig ) );
    }

    // Check if two threads configuration is not equal
    bool operator!=( const ScriptingThreadDesc& rhs ) const
    {
        return ( !( ( *this ) == rhs ) );
    }

    // ID of the configured thread
    CVSandbox::XGuid Id( ) const { return mThreadId; }

    // Thread's name
    std::string Name( ) const { return mName; }
    void SetName( const std::string& name ) { mName = name; }

    // Thread's description
    std::string Description( ) const { return mDescription; }
    void SetDescription( const std::string& description ) { mDescription = description; }

    // Thread's interval (ms), which tell how often to invoke its function
    uint32_t Interval( ) const { return mInterval; }
    void SetInterval( uint32_t interval ) { mInterval = interval; }

    // ID of the scripting plug-in
    CVSandbox::XGuid PluginId( ) const { return mPluginId; }

    // Configuration of the scripting plug-in
    std::map<std::string, CVSandbox::XVariant> PluginConfiguration( ) const { return mPluginConfig; }
    void SetPluginConfiguration( const std::map<std::string, CVSandbox::XVariant>& config ) { mPluginConfig = config; }

private:
    CVSandbox::XGuid mThreadId;
    std::string      mName;
    std::string      mDescription;
    uint32_t         mInterval;
    CVSandbox::XGuid mPluginId;

    std::map<std::string, CVSandbox::XVariant> mPluginConfig;
};

#endif // CVS_SCRIPTING_THREAD_DESC_HPP
