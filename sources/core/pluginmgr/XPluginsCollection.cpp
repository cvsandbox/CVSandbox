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

#include "XPluginsCollection.hpp"

using namespace std;
using namespace CVSandbox;

XPluginsCollection::XPluginsCollection( ) :
    mPlugins( )
{
}

XPluginsCollection::~XPluginsCollection( )
{
    Clear( );
}

const shared_ptr<XPluginsCollection> XPluginsCollection::Create( )
{
    return shared_ptr<XPluginsCollection>( new XPluginsCollection( ) );
}

void XPluginsCollection::Clear( )
{
    mPlugins.clear( );
}

// Collect plug-ins given a pointer to a function which provides plug-in descriptors
size_t XPluginsCollection::CollectPlugins( GetDescriptorFunc pluginsNest, int32_t count, PluginType typesToCollec )
{
    Clear( );

    for ( int32_t i = 0; i < count; i++ )
    {
        PluginDescriptor* desc = pluginsNest( i );

        if ( desc != 0 )
        {
            shared_ptr<const XPluginDescriptor> descriptor = XPluginDescriptor::Create( desc );

            if ( descriptor->Type( ) & typesToCollec )
            {
                Add( descriptor );
            }
        }
    }

    return mPlugins.size( );
}

// Create copy of the collection
const shared_ptr<XPluginsCollection> XPluginsCollection::Copy( ) const
{
    shared_ptr<XPluginsCollection> plugins = Create( );

    if ( plugins )
    {
        for ( map<XGuid, shared_ptr<const XPluginDescriptor> >::const_iterator i = mPlugins.begin( ); i != mPlugins.end( ); i++ )
        {
            plugins->mPlugins.insert( pair<xguid, shared_ptr<const XPluginDescriptor> >( i->first, i->second ) );
        }
    }

    return plugins;
}

// Add new plug-in descriptor to the collection
void XPluginsCollection::Add( const shared_ptr<const XPluginDescriptor>& plugin )
{
    if ( plugin )
    {
        // check if this collection already contains this plug-in
        map<XGuid, shared_ptr<const XPluginDescriptor> >::const_iterator it = mPlugins.find( plugin->ID( ) );

        if  ( it == mPlugins.end( ) )
        {
            mPlugins.insert( pair<xguid, shared_ptr<const XPluginDescriptor> >( plugin->ID( ), plugin ) );
        }
    }
}

// Copy plug-in descriptors from the specified collection into this collection
void XPluginsCollection::Add( const shared_ptr<const XPluginsCollection>& plugins )
{
    if ( plugins )
    {
        shared_ptr<const XPluginDescriptor> plugin;

        for ( XPluginsCollection::ConstIterator it = plugins->begin( ); it != plugins->end( ); it++ )
        {
            Add( *it );
        }
    }
}

// Remove plug-in descriptor from the collection
void XPluginsCollection::Remove( const XGuid& id )
{
    mPlugins.erase( id );
}

// Number of plug-in descriptors of the specified type
size_t XPluginsCollection::CountType( PluginType typeMask ) const
{
    size_t count = 0;

    for ( map<XGuid, shared_ptr<const XPluginDescriptor> >::const_iterator i = mPlugins.begin( ); i != mPlugins.end( ); i++ )
    {
        if ( ( typeMask & i->second->Type( ) ) != 0 )
            count++;
    }

    return count;
}

// Get plug-in descriptor by its short name
const shared_ptr<const XPluginDescriptor> XPluginsCollection::GetPlugin( const string& shortName ) const
{
    shared_ptr<const XPluginDescriptor> pluginDesc;

    for ( map<XGuid, shared_ptr<const XPluginDescriptor> >::const_iterator i = mPlugins.begin( ); i != mPlugins.end( ); i++ )
    {
        if ( shortName == i->second->ShortName( ) )
        {
            pluginDesc = i->second;
            break;
        }
    }

    return pluginDesc;
}

// Get plug-in descriptor by its ID
const shared_ptr<const XPluginDescriptor> XPluginsCollection::GetPlugin( const XGuid& id ) const
{
    shared_ptr<const XPluginDescriptor> pluginDesc;
    map<XGuid, shared_ptr<const XPluginDescriptor> >::const_iterator it = mPlugins.find( id );

    if  ( it != mPlugins.end( ) )
    {
        pluginDesc = it->second;
    }

    return pluginDesc;
}

// Get collection of plug-in descriptors of the specified types only
const shared_ptr<const XPluginsCollection> XPluginsCollection::GetPluginsOfType( PluginType typesMask ) const
{
    shared_ptr<XPluginsCollection> collection = XPluginsCollection::Create( );

    if ( collection )
    {
        for ( map<XGuid, shared_ptr<const XPluginDescriptor> >::const_iterator i = mPlugins.begin( ); i != mPlugins.end( ); i++ )
        {
            shared_ptr<const XPluginDescriptor> plugin = i->second;

            if ( ( typesMask & plugin->Type( ) ) != 0 )
            {
                collection->Add( plugin );
            }
        }
    }

    return collection;
}
