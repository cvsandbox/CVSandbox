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
#include <image_binary_dilatation_16x16.h>
#include "BinaryDilatation3x3Plugin.hpp"

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x00000008 };

// Register the plug-in
REGISTER_CPP_PLUGIN
(
    PluginID,
    PluginFamilyID_Morphology,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Binary Dilatation 3x3",
    "BinaryDilatation3x3",
    "Dilatation operator for grayscale images with binary data. Uses 3x3 square structuring element.",

    /* Long description */
    "The plug-in performs dilatation mathematical morphology operator. It assigns pixels in the output image "
    "to the maximum value of neighbour pixels (including the corresponding pixel itself) in the input image.<br><br>"

    "This version of the dilation operator is designed to process grayscale images with \"binary\" data - images "
    "which have pixels with either 0 or 255 values only. Also it uses 3x3 square structuring element. "
    "This means that a pixel in the output image is assigned value 255, if at least one pixel out of 9 "
    "corresponding neighbours in the source image have value 255. Otherwise the pixel is assigned value 0. "
    "<b>Note</b>: for edge pixels only the available number of neighbours is used (6 for edges and 4 for "
    "corners).<br><br>"

    "For binary images the dilatation operator allows to grow (or join) separate objects (white areas)."
    ,
    &image_binary_dilatation_16x16,
    0,
    BinaryDilatation3x3Plugin
);
