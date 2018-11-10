/*
    Scripting test application to check interfacing with plug-ins from Lua

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

#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <stdio.h>
#include <XError.hpp>

#include <XLuaPluginScripting.hpp>
#include <XDefaultScriptingHost.hpp>

using namespace std;
using namespace CVSandbox;

int main( int argc, char* argv[] )
{
    // _CrtSetBreakAlloc(229);

    printf( "Testing scripting engine ...\r\n" );
    {
        XDefaultScriptingHost host      = { map<string, string>( ) };
        XLuaPluginScripting   scripting( &host );
        XErrorCode            errorCode = scripting.Init( );

        errorCode = scripting.LoadScriptFromFile( "test1.lua" );

        if ( errorCode == SuccessCode )
        {
            printf( "Script is loaded \n" );

            errorCode = scripting.InitScript( );

            if ( errorCode == SuccessCode )
            {
                printf( "Script is initialized \n" );

                errorCode = scripting.RunScript( );

                if ( errorCode == SuccessCode )
                {
                    printf( "Script is done \n" );

                    errorCode = scripting.RunScript( );
                    errorCode = scripting.RunScript( );
                }
            }
        }

        // run second script using same scripting instance
        if ( errorCode == SuccessCode )
        {
            errorCode = scripting.LoadScriptFromFile( "test2.lua" );

            if ( errorCode == SuccessCode )
            {
                printf( "Script is loaded \n" );

                errorCode = scripting.InitScript( );

                if ( errorCode == SuccessCode )
                {
                    printf( "Script is initialized \n" );

                    errorCode = scripting.RunScript( );
                }
            }
        }

        if ( errorCode != SuccessCode )
        {
            printf( "Error: %s\n", XError::Description( errorCode ).c_str( ) );
            if ( ( errorCode == ErrorFailedRunningScript ) || ( errorCode == ErrorFailedLoadingScript ) )
            {
                printf( "Scripting error message: %s\n", scripting.GetLastErrorMessage( ).c_str( ) );
            }
        }
    }

    printf( "\n\nDone\n" );
    getchar( );

    #ifdef _MSC_VER
    _CrtDumpMemoryLeaks();
    #endif

    return 0;
}
