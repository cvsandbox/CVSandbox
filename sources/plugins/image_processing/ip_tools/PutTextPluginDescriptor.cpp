/*
    Image processing tools plug-ins of Computer Vision Sandbox

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
#include <image_put_text_16x16.h>
#include "PutTextPlugin.hpp"

static void PluginInitializer( );
static void PluginCleaner( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000007, 0x0000000C };

// Line 1 property
static PropertyDescriptor line1Property =
{ XVT_String, "Line #1", "line1", "1st text line to put.", PropertyFlag_None };
// Line 2 property
static PropertyDescriptor line2Property =
{ XVT_String, "Line #2", "line2", "2nd text line to put.", PropertyFlag_None };
// Line 3 property
static PropertyDescriptor line3Property =
{ XVT_String, "Line #3", "line3", "3rd text line to put.", PropertyFlag_None };
// Line 4 property
static PropertyDescriptor line4Property =
{ XVT_String, "Line #4", "line4", "4th text line to put.", PropertyFlag_None };
// Line 5 property
static PropertyDescriptor line5Property =
{ XVT_String, "Line #5", "line5", "5th text line to put.", PropertyFlag_None };
// Text Color property
static PropertyDescriptor textColorProperty =
{ XVT_ARGB, "Color", "color", "Color of the text to put.", PropertyFlag_None };
// Background Color property
static PropertyDescriptor backgroundColorProperty =
{ XVT_ARGB, "Background Color", "bgColor", "Background color to use.", PropertyFlag_None };
// Alignment property
static PropertyDescriptor alignmentProperty =
{ XVT_U1, "Alignment", "alignment", "Text alignment to apply.", PropertyFlag_SelectionByIndex };
// Align to Corners property
static PropertyDescriptor alignToCornersProperty =
{ XVT_Bool, "Align to Corners", "alignToCorners", "Align to corners instread of coordinates." };
// Coordinates property
static PropertyDescriptor coordinatesProperty =
{ XVT_Point, "Coordinates", "coordinates", "Text's X/Y coordinates.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &line1Property, &line2Property, &line3Property, &line4Property, &line5Property,
    &textColorProperty, &backgroundColorProperty, &alignmentProperty, &alignToCornersProperty,
    &coordinatesProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** PutTextPlugin::propertiesDescription = ( const PropertyDescriptor** ) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_Default,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Put Text",
    "PutText",
    "Puts specified lines of text on an image.",

    /* Long description */
    "The plug-in can be used to put up to 5 lines of text on a specified image. If the <b>alignToCorners</b> property "
    "is set to <b>false</b>, then the text is put at the specified X/Y coordinates and is aligned at that point using "
    "one of the four possible alignments. If the <b>alignToCorners</b> property is set to <b>true</b> however, "
    "the text is put into one of the image's corners, which corresponds to the selected allignment.<br><br>"

    "The plug-in uses standard 8x8 ASCII font. No Unicode or custom fonts support."
    ,
    &image_put_text_16x16,
    0,
    PutTextPlugin,

    sizeof( pluginProperties ) / sizeof( PropertyDescriptor* ),
    pluginProperties,
    PluginInitializer,
    PluginCleaner,
    0  // no dynamic properties update
);

// Complete properties description by initializing those parts, which were not
// initialized during properties array declaration
static void PluginInitializer( )
{
    // Text Color property
    textColorProperty.DefaultValue.type               = XVT_ARGB;
    textColorProperty.DefaultValue.value.argbVal.argb = 0xFFFFFFFF;

    // Background Color property
    backgroundColorProperty.DefaultValue.type               = XVT_ARGB;
    backgroundColorProperty.DefaultValue.value.argbVal.argb = 0xFF000000;

    // Coordinates property
    coordinatesProperty.DefaultValue.type = XVT_Point;
    coordinatesProperty.DefaultValue.value.pointVal.x = 0;
    coordinatesProperty.DefaultValue.value.pointVal.y = 0;

    // Align to Corners property
    alignToCornersProperty.DefaultValue.type = XVT_Bool;
    alignToCornersProperty.DefaultValue.value.boolVal = false;

    // Alignment property
    alignmentProperty.DefaultValue.type        = XVT_U1;
    alignmentProperty.DefaultValue.value.ubVal = 0;

    alignmentProperty.MaxValue.type        = XVT_U1;
    alignmentProperty.MaxValue.value.ubVal = 3;

    alignmentProperty.MinValue.type        = XVT_U1;
    alignmentProperty.MinValue.value.ubVal = 0;

    alignmentProperty.ChoicesCount = 4;
    alignmentProperty.Choices      = new xvariant[4];

    alignmentProperty.Choices[0].type         = XVT_String;
    alignmentProperty.Choices[0].value.strVal = XStringAlloc( "Top-Left" );

    alignmentProperty.Choices[1].type         = XVT_String;
    alignmentProperty.Choices[1].value.strVal = XStringAlloc( "Top-Right" );

    alignmentProperty.Choices[2].type         = XVT_String;
    alignmentProperty.Choices[2].value.strVal = XStringAlloc( "Bottom-Right" );

    alignmentProperty.Choices[3].type         = XVT_String;
    alignmentProperty.Choices[3].value.strVal = XStringAlloc( "Bottom-Left" );
}

// Clean-up plugin - deallocate strings
static void PluginCleaner( )
{
    for ( int i = 0; i < alignmentProperty.ChoicesCount; i++ )
    {
        XVariantClear( &alignmentProperty.Choices[i] );
    }

    delete [] alignmentProperty.Choices;
}
