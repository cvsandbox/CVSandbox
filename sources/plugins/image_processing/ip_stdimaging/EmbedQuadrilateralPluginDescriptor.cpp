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
#include <image_embed_quadrilateral_16x16.h>
#include "EmbedQuadrilateralPlugin.hpp"

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x00000025 };

// Points' properties
static PropertyDescriptor point1Property =
{ XVT_Point, "Point 1", "point1", "1st point of a quadrilateral.", PropertyFlag_None };
static PropertyDescriptor point2Property =
{ XVT_Point, "Point 2", "point2", "2nd point of a quadrilateral.", PropertyFlag_None };
static PropertyDescriptor point3Property =
{ XVT_Point, "Point 3", "point3", "3rd point of a quadrilateral.", PropertyFlag_None };
static PropertyDescriptor point4Property =
{ XVT_Point, "Point 4", "point4", "4th point of a quadrilateral.", PropertyFlag_None };

// Use interpolation property
static PropertyDescriptor useInterpolationProperty =
{ XVT_Bool, "Use interpolation", "useInterpolation", "Use interpolation or not.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &point1Property, &point2Property, &point3Property, &point4Property, &useInterpolationProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** EmbedQuadrilateralPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_TwoImageFilters,

    PluginType_ImageProcessingFilter2,
    PluginVersion,
    "Embed Quadrilateral",
    "EmbedQuadrilateral",
    "Embed another image into quadrilateral specified by 4 points in the source image.",

    /* Long description */
    "The plug-in performs the opposite of what the <a href='{AF000003-00000000-00000001-00000024}'>Extract Quadrilateral</a> plug-in does - "
    "it embeds another image into the specified quadrilateral of the target image."
    ,
    &image_embed_quadrilateral_16x16,
    nullptr,
    EmbedQuadrilateralPlugin,

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
    // Use interpolation property
    useInterpolationProperty.DefaultValue.type          = XVT_Bool;
    useInterpolationProperty.DefaultValue.value.boolVal = true;

    // Points' properties
    for ( int i = 0; i < 4; i++ )
    {
        pluginProperties[i]->DefaultValue.type = XVT_Point;

        pluginProperties[i]->MinValue.type       = XVT_I4;
        pluginProperties[i]->MinValue.value.iVal = -1000;

        pluginProperties[i]->MaxValue.type       = XVT_I4;
        pluginProperties[i]->MaxValue.value.iVal = 10000;

    }

    point1Property.DefaultValue.value.pointVal.x = 0;
    point1Property.DefaultValue.value.pointVal.y = 120;

    point2Property.DefaultValue.value.pointVal.x = 160;
    point2Property.DefaultValue.value.pointVal.y = 0;

    point3Property.DefaultValue.value.pointVal.x = 319;
    point3Property.DefaultValue.value.pointVal.y = 120;

    point4Property.DefaultValue.value.pointVal.x = 160;
    point4Property.DefaultValue.value.pointVal.y = 239;
}
