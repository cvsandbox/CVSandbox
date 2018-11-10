/*
    Plug-ins' scripting library of Computer Vision Sandbox

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

#include <assert.h>
#include <XError.hpp>
#include "XLuaPluginScripting.hpp"
#include "XLuaPluginScripting_UserTypes.hpp"

extern "C"
{
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
}

using namespace std;
using namespace CVSandbox;

namespace Private
{
    static const char   LuaRegistryKey       = 'k';
    static const int    LuaScriptingRevision = 8;

    class XLuaPluginScriptingData
    {
    public:
        XLuaPluginScriptingData( IScriptingHost* host ) :
            Host( host ),
            LuaState( luaL_newstate( ) ),
            LastErrorMessage( ),
            IsEngineInitialized( false ),
            IsScriptLoaded( false ),
            GcPeriod( 1 ), GcCounter( 0 )
        {
            assert( Host );
            assert( LuaState );
        }

        ~XLuaPluginScriptingData( )
        {
            CleanStack( );
            lua_close( LuaState );
        }

        // Retrieve error message from Lua stack if the error code is not 0
        void UpdateErrorMessage( int errorCode );
        // Pre-register Lua library
        void RegisterLibrary( const char* name, lua_CFunction libraryFunction );
        // Clean Lua stack
        void CleanStack( );

    public:
        IScriptingHost* Host;
        lua_State*      LuaState;
        string          LastErrorMessage;
        bool            IsEngineInitialized;
        bool            IsScriptLoaded;

        uint32_t        GcPeriod;
        uint32_t        GcCounter;
    };

    void XLuaPluginScriptingData::UpdateErrorMessage( int errorCode )
    {
        if ( errorCode == 0 )
        {
            LastErrorMessage = string( );
        }
        else
        {
            LastErrorMessage = string( lua_tostring( LuaState, -1 ) );
            lua_pop( LuaState, 1 );
        }
    }

    void XLuaPluginScriptingData::RegisterLibrary( const char* name, lua_CFunction libraryFunction )
    {
        lua_getglobal( LuaState, "package" );
        lua_getfield( LuaState, -1, "preload" );
        lua_pushcfunction( LuaState, libraryFunction );
        lua_setfield( LuaState, -2, name );
        lua_pop( LuaState, 2 );
    }

    void XLuaPluginScriptingData::CleanStack( )
    {
        lua_settop( LuaState, 0 );
    }

    static IScriptingHost* GetHostFromLuaRegistry( lua_State* luaState )
    {
        lua_rawgetp( luaState, LUA_REGISTRYINDEX, (void*) &Private::LuaRegistryKey );
        IScriptingHost* host = static_cast<IScriptingHost*>(
            const_cast<void*>( lua_topointer( luaState, -1 ) ) );
        lua_pop( luaState, 1 );

        assert( host );

        return host;
    }

    static int HostName( lua_State* luaState )
    {
        CheckArgumentsCount( luaState, 0 );

        lua_pushstring( luaState, GetHostFromLuaRegistry( luaState )->Name( ).c_str( ) );
        return 1;
    }

    static int HostVersion( lua_State* luaState )
    {
        CheckArgumentsCount( luaState, 0 );

        lua_pushstring( luaState, GetHostFromLuaRegistry( luaState )->Version( ).ToString( ).c_str( ) );
        return 1;
    }

    static int HostCreatePluginInstance( lua_State* luaState )
    {
        shared_ptr<const XPluginDescriptor> descriptor;
        shared_ptr<XPlugin>                 plugin;
        int                                 ret = 0;

        CheckArgumentsCount( luaState, 1 );

        luaL_checktype( luaState, 1, LUA_TSTRING );

        string     pluginName = string( lua_tostring( luaState, 1 ) );
        XErrorCode errorCode  = GetHostFromLuaRegistry( luaState )->CreatePluginInstance( pluginName, descriptor, plugin );
        string     strError   = XError::Description( errorCode );

        if ( ( errorCode == SuccessCode ) && ( ( !descriptor ) || ( !plugin  ) ) )
        {
            errorCode = ErrorFailed;
        }

        if ( errorCode == SuccessCode )
        {
            CreateLuaUserDataForPlugin( luaState, descriptor, plugin );
            ret = 1;
        }
        else
        {
            ReportXError( luaState, errorCode );
        }

        return ret;
    }

    static int HostGetImage( lua_State* luaState )
    {
        CheckArgumentsCount( luaState, 0 );

        shared_ptr<XImage>  image;
        XErrorCode          errorCode = GetHostFromLuaRegistry( luaState )->GetImage( image );
        int                 ret       = 0;

        if ( ( errorCode == SuccessCode ) && ( !image ) )
        {
            errorCode = ErrorFailed;
        }

        if ( errorCode != SuccessCode )
        {
            ReportXError( luaState, errorCode );
        }
        else
        {
            PutImageOnLuaStack( luaState, image );
            ret = 1;
        }

        return ret;
    }

    static int HostSetImage( lua_State* luaState )
    {
        CheckArgumentsCount( luaState, 1 );

        shared_ptr<XImage>  image     = GetImageFromLuaStack( luaState, 1 );
        XErrorCode          errorCode = GetHostFromLuaRegistry( luaState )->SetImage( image );

        if ( errorCode != SuccessCode )
        {
            ReportXError( luaState, errorCode );
        }

        return 0;
    }

    static int HostPrint( lua_State* luaState )
    {
        int    nargs = lua_gettop( luaState );
        string str   = string( );

        for ( int i = 1; i <= nargs; i++ )
        {
            int type = lua_type( luaState, i );

            if ( i != 1 )
            {
                str.append( " " );
            }

            switch ( type )
            {
            case LUA_TSTRING:
            case LUA_TNUMBER:
                str.append( lua_tostring( luaState, i ) );
                break;

            case LUA_TBOOLEAN:
                str.append( ( lua_toboolean( luaState, i ) ) ? "true" : "false" );
                break;

            case LUA_TNIL:
                str.append( "(null)" );
                break;

            default:
                str.append( "(" );
                str.append( lua_typename( luaState, type ) );
                str.append( ")" );
                break;
            }
        }

        GetHostFromLuaRegistry( luaState )->PrintString( str );
        return 0;
    }

    static int HostGetArguments( lua_State* luaState )
    {
        CheckArgumentsCount( luaState, 0 );

        const map<string, string> arguments = GetHostFromLuaRegistry( luaState )->GetArguments( );

        // create table to hold the arguments
        lua_newtable( luaState );

        for ( auto& argument : arguments )
        {
            if ( argument.second.empty( ) )
            {
                lua_pushboolean( luaState, 1 );
            }
            else
            {
                lua_pushstring( luaState, argument.second.c_str( ) );
            }

            lua_setfield( luaState, -2, argument.first.c_str( ) );
        }

        return 1;
    }

    static int HostGetVariable( lua_State* luaState )
    {
        CheckArgumentsCount( luaState, 1 );

        XVariant        variableValue;
        const char*     variableName = luaL_checkstring( luaState, 1 );
        IScriptingHost* host         = GetHostFromLuaRegistry( luaState );
        XErrorCode      ret          = host->GetVariable( variableName, variableValue );

        if ( ret != SuccessCode )
        {
            ReportXError( luaState, ret );
        }
        else
        {
            if ( !variableValue.IsEmpty( ) )
            {
                PushXVariantToLuaStack( luaState, variableValue );
            }
            else
            {
                shared_ptr<XImage> imageValue;

                // try getting image with the specified name
                ret = host->GetVariable( variableName, imageValue );

                if ( ret != SuccessCode )
                {
                    ReportXError( luaState, ret );
                }
                else
                {
                    if ( !imageValue )
                    {
                        lua_pushnil( luaState );
                    }
                    else
                    {
                        PutImageOnLuaStack( luaState, imageValue );
                    }
                }
            }
        }

        return 1;
    }

    static int HostSetVariable( lua_State* luaState )
    {
        CheckArgumentsCount( luaState, 2 );

        const char*     variableName = luaL_checkstring( luaState, 1 );
        IScriptingHost* host         = GetHostFromLuaRegistry( luaState );

        if ( IsImageOnLuaStack( luaState, 2 ) )
        {
            XErrorCode ret = host->SetVariable( variableName, GetImageFromLuaStack( luaState, 2 ) );

            if ( ret != SuccessCode )
            {
                ReportXError( luaState, ret );
            }
        }
        else
        {
            const XVariant variableValue = GetXVariantFromLuaStack( luaState, 2 );

            if ( variableValue.IsEmpty( ) )
            {
                ReportArgError( luaState, 2, "Can not store passed object as a host variable" );
            }
            else
            {
                XErrorCode ret = host->SetVariable( variableName, variableValue );

                if ( ret != SuccessCode )
                {
                    ReportXError( luaState, ret );
                }
            }
        }

        return 0;
    }

    static int HostGetVideoSource( lua_State* luaState )
    {
        shared_ptr<const XPluginDescriptor> descriptor;
        shared_ptr<XPlugin>                 plugin;
        int                                 ret = 0;

        CheckArgumentsCount( luaState, 0 );

        XErrorCode errorCode = GetHostFromLuaRegistry( luaState )->GetVideoSource( descriptor, plugin );
        string     strError  = XError::Description( errorCode );

        if ( ( errorCode == SuccessCode ) && ( ( !descriptor ) || ( !plugin ) ) )
        {
            errorCode = ErrorFailed;
        }

        if ( errorCode == SuccessCode )
        {
            CreateLuaUserDataForPlugin( luaState, descriptor, plugin );
            ret = 1;
        }
        else
        {
            ReportXError( luaState, errorCode );
        }

        return ret;
    }

    static const struct luaL_Reg HostLibrary[] =
    {
        { "Name",                   HostName                 },
        { "Version",                HostVersion              },
        { "GetArguments",           HostGetArguments         },
        { "CreatePluginInstance",   HostCreatePluginInstance },
        { "GetImage",               HostGetImage             },
        { "SetImage",               HostSetImage             },
        { "GetVariable",            HostGetVariable          },
        { "SetVariable",            HostSetVariable          },
        { "GetVideoSource",         HostGetVideoSource       },
        { nullptr,                  nullptr                  }
    };

    static const struct luaL_Reg LuaOverride [] =
    {
        { "print", HostPrint },
        { nullptr, nullptr   }
    };

    static int luaopen_HostLibrary( lua_State* luaState )
    {
        RegisterLuaUserDataTypes( luaState );

        luaL_newlib( luaState, HostLibrary );
        return 1;
    }
}

XLuaPluginScripting::XLuaPluginScripting( IScriptingHost* host ) :
    mData( new Private::XLuaPluginScriptingData( host ) )
{
}

XLuaPluginScripting::~XLuaPluginScripting( )
{
    delete mData;
}

const string XLuaPluginScripting::GetLastErrorMessage( ) const
{
    return mData->LastErrorMessage;
}

XErrorCode XLuaPluginScripting::Init( )
{
    if ( !mData->IsEngineInitialized )
    {
        // register some modules
        luaL_requiref( mData->LuaState, "_G", luaopen_base, 1 );
        luaL_requiref( mData->LuaState, "package", luaopen_package, 1 );
        luaL_requiref( mData->LuaState, "Host", Private::luaopen_HostLibrary, 1 );
        lua_pop( mData->LuaState, 3 );

        // pre-register some libraries, so they could be loaded with "require"
        mData->RegisterLibrary( "table", luaopen_table );
        mData->RegisterLibrary( "math", luaopen_math );
        mData->RegisterLibrary( "os", luaopen_os );
        mData->RegisterLibrary( "string", luaopen_string );
        mData->RegisterLibrary( "io", luaopen_io );
        mData->RegisterLibrary( "bit32", luaopen_bit32 );

        // override default "print" function
        lua_getglobal( mData->LuaState, "_G" );
        luaL_setfuncs( mData->LuaState, Private::LuaOverride, 0 );
        lua_pop( mData->LuaState, 1 );

        // set host for the lua state
        lua_pushlightuserdata( mData->LuaState, (void*) mData->Host );
        lua_rawsetp( mData->LuaState, LUA_REGISTRYINDEX, (void*) &Private::LuaRegistryKey );

        lua_pushinteger( mData->LuaState, Private::LuaScriptingRevision );
        lua_setglobal( mData->LuaState, "SCRIPTING_API_REVISION" );

        mData->IsEngineInitialized = true;
    }

    return SuccessCode;
}

XErrorCode XLuaPluginScripting::LoadScriptFromFile( const std::string& fileName )
{
    XErrorCode ret = SuccessCode;

    if ( !mData->IsEngineInitialized )
    {
        ret = ErrorScriptingNotInitialized;
    }
    else
    {
        mData->CleanStack( );

        int luaRet = luaL_loadfile( mData->LuaState, fileName.c_str( ) );
        mData->UpdateErrorMessage( luaRet );

        if ( luaRet != 0 )
        {
            ret = ErrorFailedLoadingScript;
            mData->IsScriptLoaded = false;
        }
        else
        {
            mData->IsScriptLoaded = true;
        }
    }

    return ret;
}

XErrorCode XLuaPluginScripting::IsMainDefined( )
{
    XErrorCode ret = SuccessCode;

    if ( !mData->IsScriptLoaded )
    {
        ret = ErrorNoScriptLoaded;
    }
    else
    {
        lua_getglobal( mData->LuaState, "Main" );
        if ( lua_isnil( mData->LuaState, -1 ) == 1 )
        {
            ret = ErrorNoEntryPointFound;
        }
        lua_pop( mData->LuaState, 1 );
    }

    return ret;
}


XErrorCode XLuaPluginScripting::InitScript( )
{
    XErrorCode ret = SuccessCode;

    if ( !mData->IsScriptLoaded )
    {
        ret = ErrorNoScriptLoaded;
    }
    else
    {
        // clone script entry point, so we can call it again
        lua_pushvalue( mData->LuaState, 1 );

        int luaRet = lua_pcall( mData->LuaState, 0, 0, 0 );
        mData->UpdateErrorMessage( luaRet );

        if ( luaRet != 0 )
        {
            ret = ErrorFailedRunningScript;
        }
    }

    return ret;
}

XErrorCode XLuaPluginScripting::RunScript( )
{
    XErrorCode ret = SuccessCode;

    if ( !mData->IsScriptLoaded )
    {
        ret = ErrorNoScriptLoaded;
    }
    else
    {
        lua_getglobal( mData->LuaState, "Main" );

        if ( lua_isnil( mData->LuaState, -1 ) == 1 )
        {
            ret = ErrorNoEntryPointFound;
            lua_pop( mData->LuaState, 1 );
        }
        else
        {
            int luaRet = lua_pcall( mData->LuaState, 0, 0, 0 );
            mData->UpdateErrorMessage( luaRet );

            if ( luaRet != 0 )
            {
                ret = ErrorFailedRunningScript;
            }

            if ( mData->GcPeriod != 0 )
            {
                // collect garbage after every Nth run of Main
                mData->GcCounter = ( mData->GcCounter + 1 ) % mData->GcPeriod;

                if ( mData->GcCounter == 0 )
                {
                    lua_gc( mData->LuaState, LUA_GCCOLLECT, 0 );
                }
            }
        }
    }

    return ret;
}

// Set/Get how often to force GC run (after every Nth run of Main(), where N = period. if 0 - let Lua do it)
void XLuaPluginScripting::SetGcPeriod( uint32_t period )
{
    mData->GcPeriod  = period;
    mData->GcCounter = 0;
}
uint32_t XLuaPluginScripting::GcPeriod( )
{
    return mData->GcPeriod;
}
