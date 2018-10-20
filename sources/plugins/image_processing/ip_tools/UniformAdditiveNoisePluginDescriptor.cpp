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
#include <image_uniform_additive_noise_16x16.h>
#include "UniformAdditiveNoisePlugin.hpp"

static void PluginInitializer( );
static XErrorCode UpdateSeedValueProperty( PropertyDescriptor* desc, const xvariant* parentValue );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000007, 0x00000001 };

// Amplitude property
static PropertyDescriptor amplitudeProperty =
{ XVT_U1, "Noise amplitude", "amplitude", "Added values are uniformly distributed in -/+ amplitude range.", PropertyFlag_None };
// Static Seed property
static PropertyDescriptor staticSeedProperty =
{ XVT_Bool, "Static seed", "staticSeed", "Use static seed for random number generator or not (use different value every time).", PropertyFlag_None };
// Seed Value property
static PropertyDescriptor seedValueProperty =
{ XVT_U4, "Seed value", "seedValue", "Seed value to used for random number generator.", PropertyFlag_Dependent };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &amplitudeProperty, &staticSeedProperty, &seedValueProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** UniformAdditiveNoisePlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_Default,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Uniform Additive Noise",
    "UniformAdditiveNoise",
    "Adds uniformly distributed noise to an image.",

    /* Long description */
    "The plug-in processes an image by adding uniformly distributed noise with the specified <b>amplitude</b>. Each pixel's "
    "value is increased by random value uniformly distributed in the [-amplitude, amplitude] range.<br><br>"

    "If the <b>static seed</b> property is not set (set to False), then random number generator used for noise generation is "
    "initialized with new seed value for every image (the seed value is based on current system's time). Otherwise the "
    "random number generator is initialized with the same configured <b>seed value</b>."
    ,
    &image_uniform_additive_noise_16x16,
    0,
    UniformAdditiveNoisePlugin,

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
    // Amplitude property
    amplitudeProperty.DefaultValue.type = XVT_U1;
    amplitudeProperty.DefaultValue.value.ubVal = 10;

    // Seed Value property
    seedValueProperty.DefaultValue.type = XVT_U4;
    seedValueProperty.DefaultValue.value.uiVal = 10;

    seedValueProperty.ParentProperty = 1;
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
