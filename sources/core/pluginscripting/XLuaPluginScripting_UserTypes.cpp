/*
    Plug-ins' scripting library of Computer Vision Sandbox

    Copyright (C) 2011-2019, cvsandbox
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

#include "XLuaPluginScripting_UserTypes.hpp"
#include <XError.hpp>
#include <XPluginsEngine.hpp>
#include <XImageImportingPlugin.hpp>
#include <XImageExportingPlugin.hpp>
#include <XImageGenerationPlugin.hpp>
#include <XImageProcessingPlugin.hpp>
#include <XImageProcessingFilterPlugin.hpp>
#include <XImageProcessingFilterPlugin2.hpp>
#include <XDevicePlugin.hpp>
#include <XCommunicationDevicePlugin.hpp>
#include <XVideoProcessingPlugin.hpp>
#include <XVariantArray.hpp>

extern "C"
{
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>
}

using namespace std;
using namespace CVSandbox;

// Some string constants
static const char* STR_ERROR_UNKNOWN_PROPERTY     = "Unknown plug-in's property was specified";
static const char* STR_ERROR_PROPERTY_INDEX       = "Index to access indexed properties must be >= 1";
static const char* STR_ERROR_UNKNOWN_PIXEL_FORMAT = "Unknown pixel format is specified";
static const char* STR_ERROR_UNSUPPORTED_IN_PLACE_FILTERING = "The image processing filter plug-in does not support in place filtering";
static const char* STR_ERROR_RELEASED_OBJECT      = "Can not access an object which was already released";
static const char* STR_ERROR_ARRAY_ANY_TYPE       = "Can not get Lua array where each element can be of arbitrary type";

// Names of metatables for Lua user data types
static const char* METATABLE_PLUGIN                          = "CVSandbox.Plugin";
static const char* METATABLE_IMAGE_IMPORTER_PLUGIN           = "CVSandbox.Plugin.ImageImporter";
static const char* METATABLE_IMAGE_EXPORTER_PLUGIN           = "CVSandbox.Plugin.ImageExporter";
static const char* METATABLE_IMAGE_GENERATOR_PLUGIN          = "CVSandbox.Plugin.ImageGenerator";
static const char* METATABLE_IMAGE_PROCESSING_PLUGIN         = "CVSandbox.Plugin.ImageProcessing";
static const char* METATABLE_IMAGE_PROCESSING_FILTER_PLUGIN  = "CVSandbox.Plugin.ImageProcessingFilter";
static const char* METATABLE_IMAGE_PROCESSING_FILTER_PLUGIN2 = "CVSandbox.Plugin.ImageProcessingFilter2";
static const char* METATABLE_DEVICE_PLUGIN                   = "CVSandbox.Plugin.Device";
static const char* METATABLE_COMMUNICATION_DEVICE_PLUGIN     = "CVSandbox.Plugin.CommunicationDevice";
static const char* METATABLE_VIDEO_PROCESSING_PLUGIN         = "CVSandbox.Plugin.VideoProcessing";
static const char* METATABLE_IMAGE                           = "CVSandbox.Image";

static int Plugin_ID( lua_State* luaState );

// Shell class to be used with plug-ins' user data type
class PluginShell
{
public:
    PluginShell( const shared_ptr<const XPluginDescriptor>& descriptor,
                 const shared_ptr<XPlugin>& plugin ) :
        Descriptor( descriptor ), Plugin( plugin )
    {
    }

    void Release( )
    {
        Plugin.reset( );
        Descriptor.reset( );
    }

    bool IsReleased( )
    {
        return ( !Plugin );
    }

    shared_ptr<const XPluginDescriptor> Descriptor;
    shared_ptr<XPlugin>                 Plugin;
};

// Shell class to be used with image user data type
class ImageShell
{
public:
    ImageShell( const std::shared_ptr<XImage>& image ) :
        Image( image )
    {
    }

    void Release( )
    {
        Image.reset( );
    }

    bool IsReleased( )
    {
        return ( !Image );
    }

    std::shared_ptr<XImage> Image;
};

// ===== Internal helper API =====

// Report error through Lua engine
static void ReportError( lua_State* luaState, const char* strErrorMessage )
{
    luaL_error( luaState, strErrorMessage );
}

// Report error for the specified error code
void ReportXError( lua_State* luaState, XErrorCode errorCode )
{
    ReportError( luaState, XError::Description( errorCode ).c_str( ) );
}

// Report argument error through Lua engine
void ReportArgError( lua_State* luaState, int stackIndex, const char* strErrorMessage )
{
    luaL_argerror( luaState, stackIndex, strErrorMessage );
}

// Report argument error for the specified error code
static void ReportArgXError( lua_State* luaState, int stackIndex, XErrorCode errorCode )
{
    ReportArgError( luaState, stackIndex, XError::Description( errorCode ).c_str( ) );
}

// Check if Lua stack contains plug-in object at the specified index
static bool IsPluginOnLuaStack( lua_State* luaState, int stackIndex )
{
    bool foundPlugin = false;

    if ( lua_getmetatable( luaState, stackIndex ) != 0 )
    {
        lua_getfield( luaState, -1, "ID" );
        foundPlugin = ( lua_topointer( luaState, -1 ) == Plugin_ID );
        lua_pop( luaState, 2 );
    }

    return foundPlugin;
}

// Helper function to check C function got expected number of arguments from Lua
void CheckArgumentsCount( lua_State* luaState, int expectedCount )
{
    CheckArgumentsCount( luaState, expectedCount, expectedCount );
}
void CheckArgumentsCount( lua_State* luaState, int expectedMinCount, int expectedMaxCount )
{
    int passedArgumentsCount = lua_gettop( luaState );

    if ( ( passedArgumentsCount < expectedMinCount ) || ( passedArgumentsCount > expectedMaxCount ) )
    {
        lua_Debug debugInfo;

        if ( !lua_getstack( luaState, 0, &debugInfo ) )
        {
            /* no stack frame? */
            luaL_error( luaState, "Invalid arguments count" );
        }

        lua_getinfo( luaState, "n", &debugInfo );

        if ( strcmp( debugInfo.namewhat, "method" ) == 0 )
        {
            if ( expectedMinCount == 0 )
            {
                luaL_error( luaState, "calling '%s' on bad self", debugInfo.name );
            }
            else
            {
                expectedMinCount--;
                expectedMaxCount--;

                if ( expectedMinCount == expectedMaxCount )
                {
                    luaL_error( luaState, "'%s' method expects %d argument%s", debugInfo.name, expectedMinCount, ( expectedMinCount == 1 ) ? "" : "s" );
                }
                else
                {
                    luaL_error( luaState, "'%s' method expects %d to %d arguments", debugInfo.name, expectedMinCount, expectedMaxCount );
                }
            }
        }
        else
        {
            if ( expectedMinCount == expectedMaxCount )
            {
                luaL_error( luaState, "'%s' function expects %d argument%s", debugInfo.name, expectedMinCount, ( expectedMinCount == 1 ) ? "" : "s" );
            }
            else
            {
                luaL_error( luaState, "'%s' function expects %d to %d arguments", debugInfo.name, expectedMinCount, expectedMaxCount );
            }
        }
    }
}

// Convert XRange to Lua's table with 2 values and push it to stack
static void PushXRangeToLuaStack( lua_State* luaState, const XRange& range )
{
    // create table for min/max values
    lua_newtable( luaState );

    lua_pushinteger( luaState, range.Min( ) );
    lua_rawseti( luaState, -2, 1 );

    lua_pushinteger( luaState, range.Max( ) );
    lua_rawseti( luaState, -2, 2 );
}

// Convert XRangeF to Lua's table with 2 values and push it to stack
static void PushXRangeFToLuaStack( lua_State* luaState, const XRangeF& range )
{
    // create table for min/max values
    lua_newtable( luaState );

    lua_pushnumber( luaState, range.Min( ) );
    lua_rawseti( luaState, -2, 1 );

    lua_pushnumber( luaState, range.Max( ) );
    lua_rawseti( luaState, -2, 2 );
}

// Convert XPoint to Lua's table with 2 values and push it to stack
static void PushXPointToLuaStack( lua_State* luaState, const XPoint& point )
{
    // create table for X/Y values
    lua_newtable( luaState );

    lua_pushinteger( luaState, point.X( ) );
    lua_rawseti( luaState, -2, 1 );

    lua_pushinteger( luaState, point.Y( ) );
    lua_rawseti( luaState, -2, 2 );
}

// Convert XPointF to Lua's table with 2 values and push it to stack
static void PushXPointFToLuaStack( lua_State* luaState, const XPointF& point )
{
    // create table for X/Y values
    lua_newtable( luaState );

    lua_pushnumber( luaState, point.X( ) );
    lua_rawseti( luaState, -2, 1 );

    lua_pushnumber( luaState, point.Y( ) );
    lua_rawseti( luaState, -2, 2 );
}

// Convert XSize to Lua's table with 2 values and push it to stack
static void PushXSizeToLuaStack( lua_State* luaState, const XSize& size )
{
    // create table for width/height values
    lua_newtable( luaState );

    lua_pushinteger( luaState, size.Width( ) );
    lua_rawseti( luaState, -2, 1 );

    lua_pushinteger( luaState, size.Height( ) );
    lua_rawseti( luaState, -2, 2 );
}

// Get XRange value from Lua's stack
static const XRange GetXRangeFromLuaStack( lua_State* luaState, int stackIndex )
{
    XRange range;

    luaL_checktype( luaState, stackIndex, LUA_TTABLE );
    if ( luaL_len( luaState, stackIndex ) != 2 )
    {
        ReportError( luaState, "The table representing range value must contain 2 numbers only" );
    }

    // min
    lua_rawgeti( luaState, stackIndex, 1 );
    range.SetMin( static_cast<int32_t>( luaL_checkinteger( luaState, -1 ) ) );
    lua_pop( luaState, 1 );

    // max
    lua_rawgeti( luaState, stackIndex, 2 );
    range.SetMax( static_cast<int32_t>( luaL_checkinteger( luaState, -1 ) ) );
    lua_pop( luaState, 1 );

    return range;
}

// Get XRangeF value from Lua's stack
static const XRangeF GetXRangeFFromLuaStack( lua_State* luaState, int stackIndex )
{
    XRangeF range;

    luaL_checktype( luaState, stackIndex, LUA_TTABLE );
    if ( luaL_len( luaState, stackIndex ) != 2 )
    {
        ReportError( luaState, "The table representing range value must contain 2 numbers only" );
    }

    // min
    lua_rawgeti( luaState, stackIndex, 1 );

    luaL_checktype( luaState, -1, LUA_TNUMBER );
    range.SetMin( static_cast<float>( lua_tonumberx( luaState, -1, nullptr ) ) );
    lua_pop( luaState, 1 );

    // max
    lua_rawgeti( luaState, stackIndex, 2 );
    luaL_checktype( luaState, -1, LUA_TNUMBER );
    range.SetMax( static_cast<float>( lua_tonumberx( luaState, -1, nullptr ) ) );
    lua_pop( luaState, 1 );

    return range;
}

