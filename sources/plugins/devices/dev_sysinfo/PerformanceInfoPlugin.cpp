/*
    System Info plug-ins of Computer Vision Sandbox

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

#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
#include "PerformanceInfoPlugin.hpp"

namespace Private
{
    class PerformanceInfoPluginData
    {
    public:
        PerformanceInfoPluginData( ) :
            ProcessorsCount( 0 ),
            PerformanceQuery( INVALID_HANDLE_VALUE ),
            SystemLoadCounter( INVALID_HANDLE_VALUE )
        {
        }

    public:
        DWORD       ProcessorsCount;

        HQUERY      PerformanceQuery;
        HCOUNTER    SystemLoadCounter;
    };
}

PerformanceInfoPlugin::PerformanceInfoPlugin( ) :
    data( new Private::PerformanceInfoPluginData( ) ),
    isConnected( false )
{
}

void PerformanceInfoPlugin::Dispose( )
{
    delete data;
    delete this;
}

// Initialize performance counters
XErrorCode PerformanceInfoPlugin::Connect( )
{
    SYSTEM_INFO sysInfo = { 0 };
    PDH_STATUS  status;
    XErrorCode  ret = ErrorConnectionFailed;

    GetSystemInfo( &sysInfo );
    data->ProcessorsCount = sysInfo.dwNumberOfProcessors;

    status = PdhOpenQuery( NULL, 0, &data->PerformanceQuery );
    if ( status == ERROR_SUCCESS )
    {
        // 238 = Processor, 6 = % Processor Time

        // add all required counters
        PdhAddCounter( data->PerformanceQuery, "\\238(_Total)\\6", 0, &data->SystemLoadCounter );

        // do initial query
        PdhCollectQueryData( data->PerformanceQuery );

        isConnected = true;
        ret = SuccessCode;
    }

    return ret;
}

// Close performance counters
void PerformanceInfoPlugin::Disconnect( )
{
    if ( isConnected )
    {
        if ( data->SystemLoadCounter != INVALID_HANDLE_VALUE )
        {
            PdhRemoveCounter( data->PerformanceQuery );
            data->SystemLoadCounter = INVALID_HANDLE_VALUE;
        }

        if ( data->PerformanceQuery != INVALID_HANDLE_VALUE )
        {
            PdhCloseQuery( data->PerformanceQuery );
            data->PerformanceQuery = INVALID_HANDLE_VALUE;
        }
        isConnected = false;
    }
}

// Check connection status
bool PerformanceInfoPlugin::IsConnected( )
{
    return isConnected;
}

// Get specified property value of the plug-in
XErrorCode PerformanceInfoPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    // although connection is not required, we still follow the common way
    if ( !isConnected )
    {
        ret = ErrorNotConnected;
    }
    else if ( ( id < 0 ) || ( id > 1 ) )
    {
        ret = ErrorInvalidProperty;
    }
    else
    {
        PDH_FMT_COUNTERVALUE    counterValue;

        if ( id == 1 )
        {
            if ( PdhCollectQueryData( data->PerformanceQuery ) != ERROR_SUCCESS )
            {
                ret = ErrorFailed;
            }
        }

        if ( ret == SuccessCode )
        {
            switch ( id )
            {
            case 0:
                value->type = XVT_U2;
                value->value.uiVal = data->ProcessorsCount;
                break;

            case 1:
                if ( PdhGetFormattedCounterValue( data->SystemLoadCounter, PDH_FMT_DOUBLE, NULL, &counterValue ) != ERROR_SUCCESS )
                {
                    ret = ErrorFailed;
                }
                else
                {
                    value->type = XVT_R4;
                    value->value.fVal = static_cast<float>( counterValue.doubleValue );
                }
                break;
            }
        }
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode PerformanceInfoPlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode ret = SuccessCode;

    XUNREFERENCED_PARAMETER( id )
    XUNREFERENCED_PARAMETER( value )

    if ( !isConnected )
    {
        ret = ErrorNotConnected;
    }
    else if ( ( id < 0 ) || ( id > 1 ) )
    {
        ret = ErrorInvalidProperty;
    }
    else
    {
        ret = ErrorReadOnlyProperty;
    }

    return ret;
}
