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

#include <QTimer>
#include <QTime>
#include <numeric>
#include <chrono>
#include <XVideoSourceFrameInfo.hpp>
#include "PerformanceMonitorService.hpp"
#include "ServiceManager.hpp"

#ifdef _WIN32
    #include <windows.h>
    #include <pdh.h>
    #include <pdhmsg.h>
#endif

using namespace std;
using namespace std::chrono;
using namespace CVSandbox::Automation;

// Number of the last FPS values to average
#define FPS_HISTORY_LENGTH (10)

namespace Private
{
    class PerformanceMonitorServiceData
    {
    public:
        PerformanceMonitorServiceData( ) :
            TotalFps( -1 ), TotalCpuLoad( -1 )
        {
            UpdateTimer.setInterval( 1000 );
/*
#ifdef _WIN32
            PerformanceQuery  = INVALID_HANDLE_VALUE;
            SystemLoadCounter = INVALID_HANDLE_VALUE;
#endif
*/
        }

        void Reset( )
        {
            LastFramesCount = vector<uint32_t>( VideoSourceIds.size( ) );
            LastFpsValues   = vector<vector<float>>( VideoSourceIds.size( ) );
            NextFpsIndex    = vector<int>( VideoSourceIds.size( ) );

            TotalFps        = -1;
            TotalCpuLoad    = -1;

            StartTime = steady_clock::now( );
            TimeSinceStarted = duration_cast<milliseconds>( StartTime - StartTime );
        }

        void UpdateFrameRateInfo( );
        void UpdateCpuLoadInfo( );

    public:
        float                   TotalFps;
        float                   TotalCpuLoad;

        QTimer                  UpdateTimer;
        QTime                   LastUpdated;
        vector<uint32_t>        VideoSourceIds;
        map<uint32_t, bool>     VideoSourceReportFpsState;
        vector<uint32_t>        LastFramesCount;
        vector<vector<float>>   LastFpsValues;
        vector<int>             NextFpsIndex;

        steady_clock::time_point StartTime;
        system_clock::duration   TimeSinceStarted;

#ifdef _WIN32
        DWORD       ProcessorsCount;

        FILETIME    SystemKernelTime;
        FILETIME    SystemUserTime;

        FILETIME    ProcessKernelTime;
        FILETIME    ProcessUserTime;

        //HQUERY      PerformanceQuery;
        //HCOUNTER    SystemLoadCounter;
#endif
    };
}

PerformanceMonitorService::PerformanceMonitorService( ) :
    mData( new ::Private::PerformanceMonitorServiceData( ) )
{
    // connect timer
    connect( &mData->UpdateTimer, SIGNAL(timeout()), this, SLOT(on_UpdateTimer_timeout()) );
}

PerformanceMonitorService::~PerformanceMonitorService( )
{
    delete mData;
}

// Start collecting performance info - single video source
void PerformanceMonitorService::Start( uint32_t sourceId )
{
    mData->VideoSourceIds.clear( );
    mData->VideoSourceIds.push_back( sourceId );
    mData->VideoSourceReportFpsState.clear( );
    mData->VideoSourceReportFpsState.insert( pair<uint32_t, bool>( sourceId, true ) );
    Start( );
}

// Start collecting performance info - multiple video sources
void PerformanceMonitorService::Start( vector<uint32_t> sourceIds, map<uint32_t, bool> reportFpsState )
{
    mData->VideoSourceIds = sourceIds;
    mData->VideoSourceReportFpsState = reportFpsState;
    Start( );
}

void PerformanceMonitorService::Start( )
{
    mData->Reset( );

#ifdef _WIN32
    FILETIME    t1, t2, t3;

    GetProcessTimes( GetCurrentProcess( ), &t1, &t2, &mData->ProcessKernelTime, &mData->ProcessUserTime );
    GetSystemTimes( &t3, &mData->SystemKernelTime, &mData->SystemUserTime );

    /*
    PDH_STATUS status = PdhOpenQuery( NULL, 0, &mData->PerformanceQuery );

    if ( status == ERROR_SUCCESS )
    {
        PdhAddCounter( mData->PerformanceQuery, L"\\238(_Total)\\6", 0, &mData->SystemLoadCounter );
    }
    */
#endif

    mData->UpdateTimer.start( );
}

// Stop collecting any performance info
void PerformanceMonitorService::Stop( )
{
#ifdef _WIN32
    /*`
    if ( mData->SystemLoadCounter != INVALID_HANDLE_VALUE )
    {
        PdhRemoveCounter( mData->PerformanceQuery );
        mData->SystemLoadCounter = INVALID_HANDLE_VALUE;
    }
    if ( mData->PerformanceQuery != INVALID_HANDLE_VALUE )
    {
        PdhCloseQuery( mData->PerformanceQuery );
        mData->PerformanceQuery = INVALID_HANDLE_VALUE;
    }
    */
#endif

    mData->UpdateTimer.stop( );
    mData->Reset( );

    emit Updated( );
}

