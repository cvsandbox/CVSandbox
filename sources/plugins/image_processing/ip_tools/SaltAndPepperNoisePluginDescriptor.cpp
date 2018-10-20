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
#include <image_salt_and_pepper_noise_16x16.h>
#include "SaltAndPepperNoisePlugin.hpp"

static void PluginInitializer( );
static XErrorCode UpdateSeedValueProperty( PropertyDescriptor* desc, const xvariant* parentValue );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000007, 0x00000002 };

// Noise Amount property
static PropertyDescriptor noiseAmountProperty =
{ XVT_R4, "Noise amount", "noiseAmount", "Number of pixels to change randomly to salt or pepper (%).", PropertyFlag_None };
// Pepper Value property
static PropertyDescriptor pepperValueProperty =
{ XVT_U1, "Pepper value", "pepperValue", "Pixel value to set for pepper noise.", PropertyFlag_None };
// Salt Value property
static PropertyDescriptor saltValueProperty =
{ XVT_U1, "Salt value", "saltValue", "Pixel value to set for salt noise.", PropertyFlag_None };
// Static Seed property
static PropertyDescriptor staticSeedProperty =
{ XVT_Bool, "Static seed", "staticSeed", "Use static seed for random number generator or not (use different value every time).", PropertyFlag_None };
// Seed Value property
static PropertyDescriptor seedValueProperty =
{ XVT_U4, "Seed value", "seedValue", "Seed value to used for random number generator.", PropertyFlag_Dependent };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &noiseAmountProperty, &pepperValueProperty, &saltValueProperty, &staticSeedProperty, &seedValueProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** SaltAndPepperNoisePlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_Default,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Salt And Pepper Noise",
    "SaltAndPepperNoise",
    "Adds salt and pepper noise to an image.",

    /* Long description */
    "The plug-in processes an image by adding random salt or pepper noise - certain percentage of image's pixels "
    "are set either to \"<b>salt</b>\" value or \"<b>pepper</b>\" value.<br><br>"

    "If the <b>static seed</b> property is not set (set to False), then random number generator used for noise generation is "
    "initialized with new seed value for every image (the seed value is based on current system's time). Otherwise the "
    "random number generator is initialized with the same configured <b>seed value</b>."
    ,
    &image_salt_and_pepper_noise_16x16,
    0,
    SaltAndPepperNoisePlugin,

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
    // Noise Amount property
    noiseAmountProperty.DefaultValue.type = XVT_R4;
    noiseAmountProperty.DefaultValue.value.fVal = 10.0f;

    noiseAmountProperty.MinValue.type = XVT_R4;
    noiseAmountProperty.MinValue.value.fVal = 0.0f;

    noiseAmountProperty.MaxValue.type = XVT_R4;
    noiseAmountProperty.MaxValue.value.fVal = 100.0f;

    // Pepper Value property
    pepperValueProperty.DefaultValue.type = XVT_U1;
    pepperValueProperty.DefaultValue.value.ubVal = 0;

    // Salt Value property
    saltValueProperty.DefaultValue.type = XVT_U1;
    saltValueProperty.DefaultValue.value.ubVal = 255;

    // Seed Value property
    seedValueProperty.DefaultValue.type = XVT_U4;
    seedValueProperty.DefaultValue.value.uiVal = 10;

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
