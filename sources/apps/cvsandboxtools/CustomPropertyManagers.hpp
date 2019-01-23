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
#ifndef CVS_CUSTOM_PROPERTY_MANAGERS_HPP
#define CVS_CUSTOM_PROPERTY_MANAGERS_HPP

#include <qtpropertymanager.h>
#include <qtvariantproperty.h>
#include <stdint.h>
#include <XVariantArray2d.hpp>

class QLabel;
class QSpinBox;
class QLineEdit;
class QToolButton;

// Base class for custom variant editing widgets
class VariantPropertyCustomEditorWidget : public QWidget
{
    Q_OBJECT

public:
    VariantPropertyCustomEditorWidget( QWidget* parent ) :
        QWidget( parent )
    {
    }

    virtual ~VariantPropertyCustomEditorWidget( ) { }

    // Notify that associated property has changed, so editor should update
    void setValue( const QVariant& value );
    // Set property's attribute to the editor
    void setAttribute( const QString& attribute, const QVariant& value );

signals:
    // Signal that value has changed in the editor
    void valueChanged( const QVariant& value );

protected:
    virtual void onValueUpdated( ) = 0;
    virtual void onAttributeChanged( const QString&, const QVariant& )
    {
        // base class is not really interested in it
    }

protected:
    QVariant mValue;
};

// Base class for custom variant editing factories
class VariantPropertyCustomEditorFactoryBase : public QtAbstractEditorFactory<QtVariantPropertyManager>
{
    Q_OBJECT

private:
    VariantPropertyCustomEditorFactoryBase( const VariantPropertyCustomEditorFactoryBase& );
    VariantPropertyCustomEditorFactoryBase& operator= ( const VariantPropertyCustomEditorFactoryBase& );

public:
    VariantPropertyCustomEditorFactoryBase( QObject* parent = 0 );
    virtual ~VariantPropertyCustomEditorFactoryBase( );

    // Set custom attribute for the specified property
    void setCustomAttribute( QtProperty*, const QString&, const QVariant& );

protected:
    void connectPropertyManager( QtVariantPropertyManager* manager );
    void disconnectPropertyManager( QtVariantPropertyManager* manager );
    QWidget* createEditor( QtVariantPropertyManager* manager, QtProperty* property, QWidget* parent );

    virtual VariantPropertyCustomEditorWidget* createEditorWidget( QWidget* parent ) = 0;

private slots:
    // Value of the specified property has changed, so update editors responsible for it
    void slotPropertyChanged( QtProperty*, const QVariant& );
    // Notification about a property being delete
    void slotPropertyDestroyed( QtProperty* property );
    // Editor's value has changed, so update corresponding property
    void slotSetValue( const QVariant& value );
    // An editor created by the factory was destroyed, so remove it from internal collections
    void slotEditorDestroyed( QObject* );

private:
    typedef QList<VariantPropertyCustomEditorWidget*>             EditorList;
    typedef QMap<QtProperty*, EditorList>                         PropertyToEditorListMap;
    typedef QMap<VariantPropertyCustomEditorWidget*, QtProperty*> EditorToPropertyMap;
    typedef QMap<QString, QVariant>                               AttributesMap;
    typedef QMap<QtProperty*, AttributesMap>                      PropertyToAttributesMap;

    PropertyToEditorListMap mCreatedEditors;
    EditorToPropertyMap     mEditorToProperty;
    PropertyToAttributesMap mAttributes;
};

// Template for custom property factories
template <class EditorWidgetClass>
class VariantPropertyCustomEditorFactory : public VariantPropertyCustomEditorFactoryBase
{
public:
    VariantPropertyCustomEditorFactory( QObject* parent = 0 ) :
        VariantPropertyCustomEditorFactoryBase( parent )
    {
    }

protected:

    virtual VariantPropertyCustomEditorWidget* createEditorWidget( QWidget* parent )
    {
        return new EditorWidgetClass( parent );
    }
};

/* ============================================================================ */

// Widget to edit string properties which provide a button for custom editor
class CustomDialogPropertyEditWidget : public VariantPropertyCustomEditorWidget
{
    Q_OBJECT

public:
    CustomDialogPropertyEditWidget( QWidget* parent, const QString& buttonIconResource,
                                                     const QString& buttonToolTip );

protected:
    virtual void onValueUpdated( );
    virtual void HandleButtonClick( ) = 0;

private slots:
    void buttonClicked( );

private:
    bool eventFilter( QObject* obj, QEvent* ev );

private:
    QLineEdit*   mEdit;
    QToolButton* mButton;
};

