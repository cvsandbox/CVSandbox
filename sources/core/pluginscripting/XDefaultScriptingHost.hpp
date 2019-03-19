/*
    Plug-ins' scripting library of Computer Vision Sandbox

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
#ifndef CVS_XDEFAULT_SCRIPTING_HOST_HPP
#define CVS_XDEFAULT_SCRIPTING_HOST_HPP

#include <vector>
#include "IScriptingHost.hpp"

namespace Private
{
    class XDefaultScriptingHostData;
}

class XDefaultScriptingHost : public IScriptingHost, private CVSandbox::Uncopyable
{
public:
    XDefaultScriptingHost( const std::map<std::string, std::string>& scriptArguments,
                           const std::string& pluginsLocation = "./cvsplugins/",
                           PluginType typesToLoad = PluginType_All );
    XDefaultScriptingHost( const std::map<std::string, std::string>& scriptArguments,
                           const std::vector<std::string>& pluginsLocation,
                           PluginType typesToLoad = PluginType_All );
    ~XDefaultScriptingHost( );

    virtual const std::string Name( ) const;
    virtual const CVSandbox::XVersion Version( ) const;
    virtual const std::map<std::string, std::string> GetArguments( ) const;

    virtual void PrintString( const std::string& message ) const;

    virtual XErrorCode CreatePluginInstance( const std::string& pluginName,
                                             std::shared_ptr<const XPluginDescriptor>& descriptor,
                                             std::shared_ptr<XPlugin>& plugin );

    virtual XErrorCode GetImage( std::shared_ptr<CVSandbox::XImage>& image ) const;
    virtual XErrorCode SetImage( const std::shared_ptr<CVSandbox::XImage>& image );

    virtual XErrorCode GetVariable( const std::string& name, CVSandbox::XVariant& value ) const;
    virtual XErrorCode SetVariable( const std::string& name, const CVSandbox::XVariant& value );

    virtual XErrorCode GetVariable( const std::string& name, std::shared_ptr<CVSandbox::XImage>& value ) const;
    virtual XErrorCode SetVariable( const std::string& name, const std::shared_ptr<CVSandbox::XImage>& value );

    virtual XErrorCode GetVideoSource( std::shared_ptr<const XPluginDescriptor>& descriptor,
                                       std::shared_ptr<XPlugin>& plugin ) const;

private:
    Private::XDefaultScriptingHostData* mData;
};

#endif // CVS_XDEFAULT_SCRIPTING_HOST_HPP
