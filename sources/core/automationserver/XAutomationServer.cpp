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

#include "XAutomationServer.hpp"
#include "XVideoSourceProcessingGraph.hpp"
#include "XVideoSourceFrameInfo.hpp"
#include <stdio.h>
#include <map>
#include <list>
#include <algorithm>
#include <numeric>
#include <chrono>

#include <XMutex.hpp>
#include <XManualResetEvent.hpp>
#include <XThread.hpp>
#include <XError.hpp>

#include <XImageProcessingFilterPlugin.hpp>
#include <XVideoProcessingPlugin.hpp>
#include <XScriptingEnginePlugin.hpp>

using namespace std;
using namespace std::chrono;
using namespace CVSandbox;
using namespace CVSandbox::Threading;
using namespace CVSandbox::Automation::Private;

namespace CVSandbox { namespace Automation
{

// Number of performance measurements to average
#define PERFORMANCE_HISTORY_LENGTH (40)

namespace Private
{
    typedef list<IAutomationVideoSourceListener*> ListenersList;

    class XAutomationServerData;

    // Internal class to group some data/functions related to video source
    class VideoSourceData : public IVideoSourcePluginListener, Uncopyable
    {
    private:
        VideoSourceData( uint32_t videoSourceId,
                         const shared_ptr<const XPluginDescriptor>& pluginDescriptor,
                         const shared_ptr<XVideoSourcePlugin>& videoSource,
                         XAutomationServerData* server ) :
            VideoSourceId( videoSourceId ), VideoSourceDescriptor( pluginDescriptor), VideoSource( videoSource ),
            Server( server ), Listeners( ), ListenerSync( ),
            LastImage( ), LastError( ), ProcessingGraph( ), ProcessingGraphBuffer( ),
            VideoProcessingSync( ), NewFrameIsAvailableEvent( ), ProcessingThreadIsFreeEvent( ),
            NeedToExitProcessingThread( false ), VideoProcessingThread( ), FrameInfo( ),
            NeedToRunPerformanceMonitor( false ), IsPerformanceMonitroRunning( false ),
            StepFailedInitialization( -1 ), StepFailedMessage( ),
            DropVideoFramesWhenBusy( false ), FramesDropped( 0 ), FramesBlocked( 0 ),
            UpdatedVideoProcessingConfig( )
        {
        }

    public:
        static shared_ptr<VideoSourceData> Create( uint32_t videoSourceId,
                                                   const shared_ptr<const XPluginDescriptor>& pluginDescriptor,
                                                   const shared_ptr<XVideoSourcePlugin>& videoSource,
                                                   XAutomationServerData* server )
        {
            return shared_ptr<VideoSourceData>( new (nothrow) VideoSourceData( videoSourceId, pluginDescriptor, videoSource, server ) );
        }

        ~VideoSourceData( )
        {
            NeedToExitProcessingThread = true;
            NewFrameIsAvailableEvent.Signal( );
            VideoProcessingThread.Join( );
        }

        // Handler of video processing thread - each video source has a separate one, so all
        // video processing is done separately without blocking video source's background thread.
        static void VideoProcessingThreadHandler( void* param );

        // New video frame notification
        virtual void OnNewImage( const shared_ptr<const XImage>& image );
        // Video source error notification
        virtual void OnError( const string& errorMessage );

    public:
        void ReportError( const string& errorMessage );

    private:
        void PreparePlugins( );
        void NotifyNewFrame( );
        void PerformNewFrameProcessing( );
        XErrorCode DoImageProcessingFilterPlugin( const shared_ptr<XImageProcessingFilterPlugin>& plugin, size_t& currrentGraphBufferIndex );
        XErrorCode DoVideoProcessingPlugin( const shared_ptr<XVideoProcessingPlugin>& plugin );
        XErrorCode DoScriptingEnginePlugin( const shared_ptr<XScriptingEnginePlugin>& plugin );

    private:
        // Callbacks for scripting engine's plugin
        static xstring ScriptingEnginePluginCallback_GetHostName( void* userParam );
        static void ScriptingEnginePluginCallback_GetHostVersion( void* userParam, xversion* version );
        static void ScriptingEnginePluginCallback_PrintString( void* userParam, xstring message );
        static XErrorCode ScriptingEnginePluginCallback_CreatePluginInstance( void* userParam, xstring pluginName, PluginDescriptor** pDescriptor, void** pPlugin );
        static XErrorCode ScriptingEnginePluginCallback_GetVariable( void* userParam, xstring name, xvariant* value );
        static XErrorCode ScriptingEnginePluginCallback_SetVariable( void* userParam, xstring name, const xvariant* value );
        static XErrorCode ScriptingEnginePluginCallback_GetImageVariable( void* userParam, xstring name, ximage** value );
        static XErrorCode ScriptingEnginePluginCallback_SetImageVariable( void* userParam, xstring name, const ximage* value );
        static XErrorCode ScriptingEnginePluginCallback_GetImage( void* userParam, ximage** image );
        static XErrorCode ScriptingEnginePluginCallback_SetImage( void* userParam, ximage* image );
        static XErrorCode ScriptingEnginePluginCallback_GetVideoSource( void* userParam, PluginDescriptor** pDescriptor, void** pPlugin );

    public:
        uint32_t                            VideoSourceId;
        shared_ptr<const XPluginDescriptor> VideoSourceDescriptor;
        shared_ptr<XVideoSourcePlugin>      VideoSource;
        XAutomationServerData*              Server;
        ListenersList                       Listeners;                      // list of listeners to notify (new frames, error, etc.)
        XMutex                              ListenerSync;                   // mutex to protect listener list
        shared_ptr<XImage>                  LastImage;                      // image given to client -last image arrived from video source
                                                                            // (if processing graph is empty) - or result video processing -
        string                              LastError;
        XVideoSourceProcessingGraph         ProcessingGraph;                // graph defining video processing steps to perform
        vector<shared_ptr<XImage>>          ProcessingGraphBuffer;          // images produced by video processing graph

        XMutex                              VideoProcessingSync;            // mutex used to guard LastImage
        XMutex                              VideoFrameInfoSync;             // mutex used to guard frame information
        XManualResetEvent                   NewFrameIsAvailableEvent;       // event to signal if there is new frame to process
        XManualResetEvent                   ProcessingThreadIsFreeEvent;    // event to signal if video processing thread is free or not
        volatile bool                       NeedToExitProcessingThread;     // a flag to signal thread to video processing thread to exit
                                                                            // NewFrameIsAvailableEvent must be also signalled)
        XThread                             VideoProcessingThread;
        struct XVideoSourceFrameInfo        FrameInfo;

        bool                                NeedToRunPerformanceMonitor;    // request to enable/disable performance monitor
        bool                                IsPerformanceMonitroRunning;    // actual current state of performance monitor
        vector<vector<float>>               ProcessingStepTimeTaken;
        vector<int>                         NextTimeIndex;
        vector<float>                       ProcessingStepAverageTime;
        vector<float>                       TotalGraphTime;
        float                               TotalAverageGraphTime;
        int                                 GraphTimeIndex;

        int                                 StepFailedInitialization;
        string                              StepFailedMessage;

        bool                                DropVideoFramesWhenBusy;       // drop or not new video frames when processing thread is still busy
        uint32_t                            FramesDropped;
        uint32_t                            FramesBlocked;

        map<int32_t, map<string, XVariant>> UpdatedVideoProcessingConfig;
    };

    // Internal class to group some data/functions related to scripting threads
    class ScriptingThreadData : private Uncopyable
    {
    private:
        ScriptingThreadData( uint32_t threadId, uint32_t msecInterval, shared_ptr<XScriptingEnginePlugin> scriptingEngine, XAutomationServerData* server ) :
            ThreadId( threadId ), MsecInterval( msecInterval ), ScriptingEngine( scriptingEngine ), Server( server ),
            ScriptProcessingThread( ), NeedToExit( )
        {
        }

    public:
        static shared_ptr<ScriptingThreadData> Create( uint32_t threadId, uint32_t msecInterval, shared_ptr<XScriptingEnginePlugin> scriptingEngine, XAutomationServerData* server )
        {
            return shared_ptr<ScriptingThreadData>( new (nothrow) ScriptingThreadData( threadId, msecInterval, scriptingEngine, server ) );
        }

        ~ScriptingThreadData( )
        {
            NeedToExit.Signal( );
            ScriptProcessingThread.Join( );
        }

        static void ScriptProcessingThreadHandler( void* param );

