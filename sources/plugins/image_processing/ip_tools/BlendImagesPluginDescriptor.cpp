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
#include <ximaging.h>
#include <image_blend_images_16x16.h>
#include "BlendImagesPlugin.hpp"

static void PluginInitializer( );
static void PluginCleaner( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 1 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000007, 0x0000000B };

// Blend Mode property
static PropertyDescriptor blendModeProperty =
{ XVT_U1, "Blend Mode", "blendMode", "Blend mode to use for blending two images.", PropertyFlag_SelectionByIndex };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &blendModeProperty,
};

// Let the class itself know description of its properties
const PropertyDescriptor** BlendImagesPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_TwoImageFilters,

    PluginType_ImageProcessingFilter2,
    PluginVersion,
    "Blend Images",
    "BlendImages",
    "Blends source image with another image of the same size/format.",

    "The plug-in implements multiply, screen and overlay <a href='https://en.wikipedia.org/wiki/Blend_modes'>blend modes</a>. "
    "The specified source image (top layer, <b>a</b>) is blended with another image (base layer, <b>b</b>) and result (<b>r</b>) is determined "
    "by the selected blend mode: <ul>"
    "<li>Multiply<br><b>r = a * b / 255</b>.</li>"
    "<li>Screen<br><b>r = 255 - (255 - a) * (255 - b) / 255</b>.</li>"
    "<li>Overlay<br><b>r = 2 * a * b / 255</b>, if b &lt; 128; <b>r = 255 - 2 * (255 - a) * (255 - b) / 255</b>, otherwise.</li>"
    "<li>Color Dodge<br><b>r = 255</b>, if b = 255; <b>min(255, a * 255 / (255 - b))</b>, otherwise."
    "<li>Color Burn<br><b>r = 0</b>, if b = 0; <b>max(0, 255 - (255 - a) * 255 / b)</b>, otherwise."
    "</ul>",

    &image_blend_images_16x16,
    nullptr,
    BlendImagesPlugin,

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
    static const char* blendModeNames[] = { "Multiply", "Screen", "Overlay", "Color Dodge", "Color Burn" };

    blendModeProperty.DefaultValue.type = XVT_U1;
    blendModeProperty.DefaultValue.value.ubVal = 0;

    blendModeProperty.MinValue.type = XVT_U1;
    blendModeProperty.MinValue.value.ubVal = 0;

    blendModeProperty.MaxValue.type = XVT_U1;
    blendModeProperty.MaxValue.value.ubVal = XARRAY_SIZE( blendModeNames ) - 1;;

    blendModeProperty.ChoicesCount = XARRAY_SIZE( blendModeNames );
    blendModeProperty.Choices = new xvariant[blendModeProperty.ChoicesCount];

    for ( int i = 0; i < blendModeProperty.ChoicesCount; i++ )
    {
        blendModeProperty.Choices[i].type = XVT_String;
        blendModeProperty.Choices[i].value.strVal = XStringAlloc( blendModeNames[i] );
    }
}

// Clean-up plugin - deallocate strings
static void PluginCleaner( )
{
    for ( int i = 0; i < blendModeProperty.ChoicesCount; i++ )
    {
        XVariantClear( &blendModeProperty.Choices[i] );
    }

    delete [] blendModeProperty.Choices;
}
