/*
    Automation server library of Computer Vision Sandbox

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

#pragma once
#ifndef CVS_XAUTOMATION_SERVER_HPP
#define CVS_XAUTOMATION_SERVER_HPP

#include <memory>
#include <xtypes.h>
#include <XInterfaces.hpp>

#include <XPluginsEngine.hpp>
#include <XVideoSourcePlugin.hpp>
#include <XScriptingEnginePlugin.hpp>

#include "IAutomationVideoSourceListener.hpp"
#include "IAutomationVariablesListener.hpp"

namespace CVSandbox { namespace Automation
{

class XVideoSourceProcessingGraph;
struct XVideoSourceFrameInfo;

namespace Private
{
    class XAutomationServerData;
}

class XAutomationServer : private CVSandbox::Uncopyable
{
private:
    XAutomationServer( const std::shared_ptr<XPluginsEngine>& pluginsEngine, const std::string& hostName, const xversion& hostVersion );

public:
    ~XAutomationServer( );

    static const std::shared_ptr<XAutomationServer> Create( const std::shared_ptr<XPluginsEngine>& pluginsEngine = std::shared_ptr<XPluginsEngine>( ),
                                                            const std::string& hostName = std::string( ),
                                                            const xversion& hostVersion = { 0, 0, 0 } );

    // Start the server
    XErrorCode Start( );
    // Signal the server to stop
    void SignalToStop( );
    // Wait till the server stops
    void WaitForStop( );
    // Check if the server (its thread) is still running
    bool IsRunning( );
    // Terminate the server - call only if it does not respond
    void Terminate( );

    // Add video source (not starting it) into the server - returns its ID
    uint32_t AddVideoSource( const std::shared_ptr<const XPluginDescriptor>& descriptor,
                             const std::shared_ptr<XVideoSourcePlugin>& videoSource );
    // Set video processing graph for the specified video source
    bool SetVideoProcessingGraph( uint32_t videoSourceId, const XVideoSourceProcessingGraph& graph );

    // Get/Set configuration of plug-in instance for the specified processing step of the video source
    const std::map<std::string, CVSandbox::XVariant> GetVideoProcessingStepConfiguration( uint32_t videoSourceId, int32_t stepIndex );
    void SetVideoProcessingStepConfiguration( uint32_t videoSourceId, int32_t stepIndex, const std::map<std::string, CVSandbox::XVariant>& configuration );

    // Get plug-in of the running video source
    std::shared_ptr<XVideoSourcePlugin> GetRunningVideoSource( uint32_t videoSourceId );

    // Start the specified video source
    bool StartVideoSource( uint32_t videoSourceId );
    // Get information about video frames received by the specified video source
    bool GetVideoSourceFrameInfo( uint32_t videoSourceId, struct XVideoSourceFrameInfo* pFrameInfo );
    // Request enabling/disabling video processing performance monitor for the specified video source
    bool EnableVideoProcessingPerformanceMonitor( uint32_t videoSourceId, bool enable );
    // Request enabling/disabling dropping of video frames, when procession thread is still busy while new frame arrives
    bool EnableVideoFrameDropping( uint32_t videoSourceId, bool enable );
    // Get average time (ms) taken by the steps of video processing graph
    std::vector<float> GetVideoProcessingGraphTiming( uint32_t videoSourceId, float* totalTime = nullptr );
    // Start all video sources
    void StartAllVideoSources( );
    // Move the specified video source into finalization queue
    bool FinalizeVideoSource( uint32_t videoSourceId );

    // Add listener for the specified video source
    bool AddVideoSourceListener( uint32_t videoSourceId, IAutomationVideoSourceListener* listener, bool notifyWithRecent = true );
    // Remove listener from the specified video source
    void RemoveVideoSourceListener( uint32_t videoSourceId, IAutomationVideoSourceListener* listener );

    // Add a thread, which will run the specified script at the specified time intervals (milliseconds)
    uint32_t AddThread( const std::shared_ptr<XScriptingEnginePlugin>& scriptToRun, uint32_t msecInterval );
    // Start the specified scripting thread
    bool StartThread( uint32_t threadId );
    // Start all scripting threads
    void StartAllThreads( );
    // Finalize the specified scripting thread
    bool FinalizeThread( uint32_t threadId );

    // Clear all variables stored in the automation server
    void ClearAllVariables( );

    // Set host variable into common storage
    XErrorCode SetVariable( const std::string& name, const CVSandbox::XVariant& value );

    // Set listener to monitor for variables changes
    void SetVariablesListener( IAutomationVariablesListener* listener, bool notifyExistingVariables = false );
    // Clear variables listener, so no more change notifications are sent
    void ClearVariablesListener( );

    // For debug purposes mostly - get number of video sources in each group
    void GetVideoSourceCount( uint32_t* notStarted, uint32_t* running, uint32_t* finalizing );

private:
    const std::auto_ptr<Private::XAutomationServerData> mData;
};

} } // namespace CVSandbox::Automation

#endif // CVS_XAUTOMATION_SERVER_HPP
