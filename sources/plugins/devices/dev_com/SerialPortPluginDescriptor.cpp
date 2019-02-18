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

#include <iplugincpp.hpp>
#include "SerialPortPlugin.hpp"

static void PluginInitializer( );
static void PluginCleaner( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000019, 0x00000001 };

// Connection properties
static PropertyDescriptor portNameProperty =
{ XVT_String, "Port Name", "portName", "Name of the serial port to open.", PropertyFlag_None };
static PropertyDescriptor baudRateProperty =
{ XVT_U4, "Baud Rate", "baudRate", "Baud rate at which communication device operates.", PropertyFlag_None };
static PropertyDescriptor byteSizeProperty =
{ XVT_U1, "Byte Size", "byteSize", "Number of bits per byte, [4-8].", PropertyFlag_None };
static PropertyDescriptor stopBitsProperty =
{ XVT_U1, "Stop Bits", "stopBits", "Number of stop bits.", PropertyFlag_SelectionByIndex };
static PropertyDescriptor parityProperty =
{ XVT_U1, "Parity", "parity", "Parity scheme to be used.", PropertyFlag_SelectionByIndex };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &portNameProperty, &baudRateProperty, &byteSizeProperty, &stopBitsProperty, &parityProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** SerialPortPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_Default,

    PluginType_CommunicationDevice,
    PluginVersion,
    "Serial Port",
    "SerialPort",
    "Plug-in to communicate with devices connected to serial port.",

    "The plug-in allows ..."
    ,

    nullptr,
    nullptr,
    SerialPortPlugin,

    XARRAY_SIZE( pluginProperties ),
    pluginProperties,
    PluginInitializer,
    PluginCleaner,
    nullptr  // no dynamic properties update
);

// Complete properties description by initializing those parts, which were not
// initialized during properties array declaration
static void PluginInitializer( )
{
    // Baud Rate
    baudRateProperty.DefaultValue.type        = XVT_U4;
    baudRateProperty.DefaultValue.value.uiVal = 9600;

    baudRateProperty.MinValue.type        = XVT_U4;
    baudRateProperty.MinValue.value.uiVal = 110;

    baudRateProperty.MaxValue.type        = XVT_U4;
    baudRateProperty.MaxValue.value.uiVal = 256000;

    // Byte Size
    byteSizeProperty.DefaultValue.type        = XVT_U1;
    byteSizeProperty.DefaultValue.value.uiVal = 8;

    byteSizeProperty.MinValue.type        = XVT_U1;
    byteSizeProperty.MinValue.value.uiVal = 4;

    byteSizeProperty.MaxValue.type        = XVT_U1;
    byteSizeProperty.MaxValue.value.uiVal = 8;
    
    // Stop Bits
    static const char* stopBitChoices[] = { "1", "1.5", "2" };
    InitSelectionProperty( &stopBitsProperty, stopBitChoices, XARRAY_SIZE( stopBitChoices ), 0 );

    // Parity
    static const char* parityChoices[] = { "No", "Odd", "Even", "Mark", "Space" };
    InitSelectionProperty( &parityProperty, parityChoices, XARRAY_SIZE( parityChoices ), 0 );
}

// Clean-up plugin - deallocate strings
static void PluginCleaner( )
{
    CleanSelectionProperty( &stopBitsProperty );
    CleanSelectionProperty( &parityProperty );
}
