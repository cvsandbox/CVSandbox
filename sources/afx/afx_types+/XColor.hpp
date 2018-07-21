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
#ifndef CVS_XCOLOR_HPP
#define CVS_XCOLOR_HPP

#include <xtypes.h>

namespace CVSandbox
{

class XColor
{
public:
    // Constructors
    XColor( uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255 );
    XColor( uint32_t argbValue = 0xFF000000 );
    XColor( const XColor& rhs );
    XColor( const xargb& rhs );

    // Assignment operators
    XColor& operator= ( const XColor& rhs );
    XColor& operator= ( const xargb& rhs );

    operator xargb() const { return mColor; }

    // Accessors
    uint8_t Red( )   const { return mColor.components.r; }
    uint8_t Green( ) const { return mColor.components.g; }
    uint8_t Blue( )  const { return mColor.components.b; }
    uint8_t Alpha( ) const { return mColor.components.a; }

    // Setters
    void SetRed  ( uint8_t red )   { mColor.components.r = red;   }
    void SetGreen( uint8_t green ) { mColor.components.g = green; }
    void SetBlue ( uint8_t blue )  { mColor.components.b = blue;  }
    void SetAlpha( uint8_t alpha ) { mColor.components.a = alpha; }

private:
    xargb mColor;

public:
    // Define some common colors
    class Common
    {
    private:
        Common( );

    public:
        static const XColor Black( )     { return XColor( 0xFF000000 ); }
        static const XColor White( )     { return XColor( 0xFFFFFFFF ); }
        static const XColor Gray( )      { return XColor( 0xFF808080 ); }

        static const XColor Red( )       { return XColor( 0xFFFF0000 ); }
        static const XColor Green( )     { return XColor( 0xFF00FF00 ); }
        static const XColor Blue( )      { return XColor( 0xFF0000FF ); }

        static const XColor Yellow( )    { return XColor( 0xFFFFFF00 ); }
        static const XColor Cyan( )      { return XColor( 0xFF00FFFF ); }
        static const XColor Magenta ( )  { return XColor( 0xFFFF00FF ); }
    };
};

} // namespace CVSandbox

#endif // CVS_XCOLOR_HPP
