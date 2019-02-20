/*
    Plug-ins' management library of Computer Vision Sandbox

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
#ifndef CVS_XCOMMUNICATION_DEVICE_PLUGIN_HPP
#define CVS_XCOMMUNICATION_DEVICE_PLUGIN_HPP

#include "XPlugin.hpp"

class XCommunicationDevicePlugin : public XPlugin
{
private:
    XCommunicationDevicePlugin( void* plugin, bool ownIt );

public:
    virtual ~XCommunicationDevicePlugin( );

    // Create plug-in wrapper
    static const std::shared_ptr<XCommunicationDevicePlugin> Create( void* plugin, bool ownIt = true );

    // Connect to device
    XErrorCode Connect( );
    // Disconnect from device
    void Disconnect( );
    // Check if connected to a device
    bool IsConnected( );

    // Write data to connected device
    XErrorCode Write( const uint8_t* buffer, uint32_t nBytesToWrite, uint32_t* nBytesWritten );
    // Read data from connected device
    XErrorCode Read( uint8_t* buffer, uint32_t nBytesToRead, uint32_t* nBytesRead );
    // Discard any data in communication buffers (write and read)
    void PurgeBuffers( );
};

#endif // CVS_XCOMMUNICATION_DEVICE_PLUGIN_HPP
