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

#include <assert.h>
#include "GlobalServiceManager.hpp"

using namespace std;

GlobalServiceManager::GlobalServiceManager( ) :
    mUiPersistenceService( nullptr ),
    mPluginsEngine( nullptr ),
    mHelpService( nullptr )
{
}

// Get instance of the service manager
GlobalServiceManager& GlobalServiceManager::Instance( )
{
    static GlobalServiceManager instance;
    return instance;
}


// Get/Set user interface persistence service
const shared_ptr<IUIPersistenceService> GlobalServiceManager::GetUiPersistenceService( ) const
{
    assert( mUiPersistenceService );
    return mUiPersistenceService;
}
void GlobalServiceManager::SetUiPersistenceService( const shared_ptr<IUIPersistenceService>& service )
{
    mUiPersistenceService = service;
}

// Get/Set plug-ins' engine
const shared_ptr<const XPluginsEngine> GlobalServiceManager::GetPluginsEngine( ) const
{
    assert( mPluginsEngine );
    return mPluginsEngine;
}
void GlobalServiceManager::SetPluginsEngine( const shared_ptr<const XPluginsEngine>& engine )
{
    mPluginsEngine = engine;
}

// Get/Set help service
const shared_ptr<IHelpService> GlobalServiceManager::GetHelpService( ) const
{
    assert( mHelpService );
    return mHelpService;
}
void GlobalServiceManager::SetHelpService( const shared_ptr<IHelpService>& helpService )
{
    mHelpService = helpService;
}

// Get/Set application's settings service
const shared_ptr<IApplicationSettingsService> GlobalServiceManager::GetSettingsService( ) const
{
    assert( mSettingsService );
    return mSettingsService;
}
void GlobalServiceManager::SetSettingsService( const shared_ptr<IApplicationSettingsService>& service )
{
    mSettingsService = service;
}
