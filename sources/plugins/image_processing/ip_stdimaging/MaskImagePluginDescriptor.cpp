/*
    Standard image processing plug-ins of Computer Vision Sandbox

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
#include <image_mask_image_16x16.h>
#include "MaskImagePlugin.hpp"

static void PluginInitializer( );
static void PluginCleaner( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 1 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x0000002B };

// Mask Mode property
static PropertyDescriptor maskModeProperty =
{ XVT_U1, "Mask Mode", "maskMode", "Masking mode to apply.", PropertyFlag_SelectionByIndex };

// Fill Color property
static PropertyDescriptor fillColorProperty =
{ XVT_ARGB, "Fill color", "fillColor", "Color used to fill filtered areas.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &maskModeProperty, &fillColorProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** MaskImagePlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_TwoImageFilters,

    PluginType_ImageProcessingFilter2,
    PluginVersion,
    "Mask Image",
    "MaskImage",
    "Applies a mask to the specified image.",

    "The plug-in sets pixels of a source image to the specified fill color if corresponding pixels of a mask "
    "image are equal to 0 while the <b>mask mode</b> property is set to <b>Fill on Zero</b>. All other pixels "
    "of the source image are left unchanged. In the <b>Fill on Non-zero</b> mode the behaviour is opposite - "
    "source image gets filled if corresponding pixels in mask image has non-zero values.<br><br>"

    "See also: <a href='{AF000003-00000000-00000001-00000017}'>Intersect Images</a>",

    &image_mask_image_16x16,
    nullptr,
    MaskImagePlugin,

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
    fillColorProperty.DefaultValue.type = XVT_ARGB;
    fillColorProperty.DefaultValue.value.argbVal.argb = 0xFF000000;

    // Mask Mode property
    maskModeProperty.DefaultValue.type = XVT_U1;
    maskModeProperty.DefaultValue.value.ubVal = 0;

    maskModeProperty.MinValue.type = XVT_U1;
    maskModeProperty.MinValue.value.ubVal = 0;

    maskModeProperty.MaxValue.type = XVT_U1;
    maskModeProperty.MaxValue.value.ubVal = 1;

    maskModeProperty.ChoicesCount = 2;
    maskModeProperty.Choices = new xvariant[3];

    maskModeProperty.Choices[0].type = XVT_String;
    maskModeProperty.Choices[0].value.strVal = XStringAlloc( "Fill on Zero" );

    maskModeProperty.Choices[1].type = XVT_String;
    maskModeProperty.Choices[1].value.strVal = XStringAlloc( "Fill on Non-zero" );
}

// Clean-up plug-in - deallocate strings
static void PluginCleaner( )
{
    for ( int i = 0; i < maskModeProperty.ChoicesCount; i++ )
    {
        XVariantClear( &maskModeProperty.Choices[i] );
    }

    delete[] maskModeProperty.Choices;
}
