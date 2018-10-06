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
#include <image_chart_16x16.h>
#include "ImageStatisticsPlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x00000026 };

// Extra range to find
static PropertyDescriptor rangeToFindProperty =
{ XVT_R4, "Range To Find", "rangeToFind", "Extra range to find, which includes specified % of histogram around its median.", PropertyFlag_None };

// Histogram properties
static PropertyDescriptor redHistogramProperty =
{ XVT_U4 | XVT_Array, "Red Histogram", "redHistogram", "Red channel histogram of RGB image.", PropertyFlag_ReadOnly };
static PropertyDescriptor greenHistogramProperty =
{ XVT_U4 | XVT_Array, "Green Histogram", "greenHistogram", "Green channel histogram of RGB image.", PropertyFlag_ReadOnly };
static PropertyDescriptor blueHistogramProperty =
{ XVT_U4 | XVT_Array, "Blue Histogram", "blueHistogram", "Blue channel histogram of RGB image.", PropertyFlag_ReadOnly };
static PropertyDescriptor grayHistogramProperty =
{ XVT_U4 | XVT_Array, "Gray Histogram", "grayHistogram", "Intensity histogram of grayscale image.", PropertyFlag_ReadOnly };

// Red channel statistics
static PropertyDescriptor redRangeProperty =
{ XVT_Range, "Red Range", "redRange", "Range of values in red channel.", PropertyFlag_ReadOnly };
static PropertyDescriptor redRangeEx0Property =
{ XVT_Range, "Red Range Excluding 0", "redRangeEx0", "Range of values in red channel excluding 0 value.", PropertyFlag_ReadOnly };
static PropertyDescriptor redRangeFoundProperty =
{ XVT_Range, "Red Range Found", "redRangeFound", "Range of values in red channel which contains requested % of histogram.", PropertyFlag_ReadOnly };
static PropertyDescriptor redMeanProperty =
{ XVT_R4, "Red Mean", "redMean", "Mean value of red channel.", PropertyFlag_ReadOnly };
static PropertyDescriptor redStdDevProperty =
{ XVT_R4, "Red StdDev", "redStdDev", "Standard deviation value of red channel.", PropertyFlag_ReadOnly };
static PropertyDescriptor redMeanx0Property =
{ XVT_R4, "Red Mean Excluding 0", "redMeanEx0", "Mean value of red channel excluding 0 value.", PropertyFlag_ReadOnly };
static PropertyDescriptor redStdDevx0Property =
{ XVT_R4, "Red StdDev Excluding 0", "redStdDevEx0", "Standard deviation value of red channel excluding 0 value.", PropertyFlag_ReadOnly };
static PropertyDescriptor redTotalEx0Property =
{ XVT_U4, "Red Total Excluding 0", "redTotalEx0", "Total none 0 values in red channel.", PropertyFlag_ReadOnly };
static PropertyDescriptor redTotalFoundProperty =
{ XVT_U4, "Red Total Found", "redTotalFound", "Total number of red channel values contributing the requested range to find.", PropertyFlag_ReadOnly };

