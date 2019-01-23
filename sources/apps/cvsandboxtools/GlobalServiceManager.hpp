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

#pragma once
#ifndef CVS_GLOBAL_SERVICE_MANAGER_HPP
#define CVS_GLOBAL_SERVICE_MANAGER_HPP

#include <memory>
#include <XPluginsEngine.hpp>
#include "IUIPersistenceService.hpp"
#include "IHelpService.hpp"
#include "IApplicationSettingsService.hpp"

#include "cvsandboxtools_global.h"

class CVS_SHARED_EXPORT GlobalServiceManager
{
private:
    GlobalServiceManager( );
    GlobalServiceManager( const GlobalServiceManager& ) { }

public:
    static GlobalServiceManager& Instance( );

    // Get/Set user interface persistence service
    const std::shared_ptr<IUIPersistenceService> GetUiPersistenceService( ) const;
    void SetUiPersistenceService( const std::shared_ptr<IUIPersistenceService>& service );

    // Get/Set plug-ins' engine
    const std::shared_ptr<const XPluginsEngine> GetPluginsEngine( ) const;
    void SetPluginsEngine( const std::shared_ptr<const XPluginsEngine>& engine );

    // Get/Set help service
    const std::shared_ptr<IHelpService> GetHelpService( ) const;
    void SetHelpService( const std::shared_ptr<IHelpService>& helpService );

    // Get/Set application's settings service
    const std::shared_ptr<IApplicationSettingsService> GetSettingsService( ) const;
    void SetSettingsService( const std::shared_ptr<IApplicationSettingsService>& service );

private:
    std::shared_ptr<IUIPersistenceService>       mUiPersistenceService;
    std::shared_ptr<const XPluginsEngine>        mPluginsEngine;
    std::shared_ptr<IHelpService>                mHelpService;
    std::shared_ptr<IApplicationSettingsService> mSettingsService;
};

#endif // CVS_GLOBAL_SERVICE_MANAGER_HPP