// Get XPoint value from Lua's stack
static const XPoint GetXPointFromLuaStack( lua_State* luaState, int stackIndex )
{
    XPoint point;

    luaL_checktype( luaState, stackIndex, LUA_TTABLE );
    if ( luaL_len( luaState, stackIndex ) != 2 )
    {
        ReportError( luaState, "The table representing point value must contain 2 numbers only" );
    }

    // x
    lua_rawgeti( luaState, stackIndex, 1 );
    point.SetX( static_cast<int32_t>( luaL_checkinteger( luaState, -1 ) ) );
    lua_pop( luaState, 1 );

    // y
    lua_rawgeti( luaState, stackIndex, 2 );
    point.SetY( static_cast<int32_t>( luaL_checkinteger( luaState, -1 ) ) );
    lua_pop( luaState, 1 );

    return point;
}

// Get XPointF value from Lua's stack
static const XPointF GetXPointFFromLuaStack( lua_State* luaState, int stackIndex )
{
    XPointF point;

    luaL_checktype( luaState, stackIndex, LUA_TTABLE );
    if ( luaL_len( luaState, stackIndex ) != 2 )
    {
        ReportError( luaState, "The table representing point value must contain 2 numbers only" );
    }

    // x
    lua_rawgeti( luaState, stackIndex, 1 );

    luaL_checktype( luaState, -1, LUA_TNUMBER );
    point.SetX( static_cast<float>( lua_tonumberx( luaState, -1, nullptr ) ) );
    lua_pop( luaState, 1 );

    // max
    lua_rawgeti( luaState, stackIndex, 2 );
    luaL_checktype( luaState, -1, LUA_TNUMBER );
    point.SetY( static_cast<float>( lua_tonumberx( luaState, -1, nullptr ) ) );
    lua_pop( luaState, 1 );

    return point;
}

// Get XSize value from Lua's stack
static const XSize GetXSizeFromLuaStack( lua_State* luaState, int stackIndex )
{
    XSize size;

    luaL_checktype( luaState, stackIndex, LUA_TTABLE );
    if ( luaL_len( luaState, stackIndex ) != 2 )
    {
        ReportError( luaState, "The table representing size value must contain 2 numbers only" );
    }

    // width
    lua_rawgeti( luaState, stackIndex, 1 );
    size.SetWidth( static_cast<int32_t>( luaL_checkinteger( luaState, -1 ) ) );
    lua_pop( luaState, 1 );

    // height
    lua_rawgeti( luaState, stackIndex, 2 );
    size.SetHeight( static_cast<int32_t>( luaL_checkinteger( luaState, -1 ) ) );
    lua_pop( luaState, 1 );

    return size;
}

// Convert XColor to Lua's table with 4 values and push it to stack
static void PushXColorToLuaStack( lua_State* luaState, const XColor& color )
{
    // create table for RGBA values
    lua_newtable( luaState );

    lua_pushinteger( luaState, color.Red( ) );
    lua_rawseti( luaState, -2, 1 );

    lua_pushinteger( luaState, color.Green( ) );
    lua_rawseti( luaState, -2, 2 );

    lua_pushinteger( luaState, color.Blue( ) );
    lua_rawseti( luaState, -2, 3 );

    lua_pushinteger( luaState, color.Alpha( ) );
    lua_rawseti( luaState, -2, 4 );
}

// Get XColor value from Lua's stack
static const XColor GetXColorFromLuaStack( lua_State* luaState, int stackIndex )
{
    XColor color;

    if ( lua_istable( luaState, stackIndex ) )
    {
        int tableLen = static_cast<int>( luaL_len( luaState, stackIndex ) );

        if ( ( tableLen != 3 ) && ( tableLen != 4 ) )
        {
            ReportError( luaState, "The table representing color value must contain 3 (rgb) or 4 (rgba) numbers" );
        }

        // red
        lua_rawgeti( luaState, stackIndex, 1 );
        color.SetRed( static_cast<uint8_t>( luaL_checkinteger( luaState, -1 ) ) );
        lua_pop( luaState, 1 );

        // green
        lua_rawgeti( luaState, stackIndex, 2 );
        color.SetGreen( static_cast<uint8_t>( luaL_checkinteger( luaState, -1 ) ) );
        lua_pop( luaState, 1 );

        // blue
        lua_rawgeti( luaState, stackIndex, 3 );
        color.SetBlue( static_cast<uint8_t>( luaL_checkinteger( luaState, -1 ) ) );
        lua_pop( luaState, 1 );

        if ( tableLen == 4 )
        {
            // alpha
            lua_rawgeti( luaState, stackIndex, 4 );
            color.SetAlpha( static_cast<uint8_t>( luaL_checkinteger( luaState, -1 ) ) );
            lua_pop( luaState, 1 );
        }
    }
    else if ( lua_isstring( luaState, stackIndex ) )
    {
        XVariant    variant   = XVariant( luaL_checkstring( luaState, stackIndex ) );
        XErrorCode  errorCode = SuccessCode;

        color = variant.ToColor( &errorCode );

        if ( errorCode != SuccessCode )
        {
            ReportArgXError( luaState, stackIndex, errorCode );
        }
    }
    else
    {
        ReportArgError( luaState, stackIndex, "table or string is expected" );
    }

    return color;
}

// Convert XVariant to something which Lua would understand and push it to stack
void PushXVariantToLuaStack( lua_State* luaState, const XVariant& variant )
{
    if ( variant.IsNullOrEmpty( ) )
    {
        lua_pushnil( luaState );
    }
    else
    {
        if ( ( variant.Type( ) & ( XVT_Array | XVT_ArrayJagged ) ) != 0 )
        {
            // create array on stack as Lua table
            const xarray* xarray = static_cast<const xvariant*>( variant )->value.arrayVal;

            lua_newtable( luaState );

            for ( uint32_t i = 0; i < xarray->length; i++ )
            {
                xvariant xvar;
                XVariant var;

                XVariantInit( &xvar );

                if ( XArrayGet( xarray, i, &xvar ) == SuccessCode )
                {
                    var = xvar;
                    XVariantClear( &xvar );
                }

                PushXVariantToLuaStack( luaState, var );
                lua_rawseti( luaState, -2, i + 1 );
            }
        }
        else if ( ( variant.Type( ) & XVT_Array2d ) == XVT_Array2d )
        {
            // create array on stack as Lua table
            const xarray2d* xarray = static_cast<const xvariant*>( variant )->value.array2Val;

            lua_newtable( luaState );

            for ( uint32_t i = 0; i < xarray->rows; i++ )
            {
                lua_newtable( luaState );

                for ( uint32_t j = 0; j < xarray->cols; j++ )
                {
                    xvariant xvar;
                    XVariant var;

                    XVariantInit( &xvar );

                    if ( XArrayGet2d( xarray, i, j, &xvar ) == SuccessCode )
                    {
                        var = xvar;
                        XVariantClear( &xvar );
                    }

                    PushXVariantToLuaStack( luaState, var );
                    lua_rawseti( luaState, -2, j + 1 );
                }

                lua_rawseti( luaState, -2, i + 1 );
            }
        }
        else
        {
            switch ( variant.Type( ) )
            {
            case XVT_Bool:
                lua_pushboolean( luaState, ( variant.ToBool( ) ) ? 1 : 0 );
                break;

            case XVT_I1:
            case XVT_I2:
            case XVT_I4:
                lua_pushinteger( luaState, variant.ToInt( ) );
                break;

            case XVT_U1:
            case XVT_U2:
            case XVT_U4:
                lua_pushinteger( luaState, variant.ToUInt( ) );
                break;

            case XVT_R4:
            case XVT_R8:
                lua_pushnumber( luaState, variant.ToDouble( ) );
                break;

            case XVT_ARGB:
                PushXColorToLuaStack( luaState, variant.ToColor( ) );
                break;

            case XVT_Range:
                PushXRangeToLuaStack( luaState, variant.ToRange( ) );
                break;

            case XVT_RangeF:
                PushXRangeFToLuaStack( luaState, variant.ToRangeF( ) );
                break;

            case XVT_Point:
                PushXPointToLuaStack( luaState, variant.ToPoint( ) );
                break;

            case XVT_PointF:
                PushXPointFToLuaStack( luaState, variant.ToPointF( ) );
                break;

            case XVT_Size:
                PushXSizeToLuaStack( luaState, variant.ToSize( ) );
                break;

            default:
                {
                    XErrorCode errorCode;
                    string str = variant.ToString( &errorCode );

                    if ( errorCode != SuccessCode )
                    {
                        ReportXError( luaState, errorCode );
                    }
                    else
                    {
                        lua_pushstring( luaState, str.c_str( ) );
                    }
                }
                break;
            }
        }
    }
}

// Get XVariant value from Lua's stack
const XVariant GetXVariantFromLuaStack( lua_State* luaState, int stackIndex )
{
    XVariant variant;

    if ( lua_isnil( luaState, stackIndex ) )
    {
        variant.SetNull( );
    }
    else if ( lua_isboolean( luaState, stackIndex ) )
    {
        variant = XVariant( lua_toboolean( luaState, stackIndex ) != 0 );
    }
    else if ( lua_isinteger( luaState, stackIndex ) )
    {
        variant = XVariant( static_cast<int32_t>( lua_tointegerx( luaState, stackIndex, 0 ) ) );
    }
    else if ( lua_isnumber( luaState, stackIndex ) )
    {
        variant = XVariant( static_cast<double>( lua_tonumberx( luaState, stackIndex, 0 ) ) );
    }
    else if ( lua_isstring( luaState, stackIndex ) )
    {
        variant = XVariant( lua_tostring( luaState, stackIndex ) );
    }
    else if ( ( IsImageOnLuaStack( luaState, stackIndex ) ) ||
              ( IsPluginOnLuaStack( luaState, stackIndex ) ) )
    {
        ReportError( luaState, "Can not get image or plug-in objects from Lua stack as generic type objects" );
    }
    else if ( lua_istable( luaState, stackIndex ) )
    {
        int           tableLen  = static_cast<int>( luaL_len( luaState, stackIndex ) );
        XVariantArray array( XVT_Any, tableLen );

        // get all array elements
        for ( int i = 1; i <= tableLen; i++ )
        {
            lua_rawgeti( luaState, stackIndex, i );
            XVariant arrayElement = GetXVariantFromLuaStack( luaState, -1 );

            // if table contained something, which this function does not support, set it to null then
            if ( arrayElement.IsEmpty( ) )
            {
                arrayElement.SetNull( );
            }

            lua_pop( luaState, 1 );

            array.Set( static_cast<uint32_t>( i - 1 ), arrayElement );
        }

        variant = XVariant( array );
    }

    return variant;
}

