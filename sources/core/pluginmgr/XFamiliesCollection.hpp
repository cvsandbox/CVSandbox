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

#pragma once
#ifndef CVS_XFAMILES_COLLECTION_HPP
#define CVS_XFAMILES_COLLECTION_HPP

#include <memory>
#include <map>
#include <stdint.h>
#include <XInterfaces.hpp>
#include <XGuid.hpp>

#include "XFamily.hpp"
#include "XMapValuesConstIterator.hpp"

class XFamiliesCollection : private CVSandbox::Uncopyable
{
private:
    XFamiliesCollection( );

public:
    ~XFamiliesCollection( );

    // Create empty collection
    static const std::shared_ptr<XFamiliesCollection> Create( );
    // Create collection with built-in families
    static const std::shared_ptr<XFamiliesCollection> GetBuiltInFamilies( );
    // Create copy of the collection
    const std::shared_ptr<XFamiliesCollection> Copy( ) const;

    // Clear collection
    void Clear( );
    // Add new family to the collection
    void Add( const std::shared_ptr<const XFamily>& family );
    // Get families count in the collection
    int32_t Count( ) const { return mFamilies.size( ); }

    // Get family by its ID
    const std::shared_ptr<const XFamily> GetFamily( const CVSandbox::XGuid& id ) const;

    // Enumeration API
    typedef XMapValuesConstIterator<CVSandbox::XGuid, std::shared_ptr<const XFamily> > ConstIterator;
    ConstIterator begin( ) const { return ConstIterator( mFamilies.begin( ) ); }
    ConstIterator end( ) const { return ConstIterator( mFamilies.end( ) ); }

private:
    std::map<CVSandbox::XGuid, std::shared_ptr<const XFamily> > mFamilies;
};

#endif // CVS_XFAMILES_COLLECTION_HPP
