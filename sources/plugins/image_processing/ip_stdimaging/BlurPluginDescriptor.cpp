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
#include <image_blur_16x16.h>
#include "BlurPlugin.hpp"

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x00000012 };

// Register the plug-in
REGISTER_CPP_PLUGIN
(
    PluginID,
    PluginFamilyID_ImageSmoothing,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Blur 5x5",
    "Blur5x5",
    "Blurs image using 5x5 square structuring element.",

    /* Long description */
    "The plug-in performs image blurring by applying the below shown 5x5 kernel. "
    "<b>Note:</b> for edge pixels only the available pixels of the source image are used "
    "(which results in adjusted divider as well - only corresponding kernel's coefficients are used).<br><br>"

    "<tt>"
    "| 1  2  3  2  1 |<br>"
    "| 2  4  5  4  2 |<br>"
    "| 3  5  6  5  3 |<br>"
    "| 2  4  5  4  2 |<br>"
    "| 1  2  3  2  1 |</tt>"
    ,
    &image_blur_16x16,
    0,
    BlurPlugin
);
