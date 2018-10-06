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
#include <image_histogram_equalization_16x16.h>
#include "HistogramEqualizationPlugin.hpp"

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x00000011 };

// Register the plug-in
REGISTER_CPP_PLUGIN
(
    PluginID,
    PluginFamilyID_ColorFilter,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Histogram Equalization",
    "HistogramEqualization",
    "Performs histogram equalization increasing global contrast in images.",

    /* Long description */
    "The plug-n performs histogram equalization - a method which usually increases the global contrast of many images, "
    "especially when the usable data of the image is represented by close contrast values. Through this adjustment, "
    "the intensities can be better distributed on the histogram. This allows for areas of lower local contrast to gain "
    "a higher contrast. Histogram equalization accomplishes this by effectively spreading out the most frequent intensity values.<br><br>"

    "<b>Note</b>: this image processing routing suites grayscale images better. For color images however it may create different "
    "color artefacts, since every color plane is equalized independently."
    ,
    &image_histogram_equalization_16x16,
    0,
    HistogramEqualizationPlugin
);
