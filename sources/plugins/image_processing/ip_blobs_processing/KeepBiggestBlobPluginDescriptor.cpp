/*
    Blobs' processing plug-ins of Computer Vision Sandbox

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
#include "KeepBiggestBlobPlugin.hpp"
#include <image_biggest_blob_16x16.h>

static void PluginInitializer( );
static void PluginCleaner( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 2 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000011, 0x00000001 };

// Size criteria property
static PropertyDescriptor sizeCriteriaProperty =
{ XVT_U1, "Size Criteria", "sizeCriteria", "Specifies size criteria to determine the biggest blob.", PropertyFlag_SelectionByIndex };
// Top left corner property
static PropertyDescriptor topLeftCornerProperty =
{ XVT_Point, "Top Left Corner", "topLeft", "Coordinates of top left corner of the found blob.", PropertyFlag_ReadOnly };
// Bottom right corner property
static PropertyDescriptor bottomRightCornerProperty =
{ XVT_Point, "Bottom Right Corner", "bottomRight", "Coordinates of bottom right corner of the found blob.", PropertyFlag_ReadOnly };
// Area property
static PropertyDescriptor areaProperty =
{ XVT_U4, "Area", "area", "Area of the found blob (number of pixels in it).", PropertyFlag_ReadOnly };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &sizeCriteriaProperty, &topLeftCornerProperty, &bottomRightCornerProperty, &areaProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** KeepBiggestBlobPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;


// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_BlobsProcessing,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Keep Biggest Blob",
    "KeepBiggestBlob",
    "Keeps only the biggest blob in the image while removing the rest.",

    /* Long description */
    "The plug-in keeps only the biggest blob, while removing (filling with black) all others. "
    "The size criteria can be either area of the blob (number of pixels in it) or area of the "
    "bounding rectangle. After the filter is applied, it is possible to retrieve coordinates "
    "of the top-left and bottom right corners of the found blob, as well as its area. <b>Note</b>: "
    "blob's coordinates and area are set to 0, if the processed image does not contain any blobs at all.<br><br>"

    "See also: <a href='{AF000003-00000000-00000011-00000003}'>Filter Blobs By Size</a>, "
    "<a href='{AF000003-00000000-00000011-00000002}'>Find Biggest Blob</a>."
    ,
    &image_biggest_blob_16x16, // small icon
    nullptr,
    KeepBiggestBlobPlugin,

    sizeof( pluginProperties ) / sizeof( PropertyDescriptor* ),
    pluginProperties,
    PluginInitializer,
    PluginCleaner,
    nullptr  // no dynamic properties update
);

// Complete properties description by initializing those parts, which were not
// initialized during properties array declaration
static void PluginInitializer( )
{
    static const char* sizeTypes[] = { "Biggest bounding rectangle", "Largest area" };

    InitSelectionProperty( &sizeCriteriaProperty, sizeTypes, XARRAY_SIZE( sizeTypes ), 0 );
}

// Clean-up plugin - deallocate strings
static void PluginCleaner( )
{
    CleanSelectionProperty( &sizeCriteriaProperty );
}
