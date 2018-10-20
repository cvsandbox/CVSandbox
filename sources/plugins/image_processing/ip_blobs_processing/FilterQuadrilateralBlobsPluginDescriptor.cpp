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
#include "FilterQuadrilateralBlobsPlugin.hpp"
#include <image_filter_quadrilaterals_16x16.h>

static void PluginInitializer( );
static void PluginCleaner( );
static XErrorCode UpdateMaxAngleErrorProperty( PropertyDescriptor* desc, const xvariant* parentValue );
static XErrorCode UpdateMaxLengthErrorProperty( PropertyDescriptor* desc, const xvariant* parentValue );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 1 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000011, 0x00000007 };

// Filter Image property
static PropertyDescriptor filterImageProperty =
{ XVT_Bool, "Filter Image", "filterImage", "Filter provided image or find quadrilaterals only.", PropertyFlag_None };
// Quadrilaterals To Keep property
static PropertyDescriptor quadrilateralsToKeepProperty =
{ XVT_U1, "Quadrilaterals To Keep", "quadrilateralsToKeep", "Specifies type of quadrilaterals to keep.", PropertyFlag_SelectionByIndex };
// Min Size property
static PropertyDescriptor minSizeProperty =
{ XVT_U4, "Min Size", "minSize", "Specifies the minimum allowed length of quadrilateral's sides.", PropertyFlag_None };
// Max Size property
static PropertyDescriptor maxSizeProperty =
{ XVT_U4, "Max Size", "maxSize", "Specifies the maximum allowed length of quadrilateral's sides.", PropertyFlag_None };
// Relative Distortion property
static PropertyDescriptor relDistortionProperty =
{ XVT_R4, "Relative Distortion", "relDistortion", "Allowed shape's distortion relative to its size (%).", PropertyFlag_None };
// Minimum Distortion property
static PropertyDescriptor minDistortionProperty =
{ XVT_R4, "Minimum Distortion", "minDistortion", "Minimum value of allowed shape's distortion.", PropertyFlag_None };
// Max Angle Error property
static PropertyDescriptor maxAngleErrorProperty =
{ XVT_R4, "Max Angle Error", "maxAngleError", "Maximum allowed angle error (deg), when checking for rectangle.", PropertyFlag_Dependent };
// Max Length Error property
static PropertyDescriptor maxLengthErrorProperty =
{ XVT_R4, "Max Length Error", "maxLengthError", "Maximum allowed length error (%), when checking for square.", PropertyFlag_Dependent };

