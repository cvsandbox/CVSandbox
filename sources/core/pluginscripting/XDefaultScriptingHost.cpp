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

#include <stdio.h>
#include <string>
#include <algorithm>
#include <xtypes.h>
#include <XVersion.hpp>
#include <XPluginsEngine.hpp>
#include <XImageImportingPlugin.hpp>
#include <XImageExportingPlugin.hpp>

#include "XDefaultScriptingHost.hpp"

using namespace std;
using namespace CVSandbox;

namespace Private
{
    class XDefaultScriptingHostData
    {
    public:
        XDefaultScriptingHostData( const map<string, string>& scriptArguments ) :
            PluginsEngine( XPluginsEngine::Create( ) ), ScriptArguments( scriptArguments )
        {
        }

    public:
        shared_ptr<XPluginsEngine>       PluginsEngine;
        map<string, string>              ScriptArguments;
        map<string, XVariant>            VariantVariables;
        map<string, shared_ptr<XImage> > ImageVariables;
    };
}


XDefaultScriptingHost::XDefaultScriptingHost( const map<string, string>& scriptArguments, const string& pluginsLocation, PluginType typesToLoad ) :
    mData( new Private::XDefaultScriptingHostData( scriptArguments ) )
{
    mData->PluginsEngine->CollectModules( pluginsLocation, typesToLoad | PluginType_ImageImporter | PluginType_ImageExporter );
}

XDefaultScriptingHost::XDefaultScriptingHost( const map<string, string>& scriptArguments, const vector<string>& pluginsLocations, PluginType typesToLoad ) :
    mData( new Private::XDefaultScriptingHostData( scriptArguments ) )
{
    for ( auto folder : pluginsLocations )
    {
        mData->PluginsEngine->CollectModules( folder, typesToLoad | PluginType_ImageImporter | PluginType_ImageExporter );
    }
}

XDefaultScriptingHost::~XDefaultScriptingHost( )
{
    delete mData;
}

const string XDefaultScriptingHost::Name( ) const
{
    return string( "Default Scripting Host" );
}

const XVersion XDefaultScriptingHost::Version( ) const
{
    return XVersion( 1, 0, 0 );
}

const map<string, string> XDefaultScriptingHost::GetArguments( ) const
{
    return mData->ScriptArguments;
}

void XDefaultScriptingHost::PrintString( const string& message ) const
{
    printf( "%s\n", message.c_str( ) );
}

XErrorCode XDefaultScriptingHost::CreatePluginInstance( const string& pluginName,
                                                        shared_ptr<const XPluginDescriptor>& descriptor,
                                                        shared_ptr<XPlugin>& plugin )
{
    XErrorCode ret = SuccessCode;

    size_t dotIndex = pluginName.find( '.' );

    descriptor.reset( );
    plugin.reset( );

    if ( dotIndex == string::npos )
    {
        descriptor = mData->PluginsEngine->GetPlugin( pluginName );
    }
    else
    {
        string moduleName = pluginName.substr( 0, dotIndex );
        string pluginNameOnly = pluginName.substr( dotIndex + 1 );

        shared_ptr<const XPluginsModule> module = mData->PluginsEngine->GetModule( moduleName );
        if ( module )
        {
            descriptor = module->GetPlugin( pluginNameOnly );
        }
    }

    if ( !descriptor )
    {
        ret = ErrorPluginNotFound;
    }
    else
    {
        plugin = descriptor->CreateInstance( );

        if ( !plugin )
        {
            ret = ErrorFailedPluginInstantiation;
            descriptor.reset( );
        }
    }

    return ret;
}

XErrorCode XDefaultScriptingHost::GetImage( shared_ptr<XImage>& image ) const
{
    XErrorCode  ret      = ErrorFailed;
    auto        searchIt = mData->ScriptArguments.find( "-i" );

    image.reset( );

    if ( searchIt != mData->ScriptArguments.end( ) )
    {
        string fileName = searchIt->second;
        size_t dotIndex = fileName.rfind( '.' );

        if ( dotIndex != string::npos )
        {
            string                               fileExtension  = fileName.substr( dotIndex + 1 );
            shared_ptr<const XPluginsCollection> imageImporters = mData->PluginsEngine->GetPluginsOfType( PluginType_ImageImporter );

            for ( auto iter = imageImporters->begin( ); iter != imageImporters->end( ); ++iter )
            {
                shared_ptr<XImageImportingPlugin> plugin = static_pointer_cast<XImageImportingPlugin>( ( *iter )->CreateInstance( ) );

                if ( plugin )
                {
                    vector<string> supportedExtension = plugin->GetSupportedExtensions( );

                    if ( std::find( supportedExtension.begin( ), supportedExtension.end( ), fileExtension ) != supportedExtension.end( ) )
                    {
                        ret = plugin->ImportImage( fileName, image );
                        break;
                    }
                }
            }
        }
    }

    return ret;
}

