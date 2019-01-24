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

#include <map>
#include <UITools.hpp>
#include "ProjectObjectOpener.hpp"
#include "ServiceManager.hpp"
#include "VideoSourceInAutomationServer.hpp"

#include "ProjectObject.hpp"
#include "CameraProjectObject.hpp"
#include "SandboxProjectObject.hpp"
#include "CamerasViewConfiguration.hpp"

#include "SingleCameraView.hpp"
#include "SandboxView.hpp"

using namespace std;
using namespace CVSandbox;
using namespace CVSandbox::Automation;

// Open generic project object
void ProjectObjectOpener::Open( const shared_ptr<ProjectObject>& po )
{
    if ( po->Type( ) == ProjectObjectType::Camera )
    {
        Open( static_pointer_cast<CameraProjectObject>( po ) );
    }
    else if ( po->Type( ) == ProjectObjectType::Sandbox )
    {
        Open( static_pointer_cast<SandboxProjectObject>( po ) );
    }
    else
    {
        Q_ASSERT( false );
    }
}

// Open camera project object
void ProjectObjectOpener::Open( const shared_ptr<CameraProjectObject>& po )
{
    IMainWindowService* mainWindowService = ServiceManager::Instance( ).GetMainWindowService( );
    QWidget*            mainWindow        = mainWindowService->GetMainAppWindow( );

    if ( mainWindowService->CentralWidgetsObjectId( ) != po->Id( ) )
    {
        // get description of the plug-in
        shared_ptr<const XPluginDescriptor> pluginDesc =
            ServiceManager::Instance( ).GetPluginsEngine( )->GetPlugin( po->PluginId( ) );

        if ( pluginDesc )
        {
            if ( pluginDesc->Type( ) == PluginType_VideoSource )
            {
                // create instance of the plug-in
                shared_ptr<XPlugin> plugin = pluginDesc->CreateInstance( );

                if ( plugin )
                {
                    shared_ptr<XAutomationServer> server = ServiceManager::Instance( ).GetAutomationServer( );
                    uint32_t                      vsId   = server->AddVideoSource( pluginDesc, static_pointer_cast<XVideoSourcePlugin>( plugin ) );

                    // set configuration of the plug-in
                    pluginDesc->SetPluginConfiguration( plugin, po->PluginProperties( ) );

                    // wrap video source managed by automation server into IVideoSourceInterface
                    shared_ptr<VideoSourceInAutomationServer> videoSource = VideoSourceInAutomationServer::Create( server, vsId );

                    SingleCameraView* cameraView = new SingleCameraView( po->Id( ), mainWindow );
                    cameraView->SetVideoSource( videoSource );

                    mainWindowService->SetCentralWidget( cameraView );

                    // finally start the video source in automation server
                    server->StartVideoSource( vsId );

                    // start monitoring performance of the video source
                    ServiceManager::Instance( ).GetPerformanceMonitor( )->Start( vsId );
                }
                else
                {
                    UITools::ShowErrorMessage( QString( "Cannot start the [<b>%0</b>] video source due to failure in creating instance of the video source plug-in." ).
                                               arg( QString::fromUtf8( po->Name( ).c_str( ) ) ), mainWindow );
                }
            }
            else
            {
                UITools::ShowErrorMessage( QString( "<b>The plug-in for this camera object is not of video source type.</b><br>"
                                                    "Project file or the plug-in might be broken.<br><br>"
                                                    "The plug-in name: %0" ).arg( QString::fromUtf8( pluginDesc->Name( ).c_str( ) ) ), mainWindow );
            }
        }
        else
        {
            UITools::ShowErrorMessage( QString( "Cannot start the [<b>%0</b>] video source because its plug-in was not found." ).
                                       arg( QString::fromUtf8( po->Name( ).c_str( ) ) ), mainWindow );
        }
    }
}

