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

#include <xmodule.h>
#include "XModulesCollection.hpp"

using namespace std;
using namespace CVSandbox;

XModulesCollection::XModulesCollection( ) :
    mModules( )
{
}

XModulesCollection::~XModulesCollection( )
{
    Clear( );
}

// Create empty collection
const shared_ptr<XModulesCollection> XModulesCollection::Create( )
{
    return shared_ptr<XModulesCollection>( new XModulesCollection( ) );
}

// Clear collection
void XModulesCollection::Clear( )
{
    mModules.clear( );
}

// Create copy of the collection
const shared_ptr<XModulesCollection> XModulesCollection::Copy( ) const
{
    shared_ptr<XModulesCollection> modules = Create( );

    if ( modules )
    {
        for ( map<XGuid, shared_ptr<const XPluginsModule> >::const_iterator i = mModules.begin( ); i != mModules.end( ); i++ )
        {
            modules->mModules.insert( pair<XGuid, shared_ptr<const XPluginsModule> >( i->first, i->second ) );
        }
    }

    return modules;
}

// Add new module to the collection
void XModulesCollection::Add( const shared_ptr<const XPluginsModule>& module )
{
    if ( module )
    {
        // check if this collection already contains this family
        map<XGuid, shared_ptr<const XPluginsModule> >::const_iterator it = mModules.find( module->ID( ) );

        if  ( it == mModules.end( ) )
        {
            mModules.insert( pair<XGuid, shared_ptr<const XPluginsModule> >( module->ID( ), module ) );
        }
    }
}

// Get plug-in descriptor by its short name
const shared_ptr<const XPluginDescriptor> XModulesCollection::GetPlugin( const std::string& shortName ) const
{
    shared_ptr<const XPluginDescriptor> pluginDesc;

    for ( map<XGuid, shared_ptr<const XPluginsModule> >::const_iterator i = mModules.begin( );
        ( i != mModules.end( ) ) && ( !pluginDesc ); i++ )
    {
        pluginDesc = i->second->GetPlugin( shortName );
    }

    return pluginDesc;
}

// Get plug-in descriptor by its ID
const shared_ptr<const XPluginDescriptor> XModulesCollection::GetPlugin( const XGuid& id ) const
{
    shared_ptr<const XPluginDescriptor> pluginDesc;

    for ( map<XGuid, shared_ptr<const XPluginsModule> >::const_iterator i = mModules.begin( );
        ( i != mModules.end( ) ) && ( !pluginDesc ); ++i )
    {
        pluginDesc = i->second->GetPlugin( id );
    }

    return pluginDesc;
}

// Get plug-in descriptor by its ID. Also provides module descriptor for it.
const shared_ptr<const XPluginDescriptor> XModulesCollection::GetPlugin( const XGuid& id, shared_ptr<const XPluginsModule>& moduleDesc ) const
{
    shared_ptr<const XPluginDescriptor> pluginDesc;

    for ( map<XGuid, shared_ptr<const XPluginsModule> >::const_iterator i = mModules.begin( ); i != mModules.end( ); ++i )
    {
        pluginDesc = i->second->GetPlugin( id );

        if ( pluginDesc )
        {
            moduleDesc = i->second;
            break;
        }
    }

    return pluginDesc;
}

// Get module descriptor which contains the plug-in
const shared_ptr<const XPluginsModule> XModulesCollection::GetPluginModule( const XGuid& id ) const
{
    shared_ptr<const XPluginsModule> module;

    for ( map<XGuid, shared_ptr<const XPluginsModule> >::const_iterator i = mModules.begin( ); i != mModules.end( ); ++i )
    {
        if ( i->second->GetPlugin( id ) )
        {
            module = i->second;
            break;
        }
    }

    return module;

}

// Get module descriptor by its short name
const shared_ptr<const XPluginsModule> XModulesCollection::GetModule( const std::string& shortName ) const
{
    shared_ptr<const XPluginsModule> module;

    for ( map<XGuid, shared_ptr<const XPluginsModule> >::const_iterator i = mModules.begin( ); i != mModules.end( ); i++ )
    {
        if ( shortName == i->second->ShortName( ) )
        {
            module = i->second;
            break;
        }
    }

    return module;
}

// Get module descriptor by its ID
const shared_ptr<const XPluginsModule> XModulesCollection::GetModule( const XGuid& id ) const
{
    shared_ptr<const XPluginsModule> module;
    map<XGuid, shared_ptr<const XPluginsModule> >::const_iterator it = mModules.find( id );

    if  ( it != mModules.end( ) )
    {
        module = it->second;
    }

    return module;
}

// Get collection of plug-in descriptors of the specified types only from ALL modules
const shared_ptr<const XPluginsCollection> XModulesCollection::GetPluginsOfType( PluginType typesMask ) const
{
    shared_ptr<XPluginsCollection> collection = XPluginsCollection::Create( );

    if ( collection )
    {
        for ( map<XGuid, shared_ptr<const XPluginsModule> >::const_iterator i = mModules.begin( ); i != mModules.end( ); i++ )
        {
            collection->Add( i->second->GetPluginsOfType( typesMask ) );
        }
    }

    return collection;
}
