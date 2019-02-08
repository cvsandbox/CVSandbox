/*
    Computer Vision Sandbox Script Runner application

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

#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <stdio.h>
#include <string.h>
#include <string>
#include <map>
#include <XLuaPluginScripting.hpp>
#include "ScriptingHost.hpp"

using namespace std;
using namespace CVSandbox;

// Application's name and version
const char* APP_NAME    =  "Computer Vision Sandbox Script Runner";
xversion    APP_VERSION = { 1, 0, 0 };

// Some error code the app may return
enum
{
    Error_NoArguments                = -1,
    Error_BadScriptName              = -2,
    Error_CanNotOpenScript           = -3,
    Error_InvalidArgument            = -4,
    Error_FailedScriptLoading        = -5,
    Error_FailedScriptInitialization = -6,
    Error_FailedRunningScript        = -7
};

// Some forward declarations -------
static int CheckArgument( int argc, char* argv[], map<string, string>& scriptArgumensts );
// ---------------------------------

// Let's finally start here
int main( int argc, char* argv[] )
{
    int rc = 0;

    //_CrtSetBreakAlloc(3871);

    {
        map<string, string> scriptArgumensts;

        rc = CheckArgument( argc, argv, scriptArgumensts );

        if ( rc == 0 )
        {
            ScriptingHost         host( scriptArgumensts );
            XLuaPluginScripting   scripting( &host );
            XErrorCode            errorCode = scripting.Init( );

            errorCode = scripting.LoadScriptFromFile( argv[1] );

            if ( errorCode != SuccessCode )
            {
                rc = Error_FailedScriptLoading;
                printf( "Error: Failed loading the script file with the Lua interpreter. \n\n" );
                printf( "%s \n\n", scripting.GetLastErrorMessage( ).c_str( ) );
            }
            else
            {
                errorCode = scripting.InitScript( );

                if ( errorCode != SuccessCode )
                {
                    rc = Error_FailedScriptInitialization;
                    printf( "Error: Failed running initialization part of the script. \n\n" );
                    printf( "%s \n\n", scripting.GetLastErrorMessage( ).c_str( ) );
                }
                else
                {
                    if ( scripting.IsMainDefined( ) == SuccessCode )
                    {
                        errorCode = scripting.RunScript( );

                        if ( errorCode != SuccessCode )
                        {
                            rc = Error_FailedRunningScript;
                            printf( "Error: Failed running the script's Main(). \n\n" );
                            printf( "%s \n\n", scripting.GetLastErrorMessage( ).c_str( ) );
                        }
                    }
                }
            }
        }
    }

    #ifdef _MSC_VER
    _CrtDumpMemoryLeaks();
    #endif

    return rc;
}

// Print application's help and usage info
static void ShowHelp( )
{
    printf( "%s v%d.%d.%d \n", APP_NAME, APP_VERSION.major, APP_VERSION.minor, APP_VERSION.revision );
    printf( "\n" );
    printf( "Usage: cvssr <script_to_run.lua> [script arguments]\n" );
    printf( "\n" );
    printf( "Script arguments is an arbitrary combination of strings starting from '-'. \n" );
    printf( "Every argument starting '-' can be optionally followed by another arbitrary string. \n");
    printf( "Example: cvssr test.lua -i some_option -o another_option -d \n");
    printf( "\n" );
    printf( "Note: If script uses Host.GetImage() API, the engine will automatically search for \n" );
    printf( "      '-i' option followed by image file name and load it. \n" );
    printf( "      If script uses Host.SetImage() API, the engine will automatically search for \n" );
    printf( "      '-o' option followed by image file name and save to it. If missing, the output \n" );
    printf( "      file name will be based on the input file name. \n" );

    printf( "\n" );
}

// Check if the scrip's name is valid and the file exists
static int CheckScriptName( const char* scriptName )
{
    const char* extensionPos = strstr( scriptName, ".lua" );
    int ret = 0;

    if ( ( extensionPos == 0 ) || ( strlen( extensionPos ) != 4 ) )
    {
        ret = Error_BadScriptName;
        printf( "Error: The specified file does not look like a Lua script. Needs to have .lua extension. \n\n" );
    }
    else
    {
        FILE* file = fopen( scriptName, "r" );

        if ( file != 0 )
        {
            fclose( file );
        }
        else
        {
            ret = Error_CanNotOpenScript;
            printf( "Error: Cannot open the script file specified. \n\n" );
        }
    }

    return ret;
}

// Check/process application's arguments
int CheckArgument( int argc, char* argv[], map<string, string>& scriptArgumensts )
{
    int ret = 0;

    scriptArgumensts.clear( );

    if ( argc <= 1 )
    {
        ret = Error_NoArguments;
        ShowHelp( );
    }
    else
    {
        ret = CheckScriptName( argv[1] );

        if ( ret == 0 )
        {
            bool    hyphenRequired = true;
            string  lastOption;

            // now get all the arguments
            for ( int i = 2; i < argc; i++ )
            {
                if ( ( hyphenRequired ) && ( argv[i][0] != '-' ) )
                {
                    printf( "Error: Don't know what to do with \"%s\". If it is an argument, it need to start with hyphen ('-'). \n\n", argv[i] );
                    ret = Error_InvalidArgument;
                    break;
                }

                if ( argv[i][0] == '-' )
                {
                    lastOption = argv[i];

                    if ( scriptArgumensts.find( lastOption ) != scriptArgumensts.end( ) )
                    {
                        printf( "Error: Multiple occurrences of the same parameter (%s) are not supported. \n\n", argv[i] );
                        ret = Error_InvalidArgument;
                        break;
                    }
                    else
                    {
                        scriptArgumensts[lastOption] = string( );
                        hyphenRequired = false;
                    }
                }
                else
                {
                    scriptArgumensts[lastOption] = argv[i];
                    hyphenRequired = true;
                }
            }
        }
    }

    return ret;
}