XErrorCode XDefaultScriptingHost::SetImage( const shared_ptr<XImage>& image )
{
    XErrorCode  ret      = ErrorFailed;
    auto        searchIt = mData->ScriptArguments.find( "-o" );
    string      fileName;

    if ( searchIt != mData->ScriptArguments.end( ) )
    {
        fileName = searchIt->second;
    }
    else
    {
        searchIt = mData->ScriptArguments.find( "-i" );

        if ( searchIt != mData->ScriptArguments.end( ) )
        {
            fileName = searchIt->second;

            size_t dotIndex = fileName.rfind( '.' );

            if ( dotIndex != string::npos )
            {
                fileName.insert( dotIndex, "-out" );
            }
        }
    }

    if ( !fileName.empty( ) )
    {
        size_t dotIndex = fileName.rfind( '.' );

        if ( dotIndex != string::npos )
        {
            string                               fileExtension = fileName.substr( dotIndex + 1 );
            shared_ptr<const XPluginsCollection> imageImporters = mData->PluginsEngine->GetPluginsOfType( PluginType_ImageExporter );

            for ( auto iter = imageImporters->begin( ); iter != imageImporters->end( ); ++iter )
            {
                shared_ptr<XImageExportingPlugin> plugin = static_pointer_cast<XImageExportingPlugin>( ( *iter )->CreateInstance( ) );

                if ( plugin )
                {
                    vector<string> supportedExtension = plugin->GetSupportedExtensions( );

                    if ( std::find( supportedExtension.begin( ), supportedExtension.end( ), fileExtension ) != supportedExtension.end( ) )
                    {
                        ret = plugin->ExportImage( fileName, image );
                        break;
                    }
                }
            }
        }
    }

    return ret;
}

XErrorCode XDefaultScriptingHost::GetVariable( const string& name, XVariant& value ) const
{
    value.SetEmpty( );

    map<string, XVariant>::const_iterator it = mData->VariantVariables.find( name );

    if ( it != mData->VariantVariables.end( ) )
    {
        value = it->second;
    }

    return SuccessCode;
}

XErrorCode XDefaultScriptingHost::SetVariable( const string& name, const XVariant& value )
{
    if ( value.IsNullOrEmpty( ) )
    {
        mData->VariantVariables.erase( name );
    }
    else
    {
        mData->VariantVariables[name] = value;
    }

    // remove any image variable with such name
    mData->ImageVariables.erase( name );

    return SuccessCode;
}

XErrorCode XDefaultScriptingHost::GetVariable( const string& name, shared_ptr<XImage>& value ) const
{
    value.reset( );

    map<string, shared_ptr<XImage> >::const_iterator it = mData->ImageVariables.find( name );

    if ( it != mData->ImageVariables.end( ) )
    {
        value = it->second->Clone( );
    }

    return SuccessCode;
}

XErrorCode XDefaultScriptingHost::SetVariable( const string& name, const shared_ptr<XImage>& value )
{
    if ( !value )
    {
        mData->ImageVariables.erase( name );
    }
    else
    {
        map<string, shared_ptr<XImage> >::iterator it = mData->ImageVariables.find( name );

        if ( it == mData->ImageVariables.end( ) )
        {
            mData->ImageVariables.insert( pair<string, shared_ptr<XImage> >( name, value->Clone( ) ) );
        }
        else
        {
            value->CopyDataOrClone( it->second );
        }
    }

    // remove any variant variable with such name
    mData->VariantVariables.erase( name );

    return SuccessCode;
}

XErrorCode XDefaultScriptingHost::GetVideoSource( shared_ptr<const XPluginDescriptor>& descriptor,
                                                  shared_ptr<XPlugin>& plugin ) const
{
    descriptor.reset( );
    plugin.reset( );

    return ErrorNotImplemented;
}
