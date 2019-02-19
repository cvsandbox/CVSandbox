/*
    Communication device plug-ins for Computer Vision Sandbox

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

#include "SerialPortPlugin.hpp"
#include <Windows.h>

#include <string>

using namespace std;

namespace Private
{
    class SerialPortPluginData
    {
    public:
        HANDLE  PortHandle;
        bool    IsConnected;

        string  PortName;
        DWORD   BaudRate;
        BYTE    ByteSize;
        BYTE    StopBits;
        BYTE    Parity;

        uint16_t IOTimeoutContant;
        uint16_t IOTimeoutMultiplier;
        bool     UseBlockingInput;

    public:
        SerialPortPluginData( ) :
            PortHandle( INVALID_HANDLE_VALUE ), IsConnected( false ),
            PortName( ), BaudRate( CBR_9600 ),
            ByteSize( 8 ), StopBits( ONESTOPBIT ), Parity( NOPARITY ),
            IOTimeoutContant( 50 ), IOTimeoutMultiplier( 0 ), UseBlockingInput( true )
        {

        }

        ~SerialPortPluginData( )
        {
            Close( );
        }

        bool Open( )
        {
            string fileName( "\\\\.\\" );

            fileName += PortName;

            // close anything previosly opened
            Close( );

            PortHandle = CreateFileA( fileName.c_str( ), GENERIC_READ | GENERIC_WRITE,
                                      0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

            if ( PortHandle != INVALID_HANDLE_VALUE )
            {
                DCB dcbSerialParams = { 0 };

                if ( GetCommState( PortHandle, &dcbSerialParams ) )
                {
                    dcbSerialParams.BaudRate    = BaudRate;
                    dcbSerialParams.ByteSize    = ByteSize;
                    dcbSerialParams.StopBits    = StopBits;
                    dcbSerialParams.Parity      = Parity;
                    dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;

                    if ( SetCommState( PortHandle, &dcbSerialParams ) )
                    {
                        // flush any remaining characters in the buffers 
                        PurgeComm( PortHandle, PURGE_RXCLEAR | PURGE_TXCLEAR );

                        IsConnected = true;
                    }
                }

                // set IO timeouts
                if ( IsConnected )
                {
                    COMMTIMEOUTS timeouts = { 0 };

                    timeouts.ReadIntervalTimeout         = 50;
                    timeouts.ReadTotalTimeoutConstant    = IOTimeoutContant;
                    timeouts.ReadTotalTimeoutMultiplier  = IOTimeoutMultiplier;
                    timeouts.WriteTotalTimeoutConstant   = IOTimeoutContant; 
                    timeouts.WriteTotalTimeoutMultiplier = IOTimeoutMultiplier;

                    SetCommTimeouts( PortHandle, &timeouts );
                }
            }

            if ( ( !IsConnected ) && ( PortHandle != INVALID_HANDLE_VALUE ) )
            {
                CloseHandle( PortHandle );
                PortHandle = INVALID_HANDLE_VALUE;
            }
            
            return IsConnected;
        }

        void Close( )
        {
            if ( PortHandle != INVALID_HANDLE_VALUE )
            {
                CloseHandle( PortHandle );
            }

            PortHandle  = INVALID_HANDLE_VALUE;
            IsConnected = false;
        }

        void PurgeBuffers( )
        {
            PurgeComm( PortHandle, PURGE_RXCLEAR | PURGE_TXCLEAR );
        }

        XErrorCode Write( const uint8_t* buffer, uint32_t nBytesToWrite, uint32_t* nBytesWritten )
        {
            XErrorCode ret = SuccessCode;

            if ( ( buffer == nullptr ) || ( nBytesWritten == nullptr ) )
            {
                ret = ErrorNullParameter;
            }
            else if ( !IsConnected )
            {
                ret = ErrorNotConnected;
            }
            else
            {
                DWORD bytesWritten = 0;

                if ( !WriteFile( PortHandle, buffer, nBytesToWrite, &bytesWritten, NULL ) )
                {
                    ret = ErrorIOFailure;
                }

                *nBytesWritten = static_cast<uint32_t>( bytesWritten );
            }

            return ret;
        }

        XErrorCode Read( uint8_t* buffer, uint32_t nBytesToRead, uint32_t* nBytesRead )
        {
            XErrorCode ret = SuccessCode;

            if ( ( buffer == nullptr ) || ( nBytesRead == nullptr ) )
            {
                ret = ErrorNullParameter;
            }
            else if ( !IsConnected )
            {
                ret = ErrorNotConnected;
            }
            else
            {
                DWORD   bytesToRead = nBytesToRead;
                DWORD   bytesRead   = 0;

                if ( !UseBlockingInput )
                {
                    COMSTAT status;
                    DWORD   errors;

                    ClearCommError( PortHandle, &errors, &status );
                    bytesToRead = XMIN( status.cbInQue, nBytesToRead );
                }

                if ( bytesToRead > 0 )
                {
                    if ( !ReadFile( PortHandle, buffer, bytesToRead, &bytesRead, NULL ) )
                    {
                        ret = ErrorIOFailure;
                    }
                }

                *nBytesRead = static_cast<uint32_t>( bytesRead );
            }

            return ret;
        }
    };
}

SerialPortPlugin::SerialPortPlugin( ) :
    mData( new ::Private::SerialPortPluginData )
{
}

void SerialPortPlugin::Dispose( )
{
    delete mData;
    delete this;
}

// Open Phidget Interface Kit
XErrorCode SerialPortPlugin::Connect( )
{
    return ( mData->Open( ) ) ? SuccessCode : ErrorConnectionFailed;
}

// Close from Phidget Interface Kit
void SerialPortPlugin::Disconnect( )
{
    mData->Close( );
}

// Check if device is open
bool SerialPortPlugin::IsConnected( )
{
    return mData->IsConnected;
}

// Write data to connected device
XErrorCode SerialPortPlugin::Write( const uint8_t* buffer, uint32_t nBytesToWrite, uint32_t* nBytesWritten )
{
    return mData->Write( buffer, nBytesToWrite, nBytesWritten );
}

// Read data from connected device
XErrorCode SerialPortPlugin::Read( uint8_t* buffer, uint32_t nBytesToRead, uint32_t* nBytesRead )
{
    return mData->Read( buffer, nBytesToRead, nBytesRead );
}

// Discard any data in communication buffers (write and read)
void SerialPortPlugin::PurgeBuffers( )
{
    mData->PurgeBuffers( );
}

// Get specified property value of the plug-in
XErrorCode SerialPortPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type         = XVT_String;
        value->value.strVal = XStringAlloc( mData->PortName.c_str( ) );
        break;

    case 1:
        value->type        = XVT_U4;
        value->value.uiVal = mData->BaudRate;
        break;

    case 2:
        value->type        = XVT_U1;
        value->value.ubVal = mData->ByteSize;
        break;

    case 3:
        value->type        = XVT_U1;
        value->value.ubVal = mData->StopBits;
        break;

    case 4:
        value->type        = XVT_U1;
        value->value.ubVal = mData->Parity;
        break;

    case 5:
        value->type        = XVT_U2;
        value->value.usVal = mData->IOTimeoutContant;
        break;

    case 6:
        value->type        = XVT_U2;
        value->value.usVal = mData->IOTimeoutMultiplier;
        break;

    case 7:
        value->type          = XVT_Bool;
        value->value.boolVal = mData->UseBlockingInput;
        break;

    default:
        ret = ErrorInvalidProperty;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode SerialPortPlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode ret = SuccessCode;

    xvariant convertedValue;
    XVariantInit( &convertedValue );

    // make sure property value has expected type
    ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 8, &convertedValue );

    if ( ret == SuccessCode )
    {
        switch ( id )
        {
        case 0:
            mData->PortName = string( convertedValue.value.strVal );
            break;

        case 1:
            mData->BaudRate = convertedValue.value.uiVal;
            break;

        case 2:
            mData->ByteSize = convertedValue.value.ubVal;
            break;

        case 3:
            mData->StopBits = convertedValue.value.ubVal;
            break;

        case 4:
            mData->Parity = convertedValue.value.ubVal;
            break;

        case 5:
            mData->IOTimeoutContant = convertedValue.value.usVal;
            break;

        case 6:
            mData->IOTimeoutMultiplier = convertedValue.value.usVal;
            break;

        case 7:
            mData->UseBlockingInput = convertedValue.value.boolVal;
            break;
        }
    }

    return ret;
}
