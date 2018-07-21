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

#pragma once
#ifndef CVS_XLIST_H
#define CVS_XLIST_H

// Contains definition of a basic single linked list structures/functions.

#include "xerrors.h"

#ifdef __cplusplus
extern "C" {
#endif

// Define node of the single linked list
typedef struct xlistnode
{
    const void* data;
    struct xlistnode* next;
}
xlistnode;

// Define structure of the single linked list
typedef struct
{
    xlistnode* head;
    xlistnode* tail;
}
xlist;

// Create initial linked list
xlist* XListCreate( );
// Free the specfied linked list and all its node if any
void XListFree( xlist* list );
// Add node to the end of the specified list
XErrorCode XListAddTail( xlist* list, const void* data );
// Add node at the beginning of the specified list
XErrorCode XListAddHead( xlist* list, const void* data );
// Count number of nodes in the specifies list
uint32_t XListCount( xlist* list );

#ifdef __cplusplus
}
#endif

#endif // CVS_XLIST_H
