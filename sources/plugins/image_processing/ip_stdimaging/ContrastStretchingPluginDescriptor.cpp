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
#include <image_contrast_stretching_16x16.h>
#include "ContrastStretchingPlugin.hpp"

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x00000010 };

// Register the plug-in
REGISTER_CPP_PLUGIN
(
    PluginID,
    PluginFamilyID_ColorFilter,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Contrast Stretching",
    "ContrastStretching",
    "Stretches the range of intensity values to span the maximum possible range.",

    /* Long description */
    "The plug-in performs contrast stretching (or as it is often called normalization), which is a simple image enhancement "
    "technique that attempts to improve the contrast in an image by 'stretching' the range of intensity values "
    "it contains to span a desired range of values, e.g. the full range of pixel values that the image type "
    "concerned allows ([0, 255] value range).<br><br>"

    "The plug-in finds minimum/maximum intensity or color planes' values and then performs linear stretching of "
    "the found [min, max] range to the [0, 255] range.<br><br>"

    "See also: <a href='{AF000003-00000000-00000001-00000004}'>Levels Linear</a>"
    ,
    &image_contrast_stretching_16x16,
    0,
    ContrastStretchingPlugin
);
