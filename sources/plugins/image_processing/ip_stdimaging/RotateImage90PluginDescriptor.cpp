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
#include <image_rotate90_16x16.h>
#include "RotateImage90Plugin.hpp"

static void PluginInitializer( );
static void PluginCleaner( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 0 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000001, 0x0000001D };

// Type property
static PropertyDescriptor angleProperty =
{ XVT_U2, "Angle", "angle", "Rotation angle.", PropertyFlag_SelectionByValue };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &angleProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** RotateImage90Plugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_Transformation,

    PluginType_ImageProcessingFilter,
    PluginVersion,
    "Rotate Image 90",
    "RotateImage90",
    "Rotates image counter clockwise by 90, 180 or 270 degrees.",

    /* Long description */
    "The plug-in performs image rotation by 90, 180 or 270 degrees, which does not "
    "require any interpolation."
    ,
    &image_rotate90_16x16,
    0,
    RotateImage90Plugin,

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
    // Type property
    angleProperty.DefaultValue.type = XVT_U2;
    angleProperty.DefaultValue.value.usVal = 90;

    angleProperty.ChoicesCount = 3;
    angleProperty.Choices = new xvariant[3];

    angleProperty.Choices[0].type = XVT_U2;
    angleProperty.Choices[0].value.usVal = 90;

    angleProperty.Choices[1].type = XVT_U2;
    angleProperty.Choices[1].value.usVal = 180;

    angleProperty.Choices[2].type = XVT_U2;
    angleProperty.Choices[2].value.usVal = 270;
}

// Clean-up plug-in - deallocate strings
static void PluginCleaner( )
{
    delete [] angleProperty.Choices;
}
