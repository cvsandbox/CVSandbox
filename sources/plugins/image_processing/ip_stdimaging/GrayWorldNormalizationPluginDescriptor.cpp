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
#include <image_gray_world_normalization_16x16.h>
#include "GrayWorldNormalizationPlugin.hpp"

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x00000034 };

// Register the plug-in
REGISTER_CPP_PLUGIN
(
    PluginID,
    PluginFamilyID_ColorFilter,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Gray World Normalization",
    "GrayWorldNormalization",
    "Performs color normalization using gray world filter.",

    /* Long description */
    "The plug-in performs <a href='https://en.wikipedia.org/wiki/Color_normalization#Grey_world'>gray world normalization</a> "
    "of a color image. It first calculates mean values for each RGB channels and a global mean value across all channels. "
    "Then it multiplies all values in the RGB channels with one of the 3 corresponding coefficients: globalMean/redMean, "
    "globalMean/greenMean and globalMean/blueMean. In the result image, mean value of RGB channels get close values."
    ,
    &image_gray_world_normalization_16x16,
    0,
    GrayWorldNormalizationPlugin
);
