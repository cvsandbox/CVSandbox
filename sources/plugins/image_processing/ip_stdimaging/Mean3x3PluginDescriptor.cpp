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
#include <image_mean_3s_16x16.h>
#include "Mean3x3Plugin.hpp"

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x0000000F };

// Register the plug-in
REGISTER_CPP_PLUGIN
(
    PluginID,
    PluginFamilyID_ImageSmoothing,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Mean 3x3",
    "Mean3x3",
    "Smoothes image by calculating mean values over 3x3 square windows.",

    /* Long description */
    "The plug-in does image smoothing by setting every pixel in result image to the "
    "average value of pixels from the corresponding 3x3 square window in the source image "
    "(for edge pixels it uses only 6 pixels of the source image and for corners it uses 4)."
    ,
    &image_mean_3s_16x16,
    0,
    Mean3x3Plugin
);
