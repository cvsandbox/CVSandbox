/*
    Core C++ types library of Computer Vision Sandbox

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
#ifndef CVS_XINTERFACES_HPP
#define CVS_XINTERFACES_HPP

#include <xtypes.h>

namespace CVSandbox
{

// Base class used by classes which should be forbidden for copy constructor and assignment operator
class Uncopyable
{
protected:
    Uncopyable( ) { }
    ~Uncopyable( ) { }

private:
    Uncopyable( const Uncopyable& );
    Uncopyable& operator= ( const Uncopyable& );
};

// Template structure to check if certain type (T) is derived from certain base class (B).
// If T is not B, then it fails during compile time.
// http://www2.research.att.com/~bs/bs_faq2.html#constraints
//
template<class T, class B> struct IsDerivedFrom
{
    static void constraints( T* p )
    {
        B* pb = p;
        XUNREFERENCED_PARAMETER( pb );
    }

    IsDerivedFrom( )
    {
        void(*p)(T*) = constraints;
        XUNREFERENCED_PARAMETER( p );
    }
};

} // namespace CVSandbox

#endif // CVS_XINTERFACES_HPP