// Get XVariant value from Lua's stack
static const XVariant GetXVariantFromLuaStack( lua_State* luaState, int stackIndex, XVarType expectedType )
{
    XVariant variant;

    if ( ( expectedType & XVT_Array ) == XVT_Array )
    {
        XVarType elementType = expectedType & XVT_Any;

        if ( elementType == XVT_Any )
        {
            ReportError( luaState, STR_ERROR_ARRAY_ANY_TYPE );
        }
        else
        {
            luaL_checktype( luaState, stackIndex, LUA_TTABLE );

            int         tableLen  = static_cast<int>( luaL_len( luaState, stackIndex ) );
            xarray*     arrayVar  = nullptr;
            XErrorCode  errorCode = XArrayAllocate( &arrayVar, elementType, tableLen );

            if ( errorCode != SuccessCode )
            {
                ReportXError( luaState, errorCode );
            }
            else
            {
                xvariant xvar;

                xvar.type           = expectedType;
                xvar.value.arrayVal = arrayVar;

                // set the empty array under control of C++ XVariant, so it could be destroyed
                // if any Lua type checking error happens below
                variant = xvar;
                XVariantClear( &xvar );
                arrayVar = static_cast< const xvariant* >( variant )->value.arrayVal;

                // get all array elements
                for ( int i = 1; i <= tableLen; i++ )
                {
                    lua_rawgeti( luaState, stackIndex, i );
                    XVariant arrayElement = GetXVariantFromLuaStack( luaState, -1, elementType );
                    lua_pop( luaState, 1 );

                    XArraySet( arrayVar, static_cast<uint32_t>( i - 1 ), arrayElement );
                }
            }
        }
    }
    else if ( ( expectedType & XVT_Array2d ) == XVT_Array2d )
    {
        XVarType elementType = expectedType & XVT_Any;

        if ( elementType == XVT_Any )
        {
            ReportError( luaState, STR_ERROR_ARRAY_ANY_TYPE );
        }
        else
        {
            luaL_checktype( luaState, stackIndex, LUA_TTABLE );

            int         rowsCount = static_cast<int>( luaL_len( luaState, stackIndex ) );
            int         colsCount = 0;
            xarray2d*   arrayVar  = nullptr;
            XErrorCode  errorCode;
            xvariant    xvar;

            XVariantInit( &xvar );

            // walk through all rows
            for ( int i = 1; i <= rowsCount; i++ )
            {
                // get next row and make sure it is a table
                lua_rawgeti( luaState, stackIndex, i );
                luaL_checktype( luaState, -1, LUA_TTABLE );

                if ( arrayVar == nullptr )
                {
                    // first row, so resolve number of columns and allocate 2D array
                    colsCount = static_cast<int>( luaL_len( luaState, -1 ) );
                    errorCode = XArrayAllocate2d( &arrayVar, elementType, rowsCount, colsCount );

                    if ( errorCode != SuccessCode )
                    {
                        ReportXError( luaState, errorCode );
                    }
                    else
                    {
                        xvar.type            = expectedType;
                        xvar.value.array2Val = arrayVar;

                        // set the empty array under control of C++ XVariant, so it could be destroyed
                        // if any Lua type checking error happens below
                        variant = xvar;
                        XVariantClear( &xvar );
                        arrayVar = static_cast<const xvariant*>( variant )->value.array2Val;
                    }
                }
                else
                {
                    // make sure all other rows (Lua tables) are of the same length
                    if ( colsCount != static_cast<int>( luaL_len( luaState, -1 ) ) )
                    {
                        ReportError( luaState, "All nested tables for 2D arrays must be of the same length" );
                    }
                }

                // walk through all elements of the row
                for ( int j = 1; j <= colsCount; j++ )
                {
                    lua_rawgeti( luaState, -1, j );
                    XVariant arrayElement = GetXVariantFromLuaStack( luaState, -1, elementType );
                    lua_pop( luaState, 1 );

                    XArraySet2d( arrayVar, static_cast<uint32_t>( i - 1 ), static_cast<uint32_t>( j - 1 ), arrayElement );
                }

                lua_pop( luaState, 1 );
            }
        }
    }
    else
    {
        switch ( expectedType )
        {
        case XVT_Bool:
            luaL_checktype( luaState, stackIndex, LUA_TBOOLEAN );
            variant = XVariant( lua_toboolean( luaState, stackIndex ) != 0 );
            break;

        case XVT_I1:
            variant = XVariant( static_cast<int8_t>( luaL_checkinteger( luaState, stackIndex ) ) );
            break;

        case XVT_I2:
            variant = XVariant( static_cast<int16_t>( luaL_checkinteger( luaState, stackIndex ) ) );
            break;

        case XVT_I4:
            variant = XVariant( static_cast<int32_t>( luaL_checkinteger( luaState, stackIndex ) ) );
            break;

        case XVT_U1:
            variant = XVariant( static_cast<uint8_t>( luaL_checkinteger( luaState, stackIndex ) ) );
            break;

        case XVT_U2:
            variant = XVariant( static_cast<uint16_t>( luaL_checkinteger( luaState, stackIndex ) ) );
            break;

        case XVT_U4:
            variant = XVariant( static_cast<uint32_t>( luaL_checkinteger( luaState, stackIndex ) ) );
            break;

        case XVT_R4:
            luaL_checktype( luaState, stackIndex, LUA_TNUMBER );
            variant = XVariant( static_cast<float>( lua_tonumberx( luaState, stackIndex, nullptr ) ) );
            break;

        case XVT_R8:
            luaL_checktype( luaState, stackIndex, LUA_TNUMBER );
            variant = XVariant( static_cast<double>( lua_tonumberx( luaState, stackIndex, nullptr ) ) );
            break;

        case XVT_String:
            variant = XVariant( luaL_checkstring( luaState, stackIndex ) );
            break;

        case XVT_Range:
            variant = XVariant( GetXRangeFromLuaStack( luaState, stackIndex ) );
            break;

        case XVT_RangeF:
            variant = XVariant( GetXRangeFFromLuaStack( luaState, stackIndex ) );
            break;

        case XVT_Point:
            variant = XVariant( GetXPointFromLuaStack( luaState, stackIndex ) );
            break;

        case XVT_PointF:
            variant = XVariant( GetXPointFFromLuaStack( luaState, stackIndex ) );
            break;

        case XVT_Size:
            variant = XVariant( GetXSizeFromLuaStack( luaState, stackIndex ) );
            break;

        case XVT_ARGB:
            variant = XVariant( GetXColorFromLuaStack( luaState, stackIndex ) );
            break;

        case XVT_Image:
            variant = XVariant( GetImageFromLuaStack( luaState, stackIndex ) );
            break;
        }
    }

    return variant;
}

// ===== Common plug-in related function =====

// Get plug-in shell from Lua stack (report argument error if it is not)
static PluginShell* GetPluginShellFromLuaStack( lua_State* luaState, int index )
{
    if ( !IsPluginOnLuaStack( luaState, index ) )
    {
        luaL_argcheck( luaState, 0, index, "plug-in object expected" );
    }

    PluginShell* pluginShell = *static_cast<PluginShell**>( lua_touserdata( luaState, index ) );

    if ( pluginShell->IsReleased( ) )
    {
        ReportError( luaState, STR_ERROR_RELEASED_OBJECT );
    }

    return pluginShell;
}

// Get plug-in shell of the specified type from Lua stack (report argument error if it is not)
static PluginShell* GetPluginShellFromLuaStack( lua_State* luaState, int index, const char* pluginMetaTable )
{
    PluginShell* pluginShell = *static_cast<PluginShell**>( luaL_checkudata( luaState, index, pluginMetaTable ) );

    if ( pluginShell->IsReleased( ) )
    {
        ReportError( luaState, STR_ERROR_RELEASED_OBJECT );
    }

    return pluginShell;
}

// Destructor for plug-in user data types
static int Plugin_Gc( lua_State* luaState )
{
    PluginShell* pluginShell = *(PluginShell**) lua_touserdata( luaState, 1 );

    if ( pluginShell != nullptr )
    {
        delete pluginShell;
    }

    return 0;
}

// Release a plug-in
static int Plugin_Release( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 1 );

    PluginShell* pluginShell = GetPluginShellFromLuaStack( luaState, 1 );

    pluginShell->Release( );

    return 0;
}

// Get ID of a plug-in
static int Plugin_ID( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 1 );

    PluginShell* pluginShell = GetPluginShellFromLuaStack( luaState, 1 );

    lua_pushstring( luaState, pluginShell->Descriptor->ID( ).ToString( ).c_str( ) );

    return 1;
}

// Get name of a plug-in
static int Plugin_Name( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 1 );

    PluginShell* pluginShell = GetPluginShellFromLuaStack( luaState, 1 );

    lua_pushstring( luaState, pluginShell->Descriptor->Name( ).c_str( ) );

    return 1;
}

// Get short name of a plug-in
static int Plugin_ShortName( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 1 );

    PluginShell* pluginShell = GetPluginShellFromLuaStack( luaState, 1 );

    lua_pushstring( luaState, pluginShell->Descriptor->ShortName( ).c_str( ) );

    return 1;
}

// Get version of a plug-in
static int Plugin_Version( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 1 );

    PluginShell* pluginShell = GetPluginShellFromLuaStack( luaState, 1 );

    lua_pushstring( luaState, pluginShell->Descriptor->Version( ).ToString( ).c_str( ) );

    return 1;
}

// Get description of a plug-in
static int Plugin_Description( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 1 );

    PluginShell* pluginShell = GetPluginShellFromLuaStack( luaState, 1 );

    lua_pushstring( luaState, pluginShell->Descriptor->Description( ).c_str( ) );

    return 1;
}

// Get type of a plug-in
static int Plugin_Type( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 1 );

    PluginShell* pluginShell   = GetPluginShellFromLuaStack( luaState, 1 );
    const char*  strPluginType = nullptr;

    switch ( pluginShell->Descriptor->Type( ) )
    {
    case PluginType_ImageImporter:
        strPluginType = "ImageImporter";
        break;
    case PluginType_ImageExporter:
        strPluginType = "ImageExporter";
        break;
    case PluginType_ImageGenerator:
        strPluginType = "ImageGenerator";
        break;
    case PluginType_ImageProcessing:
        strPluginType = "ImageProcessing";
        break;
    case PluginType_ImageProcessingFilter:
        strPluginType = "ImageProcessingFilter";
        break;
    case PluginType_ImageProcessingFilter2:
        strPluginType = "ImageProcessingFilter2";
        break;
    case PluginType_Device:
        strPluginType = "Device";
        break;
    case PluginType_VideoProcessing:
        strPluginType = "VideoProcessing";
        break;
    case PluginType_ScriptingApi:
        strPluginType = "ScriptingApi";
        break;
    case PluginType_VideoSource:
        strPluginType = "VideoSource";
        break;
    default:
        strPluginType = "Unknown";
        break;
    }

    lua_pushstring( luaState, strPluginType );

    return 1;
}

