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
#include <image_color_filter_16x16.h>
#include "ColorFilterPlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x00000006 };

static PropertyDescriptor rangeRedProperty =
{ XVT_Range, "Red Range", "redRange", "Red channel's range to process.", PropertyFlag_None };

static PropertyDescriptor rangeGreenProperty =
{ XVT_Range, "Green Range", "greenRange", "Green channel's range to process.", PropertyFlag_None };

static PropertyDescriptor rangeBlueProperty =
{ XVT_Range, "Blue Range", "blueRange", "Blue channel's range to process.", PropertyFlag_None };

static PropertyDescriptor fillOutsideProperty =
{ XVT_Bool, "Fill Outside", "fillOutside", "Flag telling to fill outside or inside of the color range.", PropertyFlag_None };

static PropertyDescriptor fillColorProperty =
{ XVT_ARGB, "Fill Color", "fillColor", "Color used to fill filtered areas.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &rangeRedProperty, &rangeGreenProperty, &rangeBlueProperty,
    &fillOutsideProperty, &fillColorProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** ColorFilterPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_ColorFilter,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Color Filter",
    "ColorFilter",
    "Filters colors outside/inside of the specified RGB color range.",

    /* Long description */
    "The plug-in filters out pixels (sets them to the specified <b>fill color</b>) which values are outside or "
    "inside of the specified RGB color range. When filling outside mode is used, the filter will keep all "
    "pixels which values are in the specified red, green and blue components' ranges. Other pixels, "
    "which values are outside of the specified range, are filled with the set color. For example, "
    "suppose color ranges are set to the next values: Red [50, 200], Green [75, 175] and Blue[60, 190]. "
    "A pixel with (R:70, G:80, B:65) color value will be kept, while a pixel with (R:70, G:80, B:55) color "
    "will be set to the specified filling color. When filling inside mode is used, it is all opposite - "
    "all pixels which get into the specified color components' range are filled with the specified color, while "
    "pixels outside of those ranges are kept as is.<br><br>"

    "<b>Note</b>: color components ranges are specified with inclusive limits. A component's value 50 is "
    "considered to be in the [50, 200] range.<br><br>"

    "See also: <a href='{AF000003-00000000-00000001-0000002C}'>Color Channels Filter</a>, "
    "<a href='{AF000003-00000000-00000001-0000002E}'>HSV Color Filter</a>, "
    "<a href='{AF000003-00000000-00000001-0000002D}'>HSL Color Filter</a>"
    ,
    &image_color_filter_16x16,
    0,
    ColorFilterPlugin,

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
    for ( int i = 0; i < 3; i++ )
    {
        pluginProperties[i]->DefaultValue.type = XVT_Range;
        pluginProperties[i]->DefaultValue.value.rangeVal.min = 0;
        pluginProperties[i]->DefaultValue.value.rangeVal.max = 255;

        pluginProperties[i]->MinValue.type = XVT_I4;
        pluginProperties[i]->MinValue.value.iVal = 0;

        pluginProperties[i]->MaxValue.type = XVT_I4;
        pluginProperties[i]->MaxValue.value.iVal = 255;
    }

    fillOutsideProperty.DefaultValue.type = XVT_Bool;
    fillOutsideProperty.DefaultValue.value.boolVal = true;

    fillColorProperty.DefaultValue.type = XVT_ARGB;
    fillColorProperty.DefaultValue.value.argbVal.argb = 0xFF000000;
}