// Green channel statistics
static PropertyDescriptor greenRangeProperty =
{ XVT_Range, "Green Range", "greenRange", "Range of values in green channel.", PropertyFlag_ReadOnly };
static PropertyDescriptor greenRangeEx0Property =
{ XVT_Range, "Green Range Excluding 0", "greenRangeEx0", "Range of values in green channel excluding 0 value.", PropertyFlag_ReadOnly };
static PropertyDescriptor greenRangeFoundProperty =
{ XVT_Range, "Green Range Found", "greenRangeFound", "Range of values in green channel which contains requested % of histogram.", PropertyFlag_ReadOnly };
static PropertyDescriptor greenMeanProperty =
{ XVT_R4, "Green Mean", "greenMean", "Mean value of green channel.", PropertyFlag_ReadOnly };
static PropertyDescriptor greenStdDevProperty =
{ XVT_R4, "Green StdDev", "greenStdDev", "Standard deviation value of green channel.", PropertyFlag_ReadOnly };
static PropertyDescriptor greenMeanx0Property =
{ XVT_R4, "Green Mean Excluding 0", "greenMeanEx0", "Mean value of green channel excluding 0 value.", PropertyFlag_ReadOnly };
static PropertyDescriptor greenStdDevx0Property =
{ XVT_R4, "Green StdDev Excluding 0", "greenStdDevEx0", "Standard deviation value of green channel excluding 0 value.", PropertyFlag_ReadOnly };
static PropertyDescriptor greenTotalEx0Property =
{ XVT_U4, "Green Total Excluding 0", "greenTotalEx0", "Total none 0 values in green channel.", PropertyFlag_ReadOnly };
static PropertyDescriptor greenTotalFoundProperty =
{ XVT_U4, "Green Total Found", "greenTotalFound", "Total number of green channel values contributing the requested range to find.", PropertyFlag_ReadOnly };

// Blue channel statistics
static PropertyDescriptor blueRangeProperty =
{ XVT_Range, "Blue Range", "blueRange", "Range of values in blue channel.", PropertyFlag_ReadOnly };
static PropertyDescriptor blueRangeEx0Property =
{ XVT_Range, "Blue Range Excluding 0", "blueRangeEx0", "Range of values in blue channel excluding 0 value.", PropertyFlag_ReadOnly };
static PropertyDescriptor blueRangeFoundProperty =
{ XVT_Range, "Blue Range Found", "blueRangeFound", "Range of values in blue channel which contains requested % of histogram.", PropertyFlag_ReadOnly };
static PropertyDescriptor blueMeanProperty =
{ XVT_R4, "Blue Mean", "blueMean", "Mean value of blue channel.", PropertyFlag_ReadOnly };
static PropertyDescriptor blueStdDevProperty =
{ XVT_R4, "Blue StdDev", "blueStdDev", "Standard deviation value of blue channel.", PropertyFlag_ReadOnly };
static PropertyDescriptor blueMeanx0Property =
{ XVT_R4, "Blue Mean Excluding 0", "blueMeanEx0", "Mean value of blue channel excluding 0 value.", PropertyFlag_ReadOnly };
static PropertyDescriptor blueStdDevx0Property =
{ XVT_R4, "Blue StdDev Excluding 0", "blueStdDevEx0", "Standard deviation value of blue channel excluding 0 value.", PropertyFlag_ReadOnly };
static PropertyDescriptor blueTotalEx0Property =
{ XVT_U4, "Blue Total Excluding 0", "blueTotalEx0", "Total none 0 values in blue channel.", PropertyFlag_ReadOnly };
static PropertyDescriptor blueTotalFoundProperty =
{ XVT_U4, "Blue Total Found", "blueTotalFound", "Total number of blue channel values contributing the requested range to find.", PropertyFlag_ReadOnly };

