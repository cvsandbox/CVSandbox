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
#ifndef CVS_ISCRIPTING_HOST_HPP
#define CVS_ISCRIPTING_HOST_HPP

#include <string>
#include <map>
#include <xtypes.h>
#include <XVersion.hpp>
#include <XVariant.hpp>
#include <XPluginDescriptor.hpp>
#include <XPlugin.hpp>

class IScriptingHost
{
public:
    virtual ~IScriptingHost( ) { };

    virtual const std::string Name( ) const = 0;
    virtual const CVSandbox::XVersion Version( ) const = 0;
    virtual const std::map<std::string, std::string> GetArguments( ) const = 0;

    virtual void PrintString( const std::string& message ) const = 0;

    virtual XErrorCode CreatePluginInstance( const std::string& pluginName,
                                             std::shared_ptr<const XPluginDescriptor>& descriptor,
                                             std::shared_ptr<XPlugin>& plugin ) = 0;

    virtual XErrorCode GetImage( std::shared_ptr<CVSandbox::XImage>& image ) const = 0;
    virtual XErrorCode SetImage( const std::shared_ptr<CVSandbox::XImage>& image ) = 0;

    virtual XErrorCode GetVariable( const std::string& name, CVSandbox::XVariant& value ) const = 0;
    virtual XErrorCode SetVariable( const std::string& name, const CVSandbox::XVariant& value ) = 0;

    virtual XErrorCode GetVariable( const std::string& name, std::shared_ptr<CVSandbox::XImage>& value ) const = 0;
    virtual XErrorCode SetVariable( const std::string& name, const std::shared_ptr<CVSandbox::XImage>& value ) = 0;

    virtual XErrorCode GetVideoSource( std::shared_ptr<const XPluginDescriptor>& descriptor,
                                       std::shared_ptr<XPlugin>& plugin ) const = 0;
};

#endif // CVS_ISCRIPTING_HOST_HPP
