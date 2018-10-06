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

#include <iplugincpp.hpp>
#include <image_levels_linear_16x16.h>
#include "LevelsLinearPlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x00000004 };

// Red channel properties
static PropertyDescriptor inRedProperty =
{ XVT_Range, "Red Input", "redIn", "Input range of red channel.", PropertyFlag_None };
static PropertyDescriptor outRedProperty =
{ XVT_Range, "Red Output", "redOut", "Output range of red channel.", PropertyFlag_None };

// Green channel properties
static PropertyDescriptor inGreenProperty =
{ XVT_Range, "Green Input", "greenIn", "Input range of green channel.", PropertyFlag_None };
static PropertyDescriptor outGreenProperty =
{ XVT_Range, "Green Output", "greenOut", "Output range of green channel.", PropertyFlag_None };

// Blue channel properties
static PropertyDescriptor inBlueProperty =
{ XVT_Range, "Blue Input", "blueIn", "Input range of blue channel.", PropertyFlag_None };
static PropertyDescriptor outBlueProperty =
{ XVT_Range, "Blue Output", "blueOut", "Output range of blue channel.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &inRedProperty,   &outRedProperty,
    &inGreenProperty, &outGreenProperty,
    &inBlueProperty,  &outBlueProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** LevelsLinearPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_ColorFilter,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Levels Linear",
    "LevelsLinear",
    "Performs linear correction of pixels' values in a color image.",

    "The filter performs linear correction of RGB channels by mapping specified channels' input ranges to output ranges. "
    "For example, if input range is [20, 240] and output range is [0, 255], then all pixels' values smaller than 20 are set to 0, "
    "all values greater than 240 are set to 255, all values in the range are linearly stretched to the output range.<br><br>"

    "For grayscale images see <a href='{AF000003-00000000-00000001-00000014}'>Levels Linear Grayscale</a> plug-in.",

    &image_levels_linear_16x16,
    0,
    LevelsLinearPlugin,

    sizeof( pluginProperties ) / sizeof( PropertyDescriptor* ),
    pluginProperties,
    PluginInitializer,
    0, // no clean-up
    0  // no dynamic properties update
);


// Complete properties description by initializing those parts, which were not 
// initialized during properties array declaration
static void PluginInitializer( )
{
    for ( int i = 0; i < 6; i++ )
    {
        pluginProperties[i]->DefaultValue.type = XVT_Range;
        pluginProperties[i]->DefaultValue.value.rangeVal.min = 0;
        pluginProperties[i]->DefaultValue.value.rangeVal.max = 255;

        pluginProperties[i]->MinValue.type = XVT_I4;
        pluginProperties[i]->MinValue.value.iVal = 0;

        pluginProperties[i]->MaxValue.type = XVT_I4;
        pluginProperties[i]->MaxValue.value.iVal = 255;
    }
}
