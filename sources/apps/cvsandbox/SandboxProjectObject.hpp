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
#ifndef CVS_SANDBOX_PROJECT_OBJECT_HPP
#define CVS_SANDBOX_PROJECT_OBJECT_HPP

#include <string>
#include <memory>
#include <map>
#include <XVideoSourceProcessingGraph.hpp>
#include "ProjectObject.hpp"
#include "SandboxSettings.hpp"
#include "ScriptingThreadDesc.hpp"

class ProjectObjectFactory;
class CamerasViewConfiguration;

class SandboxProjectObject : public ProjectObject
{
    friend class ProjectObjectFactory;

private:
    SandboxProjectObject(
            const CVSandbox::XGuid& id = CVSandbox::XGuid( ),
            const std::string& name = std::string( ), const std::string& description = std::string( ),
            const CVSandbox::XGuid& parentId = CVSandbox::XGuid( ) );

public:
    static std::shared_ptr<SandboxProjectObject> CreateNew(
            const std::string& name, const std::string& description = std::string( ),
            const CVSandbox::XGuid& parentId = CVSandbox::XGuid( ) );

    // Get/Set device list for the sandbox
    std::vector<CVSandbox::XGuid> GetSandboxDevices( ) const;
    void SetSandboxDevices( const std::vector<CVSandbox::XGuid>& devices );

    // Get/Set FPS report state for the devices of the sandbox (subject to change if more settings come)
    std::map<CVSandbox::XGuid, bool> GetDeviceFpsReportState( ) const;
    void SetDeviceFpsReportState( std::map<CVSandbox::XGuid, bool> fpsState );

    // Get/Set list of views
    std::vector<CamerasViewConfiguration> GetViews( ) const;
    void SetViews( const std::vector<CamerasViewConfiguration>& views );

    // Get/Set default view ID
    CVSandbox::XGuid DefaultViewId( ) const;
    void SetDefaultViewId( const CVSandbox::XGuid& viewId );

    // Get/Set sanbox settings
    SandboxSettings Settings( ) const;
    void SetSettings( const SandboxSettings& settings );

    // Get/Set video processing graphs
    std::map<CVSandbox::XGuid, CVSandbox::Automation::XVideoSourceProcessingGraph> GetCamerasProcessingGraphs( ) const;
    void SetCamerasProcessingGraphs( const std::map<CVSandbox::XGuid, CVSandbox::Automation::XVideoSourceProcessingGraph>& processingGraphs );

    // Get/set scripting threads configuration
    std::map<CVSandbox::XGuid, ScriptingThreadDesc> GetScriptingThreads( ) const;
    void SetScriptingThreads( const std::map<CVSandbox::XGuid, ScriptingThreadDesc>& threads );

public:
    virtual void Save( QXmlStreamWriter& xmlWriter ) const;
    virtual bool Load( QXmlStreamReader& xmlReader );

private:
    void SaveDevices( QXmlStreamWriter& xmlWriter ) const;
    bool LoadDevices( QXmlStreamReader& xmlReader );
    bool LoadViews( QXmlStreamReader& xmlReader );

private:
    std::vector<CVSandbox::XGuid>         mDevices;
    std::map<CVSandbox::XGuid, bool>      mDevicesFpsReport;
    std::vector<CamerasViewConfiguration> mViews;
    CVSandbox::XGuid                      mDefaultViewId;
    SandboxSettings                       mSandboxSettings;

    std::map<CVSandbox::XGuid, ScriptingThreadDesc>                              mScriptingThreads;
    std::map<CVSandbox::XGuid, CVSandbox::Automation::XVideoSourceProcessingGraph> mProcessingGraphs;

};

#endif // CVS_SANDBOX_PROJECT_OBJECT_HPP
