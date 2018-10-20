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
#include "FillHolesPlugin.hpp"
#include <image_fill_holes_16x16.h>

static void PluginInitializer( );
static void PluginCleaner( );
static XErrorCode UpdateBoundingRectProperties( PropertyDescriptor* desc, const xvariant* parentValue );
static XErrorCode UpdateAreaProperties( PropertyDescriptor* desc, const xvariant* parentValue );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 2 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000011, 0x00000004 };

// Fill criteria property
static PropertyDescriptor fillCriteriaProperty =
{ XVT_U1, "Fill Criteria", "fillCriteria", "Specifies criteria used for filling holes.", PropertyFlag_SelectionByIndex };
// Min Width property
static PropertyDescriptor minWidthProperty =
{ XVT_U4, "Min Width", "minWidth", "Specifies the minimum allowed holes' width to keep.", PropertyFlag_Dependent };
// Min Height property
static PropertyDescriptor minHeightProperty =
{ XVT_U4, "Min Height", "minHeight", "Specifies the minimum allowed holes' height to keep.", PropertyFlag_Dependent };
// Coupled Filtering property
static PropertyDescriptor coupleFilteringProperty =
{ XVT_Bool, "Coupled Filtering", "coupledFiltering", "Specifies if coupled size filtering must be done or not.", PropertyFlag_Dependent };
// Min Area property
static PropertyDescriptor minAreaProperty =
{ XVT_U4, "Min Area", "minArea", "Specifies the minimum allowed blobs' area.", PropertyFlag_Dependent };
// Fill Cilor property
static PropertyDescriptor fillColorProperty =
{ XVT_ARGB, "Fill Color", "fillColor", "Color used for filling holes.", PropertyFlag_None };
// Holes Filled property
static PropertyDescriptor holesFilledProperty =
{ XVT_U4, "Holes Filled", "holesFilled", "Tells the number of holes filled.", PropertyFlag_ReadOnly };
// Holes Left property
static PropertyDescriptor holesLeftProperty =
{ XVT_U4, "Holes Left", "holesLeft", "Tells the number of holes left.", PropertyFlag_ReadOnly };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &fillCriteriaProperty, &minWidthProperty, &minHeightProperty, &coupleFilteringProperty, &minAreaProperty,
    &fillColorProperty, &holesFilledProperty, &holesLeftProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** FillHolesPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_BlobsProcessing,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Fill Holes",
    "FillHoles",
    "Fills holes in objects found in the specified image.",

    /* Long description */
    "The plug-in fills holes within blobs found in the specified image. Holes are treated as black areas within "
    "non black objects of an image. In other words, it fills all black areas of an image, which are not connected "
    "to the image's edge. Instead of filling all holes, it is possible to specify minimum size/area, so that only "
    "holes which are smaller than the limit get filled."

    ,
    &image_fill_holes_16x16, // small icon
    nullptr,
    FillHolesPlugin,

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
    // Fill Criteria property
    static const char* fillType[] = { "All holes", "By bounding rectangle", "By area" };

    InitSelectionProperty( &fillCriteriaProperty, fillType, XARRAY_SIZE( fillType ), 0 );

    // Properties for bounding rectangle filtering
    for ( int i = 1; i < 3; i++ )
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

    coupleFilteringProperty.DefaultValue.type = XVT_Bool;
    coupleFilteringProperty.DefaultValue.value.boolVal = false;

    coupleFilteringProperty.ParentProperty = 0;
    coupleFilteringProperty.Updater = UpdateBoundingRectProperties;

    // Properties for area filtering
    minAreaProperty.DefaultValue.type = XVT_U4;
    minAreaProperty.MinValue.type     = XVT_U4;
    minAreaProperty.MaxValue.type     = XVT_U4;

    minAreaProperty.DefaultValue.value.uiVal = 5;
    minAreaProperty.MinValue.value.uiVal     = 1;
    minAreaProperty.MaxValue.value.uiVal     = 100000000;

    minAreaProperty.ParentProperty = 0;
    minAreaProperty.Updater = UpdateAreaProperties;

    // Fill Color property
    fillColorProperty.DefaultValue.type = XVT_ARGB;
    fillColorProperty.DefaultValue.value.argbVal.argb = 0xFFFFFFFF;
}

// Clean-up plugin - deallocate strings
static void PluginCleaner( )
{
    CleanSelectionProperty( &fillCriteriaProperty );
}

// Enable/disable properties related to filling by bounding rectangle
XErrorCode UpdateBoundingRectProperties( PropertyDescriptor* desc, const xvariant* parentValue )
{
    XErrorCode ret = ErrorFailed;
    uint8_t    fillCriteria;

    ret = XVariantToUByte( parentValue, &fillCriteria );

    if ( ret == SuccessCode )
    {
        if ( fillCriteria == 1 )
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

// Enable/disable properties related to filling by area
XErrorCode UpdateAreaProperties( PropertyDescriptor* desc, const xvariant* parentValue )
{
    XErrorCode ret = ErrorFailed;
    uint8_t    fillCriteria;

    ret = XVariantToUByte( parentValue, &fillCriteria );

    if ( ret == SuccessCode )
    {
        if ( fillCriteria == 2 )
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
