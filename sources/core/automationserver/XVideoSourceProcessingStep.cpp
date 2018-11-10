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

#include "XVideoSourceProcessingStep.hpp"

using namespace std;
using namespace CVSandbox;

namespace CVSandbox { namespace Automation
{

XVideoSourceProcessingStep::XVideoSourceProcessingStep( const string& name, const XGuid& pluginId ) :
    mName( name ), mPluginId( pluginId )
{
}

// Check if the step if valid
bool XVideoSourceProcessingStep::IsValid( ) const
{
    return ( !mPluginId.IsEmpty( ) );
}

// Check if two processing steps are equal
bool XVideoSourceProcessingStep::operator==( const XVideoSourceProcessingStep& rhs ) const
{
    return ( ( mName == rhs.mName ) && ( mPluginId == rhs.mPluginId ) && ( mPluginConfiguration == rhs.mPluginConfiguration ) );
}

// Get/Set the name of the video processing step
const string& XVideoSourceProcessingStep::Name( ) const
{
    return mName;
}
void XVideoSourceProcessingStep::SetName( const string& name )
{
    mName = name;
}

// Get ID of the plug-in used as the processing step
const XGuid& XVideoSourceProcessingStep::PluginId( ) const
{
    return mPluginId;
}

// Get/Set configuration of the plug-in
const map<string, XVariant>& XVideoSourceProcessingStep::PluginConfiguration( ) const
{
    return mPluginConfiguration;
}
void XVideoSourceProcessingStep::SetPluginConfiguration( const map<string, XVariant>& configuration )
{
    mPluginConfiguration = configuration;
}

// Create plug-in's instance for the video processing step
bool XVideoSourceProcessingStep::CreatePluginInstance( const std::shared_ptr<const XPluginsEngine>& pluginsEngine )
{
    mPluginDesc = pluginsEngine->GetPlugin( mPluginId );

    mPlugin.reset( );

    if ( mPluginDesc )
    {
        shared_ptr<XPlugin> plugin = mPluginDesc->CreateInstance( );

        if ( plugin )
        {
            mPlugin = plugin;

            // ignore return value - there is not much we can do if some configuration failed
            mPluginDesc->SetPluginConfiguration( plugin, mPluginConfiguration );
        }
        else
        {
            mPluginDesc.reset( );
        }
    }

    return ( (bool) mPlugin );
}

const shared_ptr<XPlugin> XVideoSourceProcessingStep::GetPluginInstance( ) const
{
    return mPlugin;
}

PluginType XVideoSourceProcessingStep::GetPluginType( ) const
{
    PluginType ret = PluginType_Unknown;

    if ( mPluginDesc )
    {
        ret = mPluginDesc->Type( );
    }

    return ret;
}

const map<string, XVariant> XVideoSourceProcessingStep::GetPluginInstanceConfiguration( ) const
{
    map<string, XVariant> configuration;

    if ( ( mPluginDesc ) && ( mPlugin ) )
    {
        configuration = mPluginDesc->GetPluginConfiguration( mPlugin );
    }

    return configuration;
}

void XVideoSourceProcessingStep::SetPluginInstanceConfiguration( const std::map<std::string, CVSandbox::XVariant>& configuration )
{
    if ( ( mPluginDesc ) && ( mPlugin ) )
    {
        mPluginDesc->SetPluginConfiguration( mPlugin, configuration );
    }
}

} } // namespace CVSandbox::Automation