    private:
        // Callbacks for scripting engine's plugin
        static xstring ScriptingEnginePluginCallback_GetHostName( void* userParam );
        static void ScriptingEnginePluginCallback_GetHostVersion( void* userParam, xversion* version );
        static void ScriptingEnginePluginCallback_PrintString( void* userParam, xstring message );
        static XErrorCode ScriptingEnginePluginCallback_CreatePluginInstance( void* userParam, xstring pluginName, PluginDescriptor** pDescriptor, void** pPlugin );
        static XErrorCode ScriptingEnginePluginCallback_GetVariable( void* userParam, xstring name, xvariant* value );
        static XErrorCode ScriptingEnginePluginCallback_SetVariable( void* userParam, xstring name, const xvariant* value );
        static XErrorCode ScriptingEnginePluginCallback_GetImageVariable( void* userParam, xstring name, ximage** value );
        static XErrorCode ScriptingEnginePluginCallback_SetImageVariable( void* userParam, xstring name, const ximage* value );
        static XErrorCode ScriptingEnginePluginCallback_GetImage( void* userParam, ximage** image );
        static XErrorCode ScriptingEnginePluginCallback_SetImage( void* userParam, ximage* image );
        static XErrorCode ScriptingEnginePluginCallback_GetVideoSource( void* userParam, PluginDescriptor** pDescriptor, void** pPlugin );

    public:
        uint32_t                           ThreadId;
        uint32_t                           MsecInterval;
        shared_ptr<XScriptingEnginePlugin> ScriptingEngine;
        XAutomationServerData*             Server;

        XThread                            ScriptProcessingThread;
        XManualResetEvent                  NeedToExit;
    };

    typedef map<uint32_t, shared_ptr<VideoSourceData>>     VsdMap;
    typedef map<uint32_t, shared_ptr<ScriptingThreadData>> ThreadMap;

    // Internal class to hide server's data
    class XAutomationServerData
    {
    public:
        const shared_ptr<XPluginsEngine> PluginsEngine;

        uint32_t            DeviceCounter;

        string              HostName;
        xversion            HostVersion;

        XMutex              ServerSync;
        XManualResetEvent   ExitEvent;
        XThread             ServerThread;

        VsdMap              AddedVideoSources;
        VsdMap              RunningVideoSources;
        VsdMap              FinalizingVideoSources;

        ThreadMap           AddedThreads;
        ThreadMap           RunningThreads;
        ThreadMap           FinalizingThreads;

    private:
        // Variables to share between scripts executed by scripting plug-ins
        XMutex                           VariablesSync;
        map<string, XVariant>            HostVariables;
        map<string, shared_ptr<XImage> > HostImageVariables;
        IAutomationVariablesListener*    VariablesListener;

    public:
        XAutomationServerData( const shared_ptr<XPluginsEngine>& pluginsEngine ) :
            PluginsEngine( pluginsEngine ), DeviceCounter( 0 ),
            HostName( "Automation Server" ), HostVersion( { 1, 0, 1 } ),
            ServerSync( ), ExitEvent( ), ServerThread( ),
            AddedVideoSources( ), RunningVideoSources( ), FinalizingVideoSources( ),
            AddedThreads( ), RunningThreads( ), FinalizingThreads( ),
            VariablesSync( ), HostVariables( ), HostImageVariables( ), VariablesListener( nullptr )

        {
        }

        // Server's background thread - used to monitor finalization queue
        static void ServerWorkerThreadHandler( void* param );

        // Signal video source to stop
        void FinalizeVideoSource( int id );
        // Signal all video sources to stop and move them to finalization queue
        void FinalizeAllRunningObjects( );
        // Wait till all video sources finish
        void WaitAllFinalizingObjects( );
        // Remove already finalized video sources
        void CleanAlreadyFinalized( );
        // Get host variant variable from common storage
        XErrorCode GetHostVariable( const string& name, XVariant& value );
        // Set host variant variable into common storage
        XErrorCode SetHostVariable( const string& name, const XVariant& value );
        // Get host image variable from common storage
        XErrorCode GetHostVariable( const string& name, shared_ptr<XImage>& value );
        // Set host image variable into common storage
        XErrorCode SetHostVariable( const string& name, const shared_ptr<const XImage>& value );
        // Clear all variables
        void ClearAllVariables( );
        // Set listener to monitor for variables changes
        void SetVariablesListener( IAutomationVariablesListener* listener, bool notifyExistingVariables );
        // Clear variables listener, so no more change notifications are sent
        void ClearVariablesListener( );

