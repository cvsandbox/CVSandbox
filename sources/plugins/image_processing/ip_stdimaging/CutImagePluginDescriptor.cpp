/*
    Standard image processing plug-ins of Computer Vision Sandbox

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
#include "CutImagePlugin.hpp"

static void PluginInitializer( );
static void PluginCleaner( );
static XErrorCode UpdateRelativeGapProperty( PropertyDescriptor* desc, const xvariant* parentValue );
static XErrorCode UpdateGapProperty( PropertyDescriptor* desc, const xvariant* parentValue );
static XErrorCode UpdateAbsoluteCoordinateProperty( PropertyDescriptor* desc, const xvariant* parentValue );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x0000003C };

static PropertyDescriptor cutTypeProperty =
{ XVT_U1, "Cut Type", "cutType", "Specifies type of cut to perform.", PropertyFlag_SelectionByIndex };

// Properties for relative gaps from image edges
static PropertyDescriptor relLeftGapProperty =
{ XVT_R4, "Relative Left Gap", "relLeftGap", "Sub-image's gap from the left edge (relative, in [0, 1] range).", PropertyFlag_Dependent };
static PropertyDescriptor relTopGapProperty =
{ XVT_R4, "Relative Top Gap", "relTopGap", "Sub-image's gap from the top edge (relative, in [0, 1] range).", PropertyFlag_Dependent };
static PropertyDescriptor relRightGapProperty =
{ XVT_R4, "Relative Right Gap", "relRightGap", "Sub-image's gap from the right edge (relative, in [0, 1] range).", PropertyFlag_Dependent };
static PropertyDescriptor relBottomGapProperty =
{ XVT_R4, "Relative Bottom Gap", "relBottomGap", "Sub-image's gap from the bottom edge (relative, in [0, 1] range).", PropertyFlag_Dependent };

// Properties for absolute gaps from image edges
static PropertyDescriptor leftGapProperty =
{ XVT_U4, "Left Gap", "leftGap", "Sub-image's gap from the left edge (pixels).", PropertyFlag_Dependent };
static PropertyDescriptor topGapProperty =
{ XVT_U4, "Top Gap", "topGap", "Sub-image's gap from the top edge (pixels).", PropertyFlag_Dependent };
static PropertyDescriptor rightGapProperty =
{ XVT_U4, "Right Gap", "rightGap", "Sub-image's gap from the right edge (pixels).", PropertyFlag_Dependent };
static PropertyDescriptor bottomGapProperty =
{ XVT_U4, "Bottom Gap", "bottomGap", "Sub-image's gap from the bottom edge (pixels).", PropertyFlag_Dependent };

// Properties for absolute coordinates/size of sub-image
static PropertyDescriptor xProperty =
{ XVT_U4, "Y", "x", "X coordinate of sub-image to cut.", PropertyFlag_Dependent };
static PropertyDescriptor yProperty =
{ XVT_U4, "Y", "y", "Y coordinate of sub-image to cut.", PropertyFlag_Dependent };
static PropertyDescriptor widthProperty =
{ XVT_U4, "Width", "width", "Width of sub-image to cut.", PropertyFlag_Dependent };
static PropertyDescriptor heightProperty =
{ XVT_U4, "Height", "height", "Height of sub-image to cut.", PropertyFlag_Dependent };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &cutTypeProperty,
    &relLeftGapProperty, &relTopGapProperty, &relRightGapProperty, &relBottomGapProperty,
    &leftGapProperty, &topGapProperty, &rightGapProperty, &bottomGapProperty,
    &xProperty, &yProperty, &widthProperty, &heightProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** CutImagePlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_Transformation,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Cut Image",
    "CutImage",
    "Cuts sub-image out of the source image.",

    /* Long description */
    "The plug-in cuts rectangular sub-image out of the provided source image. "
    "It allows specifying either absolute coordinates/size of the sub-image or "
    "gaps relative to the edges of the source image."
    ,
    nullptr,
    nullptr,
    CutImagePlugin,

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
    static const char* cutTypes[] = { "Relative edge gaps", "Absolute edge gaps", "Absolute coordinates" };

    InitSelectionProperty( &cutTypeProperty, cutTypes, XARRAY_SIZE( cutTypes ), 0 );

    // set dependencies and default values
    for ( int i = 1; i <= 12; i++ )
    {
        pluginProperties[i]->ParentProperty = 0;
    }
    for ( int i = 1; i <= 4; i++ )
    {
        pluginProperties[i]->Updater = UpdateRelativeGapProperty;

        pluginProperties[i]->DefaultValue.type       = XVT_R4;
        pluginProperties[i]->DefaultValue.value.fVal = 0.1f;

        pluginProperties[i]->MinValue.type       = XVT_R4;
        pluginProperties[i]->MinValue.value.fVal = 0.0f;

        pluginProperties[i]->MaxValue.type       = XVT_R4;
        pluginProperties[i]->MaxValue.value.fVal = 1.0f;
    }
    for ( int i = 5; i <= 8; i++ )
    {
        pluginProperties[i]->Updater = UpdateGapProperty;

        pluginProperties[i]->DefaultValue.type        = XVT_U4;
        pluginProperties[i]->DefaultValue.value.uiVal = 10;

        pluginProperties[i]->MinValue.type        = XVT_U4;
        pluginProperties[i]->MinValue.value.uiVal = 0;
    }
    for ( int i = 9; i <= 12; i++ )
    {
        pluginProperties[i]->Updater = UpdateAbsoluteCoordinateProperty;

        pluginProperties[i]->DefaultValue.type = XVT_U4;
        pluginProperties[i]->MinValue.type     = XVT_U4;

        if ( i <= 10 )
        {
            pluginProperties[i]->DefaultValue.value.uiVal = 0;
            pluginProperties[i]->MinValue.value.uiVal     = 0;
        }
        else
        {
            pluginProperties[i]->DefaultValue.value.uiVal = 200;
            pluginProperties[i]->MinValue.value.uiVal     = 10;
        }
    }
}

// Clean-up plug-in - deallocate strings
static void PluginCleaner( )
{
    CleanSelectionProperty( &cutTypeProperty );
}

// Enable/disable properties for setting relative gaps
XErrorCode UpdateRelativeGapProperty( PropertyDescriptor* desc, const xvariant* parentValue )
{
    XErrorCode ret = ErrorFailed;
    uint8_t    cutType;

    ret = XVariantToUByte( parentValue, &cutType );

    if ( ret == SuccessCode )
    {
        if ( cutType == 0 )
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

// Enable/disable properties for setting pixel gaps
XErrorCode UpdateGapProperty( PropertyDescriptor* desc, const xvariant* parentValue )
{
    XErrorCode ret = ErrorFailed;
    uint8_t    cutType;

    ret = XVariantToUByte( parentValue, &cutType );

    if ( ret == SuccessCode )
    {
        if ( cutType == 1 )
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

// Enable/disable properties for setting absolute coordinates
XErrorCode UpdateAbsoluteCoordinateProperty( PropertyDescriptor* desc, const xvariant* parentValue )
{
    XErrorCode ret = ErrorFailed;
    uint8_t    cutType;

    ret = XVariantToUByte( parentValue, &cutType );

    if ( ret == SuccessCode )
    {
        if ( cutType == 2 )
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