/* ============================================================================ */

// Widget to edit properties only through custom editor provided on button click
class CustomDialogPropertyWidget : public VariantPropertyCustomEditorWidget
{
    Q_OBJECT

public:
    CustomDialogPropertyWidget( QWidget* parent, const QString& buttonIconResource,
                                                 const QString& buttonToolTip );

protected:
    virtual void onValueUpdated( );
    virtual void HandleButtonClick( ) = 0;
    virtual QString GetValueText( ) const { return QString( ); }

private slots:
    void buttonClicked( );

private:
    QLabel*      mLabel;
    QToolButton* mButton;
};

/* ============================================================================ */

// Widget to edit string properties with folder browser support
class FolderPropertyEditWidget : public CustomDialogPropertyEditWidget
{
    Q_OBJECT

public:
    FolderPropertyEditWidget( QWidget* parent );

private:
    virtual void HandleButtonClick( );
};

typedef VariantPropertyCustomEditorFactory<FolderPropertyEditWidget> FolderPropertyEditorFactory;

/* ============================================================================ */

// Widget to edit string properties with file browser support
class FilePropertyEditWidget : public CustomDialogPropertyEditWidget
{
    Q_OBJECT

public:
    FilePropertyEditWidget( QWidget* parent );

private:
    virtual void HandleButtonClick( );
};

typedef VariantPropertyCustomEditorFactory<FilePropertyEditWidget> FilePropertyEditorFactory;

/* ============================================================================ */

// Widget to edit string properties representing script file name
class ScriptFilePropertyEditWidget : public VariantPropertyCustomEditorWidget
{
    Q_OBJECT

public:
    ScriptFilePropertyEditWidget( QWidget* parent );

protected:
    virtual void onValueUpdated( );
    virtual void onAttributeChanged( const QString& attribute, const QVariant& value );

private slots:
    void browseButtonClicked( );
    void editButtonClicked( );

private:
    bool eventFilter( QObject *obj, QEvent* ev );

private:
    QLineEdit*   mEdit;
    QToolButton* mButtonEdit;
    QToolButton* mButtonBrowse;
    QString      mDefaultExt;
    QString      mScriptingName;
    QString      mPluginId;

public:
    static const QString ATTR_DEFAULT_EXT;
    static const QString ATTR_SCRIPTING_NAME;
    static const QString ATTR_SCRIPTING_PLUGIN_ID;
};

typedef VariantPropertyCustomEditorFactory<ScriptFilePropertyEditWidget> ScriptPropertyEditorFactory;

/* ============================================================================ */

// Widget to edit properties representing hue value
class HuePickerPropertyEditWidget : public VariantPropertyCustomEditorWidget
{
    Q_OBJECT

public:
    HuePickerPropertyEditWidget( QWidget* parent );

protected:
    virtual void onValueUpdated( );

private slots:
    void editButtonClicked( );
    void spinBoxValueChanged( int i );

private:
    bool eventFilter( QObject *obj, QEvent* ev );

private:
    QLabel*      mLabel;
    QSpinBox*    mSpinBox;
    QToolButton* mButtonEdit;
};

typedef VariantPropertyCustomEditorFactory<HuePickerPropertyEditWidget> HuePickerPropertyEditorFactory;

// Custom manager for hue pickers
class HuePickerPropertyManager : public QtVariantPropertyManager
{
private:
    static QPixmap brushValuePixmap( const QBrush& b );

public:
    HuePickerPropertyManager( QObject* parent = 0 );

    QVariant value( const QtProperty* property ) const;
    void setValue( QtProperty* property, const QVariant& val );

protected:
    QString valueText( const QtProperty* property ) const;
    QIcon valueIcon( const QtProperty* property ) const;

    void initializeProperty( QtProperty* property );
    void uninitializeProperty( QtProperty* property );

private:
    typedef QMap<const QtProperty*, uint16_t> PropertyValueMap;
    PropertyValueMap m_values;
};

/* ============================================================================ */

// Widget to edit properties representing hue range
class HueRangePickerPropertyEditWidget : public VariantPropertyCustomEditorWidget
{
    Q_OBJECT

public:
    HueRangePickerPropertyEditWidget( QWidget* parent );

protected:
    virtual void onValueUpdated( );

private slots:
    void editButtonClicked( );

private:
    bool eventFilter( QObject *obj, QEvent* ev );

private:
    QLabel*      mHueLabel1;
    QLabel*      mHueLabel2;
    QLabel*      mValueLabel;
    QToolButton* mButtonEdit;
};