// Get the specified property of the plug-in
static int Plugin_GetPropertyByName( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 2, 3 );

    PluginShell* pluginShell   = GetPluginShellFromLuaStack( luaState, 1 );
    const char*  propertyName  = luaL_checkstring( luaState, 2 );
    bool         indexedAccess = ( lua_gettop( luaState ) == 3 );
    lua_Integer  valueIndex    = ( !indexedAccess ) ? 0 : luaL_checkinteger( luaState, 3 );
    int32_t      propertyIndex = pluginShell->Descriptor->GetPropertyIndexByName( propertyName );

    if ( propertyIndex == -1 )
    {
        ReportError( luaState, STR_ERROR_UNKNOWN_PROPERTY );
    }
    else
    {
        if ( ( indexedAccess ) && ( !pluginShell->Descriptor->GetPropertyDescriptor( propertyIndex )->IsIndexed( ) ) )
        {
            ReportXError( luaState, ErrorNotIndexedProperty );
        }
        else if ( ( indexedAccess ) && ( valueIndex < 1 ) )
        {
            ReportArgError( luaState, 3, STR_ERROR_PROPERTY_INDEX );
        }
        else
        {
            XVariant   propertyValue;
            XErrorCode errorCode = ( indexedAccess ) ?
                                    pluginShell->Plugin->GetIndexedProperty( propertyIndex, static_cast<uint32_t>( valueIndex ) - 1, propertyValue ) :
                                    pluginShell->Plugin->GetProperty( propertyIndex, propertyValue );

            if ( errorCode != SuccessCode )
            {
                ReportXError( luaState, errorCode );
            }
            else
            {
                PushXVariantToLuaStack( luaState, propertyValue );
            }
        }
    }

    return 1;
}

// Set the specified property of the plug-in
static int Plugin_SetPropertyByName( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 3, 4 );

    PluginShell* pluginShell   = GetPluginShellFromLuaStack( luaState, 1 );
    const char*  propertyName  = luaL_checkstring( luaState, 2 );
    bool         indexedAccess = ( lua_gettop( luaState ) == 4 );
    lua_Integer  valueIndex    = ( !indexedAccess ) ? 0 : luaL_checkinteger( luaState, 3 );
    int32_t      propertyIndex = pluginShell->Descriptor->GetPropertyIndexByName( propertyName );

    if ( propertyIndex == -1 )
    {
        ReportError( luaState, STR_ERROR_UNKNOWN_PROPERTY );
    }
    else
    {
        shared_ptr<const XPropertyDescriptor> propertyDesc = pluginShell->Descriptor->GetPropertyDescriptor( propertyIndex );

        if ( propertyDesc->IsReadOnly( ) )
        {
            ReportXError( luaState, ErrorReadOnlyProperty );
        }
        else if ( ( indexedAccess ) && ( !propertyDesc->IsIndexed( ) ) )
        {
            ReportXError( luaState, ErrorNotIndexedProperty );
        }
        else if ( ( indexedAccess ) && ( valueIndex < 1 ) )
        {
            ReportArgError( luaState, 3, STR_ERROR_PROPERTY_INDEX );
        }
        else
        {
            XVarType type    = propertyDesc->Type( );
            XVariant variant = GetXVariantFromLuaStack( luaState,
                                                        ( indexedAccess ) ? 4 : 3,
                                                        ( indexedAccess ) ? type & XVT_Any : type );

            variant.CheckInRange( propertyDesc->GetMinValue( ), propertyDesc->GetMaxValue( ) );

            XErrorCode errorCode = ( indexedAccess ) ?
                                    pluginShell->Plugin->SetIndexedProperty( propertyIndex, static_cast<uint32_t>( valueIndex ) - 1, variant ) :
                                    pluginShell->Plugin->SetProperty( propertyIndex, variant );

            if ( errorCode != SuccessCode )
            {
                ReportXError( luaState, errorCode );
            }
        }
    }

    return 0;
}

// Call the specified function of the plug-in
static int Plugin_CallFunction( lua_State* luaState )
{
    PluginShell* pluginShell = GetPluginShellFromLuaStack( luaState, 1 );
    const char*  funcName    = luaL_checkstring( luaState, 2 );
    int          ret         = 0;

    if ( pluginShell->Descriptor->FunctionsCount( ) == 0 )
    {
        ReportError( luaState, "The plug-in does not expose any functions to call" );
    }
    else
    {
        shared_ptr<const XFunctionDescriptor> funcDesc = pluginShell->Descriptor->GetFunctionDescriptor( funcName );

        if ( !funcDesc )
        {
            ReportError( luaState, "The plug-in does not expose the specified function" );
        }
        else
        {
            int32_t    argsCount = funcDesc->ArgumentsCount( );
            xarray*    args      = nullptr;
            xvariant   argsVar;
            XErrorCode ecode;

            XVariantInit( &argsVar );

            CheckArgumentsCount( luaState, 2 + argsCount );

            ecode = XArrayAllocate( &args, XVT_Any, argsCount );
            if ( ecode == SuccessCode )
            {
                // --> hack to make sure we get array controlled by C++ XVariant
                argsVar.type = XVT_Array;
                argsVar.value.arrayVal = args;

                XVariant arrayWrapper( argsVar );

                XVariantClear( &argsVar );

                args = static_cast<const xvariant*>( arrayWrapper )->value.arrayVal;
                // <-- end of hack

                for ( int32_t i = 0; i < argsCount; i++ )
                {
                    XVarType type = funcDesc->GetArgumentType( i );
                    XVariant arg  = GetXVariantFromLuaStack( luaState, 3 + i, type );

                    ecode = XArraySet( args, i, arg );
                    if ( ecode != SuccessCode )
                    {
                        break;
                    }
                }

                if ( ecode == SuccessCode )
                {
                    // call the function
                    XVariant funcReturn;
                    ecode = pluginShell->Plugin->CallFunction( funcDesc->ID( ), &funcReturn, args );

                    if ( ( ecode == SuccessCode ) && ( !funcReturn.IsEmpty( ) ) )
                    {
                        PushXVariantToLuaStack( luaState, funcReturn );
                        ret = 1;
                    }
                }

                args = nullptr;
            }

            if ( ecode != SuccessCode )
            {
                ReportXError( luaState, ecode );
            }
        }
    }

    return ret;
}

static const struct luaL_Reg PluginFunctions[] =
{
    { "__gc",         Plugin_Gc                },
    { "Release",      Plugin_Release           },
    { "ID",           Plugin_ID                },
    { "Name",         Plugin_Name              },
    { "ShortName",    Plugin_ShortName         },
    { "Version",      Plugin_Version           },
    { "Description",  Plugin_Description       },
    { "Type",         Plugin_Type              },
    { "GetProperty",  Plugin_GetPropertyByName },
    { "SetProperty",  Plugin_SetPropertyByName },
    { "CallFunction", Plugin_CallFunction      },
    { nullptr, nullptr }
};

// ===== Image importing plug-in's functions =====

// Get description of the file types supported by the plug-in
static int PluginImageImporter_FileTypeDescription( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 1 );

    PluginShell* pluginShell = GetPluginShellFromLuaStack( luaState, 1, METATABLE_IMAGE_IMPORTER_PLUGIN );

    lua_pushstring( luaState, static_pointer_cast<XImageImportingPlugin>( pluginShell->Plugin )->
        GetFileTypeDescription( ).c_str( ) );

    return 1;
}

// Get file extensions supported by the plug-in
static int PluginImageImporter_SupportedExtensions( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 1 );

    PluginShell*   pluginShell = GetPluginShellFromLuaStack( luaState, 1, METATABLE_IMAGE_IMPORTER_PLUGIN );
    vector<string> exts        = static_pointer_cast<XImageImportingPlugin>( pluginShell->Plugin )->GetSupportedExtensions( );
    int            counter     = 1;

    // create table for extensions
    lua_newtable( luaState );
    for ( vector<string>::const_iterator it = exts.begin( ); it != exts.end( ); it++ )
    {
        lua_pushstring( luaState, it->c_str( ) );
        lua_rawseti( luaState, -2, counter );
        counter++;
    }

    return 1;
}

// Load an image using the plug-in
static int PluginImageImporter_ImportImage( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 2 );

    PluginShell*        pluginShell = GetPluginShellFromLuaStack( luaState, 1, METATABLE_IMAGE_IMPORTER_PLUGIN );
    const char*         fileName    = luaL_checkstring( luaState, 2 );
    shared_ptr<XImage>  image;
    XErrorCode          errorCode   = static_pointer_cast<XImageImportingPlugin>( pluginShell->Plugin )->
                                      ImportImage( fileName, image );

    if ( errorCode != SuccessCode )
    {
        ReportXError( luaState, errorCode );
    }
    else
    {
        PutImageOnLuaStack( luaState, image );
    }

    return 1;
}

static const struct luaL_Reg ImageImporterPluginFunctions[] =
{
    { "FileTypeDescription", PluginImageImporter_FileTypeDescription },
    { "SupportedExtensions", PluginImageImporter_SupportedExtensions },
    { "ImportImage",         PluginImageImporter_ImportImage },
    { nullptr, nullptr }
};

// ===== Image exporting plug-in's functions =====

// Get description of the file types supported by the plug-in
static int PluginImageExporter_FileTypeDescription( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 1 );

    PluginShell* pluginShell = GetPluginShellFromLuaStack( luaState, 1, METATABLE_IMAGE_EXPORTER_PLUGIN );

    lua_pushstring( luaState, static_pointer_cast<XImageExportingPlugin>( pluginShell->Plugin )->
        GetFileTypeDescription( ).c_str( ) );

    return 1;
}

// Get file extensions supported by the plug-in
static int PluginImageExporter_SupportedExtensions( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 1 );

    PluginShell*   pluginShell = GetPluginShellFromLuaStack( luaState, 1, METATABLE_IMAGE_EXPORTER_PLUGIN );
    vector<string> exts        = static_pointer_cast<XImageExportingPlugin>( pluginShell->Plugin )->GetSupportedExtensions( );
    int            counter     = 1;

    // create table for extensions
    lua_newtable( luaState );

    for ( vector<string>::const_iterator it = exts.begin( ); it != exts.end( ); it++ )
    {
        lua_pushstring( luaState, it->c_str( ) );
        lua_rawseti( luaState, -2, counter );
        counter++;
    }

    return 1;
}

