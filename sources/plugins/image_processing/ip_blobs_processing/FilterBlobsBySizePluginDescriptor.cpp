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
#include "FilterBlobsBySizePlugin.hpp"
#include <image_filter_blobs_16x16.h>

static void PluginInitializer( );
static void PluginCleaner( );
static XErrorCode UpdateBoundingRectProperties( PropertyDescriptor* desc, const xvariant* parentValue );
static XErrorCode UpdateAreaProperties( PropertyDescriptor* desc, const xvariant* parentValue );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 3 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000011, 0x00000003 };

// Filter Image property
static PropertyDescriptor filterImageProperty =
{ XVT_Bool, "Filter Image", "filterImage", "Filter provided image or find blobs only.", PropertyFlag_None };
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
{ XVT_U4, "Blobs Left", "blobsLeft", "Tells the number of blobs left after filtering.", PropertyFlag_ReadOnly };
// Blobs Removed property
static PropertyDescriptor blobsRemovedProperty =
{ XVT_U4, "Blobs Removed", "blobsRemoved", "Tells the number of blobs removed during filtering.", PropertyFlag_ReadOnly };

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
    &filterImageProperty, &sizeCriteriaProperty,
    &minWidthProperty, &minHeightProperty, &maxWidthProperty, &maxHeightProperty, &coupleFilteringProperty,
    &minAreaProperty, &maxAreaProperty,
    &blobsLeftProperty, &blobsRemovedProperty,
    &blobsPositionProperty, &blobsSizeProperty, &blobsAreaProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** FilterBlobsBySizePlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;


// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_BlobsProcessing,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Filter Blobs By Size",
    "FilterBlobsBySize",
    "Keeps only the blobs within the specified size limits.",

    /* Long description */
    "The plug-in performs filtering of blobs by size - removing those which are smaller or bigger than the "
    "specified limit. In the coupled filtering mode, a blob is removed if both its width <b>and</b> height "
    "are smaller than the minimum limit or larger than the maximum limit. In the uncoupled filtering mode, a "
    "blob is removed if its width <b>or</b< height is smaller than minimum or larger than maximum limits.<br><br>"

    "<b>Note</b>: the filter treats all black pixels as background and everything else as blobs. So it assumes that "
    "some color filtering/segmentation was applied first.<br><br>"

    "After applying the filter, it is possible to query number of removed and kept blobs. For the kept blobs "
    "it is possible to get their position, size and area (number of pixels in a blob). If no blobs were kept, "
    "those arrays are empty (zero length).<br><br>"

    "See also: <a href='{AF000003-00000000-00000011-00000001}'>Keep Biggest Blob</a>."
    ,
    &image_filter_blobs_16x16, // small icon
    nullptr,
    FilterBlobsBySizePlugin,

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
    filterImageProperty.DefaultValue.type = XVT_Bool;
    filterImageProperty.DefaultValue.value.boolVal = true;

    // Properties for bounding rectangle filtering
    for ( int i = 2; i < 6; i++ )
    {
        pluginProperties[i]->DefaultValue.type = XVT_U4;
        pluginProperties[i]->MinValue.type     = XVT_U4;
        pluginProperties[i]->MaxValue.type     = XVT_U4;

        pluginProperties[i]->MinValue.value.uiVal = 1;
        pluginProperties[i]->MaxValue.value.uiVal = 10000;

        pluginProperties[i]->ParentProperty = 1;
        pluginProperties[i]->Updater = UpdateBoundingRectProperties;
    }

    minWidthProperty.DefaultValue.value.uiVal  = 5;
    minHeightProperty.DefaultValue.value.uiVal = 5;
    maxWidthProperty.DefaultValue.value.uiVal  = 10000;
    maxHeightProperty.DefaultValue.value.uiVal = 10000;

    coupleFilteringProperty.DefaultValue.type = XVT_Bool;
    coupleFilteringProperty.DefaultValue.value.boolVal = false;

    coupleFilteringProperty.ParentProperty = 1;
    coupleFilteringProperty.Updater = UpdateBoundingRectProperties;

    // Properties for area filtering
    for ( int i = 7; i < 9; i++ )
    {
        pluginProperties[i]->DefaultValue.type = XVT_U4;
        pluginProperties[i]->MinValue.type     = XVT_U4;
        pluginProperties[i]->MaxValue.type     = XVT_U4;

        pluginProperties[i]->MinValue.value.uiVal = 1;
        pluginProperties[i]->MaxValue.value.uiVal = 100000000;

        pluginProperties[i]->ParentProperty = 1;
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
