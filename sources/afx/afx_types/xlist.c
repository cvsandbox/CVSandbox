/*
    Core types library of Computer Vision Sandbox

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

#include <stdlib.h>
#include "xtypes.h"
#include "xlist.h"

// Create initial linked list
xlist* XListCreate( )
{
    return (xlist*) XCAlloc( 1, sizeof( xlist ) );
}

// Free the specfied linked list and all its node if any
void XListFree( xlist* list )
{
    if ( list != 0 )
    {
        xlistnode* node = list->head;

        while ( node != 0 )
        {
            xlistnode* next = node->next;
            XFree( (void**) &node );
            node = next;
        }

        XFree( (void**) &list );
    }
}

// Add node to the end of the specified list
XErrorCode XListAddTail( xlist* list, const void* data )
{
    XErrorCode ret = SuccessCode;

    if ( list == 0 )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        xlistnode* node = (xlistnode*) XCAlloc( 1, sizeof( xlistnode ) );

        if ( node == 0 )
        {
            ret = ErrorOutOfMemory;
        }
        else
        {
            node->data = data;

            if ( list->head == 0 )
            {
                // empty list so far
                list->head = node;
            }
            else
            {
                list->tail->next = node;
            }

            list->tail = node;
        }
    }

    return ret;
}

// Add node at the beginning of the specified list
XErrorCode XListAddHead( xlist* list, const void* data )
{
    XErrorCode ret = SuccessCode;

    if ( list == 0 )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        xlistnode* node = (xlistnode*) XCAlloc( 1, sizeof( xlistnode ) );

        if ( node == 0 )
        {
            ret = ErrorOutOfMemory;
        }
        else
        {
            node->data = data;
            node->next = list->head;

            list->head = node;

            if ( list->tail == 0 )
            {
                // empty list so far
                list->tail = node;
            }
        }
    }
    return ret;
}

// Count number of nodes in the specifies list
uint32_t XListCount( xlist* list )
{
    uint32_t count = 0;

    if ( list != 0 )
    {
        xlistnode* node = list->head;

        while ( node != 0 )
        {
            count++;
            node = node->next;
        }
    }

    return count;
}
