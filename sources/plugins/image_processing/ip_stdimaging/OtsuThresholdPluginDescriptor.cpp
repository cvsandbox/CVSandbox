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
#include <image_threshold_otsu_16x16.h>
#include "OtsuThresholdPlugin.hpp"

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 1 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x0000000A };

// Register the plug-in
REGISTER_CPP_PLUGIN
(
    PluginID,
    PluginFamilyID_Thresholding,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Otsu Threshold",
    "OtsuThreshold",
    "Performs thresholding of the specified grayscale image using Otsu's adaptive thresholding algorithm.",

    /* Long description */
    "The plug-in performs thresholding of grayscale images by utilize the adaptive thresholding algorithm "
    "developed by Nobuyuki Otsu and described in \"A threshold selection method from gray-level histograms\", "
    "IEEE Trans. Systems, Man and Cybernetics 9(1), pp. 62-66, 1979."
    ,
    &image_threshold_otsu_16x16,
    0, 
    OtsuThresholdPlugin
);
