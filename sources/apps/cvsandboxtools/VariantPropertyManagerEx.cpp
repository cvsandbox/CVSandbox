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

#include <assert.h>
#include "VariantPropertyManagerEx.hpp"
#include "XVariantConverter.hpp"

using namespace CVSandbox;

// Some custom types' IDs
#define TYPE_RANGE  (QMetaType::type( "XRange" ))
#define TYPE_RANGEF (QMetaType::type( "XRangeF" ))

// Class constructor
VariantPropertyManagerEx::VariantPropertyManagerEx( QObject *parent ) :
    QtVariantPropertyManager( parent )
{
    connect( this, SIGNAL( valueChanged( QtProperty*, const QVariant& ) ),
             this, SLOT( slotValueChanged( QtProperty*, const QVariant& ) ) );

    connect( this, SIGNAL( propertyDestroyed( QtProperty* ) ),
             this, SLOT( slotPropertyDestroyed( QtProperty* ) ) );
}

// Class destructor
VariantPropertyManagerEx::~VariantPropertyManagerEx( )
{
}

// Check if the control supports specified property type
bool VariantPropertyManagerEx::isPropertyTypeSupported( int propertyType ) const
{
    bool ret = false;

    if ( ( propertyType == TYPE_RANGE ) || ( propertyType == TYPE_RANGEF ) )
    {
        ret = true;
    }
    else
    {
        ret = QtVariantPropertyManager::isPropertyTypeSupported( propertyType );
    }

    return ret;
}

// Get type of value for the given type of property
int VariantPropertyManagerEx::valueType( int propertyType ) const
{
    int ret = 0;

    if ( propertyType == TYPE_RANGE )
    {
        ret = TYPE_RANGE;
    }
    else if ( propertyType == TYPE_RANGEF )
    {
        ret = TYPE_RANGEF;
    }
    else
    {
        ret = QtVariantPropertyManager::valueType( propertyType );
    }

    return ret;
}

// Get value of the specified property
QVariant VariantPropertyManagerEx::value( const QtProperty* property ) const
{
    if ( propertyToData.contains( property ) )
    {
        return propertyToData[property].value;
    }

    return QtVariantPropertyManager::value( property );
}

// Get text value of the specified property
QString VariantPropertyManagerEx::valueText( const QtProperty* property ) const
{
    QString str;

    if ( propertyToData.contains( property ) )
    {
        Data data = propertyToData[property];

        if ( ( data.type == TYPE_RANGE ) || ( data.type == TYPE_RANGEF ) )
        {
            str = getRangeTextValue( data.value );
        }
    }
    else
    {
        str = QtVariantPropertyManager::valueText( property );
    }

    return str;
}

// Set value of the specified property
void VariantPropertyManagerEx::setValue( QtProperty* property, const QVariant& value )
{
    if ( propertyToData.contains( property ) )
    {
        int type = propertyToData[property].type;

        if ( ( type == TYPE_RANGE ) || ( type == TYPE_RANGEF ) )
        {
            setRangeValue( property, value );
        }
    }
    else
    {
        QtVariantPropertyManager::setValue( property, value );
    }
}

// Set attribute for the specified property
void VariantPropertyManagerEx::setAttribute( QtProperty* property,
                const QString& attribute, const QVariant& value )
{
    if ( propertyToData.contains( property ) )
    {
        int type = propertyToData[property].type;

        if ( ( type == TYPE_RANGE ) || ( type == TYPE_RANGEF ) )
        {
            setRangeAttribute( property, attribute, value );
        }
    }
    else
    {
        QtVariantPropertyManager::setAttribute( property, attribute, value );
    }
}


// Handle change of a child property
void VariantPropertyManagerEx::slotValueChanged( QtProperty* property, const QVariant& value )
{
    if ( childToProperty.contains( property ) )
    {
        QtProperty* parent = childToProperty[property];

        if ( propertyToData.contains( parent ) )
        {
            Data data = propertyToData[parent];
            QVariant updatedValue;

            if ( ( data.type == TYPE_RANGE ) || ( data.type == TYPE_RANGEF ) )
            {
                updatedValue = getUpdatedRangeValue( property, value, data );
                setRangeValue( parent, updatedValue );
            }

            if ( !updatedValue.isNull( ) )
            {
                emit propertyChanged( parent );
                emit valueChanged( parent, updatedValue );
            }
        }
    }
}

// Handle destruction of child property
void VariantPropertyManagerEx::slotPropertyDestroyed( QtProperty* property )
{
    if ( childToProperty.contains( property ) )
    {
        QtProperty* parent = childToProperty[property];

        Data data = propertyToData[parent];
        data.children.removeAll( static_cast<QtVariantProperty*>( property  ) );
        propertyToData[parent] = data;

        childToProperty.remove( property );
    }
}

// Initialize the specified property
void VariantPropertyManagerEx::initializeProperty( QtProperty *property )
{
    if ( propertyType( property ) == TYPE_RANGE )
    {
        initializeRangeProperty( property, TYPE_RANGE );
    }
    else if ( propertyType( property ) == TYPE_RANGEF )
    {
        initializeRangeProperty( property, TYPE_RANGEF );
    }

    QtVariantPropertyManager::initializeProperty( property );
}

// Uninitialize the specified property
void VariantPropertyManagerEx::uninitializeProperty( QtProperty *property )
{
    if ( propertyToData.contains( property ) )
    {
        Data data = propertyToData[property];

        for ( QList<QtVariantProperty*>::Iterator it = data.children.begin( ); it != data.children.end( ); it++ )
        {
            childToProperty.remove( *it );
        }

        propertyToData.remove( property );
    }

    QtVariantPropertyManager::uninitializeProperty( property );
}

