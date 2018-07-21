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

#include "XColor.hpp"

namespace CVSandbox
{

XColor::XColor( uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha )
{
    mColor.components.r = red;
    mColor.components.g = green;
    mColor.components.b = blue;
    mColor.components.a = alpha;
}

XColor::XColor( uint32_t argbValue )
{
    mColor.argb = argbValue;
}

XColor::XColor( const XColor& rhs ) :
    mColor( rhs.mColor )
{
}

XColor::XColor( const xargb& rhs ) :
    mColor( rhs )
{
}

XColor& XColor::operator=( const XColor& rhs )
{
    mColor = rhs.mColor;
    return *this;
}

XColor& XColor::operator=( const xargb& rhs )
{
    mColor = rhs;
    return *this;
}

} // namespace CVSandbox
