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
#ifndef CVS_IFAMILY_H
#define CVS_IFAMILY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <xtypes.h>
#include <ximage.h>

extern const xguid PluginFamilyID_Default;
extern const xguid PluginFamilyID_ColorFilter;
extern const xguid PluginFamilyID_Thresholding;
extern const xguid PluginFamilyID_Morphology;
extern const xguid PluginFamilyID_ColorEffect;
extern const xguid PluginFamilyID_Transformation;
extern const xguid PluginFamilyID_ImageEffect;
extern const xguid PluginFamilyID_ImageSmoothing;
extern const xguid PluginFamilyID_EdgeDetector;
extern const xguid PluginFamilyID_TextureEffect;
extern const xguid PluginFamilyID_ColorReduction;
extern const xguid PluginFamilyID_TwoImageFilters;
extern const xguid PluginFamilyID_FrameGenerator;
extern const xguid PluginFamilyID_BlobsProcessing;
extern const xguid PluginFamilyID_Convolution;

extern const xguid PluginFamilyID_VideoSource;
extern const xguid PluginFamilyID_VirtualVideoSource;

extern const xguid PluginFamilyID_ImageGenerator;

extern const xguid PluginFamilyID_VideoProcessing;

extern const xguid PluginFamilyID_Scripting;

// Structure providing description of plug-ins' family
typedef struct _FamilyDescriptor
{
    xguid   ID;
    xstring Name;
    xstring Description;
    ximage* SmallIcon;
    ximage* Icon;
}
FamilyDescriptor;

// Array of common plugin families
extern const FamilyDescriptor* BuiltInPluginFamilites[];
// Number of common plugin families available
extern const int BuiltInPluginFamilitesCount;

#ifdef __cplusplus
}
#endif

#endif // CVS_IFAMILY_H
