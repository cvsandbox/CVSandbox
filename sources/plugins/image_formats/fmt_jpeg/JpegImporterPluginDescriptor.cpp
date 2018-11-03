/*
    JPEG images handling plug-ins of Computer Vision Sandbox

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
#include "JpegImporterPlugin.hpp"

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 1 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000002, 0x00000001 };

// Register the plug-in
REGISTER_CPP_PLUGIN
(
    PluginID,
    PluginFamilyID_Default,

    PluginType_ImageImporter,
    PluginVersion,
    "JPEG Importer",
    "JpegImporter",
    "JPEG files loading plug-in.",

    "The plug-in allows to load JPEG images.",
    0,
    0,
    JpegImporterPlugin
);
