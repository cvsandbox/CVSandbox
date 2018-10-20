/*
    Image processing effects plug-ins of Computer Vision Sandbox

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
#include <image_grain_16x16.h>
#include "GrainPlugin.hpp"

static void PluginInitializer( );
static XErrorCode UpdateSeedValueProperty( PropertyDescriptor* desc, const xvariant* parentValue );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000006, 0x00000014 };

// Spacing property
static PropertyDescriptor spacingProperty =
{ XVT_U2, "Spacing", "spacing", "Specifies maximum value for the random spacing between grain lines.", PropertyFlag_None };
// Density property
static PropertyDescriptor densityProperty =
{ XVT_R4, "Density", "density", "Specifies grain density - higher density results brighter grain.", PropertyFlag_None };
// IsVertical property
static PropertyDescriptor isVerticalProperty =
{ XVT_Bool, "IsVertical", "isVertical", "Specifies if grain is vertical or horizontal.", PropertyFlag_None };
// Static Seed property
static PropertyDescriptor staticSeedProperty =
{ XVT_Bool, "Static seed", "staticSeed", "Use static seed for random number generator or not (use different value every time).", PropertyFlag_None };
// Seed Value property
static PropertyDescriptor seedValueProperty =
{ XVT_U2, "Seed value", "seedValue", "Seed value to used for random number generator.", PropertyFlag_Dependent };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &spacingProperty, &densityProperty, &isVerticalProperty, &staticSeedProperty, &seedValueProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** GrainPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_ImageEffect,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Grain",
    "Grain",
    "Adds vertical or horizontal grain to an image.",

    "The image processing plug-in adds vertical or horizontal grain to an image - effect similar to vertical/horizontal scratches on an old video film.",

    &image_grain_16x16,
    0,
    GrainPlugin,

    sizeof( pluginProperties ) / sizeof( PropertyDescriptor* ),
    pluginProperties,
    PluginInitializer,
    0, // no clean-up
    0  // no dynamic properties update
);

// Complete properties description by initializing those parts, which were not 
// initialized during properties array declaration
static void PluginInitializer( )
{
    // spacing
    spacingProperty.DefaultValue.type = XVT_U2;
    spacingProperty.DefaultValue.value.usVal = 50;

    spacingProperty.MinValue.type = XVT_U2;
    spacingProperty.MinValue.value.usVal = 5;

    spacingProperty.MaxValue.type = XVT_U2;
    spacingProperty.MaxValue.value.usVal = 500;

    // density
    densityProperty.DefaultValue.type = XVT_R4;
    densityProperty.DefaultValue.value.fVal = 0.5f;

    densityProperty.MinValue.type = XVT_R4;
    densityProperty.MinValue.value.fVal = 0.0f;

    densityProperty.MaxValue.type = XVT_R4;
    densityProperty.MaxValue.value.fVal = 1.0f;

    // isVertical 
    isVerticalProperty.DefaultValue.type = XVT_Bool;
    isVerticalProperty.DefaultValue.value.boolVal = true;

    // seed value
    seedValueProperty.DefaultValue.type = XVT_U2;
    seedValueProperty.DefaultValue.value.uiVal = 0;

    seedValueProperty.MinValue.type = XVT_U2;
    seedValueProperty.MinValue.value.usVal = 0;

    seedValueProperty.MaxValue.type = XVT_U2;
    seedValueProperty.MaxValue.value.usVal = 65535;

    seedValueProperty.ParentProperty = 3;
    seedValueProperty.Updater = UpdateSeedValueProperty;
}

// Enable/disable Seed Value property depending on Static Seed value
XErrorCode UpdateSeedValueProperty( PropertyDescriptor* desc, const xvariant* parentValue )
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
