/*
    Automation server library of Computer Vision Sandbox

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
#ifndef CVS_XVIDEO_SOURCE_PROCESSING_STEP_HPP
#define CVS_XVIDEO_SOURCE_PROCESSING_STEP_HPP

#include <string>
#include <map>
#include <XGuid.hpp>
#include <XVariant.hpp>
#include <XPluginsEngine.hpp>

namespace CVSandbox { namespace Automation
{

namespace Private
{
    class VideoSourceData;
}

class XAutomationServer;

class XVideoSourceProcessingStep
{
public:
    XVideoSourceProcessingStep( const std::string& name = std::string( ), const CVSandbox::XGuid& pluginId = CVSandbox::XGuid( ) );

    // Check if the step if valid
    bool IsValid( ) const;

    // Check if two processing steps are equal
    bool operator==( const XVideoSourceProcessingStep& rhs ) const;

    // Check if two processing steps are NOT equal
    bool operator!=( const XVideoSourceProcessingStep& rhs ) const
    {
        return ( !( ( *this ) == rhs ) );
    }

    // Get/Set the name of the video processing step
    const std::string& Name( ) const;
    void SetName( const std::string& name );

    // Get ID of the plug-in used as the processing step
    const CVSandbox::XGuid& PluginId( ) const;

    // Get/Set configuration of the plug-in
    const std::map<std::string, CVSandbox::XVariant>& PluginConfiguration( ) const;
    void SetPluginConfiguration( const std::map<std::string, CVSandbox::XVariant>& configuration );

private:
    friend class XAutomationServer;
    friend class Private::VideoSourceData;

    bool CreatePluginInstance( const std::shared_ptr<const XPluginsEngine>& pluginsEngine );
    const std::shared_ptr<XPlugin> GetPluginInstance( ) const;
    PluginType GetPluginType( ) const;

    // Get/Set configuration of the created plug-in instance, not the step's configuration
    const std::map<std::string, CVSandbox::XVariant> GetPluginInstanceConfiguration( ) const;
    void SetPluginInstanceConfiguration( const std::map<std::string, CVSandbox::XVariant>& configuration );

private:
    std::string                              mName;
    CVSandbox::XGuid                           mPluginId;
    std::map<std::string, CVSandbox::XVariant> mPluginConfiguration;

    std::shared_ptr<const XPluginDescriptor> mPluginDesc;
    std::shared_ptr<XPlugin>                 mPlugin;
};

} } // namespace CVSandbox::Automation

#endif // CVS_XVIDEO_SOURCE_PROCESSING_STEP_HPP
