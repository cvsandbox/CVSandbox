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

#include "ifamily.h"
#include <image_default_plugin_16x16.h>
#include <image_color_filter_plugin_16x16.h>
#include <image_thresholding_plugin_16x16.h>
#include <image_morphology_plugin_16x16.h>
#include <image_colorset_16x16.h>
#include <image_transform_16x16.h>
#include <image_image_effects_16x16.h>
#include <image_image_smoothing_plugin_16x16.h>
#include <image_edge_detector_plugin_16x16.h>
#include <image_texture_plugin_16x16.h>
#include <image_camera_16x16.h>
#include <image_image_generator_plugin_16x16.h>
#include <image_color_reduction_16x16.h>
#include <image_two_images_16x16.h>
#include <image_frame_16x16.h>
#include <image_video_processing_16x16.h>
#include <image_scripting_plugin_16x16.h>
#include <image_blobs_processing_16x16.h>
#include <image_convolution_16x16.h>

// All defines stay here in this source file only and don't go to the outside world
#define FamilyID_Default            { 0xAF000002, 0x00000000, 0x00000001, 0x00000000 }
#define FamilyID_ColorFilter        { 0xAF000002, 0x00000000, 0x00000001, 0x00000001 }
#define FamilyID_Thresholding       { 0xAF000002, 0x00000000, 0x00000001, 0x00000002 }
#define FamilyID_Morphology         { 0xAF000002, 0x00000000, 0x00000001, 0x00000003 }
#define FamilyID_ColorEffect        { 0xAF000002, 0x00000000, 0x00000001, 0x00000004 }
#define FamilyID_Transformation     { 0xAF000002, 0x00000000, 0x00000001, 0x00000005 }
#define FamilyID_ImageEffect        { 0xAF000002, 0x00000000, 0x00000001, 0x00000006 }
#define FamilyID_ImageSmoothing     { 0xAF000002, 0x00000000, 0x00000001, 0x00000007 }
#define FamilyID_EdgeDetector       { 0xAF000002, 0x00000000, 0x00000001, 0x00000008 }
#define FamilyID_TextureEffect      { 0xAF000002, 0x00000000, 0x00000001, 0x00000009 }
#define FamilyID_ColorReduction     { 0xAF000002, 0x00000000, 0x00000001, 0x0000000A }
#define FamilyID_TwoImageFilters    { 0xAF000002, 0x00000000, 0x00000001, 0x0000000B }
#define FamilyID_FrameGenerator     { 0xAF000002, 0x00000000, 0x00000001, 0x0000000C }
#define FamilyID_BlobsProcessing    { 0xAF000002, 0x00000000, 0x00000001, 0x0000000D }
#define FamilyID_Convolution        { 0xAF000002, 0x00000000, 0x00000001, 0x0000000E }

#define FamilyID_VideoSource        { 0xAF000002, 0x00000000, 0x00000008, 0x00000001 }
#define FamilyID_VirtualVideoSource { 0xAF000002, 0x00000000, 0x00000008, 0x00000002 }

#define FamilyID_ImageGenerator     { 0xAF000002, 0x00000000, 0x00000010, 0x00000001 }

#define FamilyID_VideoProcessing    { 0xAF000002, 0x00000000, 0x00000080, 0x00000001 }

#define FamilyID_Scripting          { 0xAF000002, 0x00000000, 0x00000100, 0x00000001 }

#define FamilyID_Detection          { 0xAF000002, 0x00000000, 0x00001000, 0x00000001 }

const xguid PluginFamilyID_Default            = FamilyID_Default;
const xguid PluginFamilyID_ColorFilter        = FamilyID_ColorFilter;
const xguid PluginFamilyID_Thresholding       = FamilyID_Thresholding;
const xguid PluginFamilyID_Morphology         = FamilyID_Morphology;
const xguid PluginFamilyID_ColorEffect        = FamilyID_ColorEffect;
const xguid PluginFamilyID_Transformation     = FamilyID_Transformation;
const xguid PluginFamilyID_ImageEffect        = FamilyID_ImageEffect;
const xguid PluginFamilyID_ImageSmoothing     = FamilyID_ImageSmoothing;
const xguid PluginFamilyID_EdgeDetector       = FamilyID_EdgeDetector;
const xguid PluginFamilyID_TextureEffect      = FamilyID_TextureEffect;
const xguid PluginFamilyID_ColorReduction     = FamilyID_ColorReduction;
const xguid PluginFamilyID_TwoImageFilters    = FamilyID_TwoImageFilters;
const xguid PluginFamilyID_FrameGenerator     = FamilyID_FrameGenerator;
const xguid PluginFamilyID_BlobsProcessing    = FamilyID_BlobsProcessing;
const xguid PluginFamilyID_Convolution        = FamilyID_Convolution;

const xguid PluginFamilyID_VideoSource        = FamilyID_VideoSource;
const xguid PluginFamilyID_VirtualVideoSource = FamilyID_VirtualVideoSource;

const xguid PluginFamilyID_ImageGenerator     = FamilyID_ImageGenerator;

const xguid PluginFamilyID_VideoProcessing    = FamilyID_VideoProcessing;

const xguid PluginFamilyID_Scripting          = FamilyID_Scripting;

const xguid PluginFamilyID_Detection          = FamilyID_Detection;

static FamilyDescriptor DefaultFamily =
    { FamilyID_Default, "Other Plug-ins", "Default plug-in family.",
    (ximage*) &image_default_plugin_16x16, 0 };

