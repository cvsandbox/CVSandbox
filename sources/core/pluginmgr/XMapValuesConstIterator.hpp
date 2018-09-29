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
#ifndef CVS_XMAP_VALUES_CONST_ITERATOR_HPP
#define CVS_XMAP_VALUES_CONST_ITERATOR_HPP

#include <map>

// Custom iterator aimed for enumerating values of a map - it wraps
// map's const iterator and provides access to values only.
// http://www.drdobbs.com/cpp/184401929
//
template< class KeyType, class ValueType, class Comparer = std::less<KeyType> > class XMapValuesConstIterator :
    public std::iterator<std::forward_iterator_tag, ValueType>
{
public:
    // Type of the base iterator to wrap
    typedef typename std::map<KeyType, ValueType, Comparer>::const_iterator BaseIterator;

    XMapValuesConstIterator( ) : baseIterator( ) { }

    // Construct from base iterator
    XMapValuesConstIterator( BaseIterator iterator ) : baseIterator( iterator ) { }

    // Copy constructor
    XMapValuesConstIterator( const XMapValuesConstIterator& rhs ) : baseIterator( rhs.baseIterator ) { }

    // Assignment operator
    XMapValuesConstIterator& operator= ( const XMapValuesConstIterator& rhs ) { baseIterator = rhs.baseIterator; return*this;  }

    // Pre-fix increment
    XMapValuesConstIterator& operator++( )
    {
        ++baseIterator;
        return ( *this );
    }

    // Post-fix increment
    XMapValuesConstIterator operator++( int )
    {
        XMapValuesConstIterator result( *this );
        baseIterator++;
        return result;
    }

    // Check if two iterators are equal
    bool operator==( const XMapValuesConstIterator& rhs ) const
    {
        return ( baseIterator == rhs.baseIterator ) ;
    }

    // Check if two iterators are NOT equal
    bool operator!=( const XMapValuesConstIterator& rhs  ) const
    {
        return ( !( (*this) == rhs ) ) ;
    }

    // Get pointer to the map's value
    const ValueType* operator ->()
    {
        return &(baseIterator->second);
    }

    // Get reference to the map's value
    const ValueType& operator *()
    {
        return baseIterator->second;
    }

private:
    BaseIterator baseIterator;
};

#endif // CVS_XMAP_VALUES_CONST_ITERATOR_HPP
