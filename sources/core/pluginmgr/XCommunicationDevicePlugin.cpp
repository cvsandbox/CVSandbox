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

#include "XCommunicationDevicePlugin.hpp"

using namespace std;
using namespace CVSandbox;

XCommunicationDevicePlugin::XCommunicationDevicePlugin( void* plugin, bool ownIt ) :
    XPlugin( plugin, PluginType_CommunicationDevice, ownIt )
{
}

XCommunicationDevicePlugin::~XCommunicationDevicePlugin( )
{
}

// Create plug-in wrapper
const shared_ptr<XCommunicationDevicePlugin> XCommunicationDevicePlugin::Create( void* plugin, bool ownIt )
{
    return shared_ptr<XCommunicationDevicePlugin>( new XCommunicationDevicePlugin( plugin, ownIt ) );
}

// Connecte to device
XErrorCode XCommunicationDevicePlugin::Connect( )
{
    SDevicePlugin* dp = static_cast<SDevicePlugin*>( mPlugin );
    return dp->Connect( dp );
}

// Disconnect from device
void XCommunicationDevicePlugin::Disconnect( )
{
    SDevicePlugin* dp = static_cast<SDevicePlugin*>( mPlugin );
    dp->Disconnect( dp );
}

// Check if connected to a device
bool XCommunicationDevicePlugin::IsConnected( )
{
    SDevicePlugin* dp = static_cast<SDevicePlugin*>( mPlugin );
    return dp->IsConnected( dp );
}

// Write data to connected device
XErrorCode XCommunicationDevicePlugin::Write( const uint8_t* buffer, uint32_t nBytesToWrite, uint32_t* nBytesWritten )
{
    SCommunicationDevicePlugin* cdp = static_cast<SCommunicationDevicePlugin*>( mPlugin );
    return cdp->Write( cdp, buffer, nBytesToWrite, nBytesWritten );
}

// Read data from connected device
XErrorCode XCommunicationDevicePlugin::Read( uint8_t* buffer, uint32_t nBytesToRead, uint32_t* nBytesRead )
{
    SCommunicationDevicePlugin* cdp = static_cast<SCommunicationDevicePlugin*>( mPlugin );
    return cdp->Read( cdp, buffer, nBytesToRead, nBytesRead );
}

// Discard any data in communication buffers (write and read)
void XCommunicationDevicePlugin::PurgeBuffers( )
{
    SCommunicationDevicePlugin* cdp = static_cast<SCommunicationDevicePlugin*>( mPlugin );
    cdp->PurgeBuffers( cdp );
}
