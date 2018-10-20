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
#include "FilterCircleBlobsPlugin.hpp"
#include <image_filter_circles_16x16.h>

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000011, 0x00000006 };

// Filter Image property
static PropertyDescriptor filterImageProperty =
{ XVT_Bool, "Filter Image", "filterImage", "Filter provided image or find circles only.", PropertyFlag_None };
// Min Radius property
static PropertyDescriptor minRadiusProperty =
{ XVT_U4, "Min Radius", "minRadius", "Specifies the minimum allowed radius of circles to keep.", PropertyFlag_None };
// Max Radius property
static PropertyDescriptor maxRadiusProperty =
{ XVT_U4, "Max Radius", "maxRadius", "Specifies the maximum allowed radius of circles to keep.", PropertyFlag_None };
// Relative Distortion property
static PropertyDescriptor relDistortionProperty =
{ XVT_R4, "Relative Distortion", "relDistortion", "Allowed shape's distortion relative to its size (%).", PropertyFlag_None };
// Minimum Distortion property
static PropertyDescriptor minDistortionProperty =
{ XVT_R4, "Minimum Distortion", "minDistortion", "Minimum value of allowed shape's distortion.", PropertyFlag_None };

// Circles Found property
static PropertyDescriptor circlesFoundProperty =
{ XVT_U4, "Circles Found", "circlesFound", "Tells the mumber of found circles.", PropertyFlag_ReadOnly };

// Circles Centers property
static PropertyDescriptor circlesCentersProperty =
{ XVT_PointF | XVT_Array, "Circles Centers", "circlesCenters", "Coordinates of kept circles' centers.", PropertyFlag_ReadOnly };
// Circles Radiuses property
static PropertyDescriptor circlesRadiusesProperty =
{ XVT_R4 | XVT_Array, "Circles Radiuses", "circlesRadiuses", "Radius values of kept circles.", PropertyFlag_ReadOnly };
// Mean Deviations property
static PropertyDescriptor meanDeviationsProperty =
{ XVT_R4 | XVT_Array, "Mean Deviations", "meanDeviations", "Mean deviations of objects' edges from detected cirlces.", PropertyFlag_ReadOnly };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &filterImageProperty, &minRadiusProperty, &maxRadiusProperty, &relDistortionProperty, &minDistortionProperty,
    &circlesFoundProperty, &circlesCentersProperty, &circlesRadiusesProperty, &meanDeviationsProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** FilterCircleBlobsPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;


// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_BlobsProcessing,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Filter Circle Blobs",
    "FilterCircleBlobs",
    "Keeps blobs having circle shape and removes others.",

    /* Long description */
    "The plug-in allows filtering an image by keeping only objects with circle shape. For each found circle, "
    "it calculates its center point and radius. If the <b>Filter Image</b> property is set to <b>False</b>, the "
    "plug-in does not do any changes to the source image - only finds circles.<br><br>"

    "The plug-in implements quite a simple algorithm. It assumes that circle's center is the center of object's "
    "bounding rectangle and radius is an average of the rectangle's half width and half height. Then it checks if "
    "mean deviation between object's edge pixels and the ideal circle does not exceed the limit, which is "
    "calculated as:<br>"
    "<b>MAX( Minimum Distortion, ( Relative Distortion / 100 ) * ( Width + Height ) / 2 )</b>,<br>"
    "where <b>Width</b> and <b>Height</b> are of the object's bounding rectangle. By changing "
    "<b>Minimum Distortion</b> and <b>Relative Distortion</b> it is possible to control how perfect should an "
    "object fit the circle shape. Alternatively the calculated <b>Mean Deviations</b> property can be used for custom rules "
    "(makes sense only when searching for circles, not filtering).<br><br>"

    "<b>Note</b>: the plug-in is targeted to images, which are result of some segmentation routine. It assumes "
    "all black pixels belong to background and everything else belongs to objects.<br><br>"

    "See also: <a href='{AF000003-00000000-00000011-00000007}'>Filter Quadrilateral Blobs</a>, "
    "<a href='{AF000003-00000000-00000011-00000003}'>Filter Blobs By Size</a>."
    ,
    &image_filter_circles_16x16, // small icon
    nullptr,
    FilterCircleBlobsPlugin,

    sizeof( pluginProperties ) / sizeof( PropertyDescriptor* ),
    pluginProperties,
    PluginInitializer,
    nullptr,
    nullptr  // no dynamic properties update
);

// Complete properties description by initializing those parts, which were not
// initialized during properties array declaration
static void PluginInitializer( )
{
    filterImageProperty.DefaultValue.type = XVT_Bool;
    filterImageProperty.DefaultValue.value.boolVal = true;

    // Relative Distortion Limit property
    relDistortionProperty.DefaultValue.type = XVT_R4;
    relDistortionProperty.DefaultValue.value.fVal = 3.0f;

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
}
