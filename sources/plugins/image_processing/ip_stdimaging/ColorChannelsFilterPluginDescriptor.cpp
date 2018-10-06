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
#include <image_filter_rgb_channels_16x16.h>
#include "ColorChannelsFilterPlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x0000002C };

static PropertyDescriptor rangeRedProperty =
{ XVT_Range, "Red Range", "redRange", "Red channel's range to keep or fill.", PropertyFlag_None };

static PropertyDescriptor redFillValueProperty =
{ XVT_U1, "Red Fill Value", "redFillValue", "Value to use to fill filtered range of red channel.", PropertyFlag_None };

static PropertyDescriptor redfillOutsideProperty =
{ XVT_Bool, "Fill Red Outside", "fillRedOutside", "Flag telling to fill red channel outside of the range.", PropertyFlag_None };


static PropertyDescriptor rangeGreenProperty =
{ XVT_Range, "Green Range", "greenRange", "Green channel's range to keep or fill.", PropertyFlag_None };

static PropertyDescriptor greenFillValueProperty =
{ XVT_U1, "Green Fill Value", "greenFillValue", "Value to use to fill filtered range of green channel.", PropertyFlag_None };

static PropertyDescriptor greenfillOutsideProperty =
{ XVT_Bool, "Fill Green Outside", "fillGreenOutside", "Flag telling to fill green channel outside of the range.", PropertyFlag_None };


static PropertyDescriptor rangeBlueProperty =
{ XVT_Range, "Blue Range", "blueRange", "Blue channel's range to keep or fill.", PropertyFlag_None };

static PropertyDescriptor blueFillValueProperty =
{ XVT_U1, "Blue Fill Value", "blueFillValue", "Value to use to fill filtered range of blue channel.", PropertyFlag_None };

static PropertyDescriptor bluefillOutsideProperty =
{ XVT_Bool, "Fill Blue Outside", "fillBlueOutside", "Flag telling to fill blue channel outside of the range.", PropertyFlag_None };


// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &rangeRedProperty,   &redFillValueProperty,   &redfillOutsideProperty,
    &rangeGreenProperty, &greenFillValueProperty, &greenfillOutsideProperty,
    &rangeBlueProperty,  &blueFillValueProperty,  &bluefillOutsideProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** ColorChannelsFilterPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_ColorFilter,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Color Channels Filter",
    "ColorChannelsFilter",
    "Filters RGB color channels outside/inside of the specified range.",

    /* Long description */
    "This plug-in is similar to the <a href='{AF000003-00000000-00000001-00000006}'>Color Filter</a>, but operates on "
    "individual RGB channels. For each color channel it provides range to keep or fill, the fill value and a flag "
    "telling if filling must be done outside of the specified range or inside the range."
    ,
    &image_filter_rgb_channels_16x16,
    0,
    ColorChannelsFilterPlugin,

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
    for ( int i = 0, j = 1, k = 2; i <= 6; i += 3, j += 3, k += 3 )
    {
        pluginProperties[i]->DefaultValue.type = XVT_Range;
        pluginProperties[i]->DefaultValue.value.rangeVal.min = 0;
        pluginProperties[i]->DefaultValue.value.rangeVal.max = 255;

        pluginProperties[i]->MinValue.type = XVT_I4;
        pluginProperties[i]->MinValue.value.iVal = 0;

        pluginProperties[i]->MaxValue.type = XVT_I4;
        pluginProperties[i]->MaxValue.value.iVal = 255;

        pluginProperties[j]->DefaultValue.type = XVT_U1;
        pluginProperties[j]->DefaultValue.value.ubVal = 0;

        pluginProperties[j]->MinValue.type = XVT_U1;
        pluginProperties[j]->MinValue.value.ubVal = 0;

        pluginProperties[j]->MaxValue.type = XVT_U1;
        pluginProperties[j]->MaxValue.value.ubVal = 255;

        pluginProperties[k]->DefaultValue.type = XVT_Bool;
        pluginProperties[k]->DefaultValue.value.boolVal = true;
    }
}
