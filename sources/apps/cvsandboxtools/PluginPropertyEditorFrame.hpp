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
#ifndef CVS_PLUGIN_PROPERTY_EDITOR_FRAME_HPP
#define CVS_PLUGIN_PROPERTY_EDITOR_FRAME_HPP

#include <QFrame>
#include <XPluginsEngine.hpp>
#include <qtpropertymanager.h>
#include <qtvariantproperty.h>
#include <qttreepropertybrowser.h>

#include "cvsandboxtools_global.h"
#include "VariantPropertyManagerEx.hpp"

namespace Ui
{
    class PluginPropertyEditorFrame;
}

namespace Private
{
    class PluginPropertyEditorFrameData;
}

class CVS_SHARED_EXPORT PluginPropertyEditorFrame : public QFrame
{
    Q_OBJECT

public:
    explicit PluginPropertyEditorFrame( const std::shared_ptr<const XPluginDescriptor>& pluginDesc,
                                        const std::shared_ptr<XPlugin>& plugin,
                                        bool showDeviceRuntimeProperties = false,
                                        QWidget* parent = nullptr );
    ~PluginPropertyEditorFrame( );

    void SetPluginToConfigure( const std::shared_ptr<const XPluginDescriptor>& pluginDesc,
                               const std::shared_ptr<XPlugin>& plugin );

    void ResetPropertiesToDefaultValues( );

signals:
    void PluginUpdated( );

private:
    // Show available properties of the specified plug-in
    void ShowProperties( bool showDeviceRuntimeProperties );
    // Get index of plug-in's property from its UI property element
    int32_t GetPropertyIndex( const QtVariantProperty* property ) const;
    // Configure the specified property
    void ConfigureProperty( QtVariantProperty* item, const std::shared_ptr<const XPropertyDescriptor>& propertyDesc );
    // Configure property with integer numeric input
    void ConfigureIntegerProperty( QtVariantProperty* item, const std::shared_ptr<const XPropertyDescriptor>& propertyDesc );
    // Configure property with floating point numeric input
    void ConfigureFloatingPointProperty( QtVariantProperty* item, const std::shared_ptr<const XPropertyDescriptor>& propertyDesc );
    // Configure property with integer range input
    void ConfigureRangeProperty( QtVariantProperty* item, const std::shared_ptr<const XPropertyDescriptor>& propertyDesc );
    // Configure property with integer point or size input
    void ConfigurePointOrSizeProperty( QtVariantProperty* item, const std::shared_ptr<const XPropertyDescriptor>& propertyDesc );
    // Do simple configuration of a property - set its default value
    void ConfigureSimpleProperty( QtVariantProperty* item, const std::shared_ptr<const XPropertyDescriptor>& propertyDesc );
    // Configure enum property - add all possible choices
    void ConfigureEnumProperty( QtVariantProperty* item, const std::shared_ptr<const XPropertyDescriptor>& propertyDesc );

    // Get current value of plug-in's property and display it
    void DisplayCurrentPropertyValue( QtVariantProperty* property,
                                      const std::shared_ptr<const XPropertyDescriptor>& propertyDesc );
    // Display given value as the current value of certain plug-in's property
    void DisplayCurrentPropertyValue( QtVariantProperty* property,
                                      const std::shared_ptr<const XPropertyDescriptor>& propertyDesc,
                                      const CVSandbox::XVariant& propertyValue );

    // Set value of the specified property
    void SetPluginProperty( QtVariantProperty* property, const CVSandbox::XVariant& value, bool forceUiUpdate = true );
    // Update configuration of all dependent properties for the given parent property
    void UpdateDependentProperties( QtVariantProperty* parentProperty, const CVSandbox::XVariant& parentValue );

signals:
    void ConfigurationUpdated( );

protected:
    void focusInEvent( QFocusEvent* event );

private slots:
    // Index of selected item has changed
    void on_variantEditor_currentItemChanged( QtBrowserItem* item );
    // Value of property was changed
    void on_variantManager_valueChanged( QtProperty* property, QVariant value );

private:
    Ui::PluginPropertyEditorFrame*           ui;
    Private::PluginPropertyEditorFrameData*  mData;

    std::shared_ptr<const XPluginDescriptor> mPluginDesc;
    std::shared_ptr<XPlugin>                 mPlugin;
};

#endif // CVS_PLUGIN_PROPERTY_EDITOR_FRAME_HPP
