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

#include "LuaScriptingEnginePlugin.hpp"
#include <XLuaPluginScripting.hpp>
#include <IScriptingHost.hpp>
#include <XPluginWrapperFactory.hpp>

using namespace std;
using namespace CVSandbox;

namespace Private
{
    class ScriptingHostWrapper : public IScriptingHost
    {
    public:
        ScriptingHostWrapper( ) :
            mCallbacks( { 0 } ), mUserParam( nullptr )
        {

        }

        void SetCallbacks( const ScriptingEnginePluginCallbacks* callbacks, void* userParam )
        {
            mCallbacks = *callbacks;
            mUserParam = userParam;
        }

        const string Name( ) const
        {
            string name;

            if ( mCallbacks.GetHostName == nullptr )
            {
                name = string( "me" );
            }
            else
            {
                xstring xname = mCallbacks.GetHostName( mUserParam );

                if ( xname != nullptr )
                {
                    name = string( xname );
                    XStringFree( &xname );
                }
            }

            return name;
        }

        const XVersion Version( ) const
        {
            XVersion version;

            if ( mCallbacks.GetHostVersion != nullptr )
            {
                xversion xversion;

                mCallbacks.GetHostVersion( mUserParam, &xversion );
                version = xversion;
            }

            return version;
        }

        const map<string, string> GetArguments( ) const
        {
            return map<string, string>( );
        }

        void PrintString( const string& message ) const
        {
            if ( mCallbacks.PrintString != nullptr )
            {
                mCallbacks.PrintString( mUserParam, message.c_str( ) );
            }
        }

        XErrorCode CreatePluginInstance( const string& pluginName, shared_ptr<const XPluginDescriptor>& descriptor, shared_ptr<XPlugin>& plugin )
        {
            XErrorCode ecode = ErrorInvalidConfiguration;

            if ( mCallbacks.CreatePluginInstance != nullptr )
            {
                PluginDescriptor*   pDescriptor = nullptr;
                void*               pPlugin     = nullptr;

                ecode = mCallbacks.CreatePluginInstance( mUserParam, pluginName.c_str( ), &pDescriptor, &pPlugin );

                if ( ecode == SuccessCode )
                {
                    descriptor = XPluginDescriptor::Create( pDescriptor );
                    plugin     = XPluginWrapperFactory::CreateWrapper( pPlugin, pDescriptor->Type );

                }
            }

            return ecode;
        }

        XErrorCode GetImage( shared_ptr<XImage>& image ) const
        {
            XErrorCode ecode = ErrorInvalidConfiguration;

            if ( mCallbacks.GetImage != nullptr )
            {
                ximage* xImage = nullptr;

                ecode = mCallbacks.GetImage( mUserParam, &xImage );

                if ( ecode == SuccessCode )
                {
                    image = XImage::Create( &xImage, true );

                    if ( !image )
                    {
                        ecode = ErrorOutOfMemory;
                    }
                }
            }

            return ecode;
        }

        XErrorCode SetImage( const shared_ptr<XImage>& image )
        {
            XErrorCode ecode = ErrorInvalidConfiguration;

            if ( mCallbacks.SetImage != nullptr )
            {
                ecode = mCallbacks.SetImage( mUserParam, image->ImageData( ) );
            }

            return ecode;
        }

        XErrorCode GetVariable( const string& name, XVariant& value ) const
        {
            XErrorCode ecode = ErrorInvalidConfiguration;

            if ( mCallbacks.GetVariable != nullptr )
            {
                xvariant var;

                XVariantInit( &var );

                ecode = mCallbacks.GetVariable( mUserParam, name.c_str( ), &var );

                if ( ecode == SuccessCode )
                {
                    value = var;
                }

                XVariantClear( &var );
            }

            return ecode;
        }

        XErrorCode SetVariable( const string& name, const XVariant& value )
        {
            XErrorCode ecode = ErrorInvalidConfiguration;

            if ( mCallbacks.SetVariable != nullptr )
            {
                ecode = mCallbacks.SetVariable( mUserParam, name.c_str( ), value );
            }

            return ecode;
        }

        XErrorCode GetVariable( const string& name, shared_ptr<XImage>& value ) const
        {
            XErrorCode ecode = ErrorInvalidConfiguration;

            if ( mCallbacks.GetImageVariable != nullptr )
            {
                ximage* ximage = nullptr;

                value.reset( );

                ecode = mCallbacks.GetImageVariable( mUserParam, name.c_str( ), &ximage );

                if ( ( ecode == SuccessCode ) && ( ximage != nullptr ) )
                {
                    value = XImage::Create( &ximage, true );
                }
            }

            return ecode;
        }

        XErrorCode SetVariable( const string& name, const shared_ptr<XImage>& value )
        {
            XErrorCode ecode = ErrorInvalidConfiguration;

            if ( mCallbacks.SetImageVariable != nullptr )
            {
                ecode = mCallbacks.SetImageVariable( mUserParam, name.c_str( ), value->ImageData( ) );
            }

            return ecode;
        }

