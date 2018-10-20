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
#include <image_pencil_16x16.h>
#include "ImageDrawingPlugin.hpp"

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 1 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x00000007, 0x0000000D };

// FUNCTIONS ------------------------------------------------------------------------------------------------------

// Comon arguments
static ArgumentDescriptor imageArg      = { XVT_Image, "image",  "Image to draw on" };
static ArgumentDescriptor rectPoint1Arg = { XVT_Point, "point1", "Coordinates of rectangle's top-left corner" };
static ArgumentDescriptor rectPoint2Arg = { XVT_Point, "point2", "Coordinates of rectangle's bottom-right corner" };
static ArgumentDescriptor penColorArg   = { XVT_ARGB,  "color",  "Pen color to draw with" };
static ArgumentDescriptor brushColorArg = { XVT_ARGB,  "color",  "Brush color to fill with" };

// Draw Rectangle
static ArgumentDescriptor* drawRectangleArgs[]   = { &imageArg, &rectPoint1Arg, &rectPoint2Arg, &penColorArg };
static FunctionDescriptor  drawRectangleFunction =
{ XVT_Empty, "DrawRectangle", "Draw rectangle on the specified image.", XARRAY_SIZE( drawRectangleArgs ), drawRectangleArgs };

// Fill Rectangle
static ArgumentDescriptor* fillRectangleArgs[]   = { &imageArg, &rectPoint1Arg, &rectPoint2Arg, &brushColorArg };
static FunctionDescriptor  fillRectangleFunction =
{ XVT_Empty, "FillRectangle", "Fill rectangle in the specified image.", XARRAY_SIZE( fillRectangleArgs ), fillRectangleArgs };

// Fill Frame
static ArgumentDescriptor frameXthicknessArg = { XVT_U2, "xThickness", "Thickness of the frame in X direction" };
static ArgumentDescriptor frameYthicknessArg = { XVT_U2, "yThickness", "Thickness of the frame in Y direction" };

static ArgumentDescriptor* fillFrameArgs[] = { &imageArg, &rectPoint1Arg, &rectPoint2Arg, &frameXthicknessArg, &frameYthicknessArg, &brushColorArg };
static FunctionDescriptor  fillFrameFunction =
{ XVT_Empty, "FillFrame", "Fill frame in the specified image.", XARRAY_SIZE( fillFrameArgs ), fillFrameArgs };

// Draw Text
static ArgumentDescriptor textStringArg     = { XVT_String, "text",    "Text line to draw" };
static ArgumentDescriptor textPointArg      = { XVT_Point,  "point",   "Coordinates of the text line to draw" };
static ArgumentDescriptor textBackgroundArg = { XVT_ARGB,   "bgColor", "Background color to fill with" };

static ArgumentDescriptor* drawTextArgs[]   = { &imageArg, &textStringArg, &textPointArg, &penColorArg, &textBackgroundArg };
static FunctionDescriptor  drawTextFunction =
{ XVT_Empty, "DrawText", "Draw text line on the specified image.", XARRAY_SIZE( drawTextArgs ), drawTextArgs };

// Draw Line
static ArgumentDescriptor linePoint1Arg = { XVT_Point, "point1", "Coordinates of line's start point" };
static ArgumentDescriptor linePoint2Arg = { XVT_Point, "point2", "Coordinates of line's end point" };

static ArgumentDescriptor* drawLineArgs[]   = { &imageArg, &linePoint1Arg, &linePoint2Arg, &penColorArg };
static FunctionDescriptor  drawLineFunction =
{ XVT_Empty, "DrawLine", "Draw line on the specified image.", XARRAY_SIZE( drawLineArgs ), drawLineArgs };

// Draw Circle
static ArgumentDescriptor circleCenterArg = { XVT_Point, "center", "Coordinates of circle's center point" };
static ArgumentDescriptor circleRadiusArg = { XVT_U2,    "radius", "Circle's radius" };

static ArgumentDescriptor* drawCircleArgs[] = { &imageArg, &circleCenterArg, &circleRadiusArg, &penColorArg };
static FunctionDescriptor  drawCircleFunction =
{ XVT_Empty, "DrawCircle", "Draw circle on the specified image.", XARRAY_SIZE( drawCircleArgs ), drawCircleArgs };

// Draw Ellipse
static ArgumentDescriptor ellipseCenterArg  = { XVT_Point, "center", "Coordinates of ellipse's center point" };
static ArgumentDescriptor ellipseXradiusArg = { XVT_U2, "rx", "Ellipse's radius in X direction" };
static ArgumentDescriptor ellipseYradiusArg = { XVT_U2, "ry", "Ellipse's radius in Y direction" };

static ArgumentDescriptor* drawEllipseArgs[]   = { &imageArg, &ellipseCenterArg, &ellipseXradiusArg, &ellipseYradiusArg, &penColorArg };
static FunctionDescriptor  drawEllipseFunction =
{ XVT_Empty, "DrawEllipse", "Draw ellipse on the specified image.", XARRAY_SIZE( drawEllipseArgs ), drawEllipseArgs };

// Draw Polyline
static ArgumentDescriptor polylinePointsArg = { XVT_Array | XVT_Point, "points", "Coordinates of points to connect with line" };

static ArgumentDescriptor* drawPolylineArgs[]   = { &imageArg, &polylinePointsArg, &penColorArg };
static FunctionDescriptor  drawPolylineFunction =
{ XVT_Empty, "DrawPolyline", "Draw polyline on the specified image (connects specified points).", XARRAY_SIZE( drawPolylineArgs ), drawPolylineArgs };

