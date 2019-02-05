/*
    Gamepad device plug-in for Computer Vision Sandbox

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

#include "GamepadPlugin.hpp"
#include <Windows.h>

#include <string>

using namespace std;

namespace Private
{
    class GamepadPluginData
    {
    public:
        uint16_t DeviceId;

        string   DeviceName;
        bool     IsConnected;

        uint16_t ButtonsCount;
        uint16_t AxesCount;

        UINT     AxisMinValue[6];
        UINT     AxisMaxValue[6];

    public:
        GamepadPluginData( ) :
            DeviceId( 0 ), DeviceName( ), IsConnected( false ),
            ButtonsCount( 0 ), AxesCount( 0 )
        {

        }

        ~GamepadPluginData( )
        {
            Close( );
        }

        bool Open( )
        {
            // close anything previosly opened
            Close( );

            JOYCAPSA caps = { 0 };

            if ( joyGetDevCapsA( DeviceId, &caps, sizeof( caps ) ) == JOYERR_NOERROR )
            {
                DeviceName = string( caps.szPname );

                ButtonsCount = static_cast<uint16_t>( XMIN( caps.wNumButtons, 32 ) );
                AxesCount    = static_cast<uint16_t>( XMIN( caps.wNumAxes, 6 ) );

                // get minimum possible values for all axes
                AxisMinValue[0] = caps.wXmin;
                AxisMinValue[1] = caps.wYmin;
                AxisMinValue[2] = caps.wXmin;
                AxisMinValue[3] = caps.wRmin;
                AxisMinValue[4] = caps.wUmin;
                AxisMinValue[5] = caps.wVmin;

                // get maximum possible values for all axes
                AxisMaxValue[0] = caps.wXmax;
                AxisMaxValue[1] = caps.wYmax;
                AxisMaxValue[2] = caps.wXmax;
                AxisMaxValue[3] = caps.wRmax;
                AxisMaxValue[4] = caps.wUmax;
                AxisMaxValue[5] = caps.wVmax;

                IsConnected = true;
            }

            return IsConnected;
        }

        void Close( )
        {
            IsConnected = false;
        }
    };
}

GamepadPlugin::GamepadPlugin( ) :
    mData( new ::Private::GamepadPluginData )
{
}

void GamepadPlugin::Dispose( )
{
    delete mData;
    delete this;
}

// Open Phidget Interface Kit
XErrorCode GamepadPlugin::Connect( )
{
    return ( mData->Open( ) ) ? SuccessCode : ErrorConnectionFailed;
}

// Close from Phidget Interface Kit
void GamepadPlugin::Disconnect( )
{
    mData->Close( );
}

// Check if device is open
bool GamepadPlugin::IsConnected( )
{
    return mData->IsConnected;
}

// Get specified property value of the plug-in
XErrorCode GamepadPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    if ( ( id > 0 ) && ( id <= 6 ) && ( !mData->IsConnected ) )
    {
        ret = ErrorNotConnected;
    }
    else
    {
        switch ( id )
        {
        case 0:
            value->type = XVT_U2;
            value->value.usVal = mData->DeviceId;
            break;

        case 1:
            value->type = XVT_String;
            value->value.strVal = XStringAlloc( mData->DeviceName.c_str( ) );
            break;

        case 2:
            value->type = XVT_U2;
            value->value.usVal = mData->AxesCount;
            break;

        case 3:
            value->type = XVT_U2;
            value->value.usVal = mData->ButtonsCount;
            break;

        case 4:
            {
                static const DWORD flags[6] =
                {
                    JOY_RETURNX,
                    JOY_RETURNX | JOY_RETURNY,
                    JOY_RETURNX | JOY_RETURNY | JOY_RETURNZ,
                    JOY_RETURNX | JOY_RETURNY | JOY_RETURNZ | JOY_RETURNR,
                    JOY_RETURNX | JOY_RETURNY | JOY_RETURNZ | JOY_RETURNR | JOY_RETURNU,
                    JOY_RETURNX | JOY_RETURNY | JOY_RETURNZ | JOY_RETURNR | JOY_RETURNU | JOY_RETURNV
                };

                xarray*  array = nullptr;
                xvariant v;

                ret = XArrayAllocate( &array, XVT_R4, mData->AxesCount );

                if ( ret == SuccessCode )
                {
                    if ( mData->AxesCount != 0 )
                    {
                        v.type = XVT_R4;

                        JOYINFOEX info = { 0 };

                        info.dwSize  = sizeof( info );
                        info.dwFlags = flags[mData->AxesCount - 1];

                        if ( joyGetPosEx( mData->DeviceId, &info ) != JOYERR_NOERROR )
                        {
                            ret = ErrorIOFailure;
                        }
                        else
                        {
                            DWORD axesPos[6] = { info.dwXpos, info.dwYpos, info.dwZpos, info.dwRpos, info.dwUpos, info.dwVpos };

                            for ( uint32_t i = 0, n = mData->AxesCount; i < n; i++ )
                            {
                                v.value.fVal = static_cast<float>( axesPos[i] - mData->AxisMinValue[i] ) /
                                               ( mData->AxisMaxValue[i] - mData->AxisMinValue[i] ) * 2 - 1.0f;
                                XArraySet( array, i, &v );
                            }
                        }
                    }

                    value->type = XVT_R4 | XVT_Array;
                    value->value.arrayVal = array;
                }
            }

            break;

        case 5:
            {
                xarray*  array = nullptr;
                xvariant v;

                ret = XArrayAllocate( &array, XVT_Bool, mData->ButtonsCount );

                if ( ret == SuccessCode )
                {
                    if ( mData->ButtonsCount != 0 )
                    {
                        v.type = XVT_Bool;

                        JOYINFOEX info = { 0 };

                        info.dwSize  = sizeof( info );
                        info.dwFlags = JOY_RETURNBUTTONS;

                        if ( joyGetPosEx( mData->DeviceId, &info ) != JOYERR_NOERROR )
                        {
                            ret = ErrorIOFailure;
                        }
                        else
                        {
                            for ( uint32_t i = 0, n = mData->ButtonsCount; i < n; i++ )
                            {
                                v.value.boolVal = ( ( info.dwButtons & ( 1 << i ) ) != 0 );
                                XArraySet( array, i, &v );
                            }
                        }
                    }

                    value->type = XVT_Bool | XVT_Array;
                    value->value.arrayVal = array;
                }
            }
            break;

        case 6:
            {
                JOYINFOEX info = { 0 };

                info.dwSize  = sizeof( info );
                info.dwFlags = JOY_RETURNPOV;

                if ( joyGetPosEx( mData->DeviceId, &info ) != JOYERR_NOERROR )
                {
                    ret = ErrorIOFailure;
                }
                else
                {
                    value->type = XVT_R4;
                    value->value.fVal = ( info.dwPOV > 35900 ) ? -1.0f : static_cast<float>( info.dwPOV ) / 100.0f;
                }
            }
            break;

        default:
            ret = ErrorInvalidProperty;
            break;
        }
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode GamepadPlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode ret = SuccessCode;

    if ( id == 0 )
    {
        uint16_t deviceId = 0;

        ret = XVariantToUShort( value, &deviceId );

        if ( ret == SuccessCode )
        {
            mData->DeviceId = deviceId;
        }
    }
    else if ( id <= 5 )
    {
        ret = ErrorReadOnlyProperty;
    }
    else
    {
        ret = ErrorInvalidProperty;
    }

    return ret;
}

// Get an element of the specified indexed property of the plug-in
XErrorCode GamepadPlugin::GetIndexedProperty( int32_t id, uint32_t index, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    if ( ( id > 0 ) && ( id <= 6 ) && ( !mData->IsConnected ) )
    {
        ret = ErrorNotConnected;
    }
    else
    {
        switch ( id )
        {
        case 0:
        case 1:
        case 2:
        case 3:
        case 6:
            ret = ErrorNotIndexedProperty;
            break;

        case 4:
            {
                static const DWORD flags[6] =
                {
                    JOY_RETURNX, JOY_RETURNY, JOY_RETURNZ, JOY_RETURNR, JOY_RETURNU, JOY_RETURNV
                };

                if ( index >= mData->AxesCount )
                {
                    ret = ErrorIndexOutOfBounds;
                }
                else
                {
                    JOYINFOEX info = { 0 };

                    info.dwSize  = sizeof( info );
                    info.dwFlags = flags[index];

                    if ( joyGetPosEx( mData->DeviceId, &info ) != JOYERR_NOERROR )
                    {
                        ret = ErrorIOFailure;
                    }
                    else
                    {
                        DWORD axesPos[6]  = { info.dwXpos, info.dwYpos, info.dwZpos, info.dwRpos, info.dwUpos, info.dwVpos };

                        value->type       = XVT_R4;
                        value->value.fVal = static_cast<float>( axesPos[index] - mData->AxisMinValue[index] ) /
                                           ( mData->AxisMaxValue[index] - mData->AxisMinValue[index] ) * 2 - 1.0f;
                    }
                }
            }

            break;

        case 5:
            {
                if ( index >= mData->ButtonsCount )
                {
                    ret = ErrorIndexOutOfBounds;
                }
                else
                {
                    JOYINFOEX info = { 0 };

                    info.dwSize  = sizeof( info );
                    info.dwFlags = JOY_RETURNBUTTONS;

                    if ( joyGetPosEx( mData->DeviceId, &info ) != JOYERR_NOERROR )
                    {
                        ret = ErrorIOFailure;
                    }
                    else
                    {
                        value->type          = XVT_Bool;
                        value->value.boolVal = ( ( info.dwButtons & ( 1 << index ) ) != 0 );
                    }
                }
            }
            break;

        default:
            ret = ErrorInvalidProperty;
            break;
        }
    }

    return ret;
}

// Set an element of the specified indexed property of the plug-in
XErrorCode GamepadPlugin::SetIndexedProperty( int32_t id, uint32_t index, const xvariant* value )
{
    XErrorCode ret = ErrorInvalidProperty;

    XUNREFERENCED_PARAMETER( index )
    XUNREFERENCED_PARAMETER( value )

    if ( id == 0 )
    {
        ret = ErrorNotIndexedProperty;
    }
    else if ( id <= 5 )
    {
        ret = ErrorReadOnlyProperty;
    }

    return ret;
}