// Get pixel formats supported by the plug-in
static int PluginImageExporter_SupportedPixelFormats( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 1 );

    PluginShell*         pluginShell = GetPluginShellFromLuaStack( luaState, 1, METATABLE_IMAGE_EXPORTER_PLUGIN );
    vector<XPixelFormat> formats     = static_pointer_cast<XImageExportingPlugin>( pluginShell->Plugin )->GetSupportedPixelFormats( );
    int                  counter     = 1;

    // create table for pixel formats
    lua_newtable( luaState );

    for ( vector<XPixelFormat>::const_iterator it = formats.begin( ); it != formats.end( ); it++ )
    {
        xstring strPixelFormat = XImageGetPixelFormatShortName( *it );

        if ( strPixelFormat == nullptr )
        {
            ReportXError( luaState, ErrorOutOfMemory );
        }
        else
        {
            lua_pushstring( luaState, strPixelFormat );
            lua_rawseti( luaState, -2, counter );
            counter++;

            XStringFree( &strPixelFormat );
        }
    }

    return 1;
}

// Export specified image using the plug-in
static int PluginImageExporter_ExportImage( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 3 );

    PluginShell*       pluginShell = GetPluginShellFromLuaStack( luaState, 1, METATABLE_IMAGE_EXPORTER_PLUGIN );
    shared_ptr<XImage> image       = GetImageFromLuaStack( luaState, 3 );
    const char*        fileName    = luaL_checkstring( luaState, 2 );
    XErrorCode         errorCode   = static_pointer_cast<XImageExportingPlugin>( pluginShell->Plugin )->ExportImage( fileName, image );

    if ( errorCode != SuccessCode )
    {
        ReportXError( luaState, errorCode );
    }

    return 0;
}

static const struct luaL_Reg ImageExporterPluginFunctions[] =
{
    { "FileTypeDescription",   PluginImageExporter_FileTypeDescription },
    { "SupportedExtensions",   PluginImageExporter_SupportedExtensions },
    { "SupportedPixelFormats", PluginImageExporter_SupportedPixelFormats },
    { "ExportImage",           PluginImageExporter_ExportImage },
    { nullptr, nullptr }
};

// ===== Image generator plug-in's functions =====

// Generate an image using the plug-in
static int PluginImageGenerator_GenerateImage( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 1 );

    PluginShell*        pluginShell = GetPluginShellFromLuaStack( luaState, 1, METATABLE_IMAGE_GENERATOR_PLUGIN );
    shared_ptr<XImage>  image;
    XErrorCode          errorCode   = static_pointer_cast<XImageGenerationPlugin>( pluginShell->Plugin )->GenerateImage( image );

    if ( errorCode != SuccessCode )
    {
        ReportXError( luaState, errorCode );
    }
    else
    {
        PutImageOnLuaStack( luaState, image );
    }

    return 1;
}

static const struct luaL_Reg ImageGeneratorPluginFunctions[] =
{
    { "GenerateImage", PluginImageGenerator_GenerateImage },
    { nullptr, nullptr }
};

// ===== Image processing filter plug-in's functions =====

// Check if the image processing filter can be applied "in place"
static int PluginImageProcessingFilter_CanProcessInPlace( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 1 );

    PluginShell* pluginShell = GetPluginShellFromLuaStack( luaState, 1, METATABLE_IMAGE_PROCESSING_FILTER_PLUGIN );

    lua_pushboolean( luaState,
        ( static_pointer_cast<XImageProcessingFilterPlugin>( pluginShell->Plugin )->CanProcessInPlace( ) ) ? 1 : 0 );

    return 1;
}

// Check if the plug-in supports the specified pixel format
static int PluginImageProcessingFilter_IsPixelFormatSupported( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 2 );

    PluginShell* pluginShell    = GetPluginShellFromLuaStack( luaState, 1, METATABLE_IMAGE_PROCESSING_FILTER_PLUGIN );
    const char*  strPixelFormat = luaL_checkstring( luaState, 2 );
    XPixelFormat pixelFormat    = XImageGetPixelFormatFromShortName( strPixelFormat );

    if ( pixelFormat == XPixelFormatUnknown )
    {
        ReportError( luaState, STR_ERROR_UNKNOWN_PIXEL_FORMAT );
    }
    else
    {
        lua_pushboolean( luaState, ( static_pointer_cast<XImageProcessingFilterPlugin>( pluginShell->Plugin )->
            IsPixelFormatSupported( pixelFormat ) ) ? 1 : 0 );
    }

    return 1;
}

// Get pixel format of the output image for the given input pixel format
static int PluginImageProcessingFilter_GetOutputPixelFormat( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 2 );

    PluginShell* pluginShell    = GetPluginShellFromLuaStack( luaState, 1, METATABLE_IMAGE_PROCESSING_FILTER_PLUGIN );
    const char*  strPixelFormat = luaL_checkstring( luaState, 2 );
    XPixelFormat pixelFormat    = XImageGetPixelFormatFromShortName( strPixelFormat );

    if ( pixelFormat == XPixelFormatUnknown )
    {
        ReportError( luaState, STR_ERROR_UNKNOWN_PIXEL_FORMAT );
    }
    else
    {
        XPixelFormat outputFormat = static_pointer_cast<XImageProcessingFilterPlugin>( pluginShell->Plugin )->
            GetOutputPixelFormat( pixelFormat );
        xstring strOutputPixelFormat = XImageGetPixelFormatShortName( outputFormat );

        if ( strPixelFormat == nullptr )
        {
            ReportXError( luaState, ErrorOutOfMemory );
        }
        else
        {
            lua_pushstring( luaState, strOutputPixelFormat );
            XStringFree( &strOutputPixelFormat );
        }
    }

    return 1;
}

// Apply image processing filter to the specified image and return result as a new image
static int PluginImageProcessingFilter_ProcessImage( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 2 );

    PluginShell*        pluginShell = GetPluginShellFromLuaStack( luaState, 1, METATABLE_IMAGE_PROCESSING_FILTER_PLUGIN );
    shared_ptr<XImage>  image       = GetImageFromLuaStack( luaState, 2 );
    shared_ptr<XImage>  outputImage;
    XErrorCode          errorCode   = static_pointer_cast<XImageProcessingFilterPlugin>( pluginShell->Plugin )->ProcessImage( image, outputImage );

    if ( errorCode != SuccessCode )
    {
        ReportXError( luaState, errorCode );
    }
    else
    {
        PutImageOnLuaStack( luaState, outputImage );
    }

    return 1;
}

// Apply image processing filter to the specified image itself (in place)
static int PluginImageProcessingFilter_ProcessImageInPlace( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 2 );

    PluginShell*       pluginShell = GetPluginShellFromLuaStack( luaState, 1, METATABLE_IMAGE_PROCESSING_FILTER_PLUGIN );
    shared_ptr<XImage> image       = GetImageFromLuaStack( luaState, 2 );

    shared_ptr<XImageProcessingFilterPlugin> plugin = static_pointer_cast<XImageProcessingFilterPlugin>( pluginShell->Plugin );

    if ( !plugin->CanProcessInPlace( ) )
    {
        ReportError( luaState, STR_ERROR_UNSUPPORTED_IN_PLACE_FILTERING );
    }
    else
    {
        XErrorCode   errorCode = plugin->ProcessImage( image );

        if ( errorCode != SuccessCode )
        {
            ReportXError( luaState, errorCode );
        }
    }

    return 0;
}

static const struct luaL_Reg ImageProcessingFilterPluginFunctions[] =
{
    { "CanProcessInPlace",      PluginImageProcessingFilter_CanProcessInPlace },
    { "IsPixelFormatSupported", PluginImageProcessingFilter_IsPixelFormatSupported },
    { "GetOutputPixelFormat",   PluginImageProcessingFilter_GetOutputPixelFormat },
    { "ProcessImage",           PluginImageProcessingFilter_ProcessImage },
    { "ProcessImageInPlace",    PluginImageProcessingFilter_ProcessImageInPlace },
    { nullptr, nullptr }
};

// ===== Image processing plug-in's functions =====

// Check if the plug-in supports the specified pixel format
static int PluginImageProcessing_IsPixelFormatSupported( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 2 );

    PluginShell* pluginShell    = GetPluginShellFromLuaStack( luaState, 1, METATABLE_IMAGE_PROCESSING_PLUGIN );
    const char*  strPixelFormat = luaL_checkstring( luaState, 2 );
    XPixelFormat pixelFormat    = XImageGetPixelFormatFromShortName( strPixelFormat );

    if ( pixelFormat == XPixelFormatUnknown )
    {
        ReportError( luaState, STR_ERROR_UNKNOWN_PIXEL_FORMAT );
    }
    else
    {
        lua_pushboolean( luaState, ( static_pointer_cast<XImageProcessingPlugin>( pluginShell->Plugin )->
            IsPixelFormatSupported( pixelFormat ) ) ? 1 : 0 );
    }

    return 1;
}

// Apply image processing filter to the specified image and return result as a new image
static int PluginImageProcessing_ProcessImage( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 2 );

    PluginShell*        pluginShell = GetPluginShellFromLuaStack( luaState, 1, METATABLE_IMAGE_PROCESSING_PLUGIN );
    shared_ptr<XImage>  image       = GetImageFromLuaStack( luaState, 2 );
    XErrorCode          errorCode   = static_pointer_cast<XImageProcessingPlugin>( pluginShell->Plugin )->ProcessImage( image );

    if ( errorCode != SuccessCode )
    {
        ReportXError( luaState, errorCode );
    }

    return 0;
}

static const struct luaL_Reg ImageProcessingPluginFunctions[] =
{
    { "IsPixelFormatSupported", PluginImageProcessing_IsPixelFormatSupported },
    { "ProcessImage",           PluginImageProcessing_ProcessImage },
    { nullptr, nullptr }
};

// ===== Two source image processing filter plug-in's functions =====

// Check if the image processing filter can be applied "in place"
static int PluginImageProcessingFilter2_CanProcessInPlace( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 1 );

    PluginShell* pluginShell = GetPluginShellFromLuaStack( luaState, 1, METATABLE_IMAGE_PROCESSING_FILTER_PLUGIN2 );

    lua_pushboolean( luaState,
        ( static_pointer_cast<XImageProcessingFilterPlugin2>( pluginShell->Plugin )->CanProcessInPlace( ) ) ? 1 : 0 );

    return 1;
}

