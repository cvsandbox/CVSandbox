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
#include <image_grayscale_16x16.h>
#include "GrayscalePlugin.hpp"

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 1 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x00000003 };

// Register the plug-in
REGISTER_CPP_PLUGIN
(
    PluginID,
    PluginFamilyID_ColorFilter,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Grayscale",
    "Grayscale",
    "Grayscales color image using BT709 algorithm.",

    /* Long description */
    "The plug-in performs color image grayscaling using BT709 algorithm. The result intensity value of pixels is "
    "calculated using the next coefficients:<br>"
    "Intensity = Red * 0.2125 + Green * 0.7154 + Blue * 0.0721<br><br>"

    "The plug-in also able to convert binary images to grayscale image, where binary values are simply multiplied by 255, "
    "so 1 values from binary image are scaled to 255 values of grayscale image."
    ,
    &image_grayscale_16x16,
    0,
    GrayscalePlugin
);
