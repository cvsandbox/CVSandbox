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
#include "FindBlobsBySizePlugin.hpp"
#include <image_filter_blobs_16x16.h>

static void PluginInitializer( );
static void PluginCleaner( );
static XErrorCode UpdateBoundingRectProperties( PropertyDescriptor* desc, const xvariant* parentValue );
static XErrorCode UpdateAreaProperties( PropertyDescriptor* desc, const xvariant* parentValue );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 2 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000011, 0x00000005 };

// Size criteria property
static PropertyDescriptor sizeCriteriaProperty =
{ XVT_U1, "Size Criteria", "sizeCriteria", "Specifies size criteria to filter blobs.", PropertyFlag_SelectionByIndex };
// Min Width property
static PropertyDescriptor minWidthProperty =
{ XVT_U4, "Min Width", "minWidth", "Specifies the minimum allowed blobs' width.", PropertyFlag_Dependent };
// Min Height property
static PropertyDescriptor minHeightProperty =
{ XVT_U4, "Min Height", "minHeight", "Specifies the minimum allowed blobs' height.", PropertyFlag_Dependent };
// Max Width property
static PropertyDescriptor maxWidthProperty =
{ XVT_U4, "Max Width", "maxWidth", "Specifies the maximum allowed blobs' width.", PropertyFlag_Dependent };
// Max Height property
static PropertyDescriptor maxHeightProperty =
{ XVT_U4, "Max Height", "maxHeight", "Specifies the maximum allowed blobs' height.", PropertyFlag_Dependent };
// Coupled Filtering property
static PropertyDescriptor coupleFilteringProperty =
{ XVT_Bool, "Coupled Filtering", "coupledFiltering", "Specifies if coupled size filtering must be done or not.", PropertyFlag_Dependent };
// Min Area property
static PropertyDescriptor minAreaProperty =
{ XVT_U4, "Min Area", "minArea", "Specifies the minimum allowed blobs' area.", PropertyFlag_Dependent };
// Max Area property
static PropertyDescriptor maxAreaProperty =
{ XVT_U4, "Max Area", "maxArea", "Specifies the minimum allowed blobs' area.", PropertyFlag_Dependent };
// Blobs Left property
static PropertyDescriptor blobsLeftProperty =
{ XVT_U4, "Blobs Left", "blobsLeft", "Tells the number of found blobs satisfying size limits.", PropertyFlag_ReadOnly };
// Blobs Removed property
static PropertyDescriptor blobsRemovedProperty =
{ XVT_U4, "Blobs Removed", "blobsRemoved", "Tells the number of rejected blobs (out of size limits).", PropertyFlag_ReadOnly };

// Blobs Position property
static PropertyDescriptor blobsPositionProperty =
{ XVT_Point | XVT_Array, "Blobs Position", "blobsPosition", "Coordinates of kept blobs' top-left corners.", PropertyFlag_ReadOnly };
// Blobs Size property
static PropertyDescriptor blobsSizeProperty =
{ XVT_Size | XVT_Array, "Blobs Size", "blobsSize", "Sizes (width/height) of kept blobs.", PropertyFlag_ReadOnly };
// Blobs Area property
static PropertyDescriptor blobsAreaProperty =
{ XVT_U4 | XVT_Array, "Blobs Area", "blobsArea", "Areas of kept blobs.", PropertyFlag_ReadOnly };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &sizeCriteriaProperty,
    &minWidthProperty, &minHeightProperty, &maxWidthProperty, &maxHeightProperty, &coupleFilteringProperty,
    &minAreaProperty, &maxAreaProperty,
    &blobsLeftProperty, &blobsRemovedProperty,
    &blobsPositionProperty, &blobsSizeProperty, &blobsAreaProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** FindBlobsBySizePlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;


// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_BlobsProcessing,

    PluginType_ImageProcessing,
    PluginVersion,
    "Find Blobs By Size",
    "FindBlobsBySize",
    "Find blobs within the specified size limits.",

    /* Long description */
    "<b>Note</b>: This plug-in is deprecated. Use <a href='{AF000003-00000000-00000011-00000003}'>Filter Blobs By Size</a> "
    "plug-in instead configuring it not to change source image.<br><br>"

    "The plug-in does similar to what <a href='{AF000003-00000000-00000011-00000003}'>Filter Blobs By Size</a> "
    "plug-in does, but it only finds blobs satisfying specified size limits. After applying this plug-in, an "
    "image is kept unchanged and information about the found blobs can be obtained by querying corresponding "
    "properties.<br><br>"

    "See also: <a href='{AF000003-00000000-00000011-00000003}'>Filter Blobs By Size</a>, "
    "<a href='{AF000003-00000000-00000011-00000002}'>Find Biggest Blob</a>."
    ,
    &image_filter_blobs_16x16, // small icon
    nullptr,
    FindBlobsBySizePlugin,

    XARRAY_SIZE( pluginProperties ),
    pluginProperties,
    PluginInitializer,
    PluginCleaner,
    nullptr  // no dynamic properties update
);

// Complete properties description by initializing those parts, which were not
// initialized during properties array declaration
static void PluginInitializer( )
{
    // Properties for bounding rectangle filtering
    for ( int i = 1; i < 5; i++ )
    {
        pluginProperties[i]->DefaultValue.type = XVT_U4;
        pluginProperties[i]->MinValue.type     = XVT_U4;
        pluginProperties[i]->MaxValue.type     = XVT_U4;

        pluginProperties[i]->MinValue.value.uiVal = 1;
        pluginProperties[i]->MaxValue.value.uiVal = 10000;

        pluginProperties[i]->ParentProperty = 0;
        pluginProperties[i]->Updater = UpdateBoundingRectProperties;
    }

    minWidthProperty.DefaultValue.value.uiVal  = 5;
    minHeightProperty.DefaultValue.value.uiVal = 5;
    maxWidthProperty.DefaultValue.value.uiVal  = 10000;
    maxHeightProperty.DefaultValue.value.uiVal = 10000;

    coupleFilteringProperty.DefaultValue.type = XVT_Bool;
    coupleFilteringProperty.DefaultValue.value.boolVal = false;

    coupleFilteringProperty.ParentProperty = 0;
    coupleFilteringProperty.Updater = UpdateBoundingRectProperties;

    // Properties for area filtering
    for ( int i = 6; i < 8; i++ )
    {
        pluginProperties[i]->DefaultValue.type = XVT_U4;
        pluginProperties[i]->MinValue.type     = XVT_U4;
        pluginProperties[i]->MaxValue.type     = XVT_U4;

        pluginProperties[i]->MinValue.value.uiVal = 1;
        pluginProperties[i]->MaxValue.value.uiVal = 100000000;

        pluginProperties[i]->ParentProperty = 0;
        pluginProperties[i]->Updater = UpdateAreaProperties;
    }

    minAreaProperty.DefaultValue.value.uiVal = 5;
    maxAreaProperty.DefaultValue.value.uiVal = 100000000;

    // Size Criteria property
    static const char* sizeTypes[] = { "Biggest bounding rectangle", "Largest area" };

    InitSelectionProperty( &sizeCriteriaProperty, sizeTypes, XARRAY_SIZE( sizeTypes ), 0 );
}

// Clean-up plugin - deallocate strings
static void PluginCleaner( )
{
    CleanSelectionProperty( &sizeCriteriaProperty );
}

XErrorCode UpdateBoundingRectProperties( PropertyDescriptor* desc, const xvariant* parentValue )
{
    XErrorCode ret = ErrorFailed;
    uint8_t    sizeCriteria;

    ret = XVariantToUByte( parentValue, &sizeCriteria );

    if ( ret == SuccessCode )
    {
        if ( sizeCriteria == 0 )
        {
            desc->Flags &= ( ~PropertyFlag_Disabled );
        }
        else
        {
            desc->Flags |= PropertyFlag_Disabled;
        }
    }

    return ret;
}

XErrorCode UpdateAreaProperties( PropertyDescriptor* desc, const xvariant* parentValue )
{
    XErrorCode ret = ErrorFailed;
    uint8_t    sizeCriteria;

    ret = XVariantToUByte( parentValue, &sizeCriteria );

    if ( ret == SuccessCode )
    {
        if ( sizeCriteria == 1 )
        {
            desc->Flags &= ( ~PropertyFlag_Disabled );
        }
        else
        {
            desc->Flags |= PropertyFlag_Disabled;
        }
    }

    return ret;
}
