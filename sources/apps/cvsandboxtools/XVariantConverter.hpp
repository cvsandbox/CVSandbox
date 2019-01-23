/*
    Common tools for Computer Vision Sandbox

    Copyright (C) 2011-2019, cvsandbox
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
#ifndef CVS_XVARIANT_CONVERTER_HPP
#define CVS_XVARIANT_CONVERTER_HPP

#include <QVariant>
#include <XVariant.hpp>
#include <XVariantArray.hpp>
#include <XVariantArray2d.hpp>
#include <QMetaType>

Q_DECLARE_METATYPE( CVSandbox::XRange )
Q_DECLARE_METATYPE( CVSandbox::XRangeF )
Q_DECLARE_METATYPE( CVSandbox::XVariantArray )
Q_DECLARE_METATYPE( CVSandbox::XVariantArray2d )

class XVariantConverter
{
private:
    XVariantConverter( );

public:
    // Convert X type to Q type
    static int ToQVariantType( XVarType type );

    // Convert XVariant to QVariant value
    static const QVariant ToQVariant( const CVSandbox::XVariant& value );

    // Convert QVariant to XVariant value
    static const CVSandbox::XVariant FromQVariant( const QVariant& qvalue );
};

#endif // CVS_XVARIANT_CONVERTER_HPP
