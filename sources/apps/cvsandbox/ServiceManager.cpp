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

#include <assert.h>
#include <GlobalServiceManager.hpp>
#include "ServiceManager.hpp"

using namespace std;
using namespace CVSandbox;
using namespace CVSandbox::Automation;

ServiceManager::ServiceManager( ) :
    mMainWindow( nullptr ),
    mCamerasTreeUI( nullptr ),
    mPerformanceMonitor( nullptr ),
    mProjectManager( nullptr ),
    mAutomationServer( nullptr ),
    mFavouritePluginsManager( nullptr )
{
}

// Get instance of the service manager
ServiceManager& ServiceManager::Instance( )
{
    static ServiceManager instance;
    return instance;
}

// Get/Set main window service
IMainWindowService* ServiceManager::GetMainWindowService( ) const
{
    assert( mMainWindow );
    return mMainWindow;
}
void ServiceManager::SetMainWindowService( IMainWindowService* service )
{
    mMainWindow = service;
}

// Get/Set application's settings service
const shared_ptr<IApplicationSettingsService> ServiceManager::GetSettingsService( ) const
{
    return GlobalServiceManager::Instance( ).GetSettingsService( );
}
void ServiceManager::SetSettingsService( const shared_ptr<IApplicationSettingsService>& service )
{
    GlobalServiceManager::Instance( ).SetSettingsService( service );
}

// Get/Set user interface persistence service
const shared_ptr<IUIPersistenceService> ServiceManager::GetUiPersistenceService( ) const
{
    return GlobalServiceManager::Instance( ).GetUiPersistenceService( );
}
void ServiceManager::SetUiPersistenceService( const shared_ptr<IUIPersistenceService>& service )
{
    GlobalServiceManager::Instance( ).SetUiPersistenceService( service );
}

// Get/Set image file service
const shared_ptr<IImageFileService> ServiceManager::GetImageFileService( ) const
{
    assert( mImageFileService );
    return mImageFileService;
}
void ServiceManager::SetImageFileService( const shared_ptr<IImageFileService>& service )
{
    mImageFileService = service;
}

// Get/Set cameras' tree UI
IProjectTreeUI* ServiceManager::GetProjectTreeUI( ) const
{
    assert( mCamerasTreeUI );
    return mCamerasTreeUI;
}
void ServiceManager::SetProjectTreeUI( IProjectTreeUI* ui )
{
    mCamerasTreeUI = ui;
}

// Set/Get cameras collection manager
const shared_ptr<IProjectManager> ServiceManager::GetProjectManager( ) const
{
    assert( mProjectManager );
    return mProjectManager;
}
void ServiceManager::SetProjectManager( const shared_ptr<IProjectManager>& manager )
{
    mProjectManager = manager;
}

// Get/Set plug-ins' engine
const shared_ptr<const XPluginsEngine> ServiceManager::GetPluginsEngine( ) const
{
    return GlobalServiceManager::Instance( ).GetPluginsEngine( );
}
void ServiceManager::SetPluginsEngine( const shared_ptr<const XPluginsEngine>& engine )
{
    GlobalServiceManager::Instance( ).SetPluginsEngine( engine );
}

// Get/Set automation server
const shared_ptr<XAutomationServer> ServiceManager::GetAutomationServer( ) const
{
    assert( mAutomationServer );
    return mAutomationServer;
}
void ServiceManager::SetAutomationServer( const shared_ptr<XAutomationServer>& server )
{
    mAutomationServer = server;
}

// Get/Set help service
const std::shared_ptr<IHelpService> ServiceManager::GetHelpService( ) const
{
    return GlobalServiceManager::Instance( ).GetHelpService( );
}
void ServiceManager::SetHelpService( const std::shared_ptr<IHelpService>& helpService )
{
    GlobalServiceManager::Instance( ).SetHelpService( helpService );
}

// Get/Set performance monitor
const shared_ptr<IPerformanceMonitorService> ServiceManager::GetPerformanceMonitor( ) const
{
    assert( mPerformanceMonitor );
    return mPerformanceMonitor;
}
void ServiceManager::SetPerformanceMonitor( const shared_ptr<IPerformanceMonitorService>& performanceMonitor )
{
    mPerformanceMonitor = performanceMonitor;
}

// Get/Set favourite plug-ins manager
const shared_ptr<FavouritePluginsManager> ServiceManager::GetFavouritePluginsManager( ) const
{
    assert( mFavouritePluginsManager );
    return mFavouritePluginsManager;
}
void ServiceManager::SetFavouritePluginsManager( const shared_ptr<FavouritePluginsManager>& favouritesManager )
{
    mFavouritePluginsManager = favouritesManager;
}
