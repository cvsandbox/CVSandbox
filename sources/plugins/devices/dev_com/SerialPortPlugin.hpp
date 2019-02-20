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

#pragma once
#ifndef CVS_SERIAL_PORT_PLUGIN_HPP
#define CVS_SERIAL_PORT_PLUGIN_HPP

#include <iplugintypescpp.hpp>

namespace Private
{
    class SerialPortPluginData;
}

class SerialPortPlugin : public ICommunicationDevicePlugin
{
public:
    SerialPortPlugin( );

    // IPluginBase interface
    virtual void Dispose( );

    virtual XErrorCode GetProperty( int32_t id, xvariant* value ) const;
    virtual XErrorCode SetProperty( int32_t id, const xvariant* value );

    // ICommunicationDevicePlugin interface
    virtual XErrorCode Connect( );
    virtual void Disconnect( );
    virtual bool IsConnected( );

    virtual XErrorCode Write( const uint8_t* buffer, uint32_t nBytesToWrite, uint32_t* nBytesWritten );
    virtual XErrorCode Read( uint8_t* buffer, uint32_t nBytesToRead, uint32_t* nBytesRead );
    virtual void PurgeBuffers( );

private:
    static const PropertyDescriptor**   propertiesDescription;
    ::Private::SerialPortPluginData*    mData;
};

#endif // CVS_SERIAL_PORT_PLUGIN_HPP