    public:
        // Implementation of common scripting engine callbacks
        xstring ScriptingEnginePluginCallback_GetHostName( );
        void ScriptingEnginePluginCallback_GetHostVersion( xversion* version );
        void ScriptingEnginePluginCallback_PrintString( xstring message );
        XErrorCode ScriptingEnginePluginCallback_CreatePluginInstance( xstring pluginName, PluginDescriptor** pDescriptor, void **pPlugin );
        XErrorCode ScriptingEnginePluginCallback_GetVariable( xstring name, xvariant* value );
        XErrorCode ScriptingEnginePluginCallback_SetVariable( xstring name, const xvariant* value );
        XErrorCode ScriptingEnginePluginCallback_GetImageVariable( xstring name, ximage** value );
        XErrorCode ScriptingEnginePluginCallback_SetImageVariable( xstring name, const ximage* value );
    };
}

XAutomationServer::XAutomationServer( const shared_ptr<XPluginsEngine>& pluginsEngine, const string& hostName, const xversion& hostVersion ) :
    mData( new Private::XAutomationServerData( pluginsEngine ) )
{
    if ( !hostName.empty( ) )
    {
        mData->HostName = hostName;
    }
    if ( ( hostVersion.major != 0 ) || ( hostVersion.minor != 0 ) || ( hostVersion.revision != 0 ) )
    {
        mData->HostVersion = hostVersion;
    }
}

XAutomationServer::~XAutomationServer( )
{
}

const shared_ptr<XAutomationServer> XAutomationServer::Create( const shared_ptr<XPluginsEngine>& pluginsEngine, const string& hostName, const xversion& hostVersion )
{
    return shared_ptr<XAutomationServer>( new (nothrow) XAutomationServer( pluginsEngine, hostName, hostVersion ) );
}

// Start the server
XErrorCode XAutomationServer::Start( )
{
    XScopedLock lock( &mData->ServerSync );
    XErrorCode  ret = ErrorFailed;

    if ( IsRunning( ) )
    {
        ret = SuccessCode;
    }
    else
    {
        mData->ExitEvent.Reset( );

        if ( mData->ServerThread.Create( XAutomationServerData::ServerWorkerThreadHandler, mData.get( ) ) )
        {
            ret = SuccessCode;
        }
    }

    return ret;
}

// Signal the server to stop
void XAutomationServer::SignalToStop( )
{
    if ( IsRunning( ) )
    {
        mData->ExitEvent.Signal( );
    }
}

// Wait till the server stops
void XAutomationServer::WaitForStop( )
{
    SignalToStop( );
    mData->ServerThread.Join( );
}

// Check if the server (its thread) is still running
bool XAutomationServer::IsRunning( )
{
    return mData->ServerThread.IsRunning( );
}

// Terminate the server - call only if it does not respond
void XAutomationServer::Terminate( )
{
    if ( IsRunning( ) )
    {
        mData->ServerThread.Terminate( );
    }
}

// Add video source (not starting it) into the server - returns its ID
uint32_t XAutomationServer::AddVideoSource( const shared_ptr<const XPluginDescriptor>& descriptor,
                                            const shared_ptr<XVideoSourcePlugin>& videoSource )
{
    XScopedLock lock( &mData->ServerSync );
    uint32_t    ret = ++mData->DeviceCounter;

    mData->AddedVideoSources.insert( VsdMap::value_type( ret, VideoSourceData::Create( ret, descriptor, videoSource, mData.get( ) ) ) );

    return ret;
}

// Set video processing graph for the specified video source
bool XAutomationServer::SetVideoProcessingGraph( uint32_t videoSourceId, const XVideoSourceProcessingGraph& graph )
{
    XScopedLock         lock( &mData->ServerSync );
    bool                ret = false;
    VsdMap::iterator    itVideoSource = mData->AddedVideoSources.find( videoSourceId );

    if ( itVideoSource != mData->AddedVideoSources.end( ) )
    {
        itVideoSource->second->ProcessingGraph = graph;
        ret = true;
    }

    return ret;
}

// Get configuration of plug-in instance for the specified processing step of the video source
const map<string, XVariant> XAutomationServer::GetVideoProcessingStepConfiguration( uint32_t videoSourceId, int32_t stepIndex )
{
    XScopedLock           lock( &mData->ServerSync );
    VsdMap::iterator      vsDataIt = mData->RunningVideoSources.find( videoSourceId );
    map<string, XVariant> configuration;

    if ( vsDataIt != mData->RunningVideoSources.end( ) )
    {
        shared_ptr<VideoSourceData> vsData = vsDataIt->second;

        if ( ( stepIndex >= 0 ) && ( stepIndex < vsData->ProcessingGraph.StepsCount( ) ) )
        {
            XVideoSourceProcessingGraph::ConstIterator stepIt = vsData->ProcessingGraph.begin( ) + stepIndex;
            shared_ptr<XPlugin> plugin = stepIt->GetPluginInstance( );

            configuration = stepIt->GetPluginInstanceConfiguration( );
        }
    }

    return configuration;
}

// Set configuration of plug-in instance for the specified processing step of the video source
void XAutomationServer::SetVideoProcessingStepConfiguration( uint32_t videoSourceId, int32_t stepIndex, const map<string, XVariant>& configuration )
{
    XScopedLock           lock( &mData->ServerSync );
    VsdMap::iterator      vsDataIt = mData->RunningVideoSources.find( videoSourceId );

    if ( vsDataIt != mData->RunningVideoSources.end( ) )
    {
        shared_ptr<VideoSourceData> vsData = vsDataIt->second;

        if ( ( stepIndex >= 0 ) && ( stepIndex < vsData->ProcessingGraph.StepsCount( ) ) )
        {
            // lock frame info mutex instead of processing mutex, since info update takes less time
            XScopedLock infoLock( &vsData->VideoFrameInfoSync );
            vsData->UpdatedVideoProcessingConfig[stepIndex] = configuration;
        }
    }
}

// Get plug-in of the running video source
shared_ptr<XVideoSourcePlugin> XAutomationServer::GetRunningVideoSource( uint32_t videoSourceId )
{
    shared_ptr<XVideoSourcePlugin> videoSource;
    XScopedLock                    lock( &mData->ServerSync );
    VsdMap::iterator               vsDataIt = mData->RunningVideoSources.find( videoSourceId );

    if ( vsDataIt != mData->RunningVideoSources.end( ) )
    {
        videoSource = vsDataIt->second->VideoSource;
    }

    return videoSource;
}

// Start the specified video source
bool XAutomationServer::StartVideoSource( uint32_t videoSourceId )
{
    XScopedLock         lock( &mData->ServerSync );
    bool                ret           = false;
    VsdMap::iterator    videoSourceIt = mData->AddedVideoSources.find( videoSourceId );

    if ( videoSourceIt != mData->AddedVideoSources.end( ) )
    {
        shared_ptr<VideoSourceData>    videoSourceData = videoSourceIt->second;
        shared_ptr<XVideoSourcePlugin> videoSource     = videoSourceData->VideoSource;

        if ( videoSourceData )
        {
            // set our internal videoSourceData as a listener to the video source
            videoSource->SetListener( videoSourceData.get( ) );

            // start the video processing thread
            if ( videoSourceData->VideoProcessingThread.Create( VideoSourceData::VideoProcessingThreadHandler, videoSourceData.get( ) ) )
            {
                mData->RunningVideoSources.insert( VsdMap::value_type( videoSourceId, videoSourceData ) );

                ret = true;
            }
        }

        // remove it from added video sources anyway, since there is little can be done at this point
        mData->AddedVideoSources.erase( videoSourceIt );
    }

    return ret;
}

// Get information about video frames received by the specified video source
bool XAutomationServer::GetVideoSourceFrameInfo( uint32_t videoSourceId, struct XVideoSourceFrameInfo* pFrameInfo )
{
    XScopedLock         lock( &mData->ServerSync );
    bool                ret = false;
    VsdMap::iterator    vsDataIt = mData->RunningVideoSources.find( videoSourceId );

    if ( ( pFrameInfo != nullptr ) && ( vsDataIt != mData->RunningVideoSources.end( ) ) )
    {
        shared_ptr<VideoSourceData> vsData = vsDataIt->second;
        XScopedLock                 infoLock( &vsData->VideoFrameInfoSync );

        *pFrameInfo = vsData->FrameInfo;
        ret = true;
    }

    return ret;
}

// Request enabling/disabling video processing performance monitor for the specified video source
bool XAutomationServer::EnableVideoProcessingPerformanceMonitor( uint32_t videoSourceId, bool enable )
{
    XScopedLock         lock( &mData->ServerSync );
    bool                ret = false;
    VsdMap::iterator    vsDataIt = mData->RunningVideoSources.find( videoSourceId );

    if ( vsDataIt != mData->RunningVideoSources.end( ) )
    {
        shared_ptr<VideoSourceData> vsData = vsDataIt->second;
        XScopedLock                 infoLock( &vsData->VideoFrameInfoSync );

        vsData->NeedToRunPerformanceMonitor = enable;

        ret = true;
    }

    return ret;
}

// Request enabling/disabling dropping of video frames, when procession thread is still busy while new frame arrives
bool XAutomationServer::EnableVideoFrameDropping( uint32_t videoSourceId, bool enable )
{
    XScopedLock         lock( &mData->ServerSync );
    bool                ret = false;
    VsdMap::iterator    vsDataIt = mData->RunningVideoSources.find( videoSourceId );

    if ( vsDataIt != mData->RunningVideoSources.end( ) )
    {
        shared_ptr<VideoSourceData> vsData = vsDataIt->second;
        XScopedLock                 infoLock( &vsData->VideoFrameInfoSync );

        vsData->DropVideoFramesWhenBusy = enable;

        ret = true;
    }

    return ret;
}

// Get average time (ms) taken by the steps of video processing graph
vector<float> XAutomationServer::GetVideoProcessingGraphTiming( uint32_t videoSourceId, float* totalTime )
{
    XScopedLock         lock( &mData->ServerSync );
    VsdMap::iterator    vsDataIt = mData->RunningVideoSources.find( videoSourceId );
    vector<float>       timing;

    if ( vsDataIt != mData->RunningVideoSources.end( ) )
    {
        shared_ptr<VideoSourceData> vsData = vsDataIt->second;
        XScopedLock                 infoLock( &vsData->VideoFrameInfoSync );

        timing = vsData->ProcessingStepAverageTime;

        if ( totalTime != nullptr )
        {
            *totalTime = vsData->TotalAverageGraphTime;
        }
    }

    return timing;
}

// Start all video sources
void XAutomationServer::StartAllVideoSources( )
{
    XScopedLock lock( &mData->ServerSync );

    while ( !mData->AddedVideoSources.empty( ) )
    {
        StartVideoSource( mData->AddedVideoSources.begin( )->first );
    }
}

// Move the specified video source into finalization queue
bool XAutomationServer::FinalizeVideoSource( uint32_t videoSourceId )
{
    XScopedLock         lock( &mData->ServerSync );
    bool                ret           = false;
    VsdMap::iterator    itVideoSource = mData->RunningVideoSources.find( videoSourceId );

    if ( itVideoSource != mData->RunningVideoSources.end( ) )
    {
        mData->FinalizeVideoSource( videoSourceId );
        ret = true;
    }
    else
    {
        itVideoSource = mData->AddedVideoSources.find( videoSourceId );

        if ( itVideoSource != mData->AddedVideoSources.end( ) )
        {
            // remove something which has never run
            mData->AddedVideoSources.erase( itVideoSource );
            ret = true;
        }
    }

    return ret;
}

// Add listener for the specified video source
bool XAutomationServer::AddVideoSourceListener( uint32_t videoSourceId, IAutomationVideoSourceListener* listener, bool notifyWithRecent )
{
    bool    ret = false;

    if ( listener != nullptr )
    {
        XScopedLock         lock( &mData->ServerSync );
        VsdMap::iterator    itAddVideoSource     = mData->AddedVideoSources.find( videoSourceId );
        VsdMap::iterator    itRunningVideoSource = mData->RunningVideoSources.find( videoSourceId );

        if ( itAddVideoSource != mData->AddedVideoSources.end( ) )
        {
            itAddVideoSource->second->Listeners.push_back( listener );
            ret = true;
        }
        else if ( itRunningVideoSource != mData->RunningVideoSources.end( ) )
        {
            shared_ptr<VideoSourceData> vsData = itRunningVideoSource->second;

            {
                XScopedLock listenerLock( &vsData->ListenerSync );
                vsData->Listeners.push_back( listener );
            }

            if ( notifyWithRecent )
            {
                if ( vsData->VideoProcessingSync.TryLock( ) )
                {
                    // notify of last image and/or error if there are any
                    if ( vsData->LastImage )
                    {
                        listener->OnNewVideoFrame( videoSourceId, vsData->LastImage );
                    }
                    if ( !vsData->LastError.empty( ) )
                    {
                        listener->OnErrorMessage( videoSourceId, vsData->LastError );
                    }

                    vsData->VideoProcessingSync.Unlock( );
                }
            }

            ret = true;
        }
    }

    return ret;
}

// Remove listener from the specified video source
void XAutomationServer::RemoveVideoSourceListener( uint32_t videoSourceId, IAutomationVideoSourceListener* listener )
{
    XScopedLock         lock( &mData->ServerSync );
    VsdMap::iterator    itAddVideoSource     = mData->AddedVideoSources.find( videoSourceId );
    VsdMap::iterator    itRunningVideoSource = mData->RunningVideoSources.find( videoSourceId );

    if ( itAddVideoSource != mData->AddedVideoSources.end( ) )
    {
        itAddVideoSource->second->Listeners.remove( listener );
    }
    else if ( itRunningVideoSource != mData->RunningVideoSources.end( ) )
    {
        shared_ptr<VideoSourceData> vsData = itRunningVideoSource->second;

        XScopedLock listenerLock( &vsData->ListenerSync );
        vsData->Listeners.remove( listener );
    }
}

// Add a thread, which will run the specified script at the specified time intervals (milliseconds)
uint32_t XAutomationServer::AddThread( const std::shared_ptr<XScriptingEnginePlugin>& scriptToRun, uint32_t msecInterval )
{
    XScopedLock lock( &mData->ServerSync );
    uint32_t    ret = ++mData->DeviceCounter;

    mData->AddedThreads.insert( ThreadMap::value_type( ret, ScriptingThreadData::Create( ret, msecInterval, scriptToRun, mData.get( ) ) ) );

    return ret;
}

// Start the specified scripting thread
bool XAutomationServer::StartThread( uint32_t threadId )
{
    XScopedLock         lock( &mData->ServerSync );
    bool                ret = false;
    ThreadMap::iterator itThread = mData->AddedThreads.find( threadId );

    if ( itThread != mData->AddedThreads.end( ) )
    {
        shared_ptr<ScriptingThreadData> threadData = itThread->second;

        if ( threadData )
        {
            if ( threadData->ScriptProcessingThread.Create( ScriptingThreadData::ScriptProcessingThreadHandler, threadData.get( ) ) )
            {
                mData->RunningThreads.insert( ThreadMap::value_type( threadId, threadData ) );
                ret = true;
            }
        }

        // remove the thread from the added map
        mData->AddedThreads.erase( itThread );
    }

    return ret;
}

// Start all scripting threads
void XAutomationServer::StartAllThreads( )
{
    XScopedLock lock( &mData->ServerSync );

    while ( !mData->AddedThreads.empty( ) )
    {
        StartThread( mData->AddedThreads.begin( )->first );
    }
}

// Finalize the specified scripting thread
bool XAutomationServer::FinalizeThread( uint32_t threadId )
{
    XScopedLock         lock( &mData->ServerSync );
    bool                ret = false;
    ThreadMap::iterator itThread = mData->RunningThreads.find( threadId );

    if ( itThread != mData->RunningThreads.end( ) )
    {
        itThread->second->NeedToExit.Signal( );
        mData->FinalizingThreads.insert( ThreadMap::value_type( threadId, itThread->second ) );
        mData->RunningThreads.erase( itThread );
        ret = true;
    }
    else
    {
        itThread = mData->AddedThreads.find( threadId );

        if ( itThread != mData->AddedThreads.end( ) )
        {
            mData->AddedThreads.erase( itThread );
            ret = true;
        }
    }

    return ret;
}

// Clear all variables stored in the automation server
void XAutomationServer::ClearAllVariables( )
{
    mData->ClearAllVariables( );
}

// Set host variable into common storage
XErrorCode XAutomationServer::SetVariable( const string& name, const XVariant& value )
{
    return mData->SetHostVariable( name, value );
}

// Set listener to monitor for variables changes
void XAutomationServer::SetVariablesListener( IAutomationVariablesListener* listener, bool notifyExistingVariables )
{
    mData->SetVariablesListener( listener, notifyExistingVariables );
}

// Clear variables listener, so no more change notifications are sent
void XAutomationServer::ClearVariablesListener( )
{
    mData->ClearVariablesListener( );
}

// For debug purposes mostly - get number of video source in each group
void XAutomationServer::GetVideoSourceCount( uint32_t* notStarted, uint32_t* running, uint32_t* finalizing )
{
    XScopedLock lock( &mData->ServerSync );

    if ( notStarted != NULL )
    {
        *notStarted = mData->AddedVideoSources.size( );
    }
    if ( running != NULL )
    {
        *running = mData->RunningVideoSources.size( );
    }
    if ( finalizing != NULL )
    {
        *finalizing = mData->FinalizingVideoSources.size( );
    }
}

// Server's background thread - used to monitor finalization queue
void XAutomationServerData::ServerWorkerThreadHandler( void* param )
{
    XAutomationServerData* self = static_cast<XAutomationServerData*>( param );

    while ( !self->ExitEvent.Wait( 200 ) )
    {
        XScopedLock lock( &self->ServerSync );
        self->CleanAlreadyFinalized( );
    }

    {
        XScopedLock lock( &self->ServerSync );

        self->AddedVideoSources.clear( );
        self->AddedThreads.clear( );

        self->FinalizeAllRunningObjects( );
        self->WaitAllFinalizingObjects( );
    }
}

// Signal video source to stop
void XAutomationServerData::FinalizeVideoSource( int id )
{
    VsdMap::iterator vsDataIt = RunningVideoSources.find( id );

    if ( vsDataIt != RunningVideoSources.end( ) )
    {
        shared_ptr<VideoSourceData> vsData = vsDataIt->second;

        // signal video processing thread to finish
        vsData->NeedToExitProcessingThread = true;
        vsData->NewFrameIsAvailableEvent.Signal( );

        // don't need any notifications from the video source
        {
            XScopedLock lock( &vsData->ListenerSync );
            vsData->Listeners.clear( );
        }

        vsData->VideoSource->SetListener( nullptr );
        vsData->VideoSource->SignalToStop( );

        FinalizingVideoSources.insert( VsdMap::value_type( id, vsData ) );
        RunningVideoSources.erase( id );
    }
}

// Signal all video sources to stop and move them to finalization queue
void XAutomationServerData::FinalizeAllRunningObjects( )
{
    // finalize video shources
    while ( !RunningVideoSources.empty( ) )
    {
        VsdMap::iterator begin = RunningVideoSources.begin( );
        FinalizeVideoSource( begin->first );
    }

    // finalize scripting threads
    while ( !RunningThreads.empty( ) )
    {
        ThreadMap::iterator begin = RunningThreads.begin( );
        begin->second->NeedToExit.Signal( );
        FinalizingThreads.insert( ThreadMap::value_type( begin->first, begin->second ) );
        RunningThreads.erase( begin );
    }
}

// Wait till all video sources finish
void XAutomationServerData::WaitAllFinalizingObjects( )
{
    // wait for video source to complete
    for ( VsdMap::iterator it = FinalizingVideoSources.begin( ); it != FinalizingVideoSources.end( ); ++it )
    {
        // wait for the video source
        it->second->VideoSource->WaitForStop( );
    }

    // wait for threads to complete
    for ( ThreadMap::iterator it = FinalizingThreads.begin( ); it != FinalizingThreads.end( ); ++it )
    {
        it->second->ScriptProcessingThread.Join( );
    }

    FinalizingVideoSources.clear( );
    FinalizingThreads.clear( );
}

// Remove already finalized video sources
void XAutomationServerData::CleanAlreadyFinalized( )
{
    VsdMap::iterator it = FinalizingVideoSources.begin( );

    while ( it != FinalizingVideoSources.end( ) )
    {
        if ( !it->second->VideoSource->IsRunning( ) )
        {
            it = FinalizingVideoSources.erase( it );
        }
        else
        {
            ++it;
        }
    }

    // clean-up finished threads
    ThreadMap::iterator itThread = FinalizingThreads.begin( );

    while ( itThread != FinalizingThreads.end( ) )
    {
        if ( !itThread->second->ScriptProcessingThread.IsRunning( ) )
        {
            itThread = FinalizingThreads.erase( itThread );
        }
        else
        {
            ++itThread;
        }
    }
}

// Get host variable from common storage
XErrorCode XAutomationServerData::GetHostVariable( const string& name, XVariant& value )
{
    value.SetEmpty( );

    XScopedLock lock( &VariablesSync );

    map<string, XVariant>::const_iterator it = HostVariables.find( name );

    if ( it != HostVariables.end( ) )
    {
        value = it->second;
    }

    return SuccessCode;
}

// Set host variable into common storage
XErrorCode XAutomationServerData::SetHostVariable( const string& name, const XVariant& value )
{
    XScopedLock lock( &VariablesSync );

    if ( value.IsNullOrEmpty( ) )
    {
        HostVariables.erase( name );
    }
    else
    {
        HostVariables[name] = value;
    }

    // remove any image variable with such name
    HostImageVariables.erase( name );

    // notify listener if it was set
    if ( VariablesListener != nullptr )
    {
        VariablesListener->OnVariableSet( name, value );
    }

    return SuccessCode;
}

// Get host image variable from common storage
XErrorCode XAutomationServerData::GetHostVariable( const string& name, shared_ptr<XImage>& value )
{
    value.reset( );

    XScopedLock lock( &VariablesSync );

    map<string, shared_ptr<XImage> >::const_iterator it = HostImageVariables.find( name );

    if ( it != HostImageVariables.end( ) )
    {
        value = it->second->Clone( );
    }

    return SuccessCode;
}

// Set host image variable into common storage
XErrorCode XAutomationServerData::SetHostVariable( const string& name, const shared_ptr<const XImage>& value )
{
    XScopedLock lock( &VariablesSync );

    if ( !value )
    {
        HostImageVariables.erase( name );
    }
    else
    {
        map<string, shared_ptr<XImage> >::iterator it = HostImageVariables.find( name );

        if ( it == HostImageVariables.end( ) )
        {
            HostImageVariables.insert( pair<string, shared_ptr<XImage> >( name, value->Clone( ) ) );
        }
        else
        {
            value->CopyDataOrClone( it->second );
        }
    }

    // remove any variant variable with such name
    HostVariables.erase( name );

    return SuccessCode;
}

// Clear all variables
void XAutomationServerData::ClearAllVariables( )
{
    XScopedLock lock( &VariablesSync );

    HostVariables.clear( );
    HostImageVariables.clear( );

    // notify listener if it was set
    if ( VariablesListener != nullptr )
    {
        VariablesListener->OnClearAllVariables( );
    }
}

// Set listener to monitor for variables changes
void XAutomationServerData::SetVariablesListener( IAutomationVariablesListener* listener, bool notifyExistingVariables )
{
    XScopedLock lock( &VariablesSync );

    VariablesListener = listener;

    if ( ( notifyExistingVariables ) && ( VariablesListener != nullptr ) )
    {
        for ( auto& kvp : HostVariables )
        {
            VariablesListener->OnVariableSet( kvp.first, kvp.second );
        }

        // map<string, shared_ptr<XImage> > HostImageVariables;
    }
}

// Clear variables listener, so no more change notifications are sent
void XAutomationServerData::ClearVariablesListener( )
{
    XScopedLock lock( &VariablesSync );

    VariablesListener = nullptr;
}

// Prepares plug-ins of the video processing graph, so those are ready to be used for new frame processing
void VideoSourceData::PreparePlugins( )
{
    if ( ( ProcessingGraph.StepsCount( ) != 0 ) && ( Server->PluginsEngine ) )
    {
        int    stepCounter = 0;
        string errorMessage;

        for ( auto stepIt = ProcessingGraph.begin( ), endIt = ProcessingGraph.end( ); ( stepIt != endIt ) && ( errorMessage.empty( ) ); ++stepIt, ++stepCounter )
        {
            // first create the plug-in instance
            if ( !stepIt->CreatePluginInstance( Server->PluginsEngine ) )
            {
                StepFailedMessage = string( "Failed creating instance of \"" + stepIt->Name( ) + "\"" );
            }
            else
            {
                // perform any type specific initialization
                shared_ptr<XPlugin> plugin = stepIt->GetPluginInstance( );

                if ( plugin )
                {
                    if ( stepIt->GetPluginType( ) == PluginType_ScriptingEngine )
                    {
                        shared_ptr<XScriptingEnginePlugin> scriptingEngine = static_pointer_cast<XScriptingEnginePlugin>( plugin );
                        XErrorCode                         errorCode;
                        ScriptingEnginePluginCallbacks     callbacks;

                        callbacks.GetHostName          = ScriptingEnginePluginCallback_GetHostName;
                        callbacks.GetHostVersion       = ScriptingEnginePluginCallback_GetHostVersion;
                        callbacks.PrintString          = ScriptingEnginePluginCallback_PrintString;
                        callbacks.CreatePluginInstance = ScriptingEnginePluginCallback_CreatePluginInstance;
                        callbacks.GetImage             = ScriptingEnginePluginCallback_GetImage;
                        callbacks.SetImage             = ScriptingEnginePluginCallback_SetImage;
                        callbacks.GetVariable          = ScriptingEnginePluginCallback_GetVariable;
                        callbacks.SetVariable          = ScriptingEnginePluginCallback_SetVariable;
                        callbacks.GetImageVariable     = ScriptingEnginePluginCallback_GetImageVariable;
                        callbacks.SetImageVariable     = ScriptingEnginePluginCallback_SetImageVariable;
                        callbacks.GetVideoSource       = ScriptingEnginePluginCallback_GetVideoSource;

                        // set callback first to allow script interface with the host
                        scriptingEngine->SetCallbacks( &callbacks, this );

                        if ( ( ( errorCode = scriptingEngine->Init( ) ) != SuccessCode ) ||
                             ( ( errorCode = scriptingEngine->LoadScript( ) ) != SuccessCode ) ||
                             ( ( errorCode = scriptingEngine->InitScript( ) ) != SuccessCode ) )
                        {
                            {
                                bool detailedMessageAvailable = false;

                                StepFailedInitialization = stepCounter;
                                StepFailedMessage = string( "Error in \"" + stepIt->Name( ) + "\": " );

                                if ( ( errorCode == ErrorFailedLoadingScript ) || ( errorCode == ErrorFailedRunningScript ) )
                                {
                                    string engineErrorMessage = scriptingEngine->GetLastErrorMessage( );

                                    if ( !engineErrorMessage.empty( ) )
                                    {
                                        StepFailedMessage.append( engineErrorMessage );
                                        detailedMessageAvailable = true;
                                    }
                                }

                                if ( !detailedMessageAvailable )
                                {
                                    StepFailedMessage.append( XError::Description( errorCode ) );
                                }
                            }
                        }
                    }
                }
            }
        }

        if ( !StepFailedMessage.empty( ) )
        {
            ReportError( StepFailedMessage );
        }
    }
}

// Handler of video processing thread
void VideoSourceData::VideoProcessingThreadHandler( void* param )
{
    VideoSourceData* self = static_cast<VideoSourceData*>( param );
    XErrorCode       ecode;

    // prepare plug-ins for the video processing graph
    self->PreparePlugins( );

    // finally start the video source
    ecode = self->VideoSource->Start( );
    if ( ecode != SuccessCode )
    {
        string errorMessage = string( "Failed starting video source: " );
        errorMessage.append( XError::Description( ecode ) );
        self->ReportError( errorMessage );
    }
    else
    {
        // something to process is welcome now
        self->ProcessingThreadIsFreeEvent.Signal( );

        while ( !self->NeedToExitProcessingThread )
        {
            // TODO: better have auto reset event here
            self->NewFrameIsAvailableEvent.Wait( );
            self->NewFrameIsAvailableEvent.Reset( );

            if ( !self->NeedToExitProcessingThread )
            {
                // from now we are busy processing the new frame
                self->ProcessingThreadIsFreeEvent.Reset( );

                self->PerformNewFrameProcessing( );

                // signal we are free to process new frame
                self->ProcessingThreadIsFreeEvent.Signal( );
            }
        }
    }
}

// New video frame notification
void VideoSourceData::OnNewImage( const shared_ptr<const XImage>& image )
{
    if ( !NeedToExitProcessingThread )
    {
        bool dropIfBusy = this->DropVideoFramesWhenBusy;
        bool dropIt     = false;

        // check if processing thread is still busy
        if ( !ProcessingThreadIsFreeEvent.IsSignaled( ) )
        {
            FramesBlocked++;

            if ( dropIfBusy )
            {
                FramesDropped++;
                dropIt = true;
            }
            else
            {
                // make sure the video processing thread is free
                ProcessingThreadIsFreeEvent.Wait( );
            }
        }

        // update counters
        {
            XScopedLock lock( &VideoFrameInfoSync );
            FrameInfo.FramesBlocked = FramesBlocked;
            FrameInfo.FramesDropped = FramesDropped;

            if ( !dropIt )
            {
                FrameInfo.FramesReceived++;
            }
        }

        if ( !dropIt )
        {
            XScopedLock lock( &VideoProcessingSync );

            LastImage.reset( );

            if ( !ProcessingGraphBuffer.empty( ) )
            {
                LastImage = ProcessingGraphBuffer[0];
            }

            // make a copy of the image coming from video source
            image->CopyDataOrClone( LastImage );

            if ( !LastImage )
            {
                ReportError( "Not enough memory to get video frame" );
            }
            else
            {
                // clear any error if the video source is active
                LastError.clear( );

                // update image in the processing buffer
                if ( ProcessingGraphBuffer.empty( ) )
                {
                    ProcessingGraphBuffer.push_back( LastImage );
                }
                else
                {
                    ProcessingGraphBuffer[0] = LastImage;
                }

                // signal video processing thread that there is some job for it
                NewFrameIsAvailableEvent.Signal( );
            }
        }
    }
}

// Video source error notification
void VideoSourceData::OnError( const string& errorMessage )
{
    ReportError( errorMessage );
}

// Report error to video source listeners
void VideoSourceData::ReportError( const string& errorMessage )
{
    XScopedLock lock( &ListenerSync );

    LastError = errorMessage;

    for ( ListenersList::iterator it = Listeners.begin( ); it != Listeners.end( ); )
    {
        IAutomationVideoSourceListener* listener = *it;

        // increment before calling listener, allowing it unsubscribe from handler
        ++it;

        listener->OnErrorMessage( VideoSourceId, LastError );
    }
}

// Notify listeners of the new video frame available
void VideoSourceData::NotifyNewFrame( )
{
    XScopedLock lock( &ListenerSync );

    for ( ListenersList::iterator it = Listeners.begin( ); it != Listeners.end( ); )
    {
        IAutomationVideoSourceListener* listener = *it;

        // increment before calling listener, allowing it unsubscribe from handler
        ++it;

        listener->OnNewVideoFrame( VideoSourceId, LastImage );
    }
}

// Do processing of the new video frame and then notify listeners
void VideoSourceData::PerformNewFrameProcessing( )
{
    XScopedLock     lock( &VideoProcessingSync );
    size_t          currentImageIndex   = 0;
    int             currentStepIndex    = 0;
    string          errorMessage;
    XErrorCode      errorCode           = SuccessCode;

    int32_t         originalFrameWidth  = LastImage->Width( );
    int32_t         originalFrameHeight = LastImage->Height( );
    XPixelFormat    originalPixelFormat = LastImage->Format( );
    int32_t         videoProcessingStepsDone = 0;
    float           graphTimeTaken      = 0.0f;

    // apply video processing graph if any
    if ( ProcessingGraph.StepsCount( ) != 0 )
    {
        steady_clock::time_point    processingGraphStartTime;

        if ( IsPerformanceMonitroRunning )
        {
            processingGraphStartTime = steady_clock::now( );
        }

        for ( XVideoSourceProcessingGraph::ConstIterator stepIt = ProcessingGraph.begin( ), endIt = ProcessingGraph.end( );
              ( stepIt != endIt ) && ( errorMessage.empty( ) ); ++stepIt )
        {
            if ( currentStepIndex == StepFailedInitialization )
            {
                errorMessage = StepFailedMessage;
            }
            else
            {
                // TODO : move plug-ins' execution to separate classes
                shared_ptr<XPlugin> plugin = stepIt->GetPluginInstance( );

                if ( !plugin )
                {
                    errorMessage = string( "Failed getting plug-in instance for step \"" + stepIt->Name( ) + "\"." );
                }
                else
                {
                    steady_clock::time_point    processingStepStartTime;

                    if ( IsPerformanceMonitroRunning )
                    {
                        processingStepStartTime = steady_clock::now( );
                    }

                    switch ( stepIt->GetPluginType( ) )
                    {
                    case PluginType_ImageProcessingFilter:
                        errorCode = DoImageProcessingFilterPlugin( static_pointer_cast<XImageProcessingFilterPlugin>( plugin ), currentImageIndex );
                        break;

                    case PluginType_VideoProcessing:
                        errorCode = DoVideoProcessingPlugin( static_pointer_cast<XVideoProcessingPlugin>( plugin ) );
                        break;

                    case PluginType_ScriptingEngine:
                        errorCode = DoScriptingEnginePlugin( static_pointer_cast<XScriptingEnginePlugin>( plugin ) );
                        break;

                    default:
                        errorMessage = string( "Unknown plug-in type for step \"" + stepIt->Name( ) + "\"." );
                        break;
                    }

                    // get time taken by the video processing step if performance monitor is enabled
                    if ( IsPerformanceMonitroRunning )
                    {
                        float timeTaken = static_cast<float>(
                            duration_cast<std::chrono::microseconds>(
                            steady_clock::now( ) - processingStepStartTime ).count( ) ) / 1000.0f;


                        if ( ProcessingStepTimeTaken[currentStepIndex].size( ) < PERFORMANCE_HISTORY_LENGTH )
                        {
                            ProcessingStepTimeTaken[currentStepIndex].push_back( timeTaken );
                        }
                        else
                        {
                            ProcessingStepTimeTaken[currentStepIndex][NextTimeIndex[currentStepIndex]++] = timeTaken;
                            NextTimeIndex[currentStepIndex] %= PERFORMANCE_HISTORY_LENGTH;
                        }
                    }

                    if ( errorMessage.empty( ) )
                    {
                        switch ( errorCode )
                        {
                        case SuccessCode:
                            videoProcessingStepsDone++;
                            break;

                        case ErrorUnsupportedPixelFormat:
                            errorMessage = string( "Step \"" + stepIt->Name( ) + "\" cannot accept image format." );
                            break;

                        case ErrorFailedRunningScript:
                            {
                                string engineErrorMessage = static_pointer_cast<XScriptingEnginePlugin>( plugin )->GetLastErrorMessage( );

                                if ( !engineErrorMessage.empty( ) )
                                {
                                    errorMessage = string( "Error in \"" + stepIt->Name( ) + "\": " + engineErrorMessage );
                                    break;
                                }

                                /* !! NOTE !! Fall through to default if there is no error message provided by scripting engine */
                            }

                        default:
                            errorMessage = string( "Error in \"" + stepIt->Name( ) + "\": " + XError::Description( errorCode ) );
                            break;
                        }
                    }
                }

                ++currentStepIndex;
            }
        }

        // get total time taken by the processing graph
        if ( IsPerformanceMonitroRunning )
        {
            graphTimeTaken = static_cast<float>(
                duration_cast<std::chrono::microseconds>(
                steady_clock::now( ) - processingGraphStartTime ).count( ) ) / 1000.0f;
        }
    }

    // update video frame information
    {
        XScopedLock infoLock( &VideoFrameInfoSync );

        FrameInfo.OriginalFrameWidth       = originalFrameWidth;
        FrameInfo.OriginalFrameHeight      = originalFrameHeight;
        FrameInfo.OriginalPixelFormat      = originalPixelFormat;
        FrameInfo.VideoProcessingStepsDone = videoProcessingStepsDone;

        FrameInfo.ProcessedFrameWidth  = LastImage->Width( );
        FrameInfo.ProcessedFrameHeight = LastImage->Height( );
        FrameInfo.ProcessedPixelFormat = LastImage->Format( );

        if ( ( NeedToRunPerformanceMonitor ) && ( !IsPerformanceMonitroRunning ) )
        {
            int stepsCount = ProcessingGraph.StepsCount( );

            ProcessingStepTimeTaken   = vector<vector<float>>( stepsCount );
            NextTimeIndex             = vector<int>( stepsCount );
            ProcessingStepAverageTime = vector<float>( stepsCount );
            TotalGraphTime            = vector<float>( );
            GraphTimeIndex            = 0;

            TotalGraphTime.reserve( PERFORMANCE_HISTORY_LENGTH );
        }

        if ( IsPerformanceMonitroRunning )
        {
            for ( int i = 0, n = ProcessingGraph.StepsCount( ); i < n; i++ )
            {
                ProcessingStepAverageTime[i] = ( ProcessingStepTimeTaken[i].size( ) == 0 ) ? 0.0f : std::accumulate( ProcessingStepTimeTaken[i].begin( ), ProcessingStepTimeTaken[i].end( ), 0.0f ) / ProcessingStepTimeTaken[i].size( );
            }

            if ( TotalGraphTime.size( ) != PERFORMANCE_HISTORY_LENGTH )
            {
                TotalGraphTime.push_back( graphTimeTaken );
            }
            else
            {
                TotalGraphTime[GraphTimeIndex++] = graphTimeTaken;
                GraphTimeIndex %= PERFORMANCE_HISTORY_LENGTH;
            }

            TotalAverageGraphTime = ( TotalGraphTime.size( ) == 0 ) ? 0.0f : std::accumulate( TotalGraphTime.begin( ), TotalGraphTime.end( ), 0.0f ) / TotalGraphTime.size( );
        }

        // done at the end to minimize number of locks at the cost of extra "bool"
        IsPerformanceMonitroRunning = NeedToRunPerformanceMonitor;

        // check if there are any updates to steps' configuration
        if ( !UpdatedVideoProcessingConfig.empty( ) )
        {
            XVideoSourceProcessingGraph::Iterator firstStepIt = ProcessingGraph.begin( );

            for ( map<int32_t, map<string, XVariant>>::const_iterator confIt = UpdatedVideoProcessingConfig.begin( ), endIt = UpdatedVideoProcessingConfig.end( );
                  confIt != endIt; ++confIt )
            {
                XVideoSourceProcessingGraph::Iterator stepIt = firstStepIt + confIt->first;
                stepIt->SetPluginInstanceConfiguration( confIt->second );
            }

            UpdatedVideoProcessingConfig.clear( );
        }
    }

    // we provide the new video frame even if processing graph is not complete
    NotifyNewFrame( );

    if ( !errorMessage.empty( ) )
    {
        ReportError( errorMessage );
    }
}

// Run image processing filter plug-in on the current image
XErrorCode VideoSourceData::DoImageProcessingFilterPlugin( const shared_ptr<XImageProcessingFilterPlugin>& plugin, size_t& currrentGraphBufferIndex )
{
    XErrorCode ret = ErrorUnsupportedPixelFormat;

    if ( plugin->IsPixelFormatSupported( LastImage->Format( ) ) )
    {
        if ( plugin->CanProcessInPlace( ) )
        {
            ret = plugin->ProcessImage( LastImage );
        }
        else
        {
            shared_ptr<XImage> nextImage;

            currrentGraphBufferIndex++;

            // get image from the buffer, so we could try reusing memory
            if ( ProcessingGraphBuffer.size( ) > currrentGraphBufferIndex )
            {
                nextImage = ProcessingGraphBuffer[currrentGraphBufferIndex];
            }

            ret = plugin->ProcessImage( LastImage, nextImage );

            if ( ret == SuccessCode )
            {
                // update processing buffer
                if ( ProcessingGraphBuffer.size( ) <= currrentGraphBufferIndex )
                {
                    ProcessingGraphBuffer.push_back( nextImage );
                }
                else
                {
                    ProcessingGraphBuffer[currrentGraphBufferIndex] = nextImage;
                }

                LastImage = nextImage;
            }
        }
    }

    return ret;
}

// Run video processing plug-in on the current image
XErrorCode VideoSourceData::DoVideoProcessingPlugin( const shared_ptr<XVideoProcessingPlugin>& plugin )
{
    XErrorCode ret = ErrorUnsupportedPixelFormat;

    if ( plugin->IsPixelFormatSupported( LastImage->Format( ) ) )
    {
        ret = plugin->ProcessImage( LastImage );
    }

    return ret;

}

// Run "Main" in a script loaded into scripting engine plug-in
XErrorCode VideoSourceData::DoScriptingEnginePlugin( const shared_ptr<XScriptingEnginePlugin>& plugin )
{
    return plugin->RunScript( );
}

// ================ Callbacks for scripting engine's plugin ================

// =========================== Common callbacks  ===========================

// Callback to get name of the host running scripting engine plug-in
xstring XAutomationServerData::ScriptingEnginePluginCallback_GetHostName( )
{
    return XStringAlloc( HostName.c_str( ) );
}

// Callback to get version of the host running scripting engine plug-in
void XAutomationServerData::ScriptingEnginePluginCallback_GetHostVersion( xversion* version )
{
    if ( version )
    {
        *version = HostVersion;
    }
}

// Callback to print a string message at the host
void XAutomationServerData::ScriptingEnginePluginCallback_PrintString( xstring message )
{
    XUNREFERENCED_PARAMETER( message );
    // printf( "message from script: %s \n", message );
}

// Callback to create plug-in instance
XErrorCode XAutomationServerData::ScriptingEnginePluginCallback_CreatePluginInstance( xstring xPluginName, PluginDescriptor** pDescriptor, void **pPlugin )
{
    XErrorCode ret  = SuccessCode;

    if ( ( pDescriptor == nullptr ) || ( pPlugin == nullptr ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        string                              pluginName( xPluginName );
        size_t                              dotIndex    = pluginName.find( '.' );
        shared_ptr<const XPluginDescriptor> pluginDesc;

        if ( dotIndex == string::npos )
        {
            pluginDesc = PluginsEngine->GetPlugin( pluginName );
        }
        else
        {
            string moduleName     = pluginName.substr( 0, dotIndex );
            string pluginNameOnly = pluginName.substr( dotIndex + 1 );

            shared_ptr<const XPluginsModule> module = PluginsEngine->GetModule( moduleName );
            if ( module )
            {
                pluginDesc = module->GetPlugin( pluginNameOnly );
            }
        }

        if ( !pluginDesc )
        {
            ret = ErrorPluginNotFound;
        }
        else
        {
            PluginDescriptor* tempDescriptor = pluginDesc->GetPluginDescriptorCopy( );

            *pPlugin = tempDescriptor->Creator( );

            if ( *pPlugin == nullptr )
            {
                ret = ErrorFailedPluginInstantiation;
                FreePluginDescriptor( &tempDescriptor );
            }
            else
            {
                *pDescriptor = tempDescriptor;
            }
        }
    }

    return ret;
}

// Callback to get xvariant variable from the host side
XErrorCode XAutomationServerData::ScriptingEnginePluginCallback_GetVariable( xstring name, xvariant* value )
{
    XErrorCode ret = ErrorNullParameter;

    if ( ( name != nullptr ) && ( value != nullptr ) )
    {
        XVariant hostValue;

        ret = GetHostVariable( name, hostValue );

        if ( ret == SuccessCode )
        {
            ret = XVariantCopy( hostValue, value );
        }
    }

    return ret;
}

// Callback to store xvariant variable on the host side
XErrorCode XAutomationServerData::ScriptingEnginePluginCallback_SetVariable( xstring name, const xvariant* value )
{
    XErrorCode ret = ErrorNullParameter;

    if ( ( name != nullptr ) && ( value != nullptr ) )
    {
        if ( value->type == XVT_Image )
        {
            // still store it as image, not variant
            ret = SetHostVariable( name, XImage::Create( value->value.imageVal ) );
        }
        else
        {
            ret = SetHostVariable( name, XVariant( *value ) );
        }
    }

    return ret;
}

// Callback to get image variable from the host side
XErrorCode XAutomationServerData::ScriptingEnginePluginCallback_GetImageVariable( xstring name, ximage** value )
{
    XErrorCode ret = ErrorNullParameter;

    if ( ( name != nullptr ) && ( value != nullptr ) )
    {
        shared_ptr<XImage> image;

        ret = GetHostVariable( name, image );

        if ( ( ret == SuccessCode ) && ( image ) )
        {
            *value = image->GetImageDataOwnership( );
        }
        else
        {
            *value = nullptr;
        }
    }

    return ret;
}

// Callback to store image variable on the host side
XErrorCode XAutomationServerData::ScriptingEnginePluginCallback_SetImageVariable( xstring name, const ximage* value )
{
    XErrorCode ret = ErrorNullParameter;

    if ( ( name != nullptr ) && ( value != nullptr ) )
    {
        ret = SetHostVariable( name, XImage::Create( value ) );
    }

    return ret;
}

// ==================== Video source specific callbacks ====================

// Callback to get name of the host running scripting engine plug-in
xstring VideoSourceData::ScriptingEnginePluginCallback_GetHostName( void* userParam )
{
    return static_cast<VideoSourceData*>( userParam )->Server->ScriptingEnginePluginCallback_GetHostName( );
}

// Callback to get version of the host running scripting engine plug-in
void VideoSourceData::ScriptingEnginePluginCallback_GetHostVersion( void* userParam, xversion* version )
{
    static_cast<VideoSourceData*>( userParam )->Server->ScriptingEnginePluginCallback_GetHostVersion( version );
}

// Callback to print a string message at the host
void VideoSourceData::ScriptingEnginePluginCallback_PrintString( void* userParam, xstring message )
{
    static_cast<VideoSourceData*>( userParam )->Server->ScriptingEnginePluginCallback_PrintString( message );
}

// Callback to create plug-in instance
XErrorCode VideoSourceData::ScriptingEnginePluginCallback_CreatePluginInstance( void* userParam, xstring pluginName, PluginDescriptor** pDescriptor, void **pPlugin )
{
    return static_cast<VideoSourceData*>( userParam )->Server->ScriptingEnginePluginCallback_CreatePluginInstance( pluginName, pDescriptor , pPlugin );
}

// Callback to get xvariant variable from the host side
XErrorCode VideoSourceData::ScriptingEnginePluginCallback_GetVariable( void* userParam, xstring name, xvariant* value )
{
    return static_cast<VideoSourceData*>( userParam )->Server->ScriptingEnginePluginCallback_GetVariable( name, value );
}

// Callback to store xvariant variable on the host side
XErrorCode VideoSourceData::ScriptingEnginePluginCallback_SetVariable( void* userParam, xstring name, const xvariant* value )
{
    return static_cast<VideoSourceData*>( userParam )->Server->ScriptingEnginePluginCallback_SetVariable( name, value );
}

// Callback to get image variable from the host side
XErrorCode VideoSourceData::ScriptingEnginePluginCallback_GetImageVariable( void* userParam, xstring name, ximage** value )
{
    return static_cast<VideoSourceData*>( userParam )->Server->ScriptingEnginePluginCallback_GetImageVariable( name, value );
}

// Callback to store image variable on the host side
XErrorCode VideoSourceData::ScriptingEnginePluginCallback_SetImageVariable( void* userParam, xstring name, const ximage* value )
{
    return static_cast<VideoSourceData*>( userParam )->Server->ScriptingEnginePluginCallback_SetImageVariable( name, value );
}

// Callback to get current image available on the host side
XErrorCode VideoSourceData::ScriptingEnginePluginCallback_GetImage( void* userParam, ximage** image )
{
    VideoSourceData* self = static_cast<VideoSourceData*>( userParam );
    XErrorCode       ret  = ErrorFailed;

    if ( self->LastImage )
    {
        ximage* hostImage = self->LastImage->ImageData( );

        ret = XImageCreate( hostImage->data, hostImage->width, hostImage->height, hostImage->stride, hostImage->format, image );
    }

    return ret;
}

// Callback to set/replace current image on the host side
XErrorCode VideoSourceData::ScriptingEnginePluginCallback_SetImage( void* userParam, ximage* image )
{
    VideoSourceData* self = static_cast<VideoSourceData*>( userParam );
    XErrorCode       ret  = ErrorFailed;

    if ( self->LastImage )
    {
        ret = SuccessCode;

        if ( self->LastImage->Data( ) != image->data )
        {

            if ( !XImage::Create( image )->CopyDataOrClone( self->LastImage ) )
            {
                ret = ErrorOutOfMemory;
            }
        }
    }

    return ret;
}

// Callback to get video source associated with the running script
XErrorCode VideoSourceData::ScriptingEnginePluginCallback_GetVideoSource( void* userParam, PluginDescriptor** pDescriptor, void** pPlugin )
{
    VideoSourceData* self = static_cast<VideoSourceData*>( userParam );
    XErrorCode       ret  = SuccessCode;

    if ( ( pDescriptor == nullptr ) || ( pPlugin == nullptr ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        PluginDescriptor* tempDescriptor = self->VideoSourceDescriptor->GetPluginDescriptorCopy( );

        if ( tempDescriptor == nullptr )
        {
            ret = ErrorOutOfMemory;
        }
        else
        {
            *pDescriptor = tempDescriptor;
            *pPlugin     = self->VideoSource->PluginObject( );
        }
    }

    return ret;
}

// ================== Scripting thread specific callbacks ==================

// Callback to get name of the host running scripting engine plug-in
xstring ScriptingThreadData::ScriptingEnginePluginCallback_GetHostName( void* userParam )
{
    return static_cast<ScriptingThreadData*>( userParam )->Server->ScriptingEnginePluginCallback_GetHostName( );
}

// Callback to get version of the host running scripting engine plug-in
void ScriptingThreadData::ScriptingEnginePluginCallback_GetHostVersion( void* userParam, xversion* version )
{
    static_cast<ScriptingThreadData*>( userParam )->Server->ScriptingEnginePluginCallback_GetHostVersion( version );
}

// Callback to print a string message at the host
void ScriptingThreadData::ScriptingEnginePluginCallback_PrintString( void* userParam, xstring message )
{
    static_cast<ScriptingThreadData*>( userParam )->Server->ScriptingEnginePluginCallback_PrintString( message );
}

// Callback to create plug-in instance
XErrorCode ScriptingThreadData::ScriptingEnginePluginCallback_CreatePluginInstance( void* userParam, xstring pluginName, PluginDescriptor** pDescriptor, void **pPlugin )
{
    return static_cast<ScriptingThreadData*>( userParam )->Server->ScriptingEnginePluginCallback_CreatePluginInstance( pluginName, pDescriptor, pPlugin );
}

// Callback to get xvariant variable from the host side
XErrorCode ScriptingThreadData::ScriptingEnginePluginCallback_GetVariable( void* userParam, xstring name, xvariant* value )
{
    return static_cast<ScriptingThreadData*>( userParam )->Server->ScriptingEnginePluginCallback_GetVariable( name, value );
}

// Callback to store xvariant variable on the host side
XErrorCode ScriptingThreadData::ScriptingEnginePluginCallback_SetVariable( void* userParam, xstring name, const xvariant* value )
{
    return static_cast<ScriptingThreadData*>( userParam )->Server->ScriptingEnginePluginCallback_SetVariable( name, value );
}

// Callback to get image variable from the host side
XErrorCode ScriptingThreadData::ScriptingEnginePluginCallback_GetImageVariable( void* userParam, xstring name, ximage** value )
{
    return static_cast<ScriptingThreadData*>( userParam )->Server->ScriptingEnginePluginCallback_GetImageVariable( name, value );
}

// Callback to store image variable on the host side
XErrorCode ScriptingThreadData::ScriptingEnginePluginCallback_SetImageVariable( void* userParam, xstring name, const ximage* value )
{
    return static_cast<ScriptingThreadData*>( userParam )->Server->ScriptingEnginePluginCallback_SetImageVariable( name, value );
}

// Callback to get current image available on the host side
XErrorCode ScriptingThreadData::ScriptingEnginePluginCallback_GetImage( void* userParam, ximage** image )
{
    XUNREFERENCED_PARAMETER( userParam )
    XUNREFERENCED_PARAMETER( image )
    return ErrorNotImplemented;
}

// Callback to set/replace current image on the host side
XErrorCode ScriptingThreadData::ScriptingEnginePluginCallback_SetImage( void* userParam, ximage* image )
{
    XUNREFERENCED_PARAMETER( userParam )
    XUNREFERENCED_PARAMETER( image )
    return ErrorNotImplemented;
}

// Callback to get video source associated with the running script - not supported for threads
XErrorCode ScriptingThreadData::ScriptingEnginePluginCallback_GetVideoSource( void* userParam, PluginDescriptor** pDescriptor, void** pPlugin )
{
    XUNREFERENCED_PARAMETER( userParam )
    XUNREFERENCED_PARAMETER( pDescriptor )
    XUNREFERENCED_PARAMETER( pPlugin )
    return ErrorNotImplemented;
}

// =========================================================================

// Handler of script processing thread
void ScriptingThreadData::ScriptProcessingThreadHandler( void* param )
{
    ScriptingThreadData* self = static_cast<ScriptingThreadData*>( param );
    uint32_t             scriptInterval = self->MsecInterval;

    ScriptingEnginePluginCallbacks     callbacks;

    callbacks.GetHostName          = ScriptingEnginePluginCallback_GetHostName;
    callbacks.GetHostVersion       = ScriptingEnginePluginCallback_GetHostVersion;
    callbacks.PrintString          = ScriptingEnginePluginCallback_PrintString;
    callbacks.CreatePluginInstance = ScriptingEnginePluginCallback_CreatePluginInstance;
    callbacks.GetImage             = ScriptingEnginePluginCallback_GetImage;
    callbacks.SetImage             = ScriptingEnginePluginCallback_SetImage;
    callbacks.GetVariable          = ScriptingEnginePluginCallback_GetVariable;
    callbacks.SetVariable          = ScriptingEnginePluginCallback_SetVariable;
    callbacks.GetImageVariable     = ScriptingEnginePluginCallback_GetImageVariable;
    callbacks.SetImageVariable     = ScriptingEnginePluginCallback_SetImageVariable;
    callbacks.GetVideoSource       = ScriptingEnginePluginCallback_GetVideoSource;

    // set callback first to allow script interface with the host
    self->ScriptingEngine->SetCallbacks( &callbacks, self );

    if ( ( self->ScriptingEngine->Init( ) != SuccessCode ) ||
         ( self->ScriptingEngine->LoadScript( ) != SuccessCode ) ||
         ( self->ScriptingEngine->InitScript( ) != SuccessCode ) )
    {
        // handle script error;
        printf( "failed initializing scripting engine \n" );
    }
    else
    {
        uint32_t sleepTime = 0;

        do
        {
            steady_clock::time_point scriptStartTime = steady_clock::now( );

            if ( self->ScriptingEngine->RunScript( ) != SuccessCode )
            {
                printf( "printf failed running script in a thread \n" );
            }

            uint32_t timeTaken = static_cast<uint32_t>( duration_cast<std::chrono::milliseconds>( steady_clock::now( ) - scriptStartTime ).count( ) );

            sleepTime = ( timeTaken >= scriptInterval ) ? 1 : scriptInterval - timeTaken;

        }
        while ( !self->NeedToExit.Wait( sleepTime ) );
    }
}

} } // namespace CVSandbox::Automation
