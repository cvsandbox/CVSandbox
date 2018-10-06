/*
    Standard image processing plug-ins of Computer Vision Sandbox

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

#include <ximaging.h>
#include <iplugincpp.hpp>
#include <image_extract_channel_16x16.h>
#include "ExtractRGBChannelPlugin.hpp"

static void PluginInitializer( );
static void PluginCleaner( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x00000005 };

// Channel property
static PropertyDescriptor channelIndexProperty =
{ XVT_U1, "Channel", "channel", "RGB channel to extract from color image.", PropertyFlag_SelectionByIndex };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &channelIndexProperty,
};

// Let the class itself know description of its properties
const PropertyDescriptor** ExtractRGBChannelPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_ColorFilter,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Extract RGB Channel",
    "ExtractRgbChannel",
    "Extracts specified RGB channel out of color image.",

    "The plug-in allows extracting specified RGB channel out of a color image, so it could be "
    "processed individually as a grayscale image.<br><br>"

    "See also: <a href='{AF000003-00000000-00000001-00000030}'>Replace RGB Channel</a>, "
    "<a href='{AF000003-00000000-00000001-00000009}'>Extract nRGB Channel</a>.",

    &image_extract_channel_16x16,
    nullptr,
    ExtractRGBChannelPlugin,

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
    channelIndexProperty.DefaultValue.type = XVT_U1;
    channelIndexProperty.DefaultValue.value.ubVal = 0;

    channelIndexProperty.MaxValue.type = XVT_U1;
    channelIndexProperty.MaxValue.value.ubVal = 3;

    channelIndexProperty.ChoicesCount = 4;
    channelIndexProperty.Choices = new xvariant[4];

    channelIndexProperty.Choices[RedIndex].type = XVT_String;
    channelIndexProperty.Choices[RedIndex].value.strVal = XStringAlloc( "Red" );

    channelIndexProperty.Choices[GreenIndex].type = XVT_String;
    channelIndexProperty.Choices[GreenIndex].value.strVal = XStringAlloc( "Green" );

    channelIndexProperty.Choices[BlueIndex].type = XVT_String;
    channelIndexProperty.Choices[BlueIndex].value.strVal = XStringAlloc( "Blue" );

    channelIndexProperty.Choices[AlphaIndex].type = XVT_String;
    channelIndexProperty.Choices[AlphaIndex].value.strVal = XStringAlloc( "Alpha" );
}

// Clean-up plug-in - deallocate strings
static void PluginCleaner( )
{
    for ( int i = 0; i < channelIndexProperty.ChoicesCount; i++ )
    {
        XVariantClear( &channelIndexProperty.Choices[i] );
    }

    delete [] channelIndexProperty.Choices;
}
