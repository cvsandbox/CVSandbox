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
#include <image_hsl_filter_16x16.h>
#include "HsvColorFilterPlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x0000002E };

static PropertyDescriptor hueRangeProperty =
{ XVT_Range, "Hue Range", "hueRange", "Hue range to filter.", PropertyFlag_PreferredEditor_HueRangePicker };

static PropertyDescriptor saturationRangeProperty =
{ XVT_RangeF, "Saturation Range", "saturationRange", "Saturation range to filter.", PropertyFlag_None };

static PropertyDescriptor valueRangeProperty =
{ XVT_RangeF, "Value Range", "valueRange", "Value range to filter.", PropertyFlag_None };

static PropertyDescriptor fillOutsideProperty =
{ XVT_Bool, "Fill Outside", "fillOutside", "Flag telling to fill outside or inside of the color range.", PropertyFlag_None };

static PropertyDescriptor fillColorProperty =
{ XVT_ARGB, "Fill Color", "fillColor", "Color used to fill filtered areas.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &hueRangeProperty, &saturationRangeProperty, &valueRangeProperty,
    &fillOutsideProperty, &fillColorProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** HsvColorFilterPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_ColorFilter,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "HSV Color Filter",
    "HsvColorFilter",
    "Filters colors outside/inside of the specified HSV color range.",

    /* Long description */
    "The plug-in filters out pixels (sets them to the specified <b>fill color</b>) which values are outside or "
    "inside of the specified HSV color range. When filling outside mode is used, the filter will keep all "
    "pixels which values are in the specified Hue, Saturation and Value ranges. Other pixels, "
    "which values are outside of the specified range, are filled with the set color. When filling inside "
    "mode is used, it is all opposite - all pixels which get into the specified color components' range are "
    "filled with the specified color, while pixels outside of those ranges are kept as is.<br><br>"

    "See also: <a href='{AF000003-00000000-00000001-00000006}'>Color Filter</a>, "
    "<a href='{AF000003-00000000-00000001-0000002D}'>HSL Color Filter</a>"
    ,
    &image_hsl_filter_16x16,
    0,
    HsvColorFilterPlugin,

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
    // Hue Range property
    hueRangeProperty.DefaultValue.type = XVT_Range;
    hueRangeProperty.DefaultValue.value.rangeVal.min = 0;
    hueRangeProperty.DefaultValue.value.rangeVal.max = 359;

    hueRangeProperty.MinValue.type = XVT_I4;
    hueRangeProperty.MinValue.value.iVal = 0;

    hueRangeProperty.MaxValue.type = XVT_I4;
    hueRangeProperty.MaxValue.value.iVal = 359;

    // Saturation Range property
    saturationRangeProperty.DefaultValue.type = XVT_RangeF;
    saturationRangeProperty.DefaultValue.value.frangeVal.min = 0.0f;
    saturationRangeProperty.DefaultValue.value.frangeVal.max = 1.0f;

    saturationRangeProperty.MinValue.type = XVT_R4;
    saturationRangeProperty.MinValue.value.fVal = 0.0f;

    saturationRangeProperty.MaxValue.type = XVT_R4;
    saturationRangeProperty.MaxValue.value.fVal = 1.0f;

    // Value Range property
    valueRangeProperty.DefaultValue.type = XVT_RangeF;
    valueRangeProperty.DefaultValue.value.frangeVal.min = 0.0f;
    valueRangeProperty.DefaultValue.value.frangeVal.max = 1.0f;

    valueRangeProperty.MinValue.type = XVT_R4;
    valueRangeProperty.MinValue.value.fVal = 0.0f;

    valueRangeProperty.MaxValue.type = XVT_R4;
    valueRangeProperty.MaxValue.value.fVal = 1.0f;

    // Other properties
    fillOutsideProperty.DefaultValue.type = XVT_Bool;
    fillOutsideProperty.DefaultValue.value.boolVal = true;

    fillColorProperty.DefaultValue.type = XVT_ARGB;
    fillColorProperty.DefaultValue.value.argbVal.argb = 0xFF000000;
}
