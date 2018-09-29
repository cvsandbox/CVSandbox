/*
    Plug-ins' management library of Computer Vision Sandbox

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

#include "XDevicePlugin.hpp"

using namespace std;
using namespace CVSandbox;

XDevicePlugin::XDevicePlugin( void* plugin, bool ownIt ) :
    XPlugin( plugin, PluginType_Device, ownIt )
{
}

XDevicePlugin::~XDevicePlugin( )
{
}

// Create plug-in wrapper
const shared_ptr<XDevicePlugin> XDevicePlugin::Create( void* plugin, bool ownIt )
{
    return shared_ptr<XDevicePlugin>( new XDevicePlugin( plugin, ownIt ) );
}

// Connecte to device
XErrorCode XDevicePlugin::Connect( )
{
    SDevicePlugin* dp = static_cast<SDevicePlugin*>( mPlugin );
    return dp->Connect( dp );
}

// Disconnect from device
void XDevicePlugin::Disconnect( )
{
    SDevicePlugin* dp = static_cast<SDevicePlugin*>( mPlugin );
    dp->Disconnect( dp );
}

// Check if connected to a device
bool XDevicePlugin::IsConnected( )
{
    SDevicePlugin* dp = static_cast<SDevicePlugin*>( mPlugin );
    return dp->IsConnected( dp );
}
