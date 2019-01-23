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

#include <QColor>
#include <QPoint>
#include <QSize>
#include "XVariantConverter.hpp"

using namespace CVSandbox;

// Register some custom types
static struct Init
{
    Init( )
    {
        qRegisterMetaType<XRange> ( "XRange"  );
        qRegisterMetaType<XRangeF>( "XRangeF" );
        qRegisterMetaType<XVariantArray>( "XVariantArray" );
        qRegisterMetaType<XVariantArray2d>( "XVariantArray2d" );
    }
}
init;

// Convert X type to Q type
int XVariantConverter::ToQVariantType( XVarType type )
{
    int qtype = QVariant::Invalid;

    if ( ( type & XVT_Array ) == XVT_Array )
    {
        qtype = QMetaType::type( "XVariantArray" );
    }
    else if ( ( type & XVT_Array2d ) == XVT_Array2d )
    {
        qtype = QMetaType::type( "XVariantArray2d" );
    }
    else
    {
        switch ( type )
        {
            // convert all unsigned integer types to signed,
            // sing Property Editor does not support UInt for now
            case XVT_U1:
            case XVT_U2:
            case XVT_U4:
                qtype = QVariant::Int;
                break;

            case XVT_I1:
            case XVT_I2:
            case XVT_I4:
                qtype = QVariant::Int;
                break;

            case XVT_R4:
            case XVT_R8:
                qtype = QVariant::Double;
                break;

            case XVT_Bool:
                qtype = QVariant::Bool;
                break;

            case XVT_String:
                qtype = QVariant::String;
                break;

            case XVT_ARGB:
                qtype = QVariant::Color;
                break;

            case XVT_Point:
                qtype = QVariant::Point;
                break;

            case XVT_Size:
                qtype = QVariant::Size;
                break;

            case XVT_Range:
                qtype = QMetaType::type( "XRange" );
                break;
            case XVT_RangeF:
                qtype = QMetaType::type( "XRangeF" );
                break;
        }
    }

    return qtype;
}

// Convert XVariant to QVariant value
const QVariant XVariantConverter::ToQVariant( const XVariant& value )
{
    QVariant qValue;
    const xvariant* var = value;

    if ( ( var->type & XVT_Array ) == XVT_Array )
    {
        qValue.setValue<XVariantArray>( value.ToArray( ) );
    }
    else if ( ( var->type & XVT_Array2d ) == XVT_Array2d )
    {
        qValue.setValue<XVariantArray2d>( value.ToArray2d( ) );
    }
    else
    {
        switch ( var->type )
        {
            case XVT_U1:
                qValue = QVariant( (int) var->value.ubVal );
                break;
            case XVT_U2:
                qValue = QVariant( (int) var->value.usVal );
                break;
            case XVT_U4:
                qValue = QVariant( (int) var->value.uiVal );
                break;

            case XVT_I1:
                qValue = QVariant( (int) var->value.bVal );
                break;
            case XVT_I2:
                qValue = QVariant( (int) var->value.sVal );
                break;
            case XVT_I4:
                qValue = QVariant( (int) var->value.iVal );
                break;

            case XVT_R4:
                qValue = QVariant( (double) var->value.fVal );
                break;
            case XVT_R8:
                qValue = QVariant( var->value.dVal );
                break;

            case XVT_Bool:
                qValue = QVariant( var->value.boolVal );
                break;

            case XVT_String:
                qValue = QVariant( QString::fromUtf8( var->value.strVal ) );
                break;

            case XVT_ARGB:
                qValue = QVariant( QColor(
                                       var->value.argbVal.components.r,
                                       var->value.argbVal.components.g,
                                       var->value.argbVal.components.b,
                                       var->value.argbVal.components.a ) );
                break;

            case XVT_Point:
                qValue = QPoint( var->value.pointVal.x, var->value.pointVal.y );
                break;

            case XVT_Size:
                qValue = QSize( var->value.sizeVal.width, var->value.sizeVal.height );
                break;

            case XVT_Range:
                qValue.setValue<XRange>( value.ToRange( ) );
                break;
            case XVT_RangeF:
                qValue.setValue<XRangeF>( value.ToRangeF( ) );
                break;
        }
    }
    return qValue;
}

// Convert QVariant to XVariant value
const XVariant XVariantConverter::FromQVariant( const QVariant& qvalue )
{
    XVariant value;
    int type = qvalue.type( );

    switch ( type )
    {
        case QVariant::Int:
            value = XVariant( (int32_t) qvalue.toInt( ) );
            break;

        case QVariant::UInt:
            value = XVariant( (uint32_t) qvalue.toUInt( ) );
            break;

        case QVariant::Double:
            value = XVariant( qvalue.toDouble( ) );
            break;

        case QVariant::Bool:
            value = XVariant( qvalue.toBool( ) );
            break;

        case QVariant::String:
            value = XVariant( qvalue.toString( ).toUtf8( ).data( ) );
            break;

        case QVariant::Color:
            {
                QColor color = qvalue.value<QColor>( );
                value = XVariant( XColor( color.red( ), color.green( ), color.blue( ), color.alpha( ) ) );
            }
            break;

        case QVariant::Point:
            {
                QPoint point = qvalue.value<QPoint>( );
                value = XVariant( XPoint( point.x( ), point.y( ) ) );
            }
            break;

        case QVariant::Size:
            {
                QSize size = qvalue.value<QSize>( );
                value = XVariant( XSize( size.width( ), size.height( ) ) );
            }
            break;

        case QVariant::UserType:

            if ( qvalue.canConvert<XRange>( ) )
            {
                value = XVariant( qvariant_cast<XRange>( qvalue ) );
            }
            else if ( qvalue.canConvert<XRangeF>( ) )
            {
                value = XVariant( qvariant_cast<XRangeF>( qvalue ) );
            }
            else if ( qvalue.canConvert<XVariantArray>( ) )
            {
                value = XVariant( qvariant_cast<XVariantArray>( qvalue ) );
            }
            else if ( qvalue.canConvert<XVariantArray2d>( ) )
            {
                value = XVariant( qvariant_cast<XVariantArray2d>( qvalue ) );
            }
            break;
    }

    return value;
}