// Check if the plug-in supports the specified pixel format
static int PluginImageProcessingFilter2_IsPixelFormatSupported( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 2 );

    PluginShell* pluginShell    = GetPluginShellFromLuaStack( luaState, 1, METATABLE_IMAGE_PROCESSING_FILTER_PLUGIN2 );
    const char*  strPixelFormat = luaL_checkstring( luaState, 2 );
    XPixelFormat pixelFormat    = XImageGetPixelFormatFromShortName( strPixelFormat );

    if ( pixelFormat == XPixelFormatUnknown )
    {
        ReportError( luaState, STR_ERROR_UNKNOWN_PIXEL_FORMAT );
    }
    else
    {
        lua_pushboolean( luaState, ( static_pointer_cast<XImageProcessingFilterPlugin2>( pluginShell->Plugin )->
            IsPixelFormatSupported( pixelFormat ) ) ? 1 : 0 );
    }

    return 1;
}

// Get pixel format of the output image for the given input pixel format
static int PluginImageProcessingFilter2_GetOutputPixelFormat( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 2 );

    PluginShell* pluginShell    = GetPluginShellFromLuaStack( luaState, 1, METATABLE_IMAGE_PROCESSING_FILTER_PLUGIN2 );
    const char*  strPixelFormat = luaL_checkstring( luaState, 2 );
    XPixelFormat pixelFormat    = XImageGetPixelFormatFromShortName( strPixelFormat );

    if ( pixelFormat == XPixelFormatUnknown )
    {
        ReportError( luaState, STR_ERROR_UNKNOWN_PIXEL_FORMAT );
    }
    else
    {
        XPixelFormat outputFormat = static_pointer_cast<XImageProcessingFilterPlugin2>( pluginShell->Plugin )->
            GetOutputPixelFormat( pixelFormat );
        xstring strOutputPixelFormat = XImageGetPixelFormatShortName( outputFormat );

        if ( strPixelFormat == 0 )
        {
            ReportXError( luaState, ErrorOutOfMemory );
        }
        else
        {
            lua_pushstring( luaState, strOutputPixelFormat );
            XStringFree( &strOutputPixelFormat );
        }
    }

    return 1;
}

// Get human understandable description/purpose of the second image required by the filter
static int PluginImageProcessingFilter2_GetSecondImageDescription( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 1 );

    PluginShell* pluginShell = GetPluginShellFromLuaStack( luaState, 1, METATABLE_IMAGE_PROCESSING_FILTER_PLUGIN2 );

    lua_pushstring( luaState, static_pointer_cast<XImageProcessingFilterPlugin2>( pluginShell->Plugin )->
        GetSecondImageDescription( ).c_str( ) );

    return 1;
}

// Get supported size of the second image
static int PluginImageProcessingFilter2_GetSecondImageSupportedSize( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 1 );

    PluginShell*        pluginShell   = GetPluginShellFromLuaStack( luaState, 1, METATABLE_IMAGE_PROCESSING_FILTER_PLUGIN2 );
    XSupportedImageSize supportedSize = static_pointer_cast<XImageProcessingFilterPlugin2>( pluginShell->Plugin )->
                                                        GetSecondImageSupportedSize( );

    lua_pushstring( luaState, XImage::SupportedImageSizeName( supportedSize ).c_str( ) );

    return 1;
}

// Get supported pixel format of the second image for the given format of the source image
static int PluginImageProcessingFilter2_GetSecondImageSupportedFormat( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 2 );

    PluginShell* pluginShell    = GetPluginShellFromLuaStack( luaState, 1, METATABLE_IMAGE_PROCESSING_FILTER_PLUGIN2 );
    const char*  strPixelFormat = luaL_checkstring( luaState, 2 );
    XPixelFormat pixelFormat    = XImageGetPixelFormatFromShortName( strPixelFormat );

    if ( pixelFormat == XPixelFormatUnknown )
    {
        ReportError( luaState, STR_ERROR_UNKNOWN_PIXEL_FORMAT );
    }
    else
    {
        XPixelFormat outputFormat = static_pointer_cast<XImageProcessingFilterPlugin2>( pluginShell->Plugin )->
            GetSecondImageSupportedFormat( pixelFormat );
        xstring strOutputPixelFormat = XImageGetPixelFormatShortName( outputFormat );

        if ( strPixelFormat == nullptr )
        {
            ReportXError( luaState, ErrorOutOfMemory );
        }
        else
        {
            lua_pushstring( luaState, strOutputPixelFormat );
            XStringFree( &strOutputPixelFormat );
        }
    }

    return 1;
}

// Apply image processing filter to the specified image and return result as a new image
static int PluginImageProcessingFilter2_ProcessImage( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 3 );

    PluginShell*        pluginShell = GetPluginShellFromLuaStack( luaState, 1, METATABLE_IMAGE_PROCESSING_FILTER_PLUGIN2 );
    shared_ptr<XImage>  image1      = GetImageFromLuaStack( luaState, 2 );
    shared_ptr<XImage>  image2      = GetImageFromLuaStack( luaState, 3 );
    shared_ptr<XImage>  outputImage;
    XErrorCode          errorCode   = static_pointer_cast<XImageProcessingFilterPlugin2>( pluginShell->Plugin )->
                                        ProcessImage( image1, image2, outputImage );

    if ( errorCode != SuccessCode )
    {
        ReportXError( luaState, errorCode );
    }
    else
    {
        PutImageOnLuaStack( luaState, outputImage );
    }

    return 1;
}

// Apply image processing filter to the specified image itself (in place)
static int PluginImageProcessingFilter2_ProcessImageInPlace( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 3 );

    PluginShell*        pluginShell = GetPluginShellFromLuaStack( luaState, 1, METATABLE_IMAGE_PROCESSING_FILTER_PLUGIN2 );
    shared_ptr<XImage>  image1      = GetImageFromLuaStack( luaState, 2 );
    shared_ptr<XImage>  image2      = GetImageFromLuaStack( luaState, 3 );

    shared_ptr<XImageProcessingFilterPlugin2> plugin = static_pointer_cast<XImageProcessingFilterPlugin2>( pluginShell->Plugin );

    if ( !plugin->CanProcessInPlace( ) )
    {
        ReportError( luaState, STR_ERROR_UNSUPPORTED_IN_PLACE_FILTERING );
    }
    else
    {
        XErrorCode   errorCode = plugin->ProcessImage( image1, image2 );

        if ( errorCode != SuccessCode )
        {
            ReportXError( luaState, errorCode );
        }
    }

    return 0;
}

static const struct luaL_Reg ImageProcessingFilterPluginFunctions2[] =
{
    { "CanProcessInPlace",             PluginImageProcessingFilter2_CanProcessInPlace             },
    { "IsPixelFormatSupported",        PluginImageProcessingFilter2_IsPixelFormatSupported        },
    { "GetOutputPixelFormat",          PluginImageProcessingFilter2_GetOutputPixelFormat          },
    { "GetSecondImageDescription",     PluginImageProcessingFilter2_GetSecondImageDescription     },
    { "GetSecondImageSupportedSize",   PluginImageProcessingFilter2_GetSecondImageSupportedSize   },
    { "GetSecondImageSupportedFormat", PluginImageProcessingFilter2_GetSecondImageSupportedFormat },
    { "ProcessImage",                  PluginImageProcessingFilter2_ProcessImage                  },
    { "ProcessImageInPlace",           PluginImageProcessingFilter2_ProcessImageInPlace           },
    { nullptr, nullptr }
};

// ===== Device plug-in's functions =====

// Connect to device represented by this plug-in's instance
static int PluginDevice_Connect( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 1 );

    PluginShell* pluginShell = GetPluginShellFromLuaStack( luaState, 1, METATABLE_DEVICE_PLUGIN );
    XErrorCode   errorCode   = static_pointer_cast<XDevicePlugin>( pluginShell->Plugin )->Connect( );

    lua_pushboolean( luaState, ( errorCode == SuccessCode ) );

    return 1;
}

// Disconnect from device represented by this plug-in's instance
static int PluginDevice_Disconnect( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 1 );

    PluginShell* pluginShell = GetPluginShellFromLuaStack( luaState, 1, METATABLE_DEVICE_PLUGIN );

    static_pointer_cast<XDevicePlugin>( pluginShell->Plugin )->Disconnect( );

    return 0;
}

// Check if device is connected
static int PluginDevice_IsConnected( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 1 );

    PluginShell* pluginShell = GetPluginShellFromLuaStack( luaState, 1, METATABLE_DEVICE_PLUGIN );

    lua_pushboolean( luaState, static_pointer_cast<XDevicePlugin>( pluginShell->Plugin )->IsConnected( ) );

    return 1;
}

static const struct luaL_Reg DevicePluginFunctions[] =
{
    { "Connect",     PluginDevice_Connect     },
    { "Disconnect",  PluginDevice_Disconnect  },
    { "IsConnected", PluginDevice_IsConnected },
    { nullptr, nullptr }
};

// ===== Communication Device plug-in's functions =====

// Connect to device represented by this plug-in's instance
static int PluginCommunicationDevice_Connect( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 1 );

    PluginShell* pluginShell = GetPluginShellFromLuaStack( luaState, 1, METATABLE_COMMUNICATION_DEVICE_PLUGIN );
    XErrorCode   errorCode   = static_pointer_cast<XCommunicationDevicePlugin>( pluginShell->Plugin )->Connect( );

    lua_pushboolean( luaState, ( errorCode == SuccessCode ) );

    return 1;
}

// Disconnect from device represented by this plug-in's instance
static int PluginCommunicationDevice_Disconnect( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 1 );

    PluginShell* pluginShell = GetPluginShellFromLuaStack( luaState, 1, METATABLE_COMMUNICATION_DEVICE_PLUGIN );

    static_pointer_cast<XCommunicationDevicePlugin>( pluginShell->Plugin )->Disconnect( );

    return 0;
}

// Check if device is connected
static int PluginCommunicationDevice_IsConnected( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 1 );

    PluginShell* pluginShell = GetPluginShellFromLuaStack( luaState, 1, METATABLE_COMMUNICATION_DEVICE_PLUGIN );

    lua_pushboolean( luaState, static_pointer_cast<XCommunicationDevicePlugin>( pluginShell->Plugin )->IsConnected( ) );

    return 1;
}

// Write data to connected device
static int PluginCommunicationDevice_Write( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 2 );

    PluginShell*  pluginShell   = GetPluginShellFromLuaStack( luaState, 1, METATABLE_COMMUNICATION_DEVICE_PLUGIN );
    XVariant      variantToSend = GetXVariantFromLuaStack( luaState, 2, XVT_Array | XVT_U1 );
    uint32_t      bytesWritten  = 0;
    const xarray* arrayToSend   = ( (const xvariant*) variantToSend )->value.arrayVal;
    uint8_t*      buffer        = static_cast<uint8_t*>( malloc( arrayToSend->length ) );

    for ( uint32_t i = 0; i < arrayToSend->length; i++ )
    {
        buffer[i] = arrayToSend->elements[i].value.ubVal;
    }

    XErrorCode   errorCode = static_pointer_cast<XCommunicationDevicePlugin>( pluginShell->Plugin )->Write(
                                        buffer, arrayToSend->length, &bytesWritten );

    lua_pushinteger( luaState, bytesWritten );
    lua_pushboolean( luaState, ( errorCode == SuccessCode ) );

    free( buffer );

    return 2;
}