// Gray channel statistics
static PropertyDescriptor grayRangeProperty =
{ XVT_Range, "Gray Range", "grayRange", "Range of intensity values.", PropertyFlag_ReadOnly };
static PropertyDescriptor grayRangeEx0Property =
{ XVT_Range, "Gray Range Excluding 0", "grayRangeEx0", "Range of intensity values excluding 0 value.", PropertyFlag_ReadOnly };
static PropertyDescriptor grayRangeFoundProperty =
{ XVT_Range, "Gray Range Found", "grayRangeFound", "Range of intensity values which contains requested % of histogram.", PropertyFlag_ReadOnly };
static PropertyDescriptor grayMeanProperty =
{ XVT_R4, "Gray Mean", "grayMean", "Mean intensity value.", PropertyFlag_ReadOnly };
static PropertyDescriptor grayStdDevProperty =
{ XVT_R4, "Gray StdDev", "grayStdDev", "Standard deviation of intensity value.", PropertyFlag_ReadOnly };
static PropertyDescriptor grayMeanx0Property =
{ XVT_R4, "Gray Mean Excluding 0", "grayMeanEx0", "Mean intensity excluding 0 value.", PropertyFlag_ReadOnly };
static PropertyDescriptor grayStdDevx0Property =
{ XVT_R4, "Gray StdDev Excluding 0", "grayStdDevEx0", "Standard deviation of intensity excluding 0 value.", PropertyFlag_ReadOnly };
static PropertyDescriptor grayTotalEx0Property =
{ XVT_U4, "Gray Total Excluding 0", "grayTotalEx0", "Total none 0 values of intensity.", PropertyFlag_ReadOnly };
static PropertyDescriptor grayTotalFoundProperty =
{ XVT_U4, "Gray Total Found", "grayTotalFound", "Total number of intensity values contributing the requested range to find.", PropertyFlag_ReadOnly };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &rangeToFindProperty,

    &redHistogramProperty, &greenHistogramProperty, &blueHistogramProperty, &grayHistogramProperty,

    &redRangeProperty, &redRangeEx0Property, &redRangeFoundProperty, &redMeanProperty, &redStdDevProperty,
    &redMeanx0Property, &redStdDevx0Property, &redTotalEx0Property, &redTotalFoundProperty,

    &greenRangeProperty, &greenRangeEx0Property, &greenRangeFoundProperty, &greenMeanProperty, &greenStdDevProperty,
    &greenMeanx0Property, &greenStdDevx0Property, &greenTotalEx0Property, &greenTotalFoundProperty,

    &blueRangeProperty, &blueRangeEx0Property, &blueRangeFoundProperty, &blueMeanProperty, &blueStdDevProperty,
    &blueMeanx0Property, &blueStdDevx0Property, &blueTotalEx0Property, &blueTotalFoundProperty,

    &grayRangeProperty, &grayRangeEx0Property, &grayRangeFoundProperty, &grayMeanProperty, &grayStdDevProperty,
    &grayMeanx0Property, &grayStdDevx0Property, &grayTotalEx0Property, &grayTotalFoundProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** ImageStatisticsPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_Default,

    PluginType_ImageProcessing,
    PluginVersion,
    "Image Statistics",
    "ImageStatistics",
    "Calculates image histograms and statistics values.",

    "The plug-in can be used to calculate histograms of RGB channels for color images or intensity histogram for "
    "grayscale images. Also it provides statistical values like minimum/maximum values of RGB channels or intensity, "
    "mean and standard deviation values.<br><br>"

    "In addition it provides same statistical values, but calculated without taking 0 values into account. This can be "
    "useful if image was processed with <a href='{AF000003-00000000-00000001-00000006}'>Color Filter</a> first and "
    "as the result has some object on black background. So to get statistics for the object, zero value should be "
    "excluded.<br><br>"

    "The plug-in also calculates range of values, which represent specified percentage of histogram (around its median). "
    "This allows, for example, to get range of values, which make 90% (or whatever specified) of the histogram. The "
    "found range can be then used with <a href='{AF000003-00000000-00000001-00000004'>Level Liner</a> plug-in, for example, "
    "to stretch it to the full [0, 255] range.",

    &image_chart_16x16,
    nullptr,
    ImageStatisticsPlugin,

    XARRAY_SIZE( pluginProperties ),
    pluginProperties,
    PluginInitializer,
    nullptr, // no clean-up
    nullptr  // no dynamic properties update
);

// Complete properties description by initializing those parts, which were not 
// initialized during properties array declaration
static void PluginInitializer( )
{
    rangeToFindProperty.DefaultValue.type = XVT_R4;
    rangeToFindProperty.DefaultValue.value.fVal = 95;

    rangeToFindProperty.MaxValue.type = XVT_R4;
    rangeToFindProperty.MaxValue.value.fVal = 100;

    rangeToFindProperty.MinValue.type = XVT_R4;
    rangeToFindProperty.MinValue.value.fVal = 5;
}
