/*
    Motion detection plug-ins for Computer Vision Sandbox

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
#include "TwoFramesDifferenceDetectionPlugin.hpp"

static void PluginInitializer( );
static XErrorCode UpdateMotionHighlightProperty( PropertyDescriptor* desc, const xvariant* parentValue );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000020, 0x00000001 };

// Plug-in properties
static PropertyDescriptor pixelThresholdProperty =
{ XVT_I2, "Pixel Threshold", "pixelThreshold", "Threshold for pixel deference to treat as significant change.", PropertyFlag_None };
static PropertyDescriptor motionThresholdProperty =
{ XVT_R4, "Motion Threshold", "motionThreshold", "Percentage of changed pixels to trigger motion detection.", PropertyFlag_None };
static PropertyDescriptor motionLevelProperty =
{ XVT_R4, "Motion Level", "motionLevel", "Actual percentage of changed pixels.", PropertyFlag_ReadOnly };

static PropertyDescriptor highlightMotionProperty =
{ XVT_Bool, "Highlight Motion", "highlightMotion", "Highlight motion areas or not.", PropertyFlag_None };
static PropertyDescriptor highlightColoProperty =
{ XVT_ARGB, "Highlight Color", "highlightColor", "Color used to highlight motion areas.", PropertyFlag_Dependent };
static PropertyDescriptor highlightAmountProperty =
{ XVT_U2, "Highlight Amount", "highlightAmount", "Amount off difference map to add to source image for motion highlighting, %.", PropertyFlag_Dependent };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &pixelThresholdProperty, &motionThresholdProperty, &motionLevelProperty,
    &highlightMotionProperty, &highlightColoProperty, &highlightAmountProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** TwoFramesDifferenceDetectionPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_Default,

    PluginType_Detection,
    PluginVersion,
    "Two Frames Difference",
    "TwoFramesDifference",
    "Plug-in to detect motion in video stream based on two frames difference.",

    /* Long description */
    "???"
    ,
    nullptr,
    nullptr,
    TwoFramesDifferenceDetectionPlugin,

    sizeof( pluginProperties ) / sizeof( PropertyDescriptor* ),
    pluginProperties,
    PluginInitializer,
    nullptr,
    nullptr
);

// Complete properties description by initializing those parts, which were not
// initialized during properties array declaration
static void PluginInitializer( )
{
    // Pixel Threshol
    pixelThresholdProperty.DefaultValue.type = XVT_I2;
    pixelThresholdProperty.DefaultValue.value.sVal = 60;

    pixelThresholdProperty.MinValue.type = XVT_I2;
    pixelThresholdProperty.MinValue.value.sVal = 1;

    pixelThresholdProperty.MaxValue.type = XVT_I2;
    pixelThresholdProperty.MaxValue.value.sVal = 765;

    // Motion Threshol
    motionThresholdProperty.DefaultValue.type = XVT_R4;
    motionThresholdProperty.DefaultValue.value.fVal = 0.1f;

    motionThresholdProperty.MinValue.type = XVT_R4;
    motionThresholdProperty.MinValue.value.fVal = 0.01f;

    motionThresholdProperty.MaxValue.type = XVT_R4;
    motionThresholdProperty.MaxValue.value.fVal = 50.0f;

    // Highlight Motion
    highlightMotionProperty.DefaultValue.type = XVT_Bool;
    highlightMotionProperty.DefaultValue.value.boolVal = false;

    // Highlight Color
    highlightMotionProperty.DefaultValue.type = XVT_ARGB;
    highlightMotionProperty.DefaultValue.value.argbVal.argb = 0xFFFF0000;

    highlightMotionProperty.ParentProperty = 3;
    highlightMotionProperty.Updater = UpdateMotionHighlightProperty;

    // Highlight Amount
    highlightAmountProperty.DefaultValue.type = XVT_U2;
    highlightAmountProperty.DefaultValue.value.usVal = 30;

    highlightAmountProperty.MinValue.type = XVT_U2;
    highlightAmountProperty.MinValue.value.usVal = 10;

    highlightAmountProperty.MaxValue.type = XVT_U2;
    highlightAmountProperty.MaxValue.value.usVal = 100;

    highlightAmountProperty.ParentProperty = 3;
    highlightAmountProperty.Updater = UpdateMotionHighlightProperty;
}

static XErrorCode UpdateMotionHighlightProperty( PropertyDescriptor* desc, const xvariant* parentValue )
{
    XErrorCode ret = ErrorFailed;
    bool       boolParentValue;

    ret = XVariantToBool( parentValue, &boolParentValue );

    if ( ret == SuccessCode )
    {
        if ( boolParentValue )
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
