/*
    Automation server's test application

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

#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <stdio.h>
#include <string>

#include <XThread.hpp>
#include <XAutomationServer.hpp>
#include <XPluginsEngine.hpp>
#include <XScriptingEnginePlugin.hpp>
#include <XVideoSourceProcessingGraph.hpp>

using namespace std;
using namespace CVSandbox;
using namespace CVSandbox::Automation;
using namespace CVSandbox::Threading;

// Listener to get video source events from automation server
class VideoSourceListener : public IAutomationVideoSourceListener
{
public:
    VideoSourceListener( const string& prefix ) :
        mPrefix( prefix )
    {
    }

    virtual void OnNewVideoFrame( uint32_t videoSourceId, const shared_ptr<const XImage>& image )
    {
        printf( "%s Got image from %d, size %d x %d, format = %d \n", mPrefix.c_str( ), videoSourceId, image->Width( ), image->Height( ), image->Format( ) );
    }

    virtual void OnErrorMessage( uint32_t videoSourceId, const string& errorMessage )
    {
        printf( "Error from %d : %s \n", videoSourceId, errorMessage.c_str( ) );
    }

private:
    string mPrefix;
};

// Create instance of the specified video source and add it to the automation server
static uint32_t AddVideoSourceToServer( const shared_ptr<const XPluginDescriptor>& pluginDescription,
                                        const shared_ptr<XAutomationServer>& server )
{
    uint32_t ret = 0;

    if ( pluginDescription )
    {
        shared_ptr<XPlugin> plugin = pluginDescription->CreateInstance( );

        if ( plugin )
        {
            shared_ptr<XVideoSourcePlugin> vsp = static_pointer_cast<XVideoSourcePlugin>( plugin );

            ret = server->AddVideoSource( pluginDescription, vsp );
        }
        else
        {
            printf( "Failed creating plug-in: %s \n", pluginDescription->Name( ).c_str( ) );
        }
    }

    return ret;
}

int main( int , char* [] )
{
    //_CrtSetBreakAlloc(3720);

    printf( "Starting the test \n" );

    {
        shared_ptr<XPluginsEngine>    engine = XPluginsEngine::Create( );
        shared_ptr<XAutomationServer> server = XAutomationServer::Create( engine );
        VideoSourceListener           videoListener( "->" );
        VideoSourceListener           videoListener2( "=>" );
        int                           serverTestsCount = 1; // 10;
        int                           videoTestsCount  = 1; // 100;

        // load plug-ins and start the server
        engine->CollectModules( "./cvsplugins/" );

        // find video sources
        XGuid    videoPluginId1( 0xAF000003, 0x00000000, 0x00000009, 0x00000001 );
        XGuid    videoPluginId2( 0xAF000003, 0x00000000, 0x00000009, 0x00000002 );
        uint32_t videoId1  = 0;
        uint32_t videoId2  = 0;
        uint32_t threadId1 = 0;
        uint32_t threadId2 = 0;

        shared_ptr<const XPluginDescriptor> pluginDesc1 = engine->GetPlugin( videoPluginId1 );
        shared_ptr<const XPluginDescriptor> pluginDesc2 = engine->GetPlugin( videoPluginId2 );

        // create video processing graph
        XVideoSourceProcessingGraph graph;

        graph.AddStep( XVideoSourceProcessingStep( "Sepia", XGuid( 0xAF000003, 0x00000000, 0x00000006, 0x00000001 ) ) );
        graph.AddStep( XVideoSourceProcessingStep( "Rotate RGB", XGuid( 0xAF000003, 0x00000000, 0x00000006, 0x00000003 ) ) );

        // create Lua scripting plug-in to use as video processing step for one of the video sources
        XVideoSourceProcessingStep luaScriptStep = XVideoSourceProcessingStep( "Lua", XGuid( 0xAF000003, 0x00000000, 0x0000000D, 0x00000001 ) );
        map<string, XVariant>      config;

        config.insert( pair<string, XVariant>( "scriptFile", XVariant( "automation_test.lua" ) ) );
        luaScriptStep.SetPluginConfiguration( config );
        graph.AddStep( luaScriptStep );

        // create scripting engine plug-in to use for thread in automation server
        shared_ptr<const XPluginDescriptor> pluginDescriptor = engine->GetPlugin( XGuid( 0xAF000003, 0x00000000, 0x0000000D, 0x00000001 ) );
        shared_ptr<XScriptingEnginePlugin>  scriptingPlugin1;
        shared_ptr<XScriptingEnginePlugin>  scriptingPlugin2;

        if ( pluginDescriptor )
        {
            printf( "Found scripting engine: %s \n", pluginDescriptor->Name( ).c_str( ) );

            scriptingPlugin1 = static_pointer_cast<XScriptingEnginePlugin>( pluginDescriptor->CreateInstance( ) );
            scriptingPlugin2 = static_pointer_cast<XScriptingEnginePlugin>( pluginDescriptor->CreateInstance( ) );

            if ( scriptingPlugin1 )
            {
                scriptingPlugin1->SetScriptFile( "thread1.lua" );
            }
            if ( scriptingPlugin2 )
            {
                scriptingPlugin2->SetScriptFile( "thread2.lua" );
            }
        }

        // run the automation server
        for ( int i = 0; i < serverTestsCount; i++ )
        {
            server->Start( );

            for ( int j = 0; j < videoTestsCount; j++ )
            {
                printf( "Test iteration: %d / %d ... \n", i, j );

                if ( pluginDesc1 )
                {
                    printf( "plug-in name: %s \n", pluginDesc1->Name( ).c_str( ) );
                    videoId1 = AddVideoSourceToServer( pluginDesc1, server );
                    printf( "video 1 id: %u \n", videoId1 );
                }
                if ( pluginDesc2 )
                {
                    printf( "plug-in name: %s \n", pluginDesc2->Name( ).c_str( ) );
                    videoId2 = AddVideoSourceToServer( pluginDesc2, server );
                    printf( "video 2 id: %u \n", videoId2 );
                }
                if ( scriptingPlugin1 )
                {
                    threadId1 = server->AddThread( scriptingPlugin1, 100 );
                    printf( "thread 1 id: %u \n", threadId1 );
                }
                if ( scriptingPlugin2 )
                {
                    threadId2 = server->AddThread( scriptingPlugin2, 50 );
                    printf( "thread 2 id: %u \n", threadId2 );
                }

                server->SetVideoProcessingGraph( videoId1, graph );

                server->AddVideoSourceListener( videoId1, &videoListener );
                server->AddVideoSourceListener( videoId2, &videoListener );
                server->AddVideoSourceListener( videoId2, &videoListener2 );
                server->AddVideoSourceListener( videoId2, 0 );
                server->AddVideoSourceListener( videoId2, 0 );

                if ( !server->StartVideoSource( videoId1 ) )
                {
                    printf( "failed starting video source: %u \n", videoId1 );
                }
                if ( !server->StartVideoSource( videoId2 ) )
                {
                    printf( "failed starting video source: %u \n", videoId2 );
                }
                if ( !server->StartThread( threadId1 ) )
                {
                    printf( "failed starting thread: %u \n", threadId1 );
                }
                if ( !server->StartThread( threadId2 ) )
                {
                    printf( "failed starting thread: %u \n", threadId2 );
                }

                if ( !server->EnableVideoProcessingPerformanceMonitor( videoId1, true ) )
                {
                    printf( "failed starting performance monitor: %u \n", videoId1 );
                }

                XThread::Sleep( 100 );

                server->AddVideoSourceListener( videoId1, &videoListener2 );
                server->AddVideoSourceListener( videoId1, 0 );

                XThread::Sleep( 500 * 1 /* 10 */ );

                server->RemoveVideoSourceListener( videoId2, &videoListener2 );

                // get performance info for the 1st video source
                server->EnableVideoProcessingPerformanceMonitor( videoId1, false );
                vector<float> graphTiming = server->GetVideoProcessingGraphTiming( videoId1 );

                for ( size_t t = 0; t < graphTiming.size( ); t++ )
                {
                    printf( "Step %zd : %.2f \n", t, graphTiming[t] );
                }

                if ( !server->FinalizeVideoSource( videoId1 ) )
                {
                    printf( "failed finalizing video source: %u \n", videoId1 );
                }

                server->FinalizeVideoSource( videoId2 );
                server->FinalizeThread( threadId1 );
                server->FinalizeThread( threadId2 );
            }

            server->SignalToStop( );
            server->WaitForStop( );
        }
    }

    printf( "========================== \r\n" );
    printf( "Test Done \r\n" );
    printf( "========================== \r\n" );

    getchar( );

    #ifdef _MSC_VER
    _CrtDumpMemoryLeaks();
    #endif

    return 0;
}
