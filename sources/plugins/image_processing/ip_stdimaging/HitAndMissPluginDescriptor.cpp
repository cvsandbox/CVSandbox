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
#include <image_hit_and_miss_16x16.h>
#include "HitAndMissPlugin.hpp"

static void PluginInitializer( );
static void PluginCleaner( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x00000032 };

// Mode property
static PropertyDescriptor modeProperty =
{ XVT_U1, "Mode", "mode", "Mode of the Hit and Miss operator.", PropertyFlag_SelectionByIndex };

// Structuring Element property
static PropertyDescriptor structuringElementProperty =
{ XVT_I1 | XVT_Array2d, "Structuring Element", "structuringElement", "Structuring element to use with the operator.", PropertyFlag_PreferredEditor_HitAndMissStructuringElement };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &modeProperty, &structuringElementProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** HitAndMissPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_Morphology,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Hit and Miss",
    "HitAndMiss",

    "Applies Hit and Miss morphology operator using the specified structuring element.",
    
    /* Long description */
    "Hit and Miss is a morphological operator that detects a given pattern in a binary image (grayscale image "
    "in this case, which contains only 0 and 255 values - background and objects). The pattern is specified by "
    "a structuring element, which contains values in the [-1, 1] range: 1 - look for object pixels, 0 - look for "
    "background pixels, -1 - don't care. For example, the next structuring element can be used to look for patterns, "
    "which look like left-bottom corners of an object:<br><tt>"
    "0 1 -1<br>"
    "0 1&nbsp;&nbsp;1<br>"
    "0 0&nbsp;&nbsp;0</tt><br><br>"

    "Once the window of neighbour pixels is checked for matching with the structuring element, the corresponding pixel "
    "in destination image is set to one of the possible values, which depend on the operator's <b>mode</b>. "
    "In the <b>Hit-and-Miss</b> mode, destination pixel is set to 255 on hit (match) or 0 on miss. In the <b>Thinning</b> "
    "mode, destination pixel is set to 0 on hit or it keeps original value on miss. In the <b>Thickening</b> mode, "
    "destination pixel is set to 255 on hit or it keeps original value on miss.<br><br>"

    "See also: <a href='{AF000003-00000000-00000001-00000030}'>Morphology Operator</a>"
    ,
    &image_hit_and_miss_16x16,
    0,
    HitAndMissPlugin,

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
    static const char* modeNames[] = { "Hit-and-Miss", "Thinning", "Thickening" };

    // Type property
    modeProperty.DefaultValue.type = XVT_U1;
    modeProperty.DefaultValue.value.ubVal = 0;

    modeProperty.MinValue.type = XVT_U1;
    modeProperty.MinValue.value.ubVal = 0;

    modeProperty.MaxValue.type = XVT_U1;
    modeProperty.MaxValue.value.ubVal = XARRAY_SIZE( modeNames ) - 1;

    modeProperty.ChoicesCount = XARRAY_SIZE( modeNames );
    modeProperty.Choices = new xvariant[modeProperty.ChoicesCount];

    for ( int i = 0; i < modeProperty.ChoicesCount; i++ )
    {
        modeProperty.Choices[i].type = XVT_String;
        modeProperty.Choices[i].value.strVal = XStringAlloc( modeNames[i] );
    }

    // set information about min/max values for the elements of the array property
    structuringElementProperty.MinValue.type = XVT_I1;
    structuringElementProperty.MinValue.value.sVal = -1;

    structuringElementProperty.MaxValue.type = XVT_I1;
    structuringElementProperty.MaxValue.value.sVal = 1;
}

// Clean-up plugin - deallocate strings
static void PluginCleaner( )
{
    for ( int i = 0; i < modeProperty.ChoicesCount; i++ )
    {
        XVariantClear( &modeProperty.Choices[i] );
    }

    delete [] modeProperty.Choices;
}
