/*
    Lua Scripting Engine plug-in of Computer Vision Sandbox

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
#include "LuaScriptingEnginePlugin.hpp"
#include <image_lua_script_16x16.h>

static void PluginInitializer( );

// Version of the plug-in
static xversion PluginVersion = { 1, 0, 7 };

// ID of the plug-in
static xguid PluginID = { 0xAF000003, 0x00000000, 0x0000000D, 0x00000001 };

// Script File property
static PropertyDescriptor scriptFileProperty =
{ XVT_String, "Script file", "scriptFile", "Lua script file to run.", PropertyFlag_PreferredEditor_ScriptFile };

// GC Period property
static PropertyDescriptor gcPeriodProperty =
{ XVT_U4, "GC Period", "gcPeriod", "Sets how often to force GC to run.", PropertyFlag_None };

// Array of available properties
static PropertyDescriptor* pluginProperties[] =
{
    &scriptFileProperty, &gcPeriodProperty
};

// Let the class itself know description of its properties
const PropertyDescriptor** LuaScriptingEnginePlugin::propertiesDescription = (const PropertyDescriptor**) pluginProperties;

// Register the plug-in
REGISTER_CPP_PLUGIN_WITH_PROPS
(
    PluginID,
    PluginFamilyID_Scripting,

    PluginType_ScriptingEngine,
    PluginVersion,
    "Lua Scripting",
    "LuaScriptingEngine",
    "Plug-in to run Lua scripts interacting with the application.",

    "The plug-in allows running Lua scripts interacting with a host application. Full documentation of the Lua scripting API "
    "is available <a href='http://www.cvsandbox.com/cvsandbox/tutorials/scripting/lua_scripting_api.html'>on-line</a>, "
    "while information below provides a brief overview of the available API.<br><br>"

    "The <b>GC Period</b> property controls how often to force Lua garbage collection run. If the property is to 0, then the "
    "scripting engine does not do anything about garbage collection leaving it to Lua library to control. If it is not zero, "
    "then the engine will run garbage collection after every N<sup>th</sup> run of script's Main function, where N is the GC "
    "period value.<br><br>"

    "<h3>Lua script structure</h3>"
    "Lua scripts executed by Lua Scripting plug-in consist of two main parts - global initialization part and a <b>Main</b> function. "
    "Initialization part is called once usually and is aimed to perform whatever initialization required for a script to run further. "
    "If it is required to create instances of plug-ins to be used later or perform other time consuming job, then initialization part "
    "is the right place to do it. The <b>Main</b> function is something, which is usually called repeatedly and performs the main job "
    "a particular script is designed for.<br><br>"

    "<h3>Host interface</h3>"
    "The Host interface of the Lua scripting API allows to interact with a host application. Below is the list of methods exposed by "
    "the interface:"
    "<ul>"
    "<li>string <b>Host.Name</b>( ) - Provides name of the host application running the scripting engine plug-in.</b>"
    "<li>string <b>Host.Version</b>( ) - Provides version of the host application.</b>"
    "<li>pluginObject <b>Host.CreatePluginInstance</b>( pluginName:string ) - Creates instance of the specified plug-in and returns its "
    "instance on success.</li>"
    "<li>imageObject <b>Host.GetImage</b>( ) - Gets current image from the host to process, if it is available.</li>"
    "<li><b>Host.SetImage</b>( image:imageObject ) - Sets image back to host for further processing in a graph or displaying.</li>"
    "<li><b>Host.SetVariable</b>( name:string, var:variant ) - Stores variable on the host side. The allowed variables' types are all of "
    "the supported property types (see below), i.e. Lua scalar types and arrays, as well as image objects. This and <b>GetVariable</b>() "
    "methods allow several running scripts to exchange variables between each other. <b>Note</b>: setting variable to <b>Nil</b> removes "
    "it from host.</li>"
    "<li>variant <b>Host.GetVariable</b>( name:string ) - Gets variable stored on the host side.</li>"
    "<li>pluginObject <b>Host.GetVideoSource</b>( ) - Gets video source object for which the script is running for. Allows to get access "
    "to video source's run time properties, if any.</li>"
    "</ul><br>"

    "<h3>Image class interface</h3>"
    "Below is the list of methods exposed by the image class interface:"
    "<ul>"
    "<li>imageObject <b>Image.Create</b>( width:number, height:number, pixelFormat:string ) - Creates an image of the specified size and format.</li>"
    "<li><b>imageObject:Release</b>( ) - Release reference to the image. The object can not be used after that.</li>"
    "<li>number <b>imageObject:Width</b>( ) - Returns width of the image object.</li>"
    "<li>number <b>imageObject:Height</b>( ) - Returns height of the image object.</li>"
    "<li>number <b>imageObject:BitsPerPixel</b>( ) - Returns number of bits used to encode single pixel of the image object.</li>"
    "<li>string <b>imageObject:PixelFormat</b>( ) - Returns string name of the image's pixel format. Possible values are: 'Gray8', "
    "'RGB24', 'RGBA32', 'Gray16', 'RGB48', 'RGBA64', 'Binary1', 'Indexed1', 'Indexed2', 'Indexed4', 'Indexed8'.</li>"
    "<li>imageObject <b>imageObject:Clone</b>( ) - Creates a copy of the image by cloning it.</li>"
    "<li>imageObject <b>imageObject:GetSubImage</b>( x:number, y:number, width:number, height:number ) - Returns sub-image of the "
    "image. The sub-image starts at the specified X/Y coordinates of the source image and has the specified size. The sub-image "
    "must be completely within the source image's boundaries.</li>"
    "<li><b>imageObject:PutImage</b>( imageToPut:imageObject, x:number, y:number ) - Puts the specified image into the source image "
    "at the specified location. The coordinates can be out of the source image's boundaries. The method will put only the region "
    "which overlaps with the source image.</li>"
    "<li>color <b>imageObject:GetPixel</b>( x:number, y:number ) - Gets pixel color at the specified X/Y coordinates. Returns table of RGBA values.</li>"
    "<li><b>imageObject:SetPixel</b>( x:number, y:number, c:color ) - Sets pixel color at the specified X/Y coordinates.</li>"
    "</ul><br>"

    "<h3>Plug-in base class interface</h3>"
    "Instances of plug-ins' objects are created using <b>Host.CreatePluginInstance</b>() API. Every plug-in type inherits from a base "
    "plug-in class, which provides common interface of all plug-ins available in the system."
    "<ul>"
    "<li><b>pluginObject:Release</b>( ) - Release reference to the plug-in. The object can not be used after that.</li>"
    "<li>string <b>pluginObject:Name</b>( ) - Returns name of the plug-in as seen in user interface.</li>"
    "<li>string <b>pluginObject:ShortName</b>( ) - Returns scripting name of the plug-in which is used to create an instance of the plug-in.</li>"
    "<li>string <b>pluginObject:Version</b>( ) - Returns version of the plug-in.</li>"
    "<li>string <b>pluginObject:Type</b>( ) - Returns plug-in's type. Possible values are: 'ImageProcessingFilter', 'VideoProcessing', 'ImageImporter', 'ImageExporter', etc. "
    "(to be documented once the API is officially released).</li>"
    "<li>variant <b>pluginObject:GetProperty</b>( propertyName:string ) - Gets value of the the specified property. Type of the value depends "
    "on the property type.</li>"
    "<li><b>pluginObject:SetProperty</b>( propertyName:string, value:variant ) - Sets value of the specified plug-in's property. "
    "Type of the value must correspond to the property type.</li>"
    "<li>variant <b>pluginObject:GetProperty</b>( propertyName:string, index:integer ) - Gets individual value of an array's type property. "
    "For plug-ins, which have properties of array types, this method allows to retrieve individual elements of the array instead of entire "
    "array. Indexing starts with 1.</li>"
    "<li><b>pluginObject:SetProperty</b>( propertyName:string, index:integer, value:variant ) - Sets individual element of an array type property. "
    "Type of the value must correspond to the array type of the property. Indexing starts with 1.</li>"
    "<li>[variant] <b>pluginObject:CallFunction</b>( functionName:string [, ... variant args ] ) - Calls a function exposed by the plug-in. "
    "Returns <b>nil</b>, if the invoked function returns nothing.</li>"
    "</ul><br>"

    "<h3>Plug-in property types to Lua types mapping</h3>"
    "Below is the table, which shows mapping between plug-ins' properties types and Lua types: "
    "<table border='0' cellpadding='5' width='100%'>"
    "<tr>"
    "<th width='200'>Plug-in property type</th>"
    "<th>Lua type</th>"
    "</tr>"
    "<tr class='tro'>"
    "<td>Boolean</td>"
    "<td>boolean</td>"
    "</tr>"
    "<tr class='tre'>"
    "<td>String</td>"
    "<td>string</td>"
    "</tr>"
    "<tr class='tro'>"
    "<td>Integer</td>"
    "<td>integer number</td>"
    "</tr>"
    "<tr class='tre'>"
    "<td>Real Number (float)</td>"
    "<td>real  number</td>"
    "</tr>"
    "<tr class='tro'>"
    "<td>Range</td>"
    "<td>table<br><br>"
    "Represented as a table in Lua, which has two values : table[1] - range's minimum value; and "
    "table[2] - range's maxim value. Both min/max values are integers."
    "</td>"
    "</tr>"
    "<tr class='tre'>"
    "<td>Range (real numbers)</td>"
    "<td>table<br><br>"
    "Represented as a table in Lua, which has two values : table[1] - range's minimum value; and "
    "table[2] - range's maxim value. Both min/max value are real numbers."
    "</td>"
    "</tr>"
    "<tr class='tro'>"
    "<td>Color</td>"
    "<td>table or string<br><br>"
    "When getting a Color property, it is always returned as a table containing 4 RGBA values : table[1] - Red, "
    "table[2] - Green, table[3] - Blue and table[4] - Alpha. When setting a Color property it can be set "
    "either as table or string. If setting as table, it can have 3 or 4 values - alpha value is optional and defaults "
    "to 255. If setting as string, then the string format is either 'RRGGBB' or 'AARRGGBB', where color values "
    "are specified as HEX values."
    "</td>"
    "</tr>"
    "<tr class='tre'>"
    "<td>Point</td>"
    "<td>Represented as a table in Lua, which has two values: table[1] - point's X value; and "
    "table[2] - range's Y value. Both X/Y values are integers.</td>"
    "</tr>"
    "</tr>"
    "<tr class='tro'>"
    "<td>Point (real numbers)</td>"
    "<td>Represented as a table in Lua, which has two values: table[1] - point's X value; and "
    "table[2] - range's Y value. Both X/Y values are real numbers.</td>"
    "</tr>"
    "</tr>"
    "<tr class='tre'>"
    "<td>Size</td>"
    "<td>Represented as a table in Lua, which has two values: table[1] - size's width value; and "
    "table[2] - size's height value.</td>"
    "</tr>"
    "</tr>"
    "<tr class='tro'>"
    "<td>Array</td>"
    "<td>table, with indexes starting from 1.</td>"
    "</tr>"
    "<tr class='tre'>"
    "<td>2D Array</td>"
    "<td>table of tables, where all sub-tables must be of the same length.</td>"
    "</tr>"
    "</table><br>"

    "<h3>Image processing filter plug-in interface</h3>"
    "Below are the methods exposed by image processing filter plug-ins:"
    "<ul>"
    "<li>boolean <b>pluginObject:CanProcessInPlace</b>( ) - Returns true if the plug-in can do in-place image processing or false otherwise.</li>"
    "<li>boolean <b>pluginObject:IsPixelFormatSupported</b>( inputFormatName : string ) - Returns true if the plug-in accepts images of the "
    "specified pixel format or false otherwise.</li>"
    "<li>string <b>pluginObject:GetOutputPixelFormat</b>( inputFormatName:string ) - Returns pixel format name of an output image when plug-in "
    "is given an input image of the specified format.</li>"
    "<li>imageObject <b>pluginObject:ProcessImage</b>( inputImage:imageObject ) - Performs processing of the input image and returns new image "
    "as a result. The input image is left unchanged.</li>"
    "<b>pluginObject:ProcessImageInPlace</b>( inputImage:imageObject ) - Performs processing of the specified image if the plug-in supports "
    "in-place image processing. The method fails if the plug-in does not support it.</li>"
    "</ul><br>"

    "<h3>Image processing 2 source filter plug-in interface</h3>"
    "Below are the methods exposed by image processing 2 source filter plug-ins:"
    "<ul>"
    "<li>boolean <b>pluginObject:CanProcessInPlace</b>( ) - Returns true if the plug-in can do in-place image processing or false otherwise.</li>"
    "<li>boolean <b>pluginObject:IsPixelFormatSupported</b>( inputFormatName : string ) - Returns true if the plug-in accepts images of the "
    "specified pixel format or false otherwise.</li>"
    "<li>string <b>pluginObject:GetOutputPixelFormat</b>( inputFormatName:string ) - Returns pixel format name of an output image when plug-in "
    "is given an input image of the specified format.</li>"
    "<li>string <b>pluginObject:GetSecondImageSupportedSize</b>( ) - Returns supported size of a second image. Possible values are: "
    "'Any', 'Equal', 'Equal or Bigger' or 'Equal or Smaller'.</li>"
    "<li>string <b>pluginObject:GetSecondImageSupportedFormat</b>( inputFormatName:string ) - Returns supported pixel format name of the second "
    "image for the given format name of the source image.</li>"
    "<li>imageObject <b>pluginObject:ProcessImage</b>( inputImage:imageObject, inputImage2:imageObject ) - Performs processing of the input "
    "image and returns new image as a result. The input image is left unchanged.</li>"
    "<b>pluginObject:ProcessImageInPlace</b>( inputImage:imageObject, inputImage2:imageObject ) - Performs processing of the specified image "
    "if the plug-in supports in-place image processing. The method fails if the plug-in does not support it.</li>"
    "</ul><br>"

    "<h3>Image processing plug-in interface</h3>"
    "Below are the methods exposed by image processing plug-ins:"
    "<ul>"
    "<li>boolean <b>pluginObject:IsPixelFormatSupported</b>( inputFormatName : string ) - Returns true if the plug-in accepts images of the "
    "specified pixel format or false otherwise.</li>"
    "<li><b>pluginObject:ProcessImage</b>( inputImage:imageObject ) - Performs processing of the input image and updates its read-only "
    "properties with new values based on the performed processing.</li>"
    "</ul><br>"

    "<h3>Video processing plug-in interface</h3>"
    "Below are the methods exposed by video processing plug-ins:"
    "<ul>"
    "<li>boolean <b>pluginObject:IsReadOnlyMode</b>( ) - Returns true if the plug-in does not do any changes to images while processing "
    "them or false otherwise.</li>"
    "<li>boolean <b>pluginObject:IsPixelFormatSupported</b>( inputFormatName:string ) - Returns true if the plug-in accepts images of "
    "the specified pixel format or false otherwise.</li>"
    "<li><b>pluginObject:ProcessImage</b>( inputImage:imageObject ) - Performs processing of the input image. The input image may or "
    "may not change depending on what the plug-in does and its current mode.</li>"
    "<li><b>pluginObject:Reset</b>( ) - Resets internal run time state of the plug-in bringing it back to the state as if it was just instantiated</li>"
    "</ul><br>"

    "<h3>Image importing plug-in interface</h3>"
    "Image importing plug-ins allow loading images from different file formats. Below is the interface provided by these plug-ins:"
    "<ul>"
    "<li>array <b>pluginObject:SupportedExtensions</b>( ) - Returns an array of supported files' extensions.</li>"
    "<li>imageObject <b>pluginObject:ImportImage</b>( fileName:string ) - Loads image from the specified file and returns it as an image object.</li>"
    "</ul><br>"

    "<h3>Image exporting plug-in interface</h3>"
    "Image exporting plug-ins allow saving images into different file formats. Below is the interface provided by these plug-ins:"
    "<ul>"
    "<li>array <b>pluginObject:SupportedExtensions</b>( ) - Returns an array of supported files' extensions.</li>"
    "<li>array <b>pluginObject:SupportedPixelFormats</b>( ) - Returns an array of pixel formats supported by the image encoder.</li>"
    "<li><b>pluginObject:ExportImage</b>( fileName:string, imageToSave:imageObject ) - Saves image into the specified file.</li>"
    "</ul><br>"

    "<h3>Scripting API plug-in interface</h3>"
    "The scripting API plug-ins don't provide any extra methods on top of the base plug-ins' interface. This type of plug-ins was "
    "created only to highlight the primary intention of the plug-ins, i.e. to provide APIs to be used from scripting. The main and "
    "the only purpose of these plug-ins is to expose some functions to be called from scripting by using <b>plugin:CallFunction</b>() "
    "method.<br><br><br>"

    "More information: <a href='http://www.cvsandbox.com/cvsandbox/tutorials/scripting/lua_scripting_api.html'>Lua Scripting API</a>"
    ,
    &image_lua_script_16x16,    // small icon
    nullptr,
    LuaScriptingEnginePlugin,

    sizeof( pluginProperties ) / sizeof( PropertyDescriptor* ),
    pluginProperties,
    PluginInitializer,
    nullptr, // no clean-up
    nullptr  // no dynamic properties update
);

// Complete properties description by initializing those parts, which were not
// initialized during properties array declaration
static void PluginInitializer( )
{
    gcPeriodProperty.DefaultValue.type = XVT_U4;
    gcPeriodProperty.DefaultValue.value.uiVal = 1;

    gcPeriodProperty.MinValue.type = XVT_U4;
    gcPeriodProperty.MinValue.value.uiVal = 0;

    gcPeriodProperty.MaxValue.type = XVT_U4;
    gcPeriodProperty.MaxValue.value.uiVal = 100000;
}
