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
#include "PowerInfoPlugin.hpp"

PowerInfoPlugin::PowerInfoPlugin( ) :
    isConnected( false )
{
}

void PowerInfoPlugin::Dispose( )
{
    delete this;
}

// Nothing to do for connecting/disconnecting
XErrorCode PowerInfoPlugin::Connect( )
{
    isConnected = true;
    return SuccessCode;
}
void PowerInfoPlugin::Disconnect( )
{
    isConnected = false;
}
bool PowerInfoPlugin::IsConnected( )
{
    return isConnected;
}

// Get specified property value of the plug-in
XErrorCode PowerInfoPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    // although connection is not required, we still follow the common way
    if ( !isConnected )
    {
        ret = ErrorNotConnected;
    }
    else if ( ( id < 0 ) || ( id > 2 ) )
    {
        ret = ErrorInvalidProperty;
    }
    else
    {
        SYSTEM_POWER_STATUS powerStatus = { 0 };

        GetSystemPowerStatus( &powerStatus );

        switch ( id )
        {
        case 0:
            value->type = XVT_U1;
            value->value.ubVal = powerStatus.BatteryLifePercent;

            if ( value->value.ubVal == 255 )
            {
                // if unknown value, still set it to 0
                value->value.ubVal = 0;
            }
            break;

        case 1:
            value->type = XVT_Bool;
            value->value.boolVal = ( powerStatus.ACLineStatus == 1 );
            break;

        case 2:
            value->type = XVT_Bool;
            value->value.boolVal = ( ( powerStatus.BatteryFlag & 8 ) != 0 );
            break;
        }
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode PowerInfoPlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode ret = SuccessCode;

    XUNREFERENCED_PARAMETER( id )
    XUNREFERENCED_PARAMETER( value )

    if ( !isConnected )
    {
        ret = ErrorNotConnected;
    }
    else if ( ( id < 0 ) || ( id > 2 ) )
    {
        ret = ErrorInvalidProperty;
    }
    else
    {
        ret = ErrorReadOnlyProperty;
    }

    return ret;
}