// Read data from connected device
static int PluginCommunicationDevice_Read( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 2 );

    PluginShell* pluginShell = GetPluginShellFromLuaStack( luaState, 1, METATABLE_COMMUNICATION_DEVICE_PLUGIN );
    int          bytesToRead = static_cast<int>( luaL_checkinteger( luaState, 2 ) );
    uint8_t*     buffer      = static_cast<uint8_t*>( malloc( bytesToRead ) );

    if ( buffer == nullptr )
    {
        ReportXError( luaState, ErrorOutOfMemory );
    }

    uint32_t     bytesRead = 0;
    XErrorCode   errorCode = static_pointer_cast<XCommunicationDevicePlugin>( pluginShell->Plugin )->Read(
                                        buffer, bytesToRead, &bytesRead );

    // create table for whatever was just read from device
    lua_newtable( luaState );

    for ( uint32_t i = 0; i < bytesRead; i++ )
    {
        lua_pushinteger( luaState, buffer[i] );
        lua_rawseti( luaState, -2, i + 1 );
    }
    
    lua_pushboolean( luaState, ( errorCode == SuccessCode ) );

    free( buffer );

    return 2;
}

// Send string to connected device
static int PluginCommunicationDevice_WriteString( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 2 );

    PluginShell* pluginShell  = GetPluginShellFromLuaStack( luaState, 1, METATABLE_COMMUNICATION_DEVICE_PLUGIN );
    const char*  strToSend    = luaL_checkstring( luaState, 2 );
    uint32_t     bytesWritten = 0;

    XErrorCode   errorCode = static_pointer_cast<XCommunicationDevicePlugin>( pluginShell->Plugin )->Write(
                                        (const uint8_t*) strToSend, strlen( strToSend ), &bytesWritten );

    lua_pushinteger( luaState, bytesWritten );
    lua_pushboolean( luaState, ( errorCode == SuccessCode ) );

    return 2;
}

// Read up to the specified number of bytes from device and provide it as string
static int PluginCommunicationDevice_ReadString( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 2 );

    PluginShell* pluginShell = GetPluginShellFromLuaStack( luaState, 1, METATABLE_COMMUNICATION_DEVICE_PLUGIN );
    int          bytesToRead = static_cast<int>( luaL_checkinteger( luaState, 2 ) );
    char*        buffer      = static_cast<char*>( malloc( bytesToRead + 1 ) );

    if ( buffer == nullptr )
    {
        ReportXError( luaState, ErrorOutOfMemory );
    }

    uint32_t     bytesRead = 0;
    XErrorCode   errorCode = static_pointer_cast<XCommunicationDevicePlugin>( pluginShell->Plugin )->Read(
                                        (uint8_t*) buffer, bytesToRead, &bytesRead );

    buffer[bytesRead] = '\0';

    lua_pushstring( luaState, buffer );
    lua_pushboolean( luaState, ( errorCode == SuccessCode ) );

    free( buffer );

    return 2;
}

// Discard any data in communication buffers (write and read)
static int PluginCommunicationDevice_PurgeBuffers( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 1 );

    PluginShell* pluginShell = GetPluginShellFromLuaStack( luaState, 1, METATABLE_COMMUNICATION_DEVICE_PLUGIN );

    static_pointer_cast<XCommunicationDevicePlugin>( pluginShell->Plugin )->PurgeBuffers( );

    return 0;
}

static const struct luaL_Reg CommunicationDevicePluginFunctions[] =
{
    { "Connect",        PluginCommunicationDevice_Connect      },
    { "Disconnect",     PluginCommunicationDevice_Disconnect   },
    { "IsConnected",    PluginCommunicationDevice_IsConnected  },
    { "Write",          PluginCommunicationDevice_Write        },
    { "Read",           PluginCommunicationDevice_Read         },
    { "WriteString",    PluginCommunicationDevice_WriteString  },
    { "ReadString",     PluginCommunicationDevice_ReadString   },
    { "PurgeBuffers",   PluginCommunicationDevice_PurgeBuffers },
    { nullptr, nullptr }
};

// ===== Video Processing plug-in's functions =====

// Check if the plug-in does changes to input video frames or not
static int PluginVideoProcessing_IsReadOnlyMode( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 1 );

    PluginShell* pluginShell = GetPluginShellFromLuaStack( luaState, 1, METATABLE_VIDEO_PROCESSING_PLUGIN );
    bool         isReadOnly  = static_pointer_cast<XVideoProcessingPlugin>( pluginShell->Plugin )->IsReadOnlyMode( );

    lua_pushboolean( luaState, ( isReadOnly ) ? 1 : 0 );

    return 1;
}

// Check if the plug-in supports the specified pixel format
static int PluginVideoProcessing_IsPixelFormatSupported( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 2 );

    PluginShell* pluginShell    = GetPluginShellFromLuaStack( luaState, 1, METATABLE_VIDEO_PROCESSING_PLUGIN );
    const char*  strPixelFormat = luaL_checkstring( luaState, 2 );
    XPixelFormat pixelFormat    = XImageGetPixelFormatFromShortName( strPixelFormat );

    if ( pixelFormat == XPixelFormatUnknown )
    {
        ReportError( luaState, STR_ERROR_UNKNOWN_PIXEL_FORMAT );
    }
    else
    {
        lua_pushboolean( luaState, ( static_pointer_cast<XVideoProcessingPlugin>( pluginShell->Plugin )->
            IsPixelFormatSupported( pixelFormat ) ) ? 1 : 0 );
    }

    return 1;
}

// Process the specified image
static int PluginVideoProcessing_ProcessImage( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 2 );

    PluginShell*       pluginShell = GetPluginShellFromLuaStack( luaState, 1, METATABLE_VIDEO_PROCESSING_PLUGIN );
    shared_ptr<XImage> image       = GetImageFromLuaStack( luaState, 2 );
    XErrorCode         errorCode   = static_pointer_cast<XVideoProcessingPlugin>( pluginShell->Plugin )->ProcessImage( image );

    if ( errorCode != SuccessCode )
    {
        ReportXError( luaState, errorCode );
    }

    return 0;
}

// // Reset run time state of the video processing plug-in
static int PluginVideoProcessing_Reset( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 1 );

    PluginShell* pluginShell = GetPluginShellFromLuaStack( luaState, 1, METATABLE_VIDEO_PROCESSING_PLUGIN );

    static_pointer_cast<XVideoProcessingPlugin>( pluginShell->Plugin )->Reset( );

    return 0;
}

static const struct luaL_Reg VideoProcessingPluginFunctions[] =
{
    { "IsReadOnlyMode",         PluginVideoProcessing_IsReadOnlyMode         },
    { "IsPixelFormatSupported", PluginVideoProcessing_IsPixelFormatSupported },
    { "ProcessImage",           PluginVideoProcessing_ProcessImage           },
    { "Reset",                  PluginVideoProcessing_Reset                  },
    { nullptr, nullptr }
};

// ===== Image related functions ======

// Put image on Lua stack
void PutImageOnLuaStack( lua_State* luaState, const shared_ptr<XImage>& image )
{
    ImageShell** imageShell = (ImageShell**) lua_newuserdata( luaState, sizeof( ImageShell* ) );

    luaL_getmetatable( luaState, METATABLE_IMAGE );
    lua_setmetatable( luaState, -2 );

    *imageShell = new ImageShell( image );
}

// Get image from Lua stack
const shared_ptr<XImage> GetImageFromLuaStack( lua_State* luaState, int stackIndex )
{
    ImageShell*  imageShell = *(ImageShell**) luaL_checkudata( luaState, stackIndex, METATABLE_IMAGE );

    if ( imageShell->IsReleased( ) )
    {
        ReportError( luaState, STR_ERROR_RELEASED_OBJECT );
    }

    return imageShell->Image;
}

// Check if Lua stack contains image at the specified index
bool IsImageOnLuaStack( lua_State* luaState, int stackIndex )
{
    return ( luaL_testudata( luaState, stackIndex, METATABLE_IMAGE ) != nullptr );
}

// Destructor for image user data type
static int Image_Gc( lua_State* luaState )
{
    ImageShell* imageShell = *(ImageShell**) lua_touserdata( luaState, 1 );

    if ( imageShell != nullptr )
    {
        delete imageShell;
    }

    return 0;
}

// Release an image
static int Image_Release( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 1 );

    ImageShell* imageShell = *( ImageShell** ) luaL_checkudata( luaState, 1, METATABLE_IMAGE );

    // check and report error for consistency
    if ( imageShell->IsReleased( ) )
    {
        ReportError( luaState, STR_ERROR_RELEASED_OBJECT );
    }

    imageShell->Release( );

    return 0;
}

// Get width of an image
static int Image_Width( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 1 );

    shared_ptr<XImage> image = GetImageFromLuaStack( luaState, 1 );

    lua_pushinteger( luaState, image->Width( ) );

    return 1;
}

// Get height of an image
static int Image_Height( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 1 );

    shared_ptr<XImage> image = GetImageFromLuaStack( luaState, 1 );

    lua_pushinteger( luaState, image->Height( ) );

    return 1;
}

// Get pixel format of an image
static int Image_PixelFormat( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 1 );

    shared_ptr<XImage> image          = GetImageFromLuaStack( luaState, 1 );
    xstring            strPixelFormat = XImageGetPixelFormatShortName( image->Format( ) );

    if ( strPixelFormat == nullptr )
    {
        ReportXError( luaState, ErrorOutOfMemory );
    }
    else
    {
        lua_pushstring( luaState, strPixelFormat );
        XStringFree( &strPixelFormat );
    }

    return 1;
}

// Get number of bits per pixel of an image
static int Image_BitsPerPixel( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 1 );

    shared_ptr<XImage> image = GetImageFromLuaStack( luaState, 1 );

    lua_pushinteger( luaState, XImageBitsPerPixel( image->Format( ) ) );

    return 1;
}

// Clone the source image
static int Image_Clone( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 1 );

    shared_ptr<XImage> image       = GetImageFromLuaStack( luaState, 1 );
    shared_ptr<XImage> clonedImage = image->Clone( );

    if ( !clonedImage )
    {
        ReportXError( luaState, ErrorOutOfMemory );
    }
    else
    {
        PutImageOnLuaStack( luaState, clonedImage );
    }

    return 1;
}

