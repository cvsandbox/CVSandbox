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
#include <image_convolution_16x16.h>
#include "ConvolutionPlugin.hpp"

static void PluginInitializer( );
static void PluginCleaner( );
static XErrorCode UpdateAutoDivisorProperty( PropertyDescriptor* desc, const xvariant* parentValue );
static XErrorCode UpdateDivisorProperty( PropertyDescriptor* desc, const xvariant* parentValue );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x00000031 };

// Kernel property
static PropertyDescriptor kernelProperty =
{ XVT_R4 | XVT_Array2d, "Kernel", "kernel", "Convolution kernel to use.", PropertyFlag_PreferredEditor_ConvolutionKernel };

// Automatic Divisor property
static PropertyDescriptor autoDivisorProperty =
{ XVT_Bool, "Automatic Divisor", "autoDivisor", "Specifies if divisor should be calculated automatically based on kernel value.", PropertyFlag_Dependent };

// Divisor property
static PropertyDescriptor divisorProperty =
{ XVT_R4, "Divisor", "divisor", "The value used to divide weighted sum of pixels.", PropertyFlag_Dependent };

// Offset property
static PropertyDescriptor offsetProperty =
{ XVT_R4, "Offset", "offset", "The value to add after dividing.", PropertyFlag_None };

// Border Mode property
static PropertyDescriptor borderModeProperty =
{ XVT_U1, "Border Mode", "borderMode", "Mode of handling pixels at image borders.", PropertyFlag_SelectionByIndex };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &kernelProperty, &autoDivisorProperty, &divisorProperty, &offsetProperty, &borderModeProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** ConvolutionPlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_Convolution,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Convolution",
    "Convolution",
    "Applies convolution operator using the specified kernel.",

    /* Long description */
    "The plug-in calculates image convolution with the specified kernel - calculates weighted sum of pixels, "
    "divided by <b>divisor</b> and increased by <b>offset</b> value. Unlike the traditional convolution, "
    "this plug-in does not rotate the specified kernel while calculating weighted sum.<br><br>"

    "Suppose we deal with a 3x3 convolution kernel:<br><tt>"
    "C1 C2 C3<br>"
    "C4 C5 C6<br>"
    "C7 C8 C9<br></tt><br>"

    "And we process pixel <b>X</b> and 8 of its neighbours:<br><tt>"
    "I1 I2 I3<br>"
    "I4&nbsp;&nbsp;X I6<br>"
    "I7 I8 I8<br></tt><br>"

    "The result pixel <b>X'</b> in destination image (the one corresponding to pixel <b>X</b> in the source image) is calculated as:<br>"
    "<b>X' = ( I1*C1 + I2*C2 + I3*C3 + I4*C4 + X*C5 + I6*C6 + I7*C7 + I8*C8 + I9*C9 ) / divisor + offset</b>.<br><br>"

    "There are several modes of handling pixels at image borders (those who naturally lack neighbours). In the <b>Extend</b> mode, the "
    "image is extended with same values as at the image edge. So if first image line is processed and neighbours from above are missing, then "
    "the top edge is extended to provide those value. In the <b>Wrap</b> mode, missing neighbours on one edge are substituted by "
    "pixel on the opposite edge - image is wrapped. In the <b>Zeros</b> mode, missing neighbours are set to 0 value. Finally, in the "
    "<b>Crop</b> mode, all pixels which lack neighbours are not process - as the result, the destination image gets smaller in size."
    ,
    &image_convolution_16x16,
    0,
    ConvolutionPlugin,

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
    autoDivisorProperty.DefaultValue.type = XVT_Bool;
    autoDivisorProperty.DefaultValue.value.boolVal = true;
    autoDivisorProperty.ParentProperty = 0;
    autoDivisorProperty.Updater = UpdateAutoDivisorProperty;

    divisorProperty.ParentProperty = 1;
    divisorProperty.Updater = UpdateDivisorProperty;

    offsetProperty.DefaultValue.type = XVT_R4;
    offsetProperty.DefaultValue.value.fVal = 0;

    // Border Mode property
    borderModeProperty.DefaultValue.type = XVT_U1;
    borderModeProperty.DefaultValue.value.ubVal = BHMode_Extend;

    borderModeProperty.MinValue.type = XVT_U1;
    borderModeProperty.MinValue.value.ubVal = 0;

    borderModeProperty.MaxValue.type = XVT_U1;
    borderModeProperty.MaxValue.value.ubVal = 4;

    borderModeProperty.ChoicesCount = 4;
    borderModeProperty.Choices = new xvariant[4];

    borderModeProperty.Choices[BHMode_Extend].type = XVT_String;
    borderModeProperty.Choices[BHMode_Extend].value.strVal = XStringAlloc( "Extend" );

    borderModeProperty.Choices[BHMode_Wrap].type = XVT_String;
    borderModeProperty.Choices[BHMode_Wrap].value.strVal = XStringAlloc( "Wrap" );

    borderModeProperty.Choices[BHMode_Zeros].type = XVT_String;
    borderModeProperty.Choices[BHMode_Zeros].value.strVal = XStringAlloc( "Zeros" );

    borderModeProperty.Choices[BHMode_Crop].type = XVT_String;
    borderModeProperty.Choices[BHMode_Crop].value.strVal = XStringAlloc( "Crop" );
}

// Clean-up plugin - deallocate strings
static void PluginCleaner( )
{
    for ( int i = 0; i < borderModeProperty.ChoicesCount; i++ )
    {
        XVariantClear( &borderModeProperty.Choices[i] );
    }

    delete [] borderModeProperty.Choices;
}

// Update auto divisor property
static XErrorCode UpdateAutoDivisorProperty( PropertyDescriptor* desc, const xvariant* parentValue )
{
    XUNREFERENCED_PARAMETER( desc )
    XUNREFERENCED_PARAMETER( parentValue )
    // do nothing - we just need a chain reaction to refresh divisor
    return SuccessCode;
}

// Update state of divisor property
static XErrorCode UpdateDivisorProperty( PropertyDescriptor* desc, const xvariant* parentValue )
{
    XErrorCode ret = ErrorFailed;
    bool       autoDivisor;

    ret = XVariantToBool( parentValue, &autoDivisor );

    if ( ret == SuccessCode )
    {
        if ( !autoDivisor )
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