// Initialize property of Range type
void VariantPropertyManagerEx::initializeRangeProperty( QtProperty* property, int type )
{
    Data data;
    int  childType = ( type == TYPE_RANGEF ) ? QVariant::Double : QVariant::Int;

    // set type
    data.type = type;

    // set initial value
    XRange   defaultValue ( 0, 1 );
    XRangeF  defaultValueF( 0, 1 );
    QVariant value;

    if ( type == TYPE_RANGE )
    {
        value.setValue( defaultValue );
    }
    else if ( type == TYPE_RANGEF )
    {
        value.setValue( defaultValueF );
    }
    data.value = value;

    // add "Min" property
    QtVariantProperty* child = addProperty( childType );
    child->setPropertyName( tr( "Min" ) );

    if ( type == TYPE_RANGE )
    {
         child->setValue( defaultValue.Min( ) );
    }
    else if ( type == TYPE_RANGEF )
    {
         child->setValue( defaultValueF.Min( ) );
    }

    property->addSubProperty( child );

    childToProperty[child] = property;
    data.children.append( child );

    // add "Max" property
    child = addProperty( childType );
    child->setPropertyName( tr( "Max" ) );

    if ( type == TYPE_RANGE )
    {
        child->setValue( defaultValue.Max( ) );
    }
    else if ( type == TYPE_RANGEF )
    {
        child->setValue( defaultValueF.Max( ) );
    }

    property->addSubProperty( child );

    childToProperty[child] = property;
    data.children.append( child );

    propertyToData[property] = data;
}

// Set property of Range type
bool VariantPropertyManagerEx::setRangeValue( const QtProperty* property, const QVariant& value )
{
    bool ret = false;

    assert( propertyToData.contains( property ) );

    if ( ( value.type( ) == static_cast<QVariant::Type>( TYPE_RANGE ) ) || ( value.canConvert<XRange>( ) ) )
    {
        XRange range = qvariant_cast<XRange>( value );
        Data  data   = propertyToData[property];

        // set the value to local storage
        data.value.setValue( range );

        if ( data.children.count( ) == 2 )
        {
            // update min box
            data.children.at( 0 )->setValue( range.Min( ) );
            // update max box
            data.children.at( 1 )->setValue( range.Max( ) );
        }

        propertyToData[property] = data;
        ret = true;
    }
    else if ( ( value.type( ) == static_cast<QVariant::Type>( TYPE_RANGEF ) ) || ( value.canConvert<XRangeF>( ) ) )
    {
        XRangeF range = qvariant_cast<XRangeF>( value );
        Data    data  = propertyToData[property];

        // set the value to local storage
        data.value.setValue( range );

        if ( data.children.count( ) == 2 )
        {
            // update min box
            data.children.at( 0 )->setValue( range.Min( ) );
            // update max box
            data.children.at( 1 )->setValue( range.Max( ) );
        }

        propertyToData[property] = data;
        ret = true;
    }
    return ret;
}

// Set attribute of Range type
void VariantPropertyManagerEx::setRangeAttribute( const QtProperty* property, const QString& attribute, const QVariant& value )
{
    assert( propertyToData.contains( property ) );

    Data data = propertyToData[property];

    if ( data.children.count( ) == 2 )
    {
        // update min box
        setAttribute( data.children.at( 0 ), attribute, value );
        // update max box
        setAttribute( data.children.at( 1 ), attribute, value );
    }
}

// Get Range value as string
QString VariantPropertyManagerEx::getRangeTextValue( const QVariant& value ) const
{
    QString ret;

    if ( value.canConvert<XRange>( ) )
    {
        XRange range = qvariant_cast<XRange>( value );
        ret = QString( tr( "[%1, %2]" ).arg( QString::number( range.Min( ) ) )
                                       .arg( QString::number( range.Max( ) ) ) );
    }
    else if ( value.canConvert<XRangeF>( ) )
    {
        XRangeF range = qvariant_cast<XRangeF>( value );
        ret = QString( tr( "[%1, %2]" ).arg( QString::number( range.Min( ) ) )
                                       .arg( QString::number( range.Max( ) ) ) );
    }

    return ret;
}

// Get updated value for Range property when its child property changes
QVariant VariantPropertyManagerEx::getUpdatedRangeValue( const QtProperty* child, const QVariant& childValue, const Data& propertyData ) const
{
    QVariant value = propertyData.value;

    if ( propertyData.children.count( ) == 2 )
    {
        if ( value.canConvert<XRange>( ) )
        {
            XRange range = qvariant_cast<XRange>( value );
            int    i     = childValue.toInt( );

            if ( propertyData.children.at( 0 ) == child )
            {
                range.SetMin( i );
            }
            else if ( propertyData.children.at( 1 ) == child )
            {
                range.SetMax( i );
            }

            value.setValue<XRange>( range );
        }
        else if ( value.canConvert<XRangeF>( ) )
        {
            XRangeF range = qvariant_cast<XRangeF>( value );
            float   f     = static_cast<float>( childValue.toDouble( ) );

            if ( propertyData.children.at( 0 ) == child )
            {
                range.SetMin( f );
            }
            else if ( propertyData.children.at( 1 ) == child )
            {
                range.SetMax( f );
            }

            value.setValue<XRangeF>( range );
        }
    }

    return value;
}
