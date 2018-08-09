/*
    MJPEG streams video source library of Computer Vision Sandbox

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

#include <string.h>
#include "tools.hpp"

using namespace std;

namespace CVSandbox { namespace Video { namespace MJpeg { namespace Private
{

// Search buffer for specified sequence
int MemFind( const uint8_t* buffer, int bufferSize, const uint8_t* searchFor, int searchSize )
{
    uint8_t* ptr;
    uint8_t* searchIn   = const_cast<uint8_t*>( buffer );
    int      searchLeft = bufferSize;
    int      ret = -1;

    while ( ( searchLeft >= searchSize ) && ( ( ptr = static_cast<uint8_t*>( memchr( searchIn, searchFor[0], searchLeft ) ) ) != 0 ) )
    {
        int index = (int)( ptr - buffer );

        if ( bufferSize - index < searchSize )
        {
            // what we are looking for does not fit here for sure
            break;
        }

        if ( memcmp( ptr, searchFor, searchSize ) == 0 )
        {
            ret = index;
            break;
        }

        searchIn   = ptr + 1;
        searchLeft = bufferSize - index - 1;
    }

    return ret;
}

// Extract title of HTTP document
string ExtractTitle( const char* szHtml, bool* pIsHtml )
{
    string title;
    bool   isHtml = false;

    if ( ( strstr( (char*) szHtml, "<HTML>" ) != nullptr ) ||
         ( strstr( (char*) szHtml, "<html>" ) != nullptr ) )
    {
        isHtml = true;

        // check for title
        char* titleStart = strstr( (char*) szHtml, "<TITLE>" );

        if ( titleStart == nullptr )
        {
            titleStart = strstr( (char*) szHtml, "<title>" );
        }

        if ( titleStart != nullptr )
        {
            char* titleEnd = strstr( (char*) szHtml, "</TITLE>" );

            if ( titleEnd == nullptr )
            {
                titleEnd = strstr( (char*) szHtml, "</title>" );
            }

            if ( titleEnd != nullptr )
            {
                title = string( titleStart + 7, titleEnd );
            }
        }
    }

    if ( pIsHtml != nullptr )
    {
        *pIsHtml = isHtml;
    }

    return title;
}

} } } } // namespace CVSandbox::Video::MJpeg::Private
