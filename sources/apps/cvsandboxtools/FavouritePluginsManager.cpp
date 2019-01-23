/*
    Common tools for Computer Vision Sandbox

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

#include "FavouritePluginsManager.hpp"
#include "GlobalServiceManager.hpp"
#include <XGuid.hpp>

using namespace std;
using namespace CVSandbox;

static const QString FAVOURITE_PLUGINS( "FavouritePlugins" );

FavouritePluginsManager::FavouritePluginsManager( ) :
    mPlugins( XPluginsCollection::Create( ) )
{
}

void FavouritePluginsManager::Load( const std::shared_ptr<const XModulesCollection>& modulesCollection )
{
    shared_ptr<IApplicationSettingsService> settingsService = GlobalServiceManager::Instance( ).GetSettingsService( );

    for ( int i = 0; ; ++i )
    {
        QString value = settingsService->GetValue( FAVOURITE_PLUGINS, QString( "%0").arg( i ) ).toString( );

        if ( value.isEmpty( ) )
        {
            break;
        }
        else
        {
            XGuid id = XGuid::FromString( value.toStdString( ) );

            if ( !id.IsEmpty( ) )
            {
                shared_ptr<const XPluginDescriptor> plugin = modulesCollection->GetPlugin( id );

                if ( plugin )
                {
                    mPlugins->Add( plugin );
                }
            }
        }
    }
}

void FavouritePluginsManager::Save( )
{
    shared_ptr<IApplicationSettingsService> settingsService = GlobalServiceManager::Instance( ).GetSettingsService( );
    int counter = 0;

    for ( auto it = mPlugins->begin( ); it != mPlugins->end( ); ++it, ++counter )
    {
        settingsService->SetValue( FAVOURITE_PLUGINS, QString( "%0").arg( counter ), (*it)->ID( ).ToString( ).c_str( ) );
    }
}