// Get sub image of the source image
static int Image_GetSubImage( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 5 );

    shared_ptr<XImage> image  = GetImageFromLuaStack( luaState, 1 );
    int                x      = static_cast<int>( luaL_checkinteger( luaState, 2 ) );
    int                y      = static_cast<int>( luaL_checkinteger( luaState, 3 ) );
    int                width  = static_cast<int>( luaL_checkinteger( luaState, 4 ) );
    int                height = static_cast<int>( luaL_checkinteger( luaState, 5 ) );

    if ( ( width < 1 ) || ( height < 1 ) )
    {
        ReportError( luaState, "Sub image's width/height must be >= 1" );
    }

    if ( ( x < 0 ) || ( y < 0 ) ||
         ( x + width  > image->Width( ) ) ||
         ( y + height > image->Height( ) ) )
    {
        ReportError( luaState, "The requested sub image must be within the source image" );
    }

    shared_ptr<XImage> subImage = image->GetSubImage( x, y, width, height );

    if ( !subImage )
    {
        ReportXError( luaState, ErrorOutOfMemory );
    }
    else
    {
        PutImageOnLuaStack( luaState, subImage );
    }

    return 1;
}

// Put content of one image, into another
static int Image_PutImage( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 4 );

    shared_ptr<XImage> dstImage = GetImageFromLuaStack( luaState, 1 );
    shared_ptr<XImage> srcImage = GetImageFromLuaStack( luaState, 2 );
    int                x        = static_cast<int>( luaL_checkinteger( luaState, 3 ) );
    int                y        = static_cast<int>( luaL_checkinteger( luaState, 4 ) );

    XErrorCode  ec = dstImage->PutImage( srcImage, x, y );

    if ( ec != SuccessCode )
    {
        ReportXError( luaState, ec );
    }

    return 0;
}

// Get pixel value of the image
static int Image_GetPixel( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 3 );

    shared_ptr<XImage> srcImage = GetImageFromLuaStack( luaState, 1 );
    int                x        = static_cast<int>( luaL_checkinteger( luaState, 2 ) );
    int                y        = static_cast<int>( luaL_checkinteger( luaState, 3 ) );
    int                ret      = 0;
    xargb              color;

    XErrorCode ec = XImageGetPixelColor( srcImage->ImageData( ), x, y, &color );

    if ( ec != SuccessCode )
    {
        ReportXError( luaState, ec );
    }
    else
    {
        PushXColorToLuaStack( luaState, XColor( color ) );
        ret = 1;
    }

    return ret;
}

// Set pixel value of the image
static int Image_SetPixel( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 4 );

    shared_ptr<XImage> dstImage = GetImageFromLuaStack( luaState, 1 );
    int                x        = static_cast<int>( luaL_checkinteger( luaState, 2 ) );
    int                y        = static_cast<int>( luaL_checkinteger( luaState, 3 ) );
    XColor             color    = GetXColorFromLuaStack( luaState, 4 );

    XErrorCode ec = XImageSetPixelColor( dstImage->ImageData( ), x, y, (xargb) color );

    if ( ec != SuccessCode )
    {
        ReportXError( luaState, ec );
    }

    return 0;
}

static const struct luaL_Reg ImageFunctions[] =
{
    { "__gc",         Image_Gc           },
    { "Release",      Image_Release      },
    { "Width",        Image_Width        },
    { "Height",       Image_Height       },
    { "PixelFormat",  Image_PixelFormat  },
    { "BitsPerPixel", Image_BitsPerPixel },
    { "Clone",        Image_Clone        },
    { "GetSubImage",  Image_GetSubImage  },
    { "PutImage",     Image_PutImage     },
    { "GetPixel",     Image_GetPixel     },
    { "SetPixel",     Image_SetPixel     },
    { nullptr, nullptr }
};

// ===== Image related static functions ======

// Allocate new image
static int Image_Create( lua_State* luaState )
{
    CheckArgumentsCount( luaState, 3 );

    int          width  = static_cast<int>( luaL_checkinteger( luaState, 1 ) );
    int          height = static_cast<int>( luaL_checkinteger( luaState, 2 ) );
    const char*  strPixelFormat = luaL_checkstring( luaState, 3 );
    XPixelFormat pixelFormat = XImageGetPixelFormatFromShortName( strPixelFormat );

    if ( ( width <= 0 ) || ( height <= 0 ) )
    {
        ReportError( luaState, "Image width/height must be greater than zero" );
    }
    else if ( pixelFormat == XPixelFormatUnknown )
    {
        ReportError( luaState, STR_ERROR_UNKNOWN_PIXEL_FORMAT );
    }
    else
    {
        shared_ptr<XImage> image = XImage::Allocate( (uint32_t) width, (uint32_t) height, pixelFormat );

        if ( !image )
        {
            ReportXError( luaState, ErrorOutOfMemory );
        }
        else
        {
            PutImageOnLuaStack( luaState, image );
        }
    }

    return 1;
}

static const struct luaL_Reg ImageStaticFunctions[] =
{
    { "Create", Image_Create },
    { nullptr, nullptr }
};

// Register Image library, which gives impression of Image static methods
static int luaopen_ImageLibrary( lua_State* luaState )
{
    luaL_newlib( luaState, ImageStaticFunctions );
    return 1;
}

// ===== Lua scripting helper functions =====

// Create Lua user data for the specified plug-in leaving it on Lua' stack
void CreateLuaUserDataForPlugin( lua_State* luaState,
                                 const shared_ptr<const XPluginDescriptor>& descriptor,
                                 const shared_ptr<XPlugin>& plugin )
{
    const char* metatableName = METATABLE_PLUGIN;

    switch ( descriptor->Type( ) )
    {
    case PluginType_ImageImporter:
        metatableName = METATABLE_IMAGE_IMPORTER_PLUGIN;
        break;
    case PluginType_ImageExporter:
        metatableName = METATABLE_IMAGE_EXPORTER_PLUGIN;
        break;
    case PluginType_ImageGenerator:
        metatableName = METATABLE_IMAGE_GENERATOR_PLUGIN;
        break;
    case PluginType_ImageProcessing:
        metatableName = METATABLE_IMAGE_PROCESSING_PLUGIN;
        break;
    case PluginType_ImageProcessingFilter:
        metatableName = METATABLE_IMAGE_PROCESSING_FILTER_PLUGIN;
        break;
    case PluginType_ImageProcessingFilter2:
        metatableName = METATABLE_IMAGE_PROCESSING_FILTER_PLUGIN2;
        break;
    case PluginType_Device:
        metatableName = METATABLE_DEVICE_PLUGIN;
        break;
    case PluginType_CommunicationDevice:
        metatableName = METATABLE_COMMUNICATION_DEVICE_PLUGIN;
        break;
    case PluginType_VideoProcessing:
        metatableName = METATABLE_VIDEO_PROCESSING_PLUGIN;
        break;
    }

    PluginShell** pluginShell = (PluginShell**) lua_newuserdata( luaState, sizeof( PluginShell* ) );
    luaL_getmetatable( luaState, metatableName );
    lua_setmetatable( luaState, -2 );

    *pluginShell = new PluginShell( descriptor, plugin );
}

// Register new metatable and set function table for it
static void RegisterUserType( lua_State* luaState, const char* metaTableName, const luaL_Reg *functionsTable )
{
    // create new metatable
    luaL_newmetatable( luaState, metaTableName );
    // set its __index metamethod
    lua_pushvalue( luaState, -1 );
    lua_setfield( luaState, -2, "__index" );
    // set functions table for the metatable
    luaL_setfuncs( luaState, functionsTable, 0 );
    // stack clean-up
    lua_pop( luaState, 1 );
}

// Register new metatable and set function table for it so it contains its own function plus functions of a base type
static void RegisterInheritedUserType( lua_State* luaState, const char* metaTableName, const luaL_Reg *baseFunctionsTable, const luaL_Reg *ownFunctionsTable )
{
    // create new metatable
    luaL_newmetatable( luaState, metaTableName );
    // set its __index metamethod
    lua_pushvalue( luaState, -1 );
    lua_setfield( luaState, -2, "__index" );
    // set functions table for the metatable
    luaL_setfuncs( luaState, baseFunctionsTable, 0 );
    luaL_setfuncs( luaState, ownFunctionsTable, 0 );
    // stack clean-up
    lua_pop( luaState, 1 );
}

// Register user data types for Lua plug-ins scripting
void RegisterLuaUserDataTypes( lua_State* luaState )
{
    // register methods of an unknow plug-in type
    RegisterUserType( luaState, METATABLE_PLUGIN, PluginFunctions );

    // register methods of image importing plug-in
    RegisterInheritedUserType( luaState, METATABLE_IMAGE_IMPORTER_PLUGIN, PluginFunctions, ImageImporterPluginFunctions );

    // register methods of image exporting plug-in
    RegisterInheritedUserType( luaState, METATABLE_IMAGE_EXPORTER_PLUGIN, PluginFunctions, ImageExporterPluginFunctions );

    // register methods of image generation plug-in
    RegisterInheritedUserType( luaState, METATABLE_IMAGE_GENERATOR_PLUGIN, PluginFunctions, ImageGeneratorPluginFunctions );

    // register methods of image processing filter plug-in
    RegisterInheritedUserType( luaState, METATABLE_IMAGE_PROCESSING_FILTER_PLUGIN, PluginFunctions, ImageProcessingFilterPluginFunctions );

    // register methods of image processing filter plug-in
    RegisterInheritedUserType( luaState, METATABLE_IMAGE_PROCESSING_FILTER_PLUGIN2, PluginFunctions, ImageProcessingFilterPluginFunctions2 );

    // register methods of image processing plug-in
    RegisterInheritedUserType( luaState, METATABLE_IMAGE_PROCESSING_PLUGIN, PluginFunctions, ImageProcessingPluginFunctions );

    // register methods of device plug-in
    RegisterInheritedUserType( luaState, METATABLE_DEVICE_PLUGIN, PluginFunctions, DevicePluginFunctions );

    // register methods of communication device plug-in
    RegisterInheritedUserType( luaState, METATABLE_COMMUNICATION_DEVICE_PLUGIN, PluginFunctions, CommunicationDevicePluginFunctions );

    // register methods of video processing plug-in
    RegisterInheritedUserType( luaState, METATABLE_VIDEO_PROCESSING_PLUGIN, PluginFunctions, VideoProcessingPluginFunctions );

    // register image related methods
    RegisterUserType( luaState, METATABLE_IMAGE, ImageFunctions );

    // register some "static" image functions
    luaL_requiref( luaState, "Image", luaopen_ImageLibrary, 1 );
    lua_pop( luaState, 1 );
}