// Quadrilaterals Found property
static PropertyDescriptor quadrilateralsFoundProperty =
{ XVT_U4, "Quadrilaterals Found", "quadrilateralsFound", "Tells the mumber of found circles.", PropertyFlag_ReadOnly };
// Quadrilaterals property
static PropertyDescriptor quadrilateralsProperty =
{ XVT_Point | XVT_ArrayJagged, "Quadrilaterals", "quadrilaterals", "Coordinates of detected quadrilaterals (4 corners for each object).", PropertyFlag_ReadOnly };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &filterImageProperty, &quadrilateralsToKeepProperty,
    &minSizeProperty, &maxSizeProperty, &relDistortionProperty, &minDistortionProperty, &maxAngleErrorProperty, &maxLengthErrorProperty,
    &quadrilateralsFoundProperty, &quadrilateralsProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** FilterQuadrilateralBlobsPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_BlobsProcessing,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Filter Quadrilateral Blobs",
    "FilterQuadrilateralBlobs",
    "Keeps blobs having quadrilateral shape and removes others.",

    /* Long description */
    "The plug-in allows filtering an image by keeping only objects with quadrilateral shape (it can be "
    "set to rectangle or square shape). For each found quadrilateral, it finds its 4 corners. If the "
    "<b>Filter Image</b> property is set to <b>False</b>, the plug-in does not do any changes to the "
    "source image - only finds quadrilateral.<br><br>"

    "The plug-in first tries finding four candidate corners of a quadrilateral. If found, it then checks "
    "mean deviation of object's edge pixels from the lines connecting the found corners. The maximum allowed "
    "mean deviation is calculated for each side as:<br>"
    "<b>MAX( Minimum Distortion, ( Relative Distortion / 100 ) * MAX( LengthX, LengthY ) )</b>,<br>"
    "where <b>LengthX</b> and <b>LengthY</b> are side's lengths in X/Y directions. By changing "
    "<b>Minimum Distortion</b> and <b>Relative Distortion</b> it is possible to control how perfect should an "
    "object fit the quadrilateral shape.<br><br>"

    "If looking for rectangles, the <b>Max Angle Error</b> property controls the allowed error of rectangle's "
    "angles. If looking for squares, the <b>Max Length Error</b> controls the allowed difference between length "
    "of adjacent sides of a square.<br><br>"

    "<b>Note</b>: the plug-in is targeted to images, which are result of some segmentation routine. It assumes "
    "all black pixels belong to background and everything else belongs to objects.<br><br>"

    "See also: <a href='{AF000003-00000000-00000011-00000006}'>Filter Circle Blobs</a>, "
    "<a href='{AF000003-00000000-00000011-00000003}'>Filter Blobs By Size</a>."
    ,
    &image_filter_quadrilaterals_16x16, // small icon
    nullptr,
    FilterQuadrilateralBlobsPlugin,

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
    filterImageProperty.DefaultValue.type = XVT_Bool;
    filterImageProperty.DefaultValue.value.boolVal = true;

    minSizeProperty.DefaultValue.type = XVT_U4;
    minSizeProperty.DefaultValue.value.uiVal = 10;

    minSizeProperty.MinValue.type = XVT_U4;
    minSizeProperty.MinValue.value.uiVal = 5;

    minSizeProperty.MaxValue.type = XVT_U4;
    minSizeProperty.MaxValue.value.uiVal = 10000;

    maxSizeProperty.DefaultValue.type = XVT_U4;
    maxSizeProperty.DefaultValue.value.uiVal = 10000;

    maxSizeProperty.MinValue.type = XVT_U4;
    maxSizeProperty.MinValue.value.uiVal = 5;

    maxSizeProperty.MaxValue.type = XVT_U4;
    maxSizeProperty.MaxValue.value.uiVal = 10000;

    // Relative Distortion Limit property
    relDistortionProperty.DefaultValue.type = XVT_R4;
    relDistortionProperty.DefaultValue.value.fVal = 5.0f;

    relDistortionProperty.MinValue.type = XVT_R4;
    relDistortionProperty.MinValue.value.fVal = 1.0f;

    relDistortionProperty.MaxValue.type = XVT_R4;
    relDistortionProperty.MaxValue.value.fVal = 50.0f;

    // Minimum Distortion Limit property
    minDistortionProperty.DefaultValue.type = XVT_R4;
    minDistortionProperty.DefaultValue.value.fVal = 0.5f;

    minDistortionProperty.MinValue.type = XVT_R4;
    minDistortionProperty.MinValue.value.fVal = 0.05f;

    minDistortionProperty.MaxValue.type = XVT_R4;
    minDistortionProperty.MaxValue.value.fVal = 10.0f;

    // Max Angle Error property
    maxAngleErrorProperty.DefaultValue.type = XVT_R4;
    maxAngleErrorProperty.DefaultValue.value.fVal = 7.0f;

    maxAngleErrorProperty.MinValue.type = XVT_R4;
    maxAngleErrorProperty.MinValue.value.fVal = 0.0f;

    maxAngleErrorProperty.MaxValue.type = XVT_R4;
    maxAngleErrorProperty.MaxValue.value.fVal = 20.0f;

    maxAngleErrorProperty.ParentProperty = 1;
    maxAngleErrorProperty.Updater = UpdateMaxAngleErrorProperty;

    // Max Length Error property
    maxLengthErrorProperty.DefaultValue.type = XVT_R4;
    maxLengthErrorProperty.DefaultValue.value.fVal = 15.0f;

    maxLengthErrorProperty.MinValue.type = XVT_R4;
    maxLengthErrorProperty.MinValue.value.fVal = 0.0f;

    maxLengthErrorProperty.MaxValue.type = XVT_R4;
    maxLengthErrorProperty.MaxValue.value.fVal = 50.0f;

    maxLengthErrorProperty.ParentProperty = 1;
    maxLengthErrorProperty.Updater = UpdateMaxLengthErrorProperty;

    // Quadrilaterals To Keep property
    static const char* quadTypes[] = { "All", "Rectangles", "Squares" };

    InitSelectionProperty( &quadrilateralsToKeepProperty, quadTypes, XARRAY_SIZE( quadTypes ), 0 );
}

// Clean-up plugin - deallocate strings
static void PluginCleaner( )
{
    CleanSelectionProperty( &quadrilateralsToKeepProperty );
}

// Enable/disable Max Angle Error property depending on the value of parent property
XErrorCode UpdateMaxAngleErrorProperty( PropertyDescriptor* desc, const xvariant* parentValue )
{
    XErrorCode ret = ErrorFailed;
    uint8_t    quadsToKeep;

    ret = XVariantToUByte( parentValue, &quadsToKeep );

    if ( ret == SuccessCode )
    {
        if ( quadsToKeep >= 1)
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

// Enable/disable Max Length Error property depending on the value of parent property
XErrorCode UpdateMaxLengthErrorProperty( PropertyDescriptor* desc, const xvariant* parentValue )
{
    XErrorCode ret = ErrorFailed;
    uint8_t    quadsToKeep;

    ret = XVariantToUByte( parentValue, &quadsToKeep );

    if ( ret == SuccessCode )
    {
        if ( quadsToKeep == 2 )
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