static FamilyDescriptor ColorFiltersFamily =
    { FamilyID_ColorFilter,"Color Filters", "Image processing filters for recoloring image's pixels (and removing some colors).",
    (ximage*) &image_color_filter_plugin_16x16, 0 };

static FamilyDescriptor ThresholdingFamily =
    { FamilyID_Thresholding, "Thresholding", "Image processing filters for thresholding images.",
    (ximage*) &image_thresholding_plugin_16x16, 0 };

static FamilyDescriptor MorphologyFamily =
    { FamilyID_Morphology, "Morphology", "Image processing filters based on mathematical morphology.",
    (ximage*) &image_morphology_plugin_16x16, 0 };

static FamilyDescriptor ColorEffectFamily =
    { FamilyID_ColorEffect, "Color Effects", "Image processing filters which perform some color effects.",
    (ximage*) &image_colorset_16x16, 0 };

static FamilyDescriptor TransformationFamily =
    { FamilyID_Transformation, "Image Transformation", "Image processing filters which do image transformation.",
    (ximage*) &image_transform_16x16, 0 };

static FamilyDescriptor ImageEffectFamily =
    { FamilyID_ImageEffect, "Image Effects", "Image processing filters which perform some image effects.",
    (ximage*) &image_image_effects_16x16, 0 };

static FamilyDescriptor ImageSmoothingFamily =
    { FamilyID_ImageSmoothing, "Image Smoothing", "Image processing filters which perform smoothing of images.",
    (ximage*) &image_image_smoothing_plugin_16x16, 0 };

static FamilyDescriptor EdgeDetectorFamily =
    { FamilyID_EdgeDetector, "Edge Detectors", "Image processing filters which perform edge detection.",
    (ximage*) &image_edge_detector_plugin_16x16, 0 };

static FamilyDescriptor TextureEffectFamily =
    { FamilyID_TextureEffect, "Texture Effects", "Image processing filters applying different textures to an image.",
    (ximage*) &image_texture_plugin_16x16, 0 };

static FamilyDescriptor ColorReductionFamily =
    { FamilyID_ColorReduction, "Color Reduction", "Image processing filters aimed for color reduction.",
    (ximage*) &image_color_reduction_16x16, 0 };

static FamilyDescriptor TwoImageFiltersFamily =
    { FamilyID_TwoImageFilters, "Two Image Filters", "Image processing routines which operate with two images.",
    (ximage*) &image_two_images_16x16, 0 };

static FamilyDescriptor FrameGeneratorFamily =
    { FamilyID_FrameGenerator, "Frames & Borders", "Image processing routines which add frames/borders to the selected image.",
    (ximage*) &image_frame_16x16, 0 };

static FamilyDescriptor BlobsProcessingFamily =
    { FamilyID_BlobsProcessing, "Blobs Processing", "Image processing tools which perform different operations with blobs.",
    (ximage*) &image_blobs_processing_16x16, 0 };

static FamilyDescriptor ConvolutionFamily =
    { FamilyID_Convolution, "Convolution", "Image convolution plug-ins.",
    (ximage*) &image_convolution_16x16, 0 };

static FamilyDescriptor VideoSourceFamily =
    { FamilyID_VideoSource, "Video Sources", "Video sources continuously providing each video frame's image.",
    (ximage*) &image_camera_16x16, 0 };

static FamilyDescriptor VirtualVideoSourceFamily =
    { FamilyID_VirtualVideoSource, "Virtual Video Sources", "Virtual video sources generating some images for testing purposes.",
    (ximage*) &image_camera_16x16, 0 };

static FamilyDescriptor ImageGeneratorFamily =
    { FamilyID_ImageGenerator, "Image Generators", "Image generation plug-ins.",
    (ximage*) &image_image_generator_plugin_16x16, 0 };

static FamilyDescriptor VideoProcessingFamily =
    { FamilyID_VideoProcessing, "Video Processing", "Plug-ins which perform different video processing routines.",
    (ximage*) &image_video_processing_16x16, 0 };

static FamilyDescriptor ScriptingFamily =
    { FamilyID_Scripting, "Scripting", "Scripting engine plug-ins.",
    (ximage*) &image_scripting_plugin_16x16, 0 };

static FamilyDescriptor DetectionFamily =
    { FamilyID_Detection, "Detection", "Video processing plug-ins aimed for detection of specific objects/events.",
    (ximage*) &image_default_plugin_16x16, 0 };


const FamilyDescriptor* BuiltInPluginFamilites[] =
{
    &DefaultFamily,
    &ColorFiltersFamily,
    &ThresholdingFamily,
    &MorphologyFamily,
    &ColorEffectFamily,
    &TransformationFamily,
    &ImageEffectFamily,
    &ImageSmoothingFamily,
    &EdgeDetectorFamily,
    &TextureEffectFamily,
    &ColorReductionFamily,
    &TwoImageFiltersFamily,
    &FrameGeneratorFamily,
    &BlobsProcessingFamily,
    &ConvolutionFamily,

    &VideoSourceFamily,
    &VirtualVideoSourceFamily,

    &ImageGeneratorFamily,

    &VideoProcessingFamily,

    &ScriptingFamily,

    &DetectionFamily
};

const int BuiltInPluginFamilitesCount = XARRAY_SIZE( BuiltInPluginFamilites );
