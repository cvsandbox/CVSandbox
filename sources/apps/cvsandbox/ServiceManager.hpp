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

#pragma once
#ifndef CVS_SERVICEMANAGER_HPP
#define CVS_SERVICEMANAGER_HPP

#include <memory>
#include <XPluginsEngine.hpp>
#include <XAutomationServer.hpp>
#include <IUIPersistenceService.hpp>
#include <IApplicationSettingsService.hpp>
#include <IHelpService.hpp>
#include <IImageFileService.hpp>
#include <FavouritePluginsManager.hpp>
#include "IMainWindowService.hpp"
#include "IProjectTreeUI.hpp"
#include "IProjectManager.hpp"
#include "IPerformanceMonitorService.hpp"

// Singleton class which is used to access some common application wide services
class ServiceManager
{
private:
    ServiceManager( );
    ServiceManager( const ServiceManager& ) { }

public:
    static ServiceManager& Instance( );

    // Get/Set main window service
    IMainWindowService* GetMainWindowService( ) const;
    void SetMainWindowService( IMainWindowService* service );

    // Get/Set application's settings service
    const std::shared_ptr<IApplicationSettingsService> GetSettingsService( ) const;
    void SetSettingsService( const std::shared_ptr<IApplicationSettingsService>& service );

    // Get/Set user interface persistence service
    const std::shared_ptr<IUIPersistenceService> GetUiPersistenceService( ) const;
    void SetUiPersistenceService( const std::shared_ptr<IUIPersistenceService>& service );

    // Get/Set image file service
    const std::shared_ptr<IImageFileService> GetImageFileService( ) const;
    void SetImageFileService( const std::shared_ptr<IImageFileService>& service );

    // Get/Set cameras' tree UI
    IProjectTreeUI* GetProjectTreeUI( ) const;
    void SetProjectTreeUI( IProjectTreeUI* ui );

    // Set/Get cameras collection manager
    const std::shared_ptr<IProjectManager> GetProjectManager( ) const;
    void SetProjectManager( const std::shared_ptr<IProjectManager>& manager );

    // Get/Set plug-ins' engine
    const std::shared_ptr<const XPluginsEngine> GetPluginsEngine( ) const;
    void SetPluginsEngine( const std::shared_ptr<const XPluginsEngine>& engine );

    // Get/Set automation server
    const std::shared_ptr<CVSandbox::Automation::XAutomationServer> GetAutomationServer( ) const;
    void SetAutomationServer( const std::shared_ptr<CVSandbox::Automation::XAutomationServer>& server );

    // Get/Set help service
    const std::shared_ptr<IHelpService> GetHelpService( ) const;
    void SetHelpService( const std::shared_ptr<IHelpService>& helpService );

    // Get/Set performance monitor
    const std::shared_ptr<IPerformanceMonitorService> GetPerformanceMonitor( ) const;
    void SetPerformanceMonitor( const std::shared_ptr<IPerformanceMonitorService>& performanceMonitor );

    // Get/Set favourite plug-ins manager
    const std::shared_ptr<FavouritePluginsManager> GetFavouritePluginsManager( ) const;
    void SetFavouritePluginsManager( const std::shared_ptr<FavouritePluginsManager>& favouritesManager );

private:
    IMainWindowService* mMainWindow;
    IProjectTreeUI*     mCamerasTreeUI;

    std::shared_ptr<IImageFileService>           mImageFileService;
    std::shared_ptr<IPerformanceMonitorService>  mPerformanceMonitor;
    std::shared_ptr<IProjectManager>             mProjectManager;
    std::shared_ptr<CVSandbox::Automation::XAutomationServer> mAutomationServer;
    std::shared_ptr<FavouritePluginsManager>     mFavouritePluginsManager;
};

#endif // CVS_SERVICEMANAGER_HPP