        XErrorCode GetVideoSource( shared_ptr<const XPluginDescriptor>& descriptor, shared_ptr<XPlugin>& plugin ) const
        {
            XErrorCode ecode = ErrorInvalidConfiguration;

            if ( mCallbacks.GetVideoSource != nullptr )
            {
                PluginDescriptor*   pDescriptor = nullptr;
                void*               pPlugin     = nullptr;

                ecode = mCallbacks.GetVideoSource( mUserParam, &pDescriptor, &pPlugin );

                if ( ecode == SuccessCode )
                {
                    descriptor = XPluginDescriptor::Create( pDescriptor );
                    plugin     = XPluginWrapperFactory::CreateWrapper( pPlugin, pDescriptor->Type, false );
                }
            }

            return ecode;
        }

    private:
        ScriptingEnginePluginCallbacks  mCallbacks;
        void*                           mUserParam;
    };



    class LuaScriptingEnginePluginData
    {
    public:
        LuaScriptingEnginePluginData( ) :
            ScriptFile( ), ScriptingHost( ), ScriptingEngine( &ScriptingHost ), RunningScriptFileName( )
        {

        }

    public:
        string                  ScriptFile;
        ScriptingHostWrapper    ScriptingHost;
        XLuaPluginScripting     ScriptingEngine;
        string                  RunningScriptFileName;
    };
}

LuaScriptingEnginePlugin::LuaScriptingEnginePlugin( ) :
    mData( new ::Private::LuaScriptingEnginePluginData( ) )
{
}

LuaScriptingEnginePlugin::~LuaScriptingEnginePlugin( )
{
    delete mData;
}

void LuaScriptingEnginePlugin::Dispose( )
{
    delete this;
}

// Get specified property value of the plug-in
XErrorCode LuaScriptingEnginePlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type = XVT_String;
        value->value.strVal = XStringAlloc( mData->ScriptFile.c_str( ) );
        break;

    case 1:
        value->type = XVT_U4;
        value->value.uiVal = mData->ScriptingEngine.GcPeriod( );
        break;

    default:
        ret = ErrorInvalidProperty;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode LuaScriptingEnginePlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode  ret = ErrorFailed;
    xvariant    convertedValue;

    XVariantInit( &convertedValue );

    // make sure property value has expected type
    ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 2, &convertedValue );

    if ( ret == SuccessCode )
    {
        XVariant xvar( convertedValue );

        switch ( id )
        {
        case 0:
            mData->ScriptFile = xvar.ToString( );
            break;

        case 1:
            mData->ScriptingEngine.SetGcPeriod( xvar.ToUInt( ) );
            break;

        default:
            ret = ErrorInvalidProperty;
            break;
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}


// Set callback to interface with the calling host
void LuaScriptingEnginePlugin::SetCallbacks( const ScriptingEnginePluginCallbacks* callbacks, void* userParam )
{
    mData->ScriptingHost.SetCallbacks( callbacks, userParam );
}

// Get default file name extension for the engine's scripts
xstring LuaScriptingEnginePlugin::GetDefaultExtension( )
{
    return XStringAlloc( "lua" );
}

// Perform any necessary initialization of the scripting engine
XErrorCode LuaScriptingEnginePlugin::Init( )
{
    return mData->ScriptingEngine.Init( );
}

// Set script file to be loaded (this might be also exposed as plug-ins property,
// but duplicated for clients who would like to rely on the set interface)
void LuaScriptingEnginePlugin::SetScriptFile( xstring fileName )
{
    mData->ScriptFile = fileName;
}

// Load previously set script from file
XErrorCode LuaScriptingEnginePlugin::LoadScript( )
{
    size_t slashPos = mData->ScriptFile.rfind( '/' );

    if ( slashPos == string::npos )
    {
        slashPos = mData->ScriptFile.rfind( '\\' );
    }

    if ( slashPos == string::npos )
    {
        mData->RunningScriptFileName = mData->ScriptFile;
    }
    else
    {
        mData->RunningScriptFileName = mData->ScriptFile.substr( slashPos + 1 );
    }

    return mData->ScriptingEngine.LoadScriptFromFile( mData->ScriptFile );
}

// Run initialization part of the loaded script (any code outside of Main)
XErrorCode LuaScriptingEnginePlugin::InitScript( )
{
    return mData->ScriptingEngine.InitScript( );
}

// Run the Main function of the script
XErrorCode LuaScriptingEnginePlugin::RunScript( )
{
    return mData->ScriptingEngine.RunScript( );
}

// Get error message which scripting engine may generate if LoadScript() returns ErrorFailedLoadingScript or InitScript()/RunScript() return ErrorFailedRunningScript
xstring LuaScriptingEnginePlugin::GetLastErrorMessage( )
{
    string errorMessage = mData->ScriptingEngine.GetLastErrorMessage( );
    size_t fileNamePos  = errorMessage.find( mData->RunningScriptFileName );

    if ( fileNamePos != string::npos )
    {
        errorMessage = errorMessage.substr( fileNamePos );
    }

    return XStringAlloc( errorMessage.c_str( ) );
}

