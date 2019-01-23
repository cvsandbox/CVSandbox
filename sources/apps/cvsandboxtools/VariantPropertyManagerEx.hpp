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
#ifndef CVS_VARIANT_PROPERTY_MANAGER_EX_HPP
#define CVS_VARIANT_PROPERTY_MANAGER_EX_HPP

#include "cvsandboxtools_global.h"
#include <qtpropertymanager.h>
#include <qtvariantproperty.h>

// Extended variant property manager, which supports some
// custom CVSandbox types
class CVS_SHARED_EXPORT VariantPropertyManagerEx : public QtVariantPropertyManager
{
    Q_OBJECT

public:
    VariantPropertyManagerEx( QObject* parent = 0 );
    ~VariantPropertyManagerEx( );

    // Check if the control supports specified property type
    virtual bool isPropertyTypeSupported( int propertyType ) const;
    // Get type of value for the given type of property
    virtual int valueType( int propertyType ) const;

    // Get value of the specified property
    virtual QVariant value( const QtProperty* property ) const;
    // Get text value of the specified property
    virtual QString valueText( const QtProperty* property ) const;

public slots:
    // Set value of the specified property
    virtual void setValue( QtProperty* property, const QVariant& value );
    // Set attribute for the specified property
    virtual void setAttribute( QtProperty* property,
                    const QString& attribute, const QVariant& value );

protected:
    // Initialize the specified property
    virtual void initializeProperty( QtProperty* property );
    // Uninitialize the specified property
    virtual void uninitializeProperty( QtProperty* property );

private slots:
    // Handle change of a child property
    void slotValueChanged( QtProperty* property, const QVariant& value );
    // Handle destruction of child property
    void slotPropertyDestroyed( QtProperty* property );

private:
    struct Data
    {
        int type;
        QVariant value;
        QList<QtVariantProperty*> children;
    };

private:
    // Range value helper methods
    void initializeRangeProperty( QtProperty* property, int type );
    bool setRangeValue( const QtProperty* property, const QVariant& value );
    void setRangeAttribute( const QtProperty* property, const QString& attribute, const QVariant& value );
    QString getRangeTextValue( const QVariant& value ) const;
    QVariant getUpdatedRangeValue( const QtProperty* child, const QVariant& childValue, const Data& propertyData ) const;

private:
    QMap<const QtProperty*, Data> propertyToData;
    QMap<const QtProperty*, QtProperty*> childToProperty;
};

#endif // CVS_VARIANT_PROPERTY_MANAGER_EX_HPP