// Open sandbox project object
void ProjectObjectOpener::Open( const shared_ptr<SandboxProjectObject>& po )
{
    IMainWindowService*  mainWindowService = ServiceManager::Instance( ).GetMainWindowService( );
    QWidget*             mainWindow        = mainWindowService->GetMainAppWindow( );

    if ( mainWindowService->CentralWidgetsObjectId( ) != po->Id( ) )
    {
        vector<XGuid>                   sandboxDevices        = po->GetSandboxDevices( );
        map<XGuid, ScriptingThreadDesc> sandboxThreads        = po->GetScriptingThreads( );
        map<XGuid, bool>                devicesReportFpsState = po->GetDeviceFpsReportState( );
        bool                            devicesAreFine        = true;
        bool                            threadsAreFine        = true;

        const shared_ptr<const IProjectManager>& projectManager = ServiceManager::Instance( ).GetProjectManager( );
        const shared_ptr<const XPluginsEngine>&  pluginsEngine  = ServiceManager::Instance( ).GetPluginsEngine( );
        const shared_ptr<XAutomationServer>&     server         = ServiceManager::Instance( ).GetAutomationServer( );

        const map<XGuid, XVideoSourceProcessingGraph> videoProcessingGraphs = po->GetCamerasProcessingGraphs( );

        map<XGuid, uint32_t> videoSourceMap;
        map<XGuid, uint32_t> threadsMap;
        vector<uint32_t>     videoSourceIds;
        map<uint32_t, bool>  videoSourceReportFpsState;

        videoSourceIds.reserve( sandboxDevices.size( ) );

        // prepare all video sources
        for ( vector<XGuid>::const_iterator it = sandboxDevices.begin( ); it != sandboxDevices.end( ); ++it )
        {
            const shared_ptr<ProjectObject>& deviceObject = projectManager->GetProjectObject( *it );

            if ( ( deviceObject ) && ( deviceObject->Type( ) == ProjectObjectType::Camera ) )
            {
                const shared_ptr<CameraProjectObject>& cameraObject  = static_pointer_cast<CameraProjectObject>( deviceObject );
                shared_ptr<const XPluginDescriptor>    pluginDesc    = pluginsEngine->GetPlugin( cameraObject->PluginId( ) );
                bool                                   pluginCreated = false;

                if ( ( pluginDesc ) && ( pluginDesc->Type( ) == PluginType_VideoSource ) )
                {
                    // create instance of the plug-in
                    shared_ptr<XPlugin> plugin = pluginDesc->CreateInstance( );

                    if ( plugin )
                    {
                        uint32_t vsId           = server->AddVideoSource( pluginDesc, static_pointer_cast<XVideoSourcePlugin>( plugin ) );
                        bool     reportFpsState = true;

                        // check report FPS state for the video source
                        map<XGuid, bool>::const_iterator itReportFps = devicesReportFpsState.find( *it );
                        if ( itReportFps != devicesReportFpsState.end( ) )
                        {
                            reportFpsState = itReportFps->second;
                        }

                        // check if there is any video processing graph for this device
                        map<XGuid, XVideoSourceProcessingGraph>::const_iterator graphIt = videoProcessingGraphs.find( cameraObject->Id( ) );
                        if ( graphIt != videoProcessingGraphs.end( ) )
                        {
                            server->SetVideoProcessingGraph( vsId, graphIt->second );
                        }

                        // set confguration of the plugin
                        pluginDesc->SetPluginConfiguration( plugin, cameraObject->PluginProperties( ) );

                        // create mapping between device ID and video source managed by automation server
                        videoSourceMap.insert( pair<XGuid, uint32_t>( cameraObject->Id( ), vsId ) );

                        videoSourceIds.push_back( vsId );
                        videoSourceReportFpsState.insert( pair<uint32_t, bool>( vsId, reportFpsState ) );
                        pluginCreated = true;
                    }
                }

                devicesAreFine &= pluginCreated;
            }
        }

        // prepare all threads
        for ( auto kvp : sandboxThreads )
        {
            const ScriptingThreadDesc           threadDesc = kvp.second;
            shared_ptr<const XPluginDescriptor> pluginDesc = pluginsEngine->GetPlugin( threadDesc.PluginId( ) );

            if ( pluginDesc )
            {
                shared_ptr<XPlugin> plugin = pluginDesc->CreateInstance( );

                if ( plugin )
                {
                    // set configuration of the thread
                    pluginDesc->SetPluginConfiguration( plugin, threadDesc.PluginConfiguration( ) );

                    uint32_t threadServerId = server->AddThread( static_pointer_cast<XScriptingEnginePlugin>( plugin ), threadDesc.Interval( ) );

                    threadsMap.insert( pair<XGuid, uint32_t>( threadDesc.Id( ), threadServerId ) );
                }
                else
                {
                    threadsAreFine = false;
                }
            }
        }

        // create sandbox view
        SandboxView* sandboxView = new SandboxView( po->Id( ), mainWindow );
        sandboxView->SetViews( po->GetViews( ), po->DefaultViewId( ), videoSourceMap, po->Settings( ) );
        sandboxView->SetThreads( threadsMap );
        mainWindowService->SetCentralWidget( sandboxView );

        // start all threads and cameras
        server->StartAllThreads( );
        server->StartAllVideoSources( );

        // check if video frames should be dropped on slow processing
        if ( po->Settings( ).DropFramesOnSlowProcessing( ) )
        {
            for ( int id : videoSourceIds )
            {
                server->EnableVideoFrameDropping( id, true );
            }
        }

        // start monitoring performance of the video sources
        ServiceManager::Instance( ).GetPerformanceMonitor( )->Start( videoSourceIds, videoSourceReportFpsState );

        // check if all video source plug-ins were created
        if ( !devicesAreFine )
        {
            UITools::ShowErrorMessage( QString( "Failed creating some video sources in the [<b>%0</b>] sandbox." ).
                                       arg( QString::fromUtf8( po->Name( ).c_str( ) ) ), mainWindow );
        }
        // check if all threads plug-ins were created
        if ( !threadsAreFine )
        {
            UITools::ShowErrorMessage( QString( "Failed creating some scripting threads in the [<b>%0</b>] sandbox." ).
                                       arg( QString::fromUtf8( po->Name( ).c_str( ) ) ), mainWindow );
        }
    }
}
