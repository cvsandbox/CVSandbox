/*
    Plug-ins' interface library of Computer Vision Sandbox

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

#pragma once
#ifndef CVS_IPROPERTY_H
#define CVS_IPROPERTY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <xtypes.h>

// Flags specifying property some properties' features
enum
{
    // An empty flag
    PropertyFlag_None = 0,

    // A property is read-only
    PropertyFlag_ReadOnly = 0x01,

    // NOTE: The below two flags are not supposed to be used together

    // A property is a selection from some defined values,
    // where selected INDEX is set/get as property value
    PropertyFlag_SelectionByIndex = 0x02,

    // A property is a selection from some defined values,
    // where selected VALUE is set/get as property value
    PropertyFlag_SelectionByValue = 0x04,

    // A dynamic property which keeps changing. Usually it comes from
    // some device/sensor and in many cases it is a read only property.
    PropertyFlag_Dynamic = 0x08,

    // Dependent property, which configuration depends on the value of
    // another property. For example, a property may get enabled/disabled 
    // depending on the value of parent property. A selection property
    // may change the list of possible choices depending on parent's value.
    PropertyFlag_Dependent = 0x10,

    // Flag to specify the property is currently disabled (cannot be modified).
    PropertyFlag_Disabled = 0x20,

    // A property which configuration (min/max/defaul/choices) is determined at run time.
    // For example, a property which lists available devices in the system or
    // properties of a selected device - things which are not known before running.
    // This property is not device specific. It is more like system specific. Like
    // what are the connected devices.
    PropertyFlag_RuntimeConfiguration = 0x40,

    // Similar to the above, but requires a running/connected device. This property is
    // device specific and so its configuration/value must be queried from a running
    // device (video source and device plug-ins for now)
    PropertyFlag_DeviceRuntimeConfiguration = 0x80,

    // A property, which is not visible in configuration/help for whatever reason.
    // It can be something undocumented, which can be set only from script if known.
    // Or, it can be a property, which is not supported by particular device. For
    // example, device runtime configuration property can be set to hidden, if it is
    // not supported.
    PropertyFlag_Hidden = 0x100,

    // ==================================
    // 12 most significant bits of the flags are used to specify preferred editor for
    // a property (if the system supports)
    // ----------------------------------

    // An editor for a string property which represents folder path
    PropertyFlag_PreferredEditor_FolderBrowser = 0x100000,

    // An editor for a string property which represents script file name
    PropertyFlag_PreferredEditor_ScriptFile = 0x200000,

    // An editor for a string property which represents a file name (generic file type)
    PropertyFlag_PreferredEditor_FileBrowser = 0x300000,

    // An editor for entering hue value
    PropertyFlag_PreferredEditor_HuePicker = 0x400000,

    // An editor for hue range value
    PropertyFlag_PreferredEditor_HueRangePicker = 0x500000,

    // An editor for structuring elements used by morphology operators
    PropertyFlag_PreferredEditor_MorphologyStructuringElement = 0x600000,

    // An editor for structuring elements used by hit-and-miss morphology operator
    PropertyFlag_PreferredEditor_HitAndMissStructuringElement = 0x700000,

    // An editor for kernels used by morphology convolution
    PropertyFlag_PreferredEditor_ConvolutionKernel = 0x800000,

    // Mask to use for exacting property editor from property flags
    PropertyFlag_PreferredEditor_Mask = 0xFFF00000
};
typedef uint32_t PropertyFlags;

struct _PropertyDescriptor;

// Function to update dependent property based on its parent's property value
typedef XErrorCode ( *DependentPropertyDescriptorUpdater )( struct _PropertyDescriptor* desc, const xvariant* parentValue );

// Structure providing description of a plug-in's property
typedef struct _PropertyDescriptor
{
    XVarType      Type;
    xstring       Name;
    xstring       ShortName;
    xstring       Description;
    PropertyFlags Flags;
    xvariant      DefaultValue;
    xvariant      MinValue;
    xvariant      MaxValue;

    int16_t       ChoicesCount;
    xvariant*     Choices;

    int16_t       ParentProperty;
    DependentPropertyDescriptorUpdater Updater;
}
PropertyDescriptor;

// Create a copy of the specified property descriptor
PropertyDescriptor* CopyPropertyDescriptor( const PropertyDescriptor* src );
// Free memory taken by the specified property descriptor
void FreePropertyDescriptor( PropertyDescriptor** pDescriptor );
// Free memory taken by choices only
void FreePropertyChoices( PropertyDescriptor* pDescriptor );

// Helper function to convert input value of a property to the required type
XErrorCode PropertyChangeTypeHelper( int32_t id, const xvariant* value, const PropertyDescriptor** descriptors,
                                     int32_t count, xvariant* convertedValue );
XErrorCode PropertyChangeTypeIndexedHelper( int32_t id, const xvariant* value, const PropertyDescriptor** descriptors,
                                            int32_t count, xvariant* convertedValue );

// Helper function to initialize/clean selection by value property with string value
XErrorCode InitSelectionProperty( PropertyDescriptor* descriptor, const char** values, uint8_t count, uint8_t defaultValue );
void CleanSelectionProperty( PropertyDescriptor* descriptor );

#ifdef __cplusplus
}
#endif

#endif // CVS_IPROPERTY_H