// Update all info and fire a signal
void PerformanceMonitorService::on_UpdateTimer_timeout( )
{
    mData->UpdateFrameRateInfo( );
    mData->UpdateCpuLoadInfo( );

    mData->TimeSinceStarted = duration_cast<milliseconds>( steady_clock::now( ) - mData->StartTime );

    emit Updated( );
}

float PerformanceMonitorService::TotalCpuLoad( ) const
{
    return mData->TotalCpuLoad;
}

float PerformanceMonitorService::TotalFrameRate( ) const
{
    return mData->TotalFps;
}

system_clock::duration PerformanceMonitorService::UpTime( ) const
{
    return mData->TimeSinceStarted;
}

namespace Private
{

// Update total frame rate from all running cameras
void PerformanceMonitorServiceData::UpdateFrameRateInfo( )
{
    struct XVideoSourceFrameInfo frameInfo;
    auto automationServer = ServiceManager::Instance( ).GetAutomationServer( );

    TotalFps = 0;

    for ( int i = 0, n = VideoSourceIds.size( ); i < n; i++ )
    {
        if ( VideoSourceReportFpsState[VideoSourceIds[i]] )
        {
            if ( automationServer->GetVideoSourceFrameInfo( VideoSourceIds[i], &frameInfo ) )
            {
                // frame rate
                if ( (  LastFramesCount[i] != 0 ) && ( LastFramesCount[i] <= frameInfo.FramesReceived ) )
                {
                    float fps = static_cast<float>( LastUpdated.elapsed( ) ) * ( frameInfo.FramesReceived - LastFramesCount[i] ) / 1000.0f;

                    if ( LastFpsValues[i].size( ) < FPS_HISTORY_LENGTH )
                    {
                        LastFpsValues[i].push_back( fps );
                    }
                    else
                    {
                        LastFpsValues[i][NextFpsIndex[i]] = fps;

                        NextFpsIndex[i]++;
                        NextFpsIndex[i] %= FPS_HISTORY_LENGTH;
                    }

                    // get average value
                    double fpsAvg = std::accumulate( LastFpsValues[i].begin( ), LastFpsValues[i].end( ), 0.0f ) / LastFpsValues[i].size( );

                    TotalFps += static_cast<float>( fpsAvg );
                }

                LastFramesCount[i] = frameInfo.FramesReceived;
            }
        }
    }

    LastUpdated.restart( );
}

// Helper to get file time difference
static uint64_t FileTimeDiff( const FILETIME& timeOld, const FILETIME& timeNew )
{
    ULARGE_INTEGER  t1, t2;

    t1.LowPart  = timeOld.dwLowDateTime;
    t1.HighPart = timeOld.dwHighDateTime;

    t2.LowPart  = timeNew.dwLowDateTime;
    t2.HighPart = timeNew.dwHighDateTime;

    return ( t2.QuadPart - t1.QuadPart );
}

// Update total CPU load in %
void PerformanceMonitorServiceData::UpdateCpuLoadInfo( )
{
#ifdef _WIN32
    FILETIME t1, t2, t3;
    FILETIME systemKernelTimeNew;
    FILETIME systemUserTimeNew;

    FILETIME processKernelTimeNew;
    FILETIME processUserTimeNew;

    if ( ( GetProcessTimes( GetCurrentProcess( ), &t1, &t2, &processKernelTimeNew, &processUserTimeNew ) ) &&
         ( GetSystemTimes( &t3, &systemKernelTimeNew, &systemUserTimeNew ) ) )
    {
        // FileTimeDiff( ProcessKernelTime, processKernelTimeNew )
        TotalCpuLoad = static_cast<float>(
                       static_cast<double>( FileTimeDiff( ProcessKernelTime, processKernelTimeNew ) +
                                            FileTimeDiff( ProcessUserTime, processUserTimeNew ) ) * 100 /
                                        ( ( FileTimeDiff( SystemKernelTime, systemKernelTimeNew ) +
                                            FileTimeDiff( SystemUserTime, systemUserTimeNew ) ) ) );

        ProcessKernelTime = processKernelTimeNew;
        ProcessUserTime   = processUserTimeNew;
        SystemKernelTime  = systemKernelTimeNew;
        SystemUserTime    = systemUserTimeNew;
    }

    /*
    if ( ( SystemLoadCounter != INVALID_HANDLE_VALUE ) && ( PdhCollectQueryData( PerformanceQuery ) == ERROR_SUCCESS ) )
    {
        PDH_FMT_COUNTERVALUE counterValue;

        if ( PdhGetFormattedCounterValue( SystemLoadCounter, PDH_FMT_DOUBLE, NULL, &counterValue ) == ERROR_SUCCESS )
        {
            TotalCpuLoad = static_cast<float>( counterValue.doubleValue );
        }
    }*/
#endif
}

} // namespace Private