// Draw Polygon
static ArgumentDescriptor polygonPointsArg = { XVT_Array | XVT_Point, "points", "Coordinates of polygon's vertices" };

static ArgumentDescriptor* drawPolygonArgs[]   = { &imageArg, &polygonPointsArg, &penColorArg };
static FunctionDescriptor  drawPolygonFunction =
{ XVT_Empty, "DrawPolygon", "Draw polygon on the specified image.", XARRAY_SIZE( drawPolygonArgs ), drawPolygonArgs };

// Fill Circle
static ArgumentDescriptor* fillCircleArgs[]   = { &imageArg, &circleCenterArg, &circleRadiusArg, &brushColorArg };
static FunctionDescriptor  fillCircleFunction =
{ XVT_Empty, "FillCircle", "Fill circle in the specified image.", XARRAY_SIZE( fillCircleArgs ), fillCircleArgs };

// Fill Ellipse
static ArgumentDescriptor* fillEllipseArgs[] = { &imageArg, &ellipseCenterArg, &ellipseXradiusArg, &ellipseYradiusArg, &brushColorArg };
static FunctionDescriptor  fillEllipseFunction =
{ XVT_Empty, "FillEllipse", "Fill ellipse in the specified image.", XARRAY_SIZE( fillEllipseArgs ), fillEllipseArgs };

// Fill Ring
static ArgumentDescriptor ringRadius1Arg = { XVT_U2, "radius1", "Outer radius of the circle" };
static ArgumentDescriptor ringRadius2Arg = { XVT_U2, "radius2", "Inner radius of the circle" };

static ArgumentDescriptor* fillRingArgs[]   = { &imageArg, &circleCenterArg, &ringRadius1Arg, &ringRadius2Arg, &brushColorArg };
static FunctionDescriptor  fillRingFunction =
{ XVT_Empty, "FillRing", "Fill ring in the specified image.", XARRAY_SIZE( fillRingArgs ), fillRingArgs };

// Fill Pie
static ArgumentDescriptor pieStartArg = { XVT_R4, "startAngle", "Start angle of the pie, [0, 360)" };
static ArgumentDescriptor pieEndArg   = { XVT_R4, "endAngle", "End angle of the pie, [0, 360)" };

static ArgumentDescriptor* fillPieArgs[]   = { &imageArg, &circleCenterArg, &ringRadius1Arg, &ringRadius2Arg, &pieStartArg, &pieEndArg, &brushColorArg };
static FunctionDescriptor  fillPieFunction =
{ XVT_Empty, "FillPie", "Fill pie in the specified image. Similar to FillRing, but fills it between specified angles.", XARRAY_SIZE( fillPieArgs ), fillPieArgs };

// Fill Convex Polygon
static ArgumentDescriptor* fillConvexPolygonArgs[]   = { &imageArg, &polygonPointsArg, &brushColorArg };
static FunctionDescriptor  fillConvexPolygonFunction =
{ XVT_Empty, "FillConvexPolygon", "Fill convex polygon in the specified image.", XARRAY_SIZE( fillConvexPolygonArgs ), fillConvexPolygonArgs };

// Draw image
static ArgumentDescriptor imageToDrawArg = { XVT_Image, "imageToDraw", "Image to draw on the target image" };
static ArgumentDescriptor imagePointArg  = { XVT_Point, "point", "Coordinates to draw image at" };

static ArgumentDescriptor* drawImageArgs[] = { &imageArg, &imageToDrawArg, &imagePointArg };
static FunctionDescriptor  drawImageFunction =
{ XVT_Empty, "DrawImage", "Draw another image at the specified location. The image to draw does not have to have same pixel format as target. Its format will be converted to the target.",
                          XARRAY_SIZE( drawImageArgs ), drawImageArgs };

// Array of functions
static FunctionDescriptor* pluginFunctions[] =
{
    &drawLineFunction, &drawRectangleFunction, &drawCircleFunction, &drawEllipseFunction, &drawTextFunction, &drawImageFunction,
    &drawPolylineFunction, &drawPolygonFunction,
    &fillRectangleFunction, &fillFrameFunction, &fillCircleFunction, &fillEllipseFunction, &fillRingFunction, &fillPieFunction,
    &fillConvexPolygonFunction
};


// Let the class itself know description of its function
const FunctionDescriptor** ImageDrawingPlugin::functionsDescription = (const FunctionDescriptor**) pluginFunctions;
const int32_t              ImageDrawingPlugin::functionsCount       = XARRAY_SIZE( pluginFunctions );

// ----------------------------------------------------------------------------------------------------------------

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS_AND_FUNCS
(
    PluginID,
    PluginFamilyID_Default,

    PluginType_ScriptingApi,
    PluginVersion,
    "Image Drawing",
    "ImageDrawing",
    "Plug-in providing some functions to draw on images.",

    "The plug-in provides different functions to draw on images - lines, circles and ellipses, rectangles, text, images, etc. "
    "Supported pixel formats of target images to draw on: 8bb grayscale and 24/32 bpp color images.",

    &image_pencil_16x16,
    nullptr,
    ImageDrawingPlugin,

    0,       // no properties
    nullptr,

    nullptr, // no initializer
    nullptr, // no clean-up
    nullptr, // no dynamic properties update
    XARRAY_SIZE( pluginFunctions ),
    pluginFunctions
);
