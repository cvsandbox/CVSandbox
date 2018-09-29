/*
    Plug-ins' management library of Computer Vision Sandbox

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

#include "XFamiliesCollection.hpp"

using namespace std;
using namespace CVSandbox;

XFamiliesCollection::XFamiliesCollection( ) :
    mFamilies( )
{
}

XFamiliesCollection::~XFamiliesCollection( )
{
    Clear( );
}

// Create empty collection
const shared_ptr<XFamiliesCollection> XFamiliesCollection::Create( )
{
    return shared_ptr<XFamiliesCollection>( new XFamiliesCollection( ) );
}

// Create collection with default families
const shared_ptr<XFamiliesCollection> XFamiliesCollection::GetBuiltInFamilies( )
{
    shared_ptr<XFamiliesCollection> families = Create( );

    if ( families )
    {
        for ( int i = 0; i < BuiltInPluginFamilitesCount; i++ )
        {
            families->Add( XFamily::Create( BuiltInPluginFamilites[i] ) );
        }
    }

    return families;
}

// Create copy of the collection
const shared_ptr<XFamiliesCollection> XFamiliesCollection::Copy( ) const
{
    shared_ptr<XFamiliesCollection> families = Create( );

    if ( families )
    {
        for ( map<XGuid, shared_ptr<const XFamily> >::const_iterator i = mFamilies.begin( ); i != mFamilies.end( ); i++ )
        {
            families->mFamilies.insert( pair<XGuid, shared_ptr<const XFamily> >( i->first, i->second ) );
        }
    }

    return families;
}

// Clear collection
void XFamiliesCollection::Clear( )
{
    mFamilies.clear( );
}

// Add new family to the collection
void XFamiliesCollection::Add( const shared_ptr<const XFamily>& family )
{
    if ( family )
    {
        // check if this collection already contains this family
        map<XGuid, shared_ptr<const XFamily> >::const_iterator it = mFamilies.find( family->ID( ) );

        if  ( it == mFamilies.end( ) )
        {
            mFamilies.insert( pair<XGuid, shared_ptr<const XFamily> >( family->ID( ), family ) );
        }
    }
}

// Get families count in the collection
const shared_ptr<const XFamily> XFamiliesCollection::GetFamily( const XGuid& id ) const
{
    shared_ptr<const XFamily> family;
    map<XGuid, shared_ptr<const XFamily> >::const_iterator it = mFamilies.find( id );

    if  ( it != mFamilies.end( ) )
    {
        family = it->second;
    }

    return family;
}
