/*
    Bar codes detection and recognition plug-ins for Computer Vision Sandbox

    Copyright (C) 2011-2019, cvsandbox
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
#include "BarCodeDetectorPlugin.hpp"
#include <image_barcode_16x16.h>

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 1 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000016, 0x00000001 };

// Max Barcodes property
static PropertyDescriptor maxBarcodesProperty =
{ XVT_U4, "Max Barcodes", "maxBarcodes", "Maximum number of barcodes to look for.", PropertyFlag_None };

// Barcodes Found property
static PropertyDescriptor barcodesFoundProperty =
{ XVT_U4, "Barcodes Found", "barcodesFound", "Number of found barcodes in the processed image.", PropertyFlag_ReadOnly };

// Barcode Quadrilateral property
static PropertyDescriptor barcodeQuadrilateralsProperty =
{ XVT_Point | XVT_ArrayJagged, "Barcode Quadrilaterals", "barcodeQuadrilaterals", "Quadrilaterals of the detected barcodes (4 corners for each code).", PropertyFlag_ReadOnly };
// Barcode Rectangle Position property
static PropertyDescriptor barcodeRectanglePositionProperty =
{ XVT_Point | XVT_Array, "Barcode Rectangle Position", "barcodeRectanglePosition", "Top-left corner coordinates of barcodes' bounding rectangle.", PropertyFlag_ReadOnly };
// Barcode Rectangle Size property
static PropertyDescriptor barcodeRectangleSizeProperty =
{ XVT_Size | XVT_Array, "Barcode Rectangle Size", "barcodeRectangleSize", "Size (width/height) of barcodes' bounding rectangle.", PropertyFlag_ReadOnly };
// Barcode Size property
static PropertyDescriptor isVerticalProperty =
{ XVT_Bool | XVT_Array, "Is Vertical", "isVertical", "Indicates if barcode lines have near vertical orientation.", PropertyFlag_ReadOnly };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &maxBarcodesProperty, &barcodesFoundProperty, &barcodeQuadrilateralsProperty,
    &barcodeRectanglePositionProperty, &barcodeRectangleSizeProperty, &isVerticalProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** BarCodeDetectorPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_Default,

    PluginType_ImageProcessing,
    PluginVersion,
    "Bar Code Detector",
    "BarCodeDetector",
    "Detects 1D linear bard codes in images.",

    /* Long description */
    "The plug-in detects 1D bar codes in images (it does not do recognition). For each detected bar code, it "
    "provides its quadrilateral (coordinates of 4 corners), bounding rectangle and orientation - if lines look "
    "more vertical or horizontal."
    ,
    &image_barcode_16x16, // small icon
    nullptr,
    BarCodeDetectorPlugin,

    XARRAY_SIZE( pluginProperties ),
    pluginProperties,
    PluginInitializer,
    nullptr,
    nullptr  // no dynamic properties update
);

// Complete properties description by initializing those parts, which were not
// initialized during properties array declaration
static void PluginInitializer( )
{
    // Max Barcodes property
    maxBarcodesProperty.DefaultValue.type        = XVT_U4;
    maxBarcodesProperty.DefaultValue.value.uiVal = 3;

    maxBarcodesProperty.MinValue.type        = XVT_U4;
    maxBarcodesProperty.MinValue.value.uiVal = 1;

    maxBarcodesProperty.MaxValue.type        = XVT_U4;
    maxBarcodesProperty.MaxValue.value.uiVal = 5;
}