typedef VariantPropertyCustomEditorFactory<HueRangePickerPropertyEditWidget> HueRangePickerPropertyEditorFactory;

// Custom manager for hue range pickers
class HueRangePickerPropertyManager : public QtVariantPropertyManager
{
    Q_OBJECT

private:
    static QPixmap brushValuePixmap( const QBrush& b1, const QBrush& b2 );

public:
    HueRangePickerPropertyManager( QObject* parent = 0 );

    bool isPropertyTypeSupported( int propertyType ) const;

    QVariant value( const QtProperty* property ) const;
    void setValue( QtProperty* property, const QVariant& val );

    HuePickerPropertyManager* subHuePropertyManager( ) const;

protected:
    QString valueText( const QtProperty* property ) const;
    QIcon valueIcon( const QtProperty* property ) const;

    void initializeProperty( QtProperty* property );
    void uninitializeProperty( QtProperty* property );

private slots:
    void hueChanged( QtProperty* property, const QVariant& value );
    void huePropertyDestroyed( QtProperty* property );

private:
    typedef QMap<const QtProperty*, QVariant> PropertyValueMap;
    PropertyValueMap m_values;

    QMap<const QtProperty*, QtProperty*> m_propertyToMin;
    QMap<const QtProperty*, QtProperty*> m_propertyToMax;

    QMap<const QtProperty*, QtProperty*> m_minToProperty;
    QMap<const QtProperty*, QtProperty*> m_maxToProperty;

    HuePickerPropertyManager* m_huePickerPropertyManager;
};

/* ============================================================================ */

// Widget to edit structuring elements of morphology operators
class StructuringElementPropertyWidget : public CustomDialogPropertyWidget
{
    Q_OBJECT

public:
    StructuringElementPropertyWidget( QWidget* parent );

protected:
    virtual QString GetValueText( ) const;
    virtual void onAttributeChanged( const QString& attribute, const QVariant& value );

private:
    virtual void HandleButtonClick( );

public:
    static const QString ATTR_MODE;

private:
    int mMode;
};

typedef VariantPropertyCustomEditorFactory<StructuringElementPropertyWidget> StructuringElementPropertyEditorFactory;

// Custom property manager for structuring elements' editors
class StructuringElementPropertyManager : public QtVariantPropertyManager
{
public:
    StructuringElementPropertyManager( QObject* parent = 0 );

    bool isPropertyTypeSupported( int propertyType ) const;

    QVariant value( const QtProperty* property ) const;
    void setValue( QtProperty* property, const QVariant& val );

protected:
    QString valueText( const QtProperty* property ) const;

    void initializeProperty( QtProperty* property );
    void uninitializeProperty( QtProperty* property );

private:
    typedef QMap<const QtProperty*, CVSandbox::XVariantArray2d> PropertyValueMap;
    PropertyValueMap m_values;
};

/* ============================================================================ */

// Widget to edit convolution kernels
class ConvolutionKernelPropertyWidget : public CustomDialogPropertyWidget
{
    Q_OBJECT

public:
    ConvolutionKernelPropertyWidget( QWidget* parent );

protected:
    virtual QString GetValueText( ) const;

private:
    virtual void HandleButtonClick( );
};

typedef VariantPropertyCustomEditorFactory<ConvolutionKernelPropertyWidget> ConvolutionKernelPropertyEditorFactory;

// Custom property manager for convolution kernels' editors
class ConvolutionKernelPropertyManager : public QtVariantPropertyManager
{
public:
    ConvolutionKernelPropertyManager( QObject* parent = 0 );

    bool isPropertyTypeSupported( int propertyType ) const;

    QVariant value( const QtProperty* property ) const;
    void setValue( QtProperty* property, const QVariant& val );

protected:
    QString valueText( const QtProperty* property ) const;

    void initializeProperty( QtProperty* property );
    void uninitializeProperty( QtProperty* property );

private:
    typedef QMap<const QtProperty*, CVSandbox::XVariantArray2d> PropertyValueMap;
    PropertyValueMap m_values;
};

/* ============================================================================ */

#endif // CVS_CUSTOM_PROPERTY_MANAGERS_HPP
