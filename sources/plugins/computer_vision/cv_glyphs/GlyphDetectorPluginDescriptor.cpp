/*
    Glyphs recognition and tracking plug-ins for Computer Vision Sandbox

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
#include "GlyphDetectorPlugin.hpp"
#include <image_glyph_16x16.h>

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 1 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000015, 0x00000001 };

// Glyph Size property
static PropertyDescriptor glyphSizeProperty =
{ XVT_U4, "Glyph Size", "glyphSize", "Size of square glyphs to detect (S x S).", PropertyFlag_None };
// Max Glyphs property
static PropertyDescriptor maxGlyphsProperty =
{ XVT_U4, "Max Glyphs", "maxGlyphs", "Maximum number of glyphs to look for.", PropertyFlag_None };

// Glyphs Found property
static PropertyDescriptor glyphsFoundProperty =
{ XVT_U4, "Glyphs Found", "glyphsFound", "Number of found glyphs in the processed image.", PropertyFlag_ReadOnly };

// Glyph Code property
static PropertyDescriptor glyphCodesProperty =
{ XVT_String | XVT_Array, "Glyph Codes", "glyphCodes", "Code strings for the detected glyphs.", PropertyFlag_ReadOnly };
// Glyph Quadrilateral property
static PropertyDescriptor glyphQuadrilateralsProperty =
{ XVT_Point | XVT_ArrayJagged, "Glyph Quadrilaterals", "glyphQuadrilaterals", "Quadrilaterals of the detected glyphs (4 corners for each glyph).", PropertyFlag_ReadOnly };
// Glyph Rectangle Position property
static PropertyDescriptor glyphRectanglePositionProperty =
{ XVT_Point | XVT_Array, "Glyph Rectangle Position", "glyphRectanglePosition", "Top-left corner coordinates of glyphs' bounding rectangle.", PropertyFlag_ReadOnly };
// Glyph Rectangle Size property
static PropertyDescriptor glyphRectangleSizeProperty =
{ XVT_Size | XVT_Array, "Glyph Rectangle Size", "glyphRectangleSize", "Size (width/height) of glyphs' bounding rectangle.", PropertyFlag_ReadOnly };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &glyphSizeProperty, &maxGlyphsProperty, &glyphsFoundProperty, &glyphCodesProperty, &glyphQuadrilateralsProperty,
    &glyphRectanglePositionProperty, &glyphRectangleSizeProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** GlyphDetectorPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_Default,

    PluginType_ImageProcessing,
    PluginVersion,
    "Glyph Detector",
    "GlyphDetector",
    "Detects square binary glyphs in images.",

    /* Long description */
    "The plug-in detects square binary glyphs in images. A binary glyph is an object, which contains black and "
    "white cells. This plug-in assumes that every glyph is surrounded by extra black cells from each side. So, "
    "if the plug-in is configured to detect 3x3 glyphs, then it will actually look for objects having 5x5 cells "
    "- all outer cells must be black, while the inner 3x3 cells can have either black or white color. The inner "
    "pattern is decoded and provided as a string having '0' for black cells and '1' for white cells. For each "
    "detected glyph the plug-in also provides its quadrilateral (coordinates of 4 corners) and bounding rectangle."
    ,
    &image_glyph_16x16, // small icon
    nullptr,
    GlyphDetectorPlugin,

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
    // Glyph Size property
    glyphSizeProperty.DefaultValue.type        = XVT_U4;
    glyphSizeProperty.DefaultValue.value.uiVal = 3;

    glyphSizeProperty.MinValue.type        = XVT_U4;
    glyphSizeProperty.MinValue.value.uiVal = 2;

    glyphSizeProperty.MaxValue.type        = XVT_U4;
    glyphSizeProperty.MaxValue.value.uiVal = 10;

    // Max Glyphs property
    maxGlyphsProperty.DefaultValue.type        = XVT_U4;
    maxGlyphsProperty.DefaultValue.value.uiVal = 3;

    maxGlyphsProperty.MinValue.type        = XVT_U4;
    maxGlyphsProperty.MinValue.value.uiVal = 1;

    maxGlyphsProperty.MaxValue.type        = XVT_U4;
    maxGlyphsProperty.MaxValue.value.uiVal = 10;
}
