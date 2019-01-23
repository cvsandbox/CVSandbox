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

#define __STDC_LIMIT_MACROS

#include <assert.h>
#include <stdint.h>
#include <math.h>
#include "ui_PluginPropertyEditorFrame.h"
#include "PluginPropertyEditorFrame.hpp"
#include "XVariantConverter.hpp"
#include "CustomPropertyManagers.hpp"

#include "XScriptingEnginePlugin.hpp"

using namespace std;
using namespace CVSandbox;

static const QString STR_MINIMUM     = QString::fromStdString( "minimum" );
static const QString STR_MAXIMUM     = QString::fromStdString( "maximum" );
static const QString STR_SINGLE_STEP = QString::fromStdString( "singleStep" );
static const QString STR_ENUM_NAMES  = QString::fromStdString( "enumNames" );

// ============================================================================

// Namespace to hide some private stuff
namespace Private
{
    class PluginPropertyEditorFrameData
    {
    public:
        PluginPropertyEditorFrameData( bool deviceRuntimePropertiesOnly ) :
            acceptValueChange( true ), showDeviceRuntimeProperties( deviceRuntimePropertiesOnly )
        {
        }

    public:
        bool acceptValueChange;
        bool showDeviceRuntimeProperties;

        QtVariantEditorFactory   mVariantFactory;
        VariantPropertyManagerEx mVariantManager;

        FolderPropertyEditorFactory mFolderEditorFactory;
        QtVariantPropertyManager    mFolderManager;

        FilePropertyEditorFactory   mFileEditorFactory;
        QtVariantPropertyManager    mFilePropertyManager;

        ScriptPropertyEditorFactory mScriptPropertyEditorFactory;
        QtVariantPropertyManager    mScriptPropertyManager;

        HuePickerPropertyEditorFactory mHuePickerPropertyEditorFactory;
        HuePickerPropertyManager       mHuePickerPropertyManager;

        HueRangePickerPropertyEditorFactory mHueRangePickerPropertyEditorFactory;
        HueRangePickerPropertyManager       mHueRangePickerPropertyManager;

        StructuringElementPropertyEditorFactory mStructuringElementPropertyEditorFactory;
        StructuringElementPropertyManager       mStructuringElementPropertyManager;

        ConvolutionKernelPropertyEditorFactory  mConvolutionKernelPropertyEditorFactory;
        ConvolutionKernelPropertyManager        mConvolutionKernelPropertyManager;

        QMap<const QtVariantProperty*, int32_t> PropertiesIndexMap;

        // keep it last, so it is destroyed first
        QtTreePropertyBrowser mVariantEditor;
    };
}

// Class constructor
PluginPropertyEditorFrame::PluginPropertyEditorFrame(
                            const shared_ptr<const XPluginDescriptor>& pluginDesc,
                            const shared_ptr<XPlugin>& plugin,
                            bool showDeviceRuntimeProperties,
                            QWidget* parent ) :
    QFrame( parent ),
    ui( new Ui::PluginPropertyEditorFrame ),
    mData( new Private::PluginPropertyEditorFrameData( showDeviceRuntimeProperties ) ),
    mPluginDesc( pluginDesc ),
    mPlugin( plugin )
{
    ui->setupUi( this );

    mData->mVariantEditor.setFactoryForManager( static_cast<QtVariantPropertyManager*>( &mData->mVariantManager ), &mData->mVariantFactory );
    mData->mVariantEditor.setFactoryForManager( static_cast<QtVariantPropertyManager*>( &mData->mFolderManager ), &mData->mFolderEditorFactory );
    mData->mVariantEditor.setFactoryForManager( static_cast<QtVariantPropertyManager*>( &mData->mFilePropertyManager ), &mData->mFileEditorFactory );
    mData->mVariantEditor.setFactoryForManager( static_cast<QtVariantPropertyManager*>( &mData->mScriptPropertyManager ), &mData->mScriptPropertyEditorFactory );
    mData->mVariantEditor.setFactoryForManager( static_cast<QtVariantPropertyManager*>( &mData->mHuePickerPropertyManager ), &mData->mHuePickerPropertyEditorFactory );
    mData->mVariantEditor.setFactoryForManager( static_cast<QtVariantPropertyManager*>( &mData->mHueRangePickerPropertyManager ), &mData->mHueRangePickerPropertyEditorFactory );
    mData->mVariantEditor.setFactoryForManager( static_cast<QtVariantPropertyManager*>( mData->mHueRangePickerPropertyManager.subHuePropertyManager( ) ), &mData->mHuePickerPropertyEditorFactory );
    mData->mVariantEditor.setFactoryForManager( static_cast<QtVariantPropertyManager*>( &mData->mStructuringElementPropertyManager ), &mData->mStructuringElementPropertyEditorFactory );
    mData->mVariantEditor.setFactoryForManager( static_cast<QtVariantPropertyManager*>( &mData->mConvolutionKernelPropertyManager ), &mData->mConvolutionKernelPropertyEditorFactory );

    mData->mVariantEditor.setAlternatingRowColors( true );
    mData->mVariantEditor.setResizeMode( QtTreePropertyBrowser::ResizeToContents );
    ui->verticalLayout_3->addWidget( &mData->mVariantEditor );

    connect( &mData->mVariantEditor, SIGNAL( currentItemChanged( QtBrowserItem* ) ),
             this, SLOT( on_variantEditor_currentItemChanged( QtBrowserItem* ) ) );
    connect( &mData->mVariantManager, SIGNAL( valueChanged( QtProperty*, QVariant ) ),
             this, SLOT( on_variantManager_valueChanged( QtProperty*, QVariant ) ) );
    connect( &mData->mFolderManager, SIGNAL( valueChanged( QtProperty*, QVariant ) ),
             this, SLOT( on_variantManager_valueChanged( QtProperty*, QVariant ) ) );
    connect( &mData->mFilePropertyManager, SIGNAL( valueChanged( QtProperty*, QVariant ) ),
             this, SLOT( on_variantManager_valueChanged( QtProperty*, QVariant ) ) );
    connect( &mData->mScriptPropertyManager, SIGNAL( valueChanged( QtProperty*, QVariant ) ),
             this, SLOT( on_variantManager_valueChanged( QtProperty*, QVariant ) ) );
    connect( &mData->mHuePickerPropertyManager, SIGNAL( valueChanged( QtProperty*, QVariant ) ),
             this, SLOT( on_variantManager_valueChanged( QtProperty*, QVariant ) ) );
    connect( &mData->mHueRangePickerPropertyManager, SIGNAL( valueChanged( QtProperty*, QVariant ) ),
             this, SLOT( on_variantManager_valueChanged( QtProperty*, QVariant ) ) );
    connect( &mData->mStructuringElementPropertyManager, SIGNAL( valueChanged( QtProperty*, QVariant ) ),
             this, SLOT( on_variantManager_valueChanged( QtProperty*, QVariant ) ) );
    connect( &mData->mConvolutionKernelPropertyManager, SIGNAL( valueChanged( QtProperty*, QVariant ) ),
             this, SLOT( on_variantManager_valueChanged( QtProperty*, QVariant ) ) );

    if ( ( mPluginDesc ) && ( mPlugin ) )
    {
        ShowProperties( showDeviceRuntimeProperties );
    }
}

PluginPropertyEditorFrame::~PluginPropertyEditorFrame( )
{
    mData->mVariantEditor.unsetFactoryForManager( static_cast<QtVariantPropertyManager*>( &mData->mVariantManager ) );
    mData->mVariantEditor.unsetFactoryForManager( static_cast<QtVariantPropertyManager*>( &mData->mFolderManager ) );
    mData->mVariantEditor.unsetFactoryForManager( static_cast<QtVariantPropertyManager*>( &mData->mFilePropertyManager ) );
    mData->mVariantEditor.unsetFactoryForManager( static_cast<QtVariantPropertyManager*>( &mData->mScriptPropertyManager ) );
    mData->mVariantEditor.unsetFactoryForManager( static_cast<QtVariantPropertyManager*>( &mData->mHuePickerPropertyManager ) );
    mData->mVariantEditor.unsetFactoryForManager( static_cast<QtVariantPropertyManager*>( &mData->mHueRangePickerPropertyManager ) );
    mData->mVariantEditor.unsetFactoryForManager( static_cast<QtVariantPropertyManager*>( mData->mHueRangePickerPropertyManager.subHuePropertyManager( ) ) );
    mData->mVariantEditor.unsetFactoryForManager( static_cast<QtVariantPropertyManager*>( &mData->mStructuringElementPropertyManager ) );
    mData->mVariantEditor.unsetFactoryForManager( static_cast<QtVariantPropertyManager*>( &mData->mConvolutionKernelPropertyManager ) );

    SetPluginToConfigure( shared_ptr<const XPluginDescriptor>( ), shared_ptr<XPlugin>( ) );

    delete mData;
    delete ui;
}

void PluginPropertyEditorFrame::SetPluginToConfigure( const shared_ptr<const XPluginDescriptor>& pluginDesc,
                                                      const shared_ptr<XPlugin>& plugin )
{
    mData->mVariantManager.clear( );

    mPluginDesc = pluginDesc;
    mPlugin     = plugin;

    if ( ( mPluginDesc ) && ( mPlugin ) )
    {
        ShowProperties( mData->showDeviceRuntimeProperties );
    }
}

// Reset all currently displayed properties to their default values
void PluginPropertyEditorFrame::ResetPropertiesToDefaultValues( )
{
    if ( ( mPluginDesc ) && ( mPlugin ) )
    {
        // second pass to update dependent properties
        for ( QMap<const QtVariantProperty*, int32_t>::const_iterator it = mData->PropertiesIndexMap.begin( );
              it != mData->PropertiesIndexMap.end( ); ++it )
        {
            XVariant defaultValue = mPluginDesc->GetPropertyDescriptor( it.value( ) )->GetDefaultValue( );

            if ( !defaultValue.IsEmpty( ) )
            {
                SetPluginProperty( const_cast<QtVariantProperty*>( it.key( ) ), defaultValue );
            }
        }
    }
}

// Pass focus to property editor
void PluginPropertyEditorFrame::focusInEvent( QFocusEvent* )
{
    mData->mVariantEditor.setFocus( );
}

// Show available properties of the specified plug-in
void PluginPropertyEditorFrame::ShowProperties( bool showDeviceRuntimeProperties )
{
    QtProperty* topItem = mData->mVariantManager.addProperty( QtVariantPropertyManager::groupTypeId( ),
        QString::fromUtf8( mPluginDesc->Name( ).c_str( ) ) + QString::fromStdString( " properties" ) );

    int32_t propertiesCount = mPluginDesc->PropertiesCount( );

    mData->PropertiesIndexMap.clear( );

    for ( int32_t i = 0; i < propertiesCount; i++ )
    {
        shared_ptr<const XPropertyDescriptor> propertyDesc = mPluginDesc->GetPropertyDescriptor( i );

        // decide if we want to show the property
        if ( showDeviceRuntimeProperties != propertyDesc->IsDeviceRuntimeConfiguration( ) )
        {
            continue;
        }

        // update property description, if it is dependent
        if ( propertyDesc->IsDependent( ) )
        {
            XVariant parentValue;

            if ( mPlugin->GetProperty( propertyDesc->GetParentPropertyID( ), parentValue ) == SuccessCode )
            {
                propertyDesc->Update( parentValue );
            }
        }

        // don't show hidden or read only properties
        if ( ( propertyDesc->IsReadOnly( ) ) || ( propertyDesc->IsHidden( ) ) )
        {
            continue;
        }

        QString propertyName = QString::fromUtf8( propertyDesc->Name( ).c_str( ) );

        if ( !propertyDesc->IsSelection( ) )
        {
            QtVariantProperty* item = nullptr;

            if ( ( propertyDesc->PreferredEditor( ) == PropertyFlag_PreferredEditor_FolderBrowser ) &&
                 ( propertyDesc->Type( ) == XVT_String ) )
            {
                item = mData->mFolderManager.addProperty( QVariant::String, propertyName );
            }
            else if ( ( propertyDesc->PreferredEditor( ) == PropertyFlag_PreferredEditor_FileBrowser ) &&
                      ( propertyDesc->Type( ) == XVT_String ) )
            {
                item = mData->mFilePropertyManager.addProperty( QVariant::String, propertyName );
            }
            else if ( ( propertyDesc->PreferredEditor( ) == PropertyFlag_PreferredEditor_ScriptFile ) &&
                      ( propertyDesc->Type( ) == XVT_String ) )
            {
                item = mData->mScriptPropertyManager.addProperty( QVariant::String, propertyName );

                // set scripting plug-in ID
                mData->mScriptPropertyEditorFactory.setCustomAttribute( item, ScriptFilePropertyEditWidget::ATTR_SCRIPTING_PLUGIN_ID,
                                                                        QVariant::fromValue( QString::fromStdString( mPluginDesc->ID( ).ToString( ) ) ) );

                // set scripting name attribute
                mData->mScriptPropertyEditorFactory.setCustomAttribute( item, ScriptFilePropertyEditWidget::ATTR_SCRIPTING_NAME,
                                                                        QVariant::fromValue( QString::fromUtf8( mPluginDesc->Name( ).c_str( ) ) ) );

                if ( mPlugin )
                {
                    shared_ptr<XScriptingEnginePlugin> scriptingPlugin = static_pointer_cast<XScriptingEnginePlugin>( mPlugin );

                    // set default extension attribute
                    mData->mScriptPropertyEditorFactory.setCustomAttribute( item, ScriptFilePropertyEditWidget::ATTR_DEFAULT_EXT,
                                                                            QVariant::fromValue( QString::fromUtf8( scriptingPlugin->GetDefaultExtension( ).c_str( ) ) ) );
                }
            }
            else if ( ( propertyDesc->PreferredEditor( ) == PropertyFlag_PreferredEditor_HuePicker ) &&
                      ( propertyDesc->Type( ) == XVT_U2 ) )
            {
                item = mData->mHuePickerPropertyManager.addProperty( QVariant::Int, propertyName );
            }
            else if ( ( propertyDesc->PreferredEditor( ) == PropertyFlag_PreferredEditor_HueRangePicker ) &&
                      ( propertyDesc->Type( ) == XVT_Range ) )
            {
                item = mData->mHueRangePickerPropertyManager.addProperty( XVariantConverter::ToQVariantType( XVT_Range ), propertyName );
            }
            else if ( ( ( propertyDesc->PreferredEditor( ) == PropertyFlag_PreferredEditor_MorphologyStructuringElement ) ||
                        ( propertyDesc->PreferredEditor( ) == PropertyFlag_PreferredEditor_HitAndMissStructuringElement ) ) &&
                      ( propertyDesc->Type( ) == ( XVT_Array2d | XVT_I1 ) ) )
            {
                item = mData->mStructuringElementPropertyManager.addProperty( XVariantConverter::ToQVariantType( XVT_Array2d ), propertyName );

                // set mode of the editor
                mData->mStructuringElementPropertyEditorFactory.setCustomAttribute( item, StructuringElementPropertyWidget::ATTR_MODE,
                    QVariant::fromValue( ( propertyDesc->PreferredEditor( ) == PropertyFlag_PreferredEditor_MorphologyStructuringElement ) ? 0 : 1 ) );
            }
            else if ( ( propertyDesc->PreferredEditor( ) == PropertyFlag_PreferredEditor_ConvolutionKernel ) &&
                      ( propertyDesc->Type( ) == ( XVT_Array2d | XVT_R4 ) ) )
            {
                item = mData->mConvolutionKernelPropertyManager.addProperty( XVariantConverter::ToQVariantType( XVT_Array2d ), propertyName );
            }
            else
            {
                // resolve property type to add
                int propertyType = XVariantConverter::ToQVariantType( propertyDesc->Type( ) );

                if ( propertyType != QVariant::Invalid )
                {
                    item = mData->mVariantManager.addProperty( propertyType, propertyName );
                }
            }

            if ( item != nullptr )
            {
                mData->PropertiesIndexMap[item] = i;
                ConfigureProperty( item, propertyDesc );
                topItem->addSubProperty( item );
            }
        }
        else
        {
            // add drop down property
            QtVariantProperty* item = mData->mVariantManager.addProperty( QtVariantPropertyManager::enumTypeId( ), propertyName );

            if ( item != 0 )
            {
                mData->PropertiesIndexMap[item] = i;
                ConfigureEnumProperty( item, propertyDesc );
                topItem->addSubProperty( item );
            }
        }
    }

    // insert top item and configure it
    QtBrowserItem* item = mData->mVariantEditor.addProperty( topItem );

    mData->mVariantEditor.setBackgroundColor( item, QColor( 0xDF, 0xFF, 0xDF ) );
    mData->mVariantEditor.setCurrentItem( item );

    // second pass to update dependent properties
    for ( QMap<const QtVariantProperty*, int32_t>::const_iterator it = mData->PropertiesIndexMap.begin( );
          it != mData->PropertiesIndexMap.end( ); ++it )
    {
        XVariant value;

        if ( mPlugin->GetProperty( it.value( ), value ) == SuccessCode )
        {
            UpdateDependentProperties( const_cast<QtVariantProperty*>( it.key( ) ), value );
        }
    }
}

// Get index of plug-in's property from its UI property element
int32_t PluginPropertyEditorFrame::GetPropertyIndex( const QtVariantProperty* property ) const
{
    int32_t propertyIndex = -1;

    if ( mData->PropertiesIndexMap.contains( property ) )
    {
        propertyIndex = mData->PropertiesIndexMap[property];
    }

    return propertyIndex;
}

// Configure the specified property
void PluginPropertyEditorFrame::ConfigureProperty( QtVariantProperty* item, const shared_ptr<const XPropertyDescriptor>& propertyDesc )
{
    switch ( propertyDesc->Type( ) )
    {
        case XVT_U1:
        case XVT_U2:
        case XVT_U4:
        case XVT_I1:
        case XVT_I2:
        case XVT_I4:
            ConfigureIntegerProperty( item, propertyDesc );
            break;

        case XVT_R4:
        case XVT_R8:
            ConfigureFloatingPointProperty( item, propertyDesc );
            break;

        case XVT_Range:
        case XVT_RangeF:
            ConfigureRangeProperty( item, propertyDesc );
            break;

        case XVT_Point:
        case XVT_Size:
            ConfigurePointOrSizeProperty( item, propertyDesc );
            break;

        default:
            ConfigureSimpleProperty( item, propertyDesc );
            break;
    }
}

// Configure property with integer numeric input
void PluginPropertyEditorFrame::ConfigureIntegerProperty( QtVariantProperty* item, const shared_ptr<const XPropertyDescriptor>& propertyDesc )
{
    XVariant    minValue = propertyDesc->GetMinValue( );
    XVariant    maxValue = propertyDesc->GetMaxValue( );
    XVariant    currentPropertyValue;
    XErrorCode  getPropertyError = mPlugin->GetProperty( GetPropertyIndex( item ), currentPropertyValue );

    mData->acceptValueChange = false;

    switch ( propertyDesc->Type( ) )
    {
        case XVT_U1:
            item->setAttribute( STR_MINIMUM, ( minValue.Type( ) != XVT_U1 ) ? 0 : minValue.ToUByte( ) );
            item->setAttribute( STR_MAXIMUM, ( maxValue.Type( ) != XVT_U1 ) ? UINT8_MAX : maxValue.ToUByte( ) );
            break;
        case XVT_U2:
            item->setAttribute( STR_MINIMUM, ( minValue.Type( ) != XVT_U2 ) ? 0 : minValue.ToUShort( ) );
            item->setAttribute( STR_MAXIMUM, ( maxValue.Type( ) != XVT_U2 ) ? UINT16_MAX : maxValue.ToUShort( ) );
            break;
        case XVT_U4:
            item->setAttribute( STR_MINIMUM, ( minValue.Type( ) != XVT_U4 ) ? 0 : minValue.ToUInt( ) );
            item->setAttribute( STR_MAXIMUM, ( maxValue.Type( ) != XVT_U4 ) ? INT32_MAX : maxValue.ToUInt( ) );
            break;
        case XVT_I1:
            item->setAttribute( STR_MINIMUM, ( minValue.Type( ) != XVT_I1 ) ? INT8_MIN : minValue.ToByte( ) );
            item->setAttribute( STR_MAXIMUM, ( maxValue.Type( ) != XVT_I1 ) ? INT8_MAX : maxValue.ToByte( ) );
            break;
        case XVT_I2:
            item->setAttribute( STR_MINIMUM, ( minValue.Type( ) != XVT_I2 ) ? INT16_MIN : minValue.ToShort( ) );
            item->setAttribute( STR_MAXIMUM, ( maxValue.Type( ) != XVT_I2 ) ? INT16_MAX : maxValue.ToShort( ) );
            break;
        case XVT_I4:
            item->setAttribute( STR_MINIMUM, ( minValue.Type( ) != XVT_I4 ) ? INT32_MIN : minValue.ToInt( ) );
            item->setAttribute( STR_MAXIMUM, ( maxValue.Type( ) != XVT_I4 ) ? INT32_MAX : maxValue.ToInt( ) );
            break;
    }

    mData->acceptValueChange = true;

    if ( getPropertyError == SuccessCode )
    {
        DisplayCurrentPropertyValue( item, propertyDesc, currentPropertyValue );
    }
    else
    {
        SetPluginProperty( item, propertyDesc->GetDefaultValue( ) );
    }
}

// Configure property with floating point numeric input
void PluginPropertyEditorFrame::ConfigureFloatingPointProperty( QtVariantProperty* item, const shared_ptr<const XPropertyDescriptor>& propertyDesc )
{
    XVariant    minValue = propertyDesc->GetMinValue( );
    XVariant    maxValue = propertyDesc->GetMaxValue( );
    double      diff     = maxValue.ToDouble( ) - minValue.ToDouble( );
    double      step     = ( diff == 0.0 ) ? 1.0 : pow( 10 , floor( log10( diff ) + 0.5 ) ) / 100;
    XVariant    currentPropertyValue;
    XErrorCode  getPropertyError = mPlugin->GetProperty( GetPropertyIndex( item ), currentPropertyValue );

    mData->acceptValueChange = false;

    switch ( propertyDesc->Type( ) )
    {
    case XVT_R4:
        if ( minValue.Type( ) == XVT_R4 )
        {
            item->setAttribute( STR_MINIMUM, minValue.ToFloat( ) );
        }
        if ( maxValue.Type( ) == XVT_R4 )
        {
            item->setAttribute( STR_MAXIMUM, maxValue.ToFloat( ) );
        }
        break;

    case XVT_R8:
        if ( minValue.Type( ) == XVT_R8 )
        {
            item->setAttribute( STR_MINIMUM, minValue.ToDouble( ) );
        }
        if ( maxValue.Type( ) == XVT_R8 )
        {
            item->setAttribute( STR_MAXIMUM, maxValue.ToDouble( ) );
        }
        break;
    }

    item->setAttribute( STR_SINGLE_STEP, step );

    mData->acceptValueChange = true;

    if ( getPropertyError == SuccessCode )
    {
        DisplayCurrentPropertyValue( item, propertyDesc, currentPropertyValue );
    }
    else
    {
        SetPluginProperty( item, propertyDesc->GetDefaultValue( ) );
    }
}

// Configure property with integer range input
void PluginPropertyEditorFrame::ConfigureRangeProperty( QtVariantProperty* item, const shared_ptr<const XPropertyDescriptor>& propertyDesc )
{
    XVariant    minValue = propertyDesc->GetMinValue( );
    XVariant    maxValue = propertyDesc->GetMaxValue( );
    XVariant    currentPropertyValue;
    XErrorCode  getPropertyError = mPlugin->GetProperty( GetPropertyIndex( item ), currentPropertyValue );

    mData->acceptValueChange = false;

    if ( !minValue.IsNullOrEmpty( ) )
    {
        mData->mVariantManager.setAttribute( item, STR_MINIMUM, XVariantConverter::ToQVariant( minValue ) );
    }
    if ( !maxValue.IsNullOrEmpty( ) )
    {
        mData->mVariantManager.setAttribute( item, STR_MAXIMUM, XVariantConverter::ToQVariant( maxValue ) );
    }

    if ( item->propertyType( ) == XVariantConverter::ToQVariantType( XVT_RangeF ) )
    {
        double diff = maxValue.ToDouble( ) - minValue.ToDouble( );
        double step = ( diff == 0.0 ) ? 1.0 : pow( 10 , floor( log10( diff ) + 0.5 ) ) / 100;

        item->setAttribute( STR_SINGLE_STEP, step );
    }

    mData->acceptValueChange = true;

    if ( getPropertyError == SuccessCode )
    {
        DisplayCurrentPropertyValue( item, propertyDesc, currentPropertyValue );
    }
    else
    {
        SetPluginProperty( item, propertyDesc->GetDefaultValue( ) );
    }
}

// Configure property with integer point input
void PluginPropertyEditorFrame::ConfigurePointOrSizeProperty( QtVariantProperty* item, const shared_ptr<const XPropertyDescriptor>& propertyDesc )
{
    XVariant    minValue = propertyDesc->GetMinValue( );
    XVariant    maxValue = propertyDesc->GetMaxValue( );
    XVariant    currentPropertyValue;
    XErrorCode  getPropertyError = mPlugin->GetProperty( GetPropertyIndex( item ), currentPropertyValue );

    mData->acceptValueChange = false;

    // idieally QtPropertyBrowser should be changed for this, but it was quicker to hack it this way
    if ( !minValue.IsNullOrEmpty( ) )
    {
        mData->mVariantManager.setAttribute( item->subProperties( ).at( 0 ), STR_MINIMUM, XVariantConverter::ToQVariant( minValue ) );
        mData->mVariantManager.setAttribute( item->subProperties( ).at( 1 ), STR_MINIMUM, XVariantConverter::ToQVariant( minValue ) );
    }

    if ( !maxValue.IsNullOrEmpty( ) )
    {
        mData->mVariantManager.setAttribute( item->subProperties( ).at( 0 ), STR_MAXIMUM, XVariantConverter::ToQVariant( maxValue ) );
        mData->mVariantManager.setAttribute( item->subProperties( ).at( 1 ), STR_MAXIMUM, XVariantConverter::ToQVariant( maxValue ) );
    }

    mData->acceptValueChange = true;

    if ( getPropertyError == SuccessCode )
    {
        DisplayCurrentPropertyValue( item, propertyDesc, currentPropertyValue );
    }
    else
    {
        SetPluginProperty( item, propertyDesc->GetDefaultValue( ) );
    }
}

// Do simple configuration of a property - display it current value
void PluginPropertyEditorFrame::ConfigureSimpleProperty( QtVariantProperty* item, const shared_ptr<const XPropertyDescriptor>& propertyDesc )
{
    DisplayCurrentPropertyValue( item, propertyDesc );
}

// Configure enum property - add all possible choices
void PluginPropertyEditorFrame::ConfigureEnumProperty( QtVariantProperty* item, const shared_ptr<const XPropertyDescriptor>& propertyDesc )
{
    QStringList enumList;
    int         choicesCount         = propertyDesc->ChoicesCount( );
    XVariant    currentPropertyValue;
    XErrorCode  getPropertyError     = mPlugin->GetProperty( GetPropertyIndex( item ), currentPropertyValue );

    for ( int i = 0; i < choicesCount; i++ )
    {
        QString displayValue = QString::fromUtf8( propertyDesc->GetPossibleChoice( i ).ToString( ).c_str( ) );
        int     newLineIndex = displayValue.indexOf( "\n" );

        if ( newLineIndex != -1 )
        {
            displayValue.truncate( newLineIndex );
        }

        enumList.append( displayValue );
    }

    mData->acceptValueChange = false;
    item->setAttribute( STR_ENUM_NAMES, enumList );
    mData->acceptValueChange = true;

    // display current value or set default one
    if ( getPropertyError == SuccessCode )
    {
        DisplayCurrentPropertyValue( item, propertyDesc, currentPropertyValue );
    }
    else
    {
        SetPluginProperty( item, propertyDesc->GetDefaultValue( ) );
    }
}

// Get current value of plug-in's property and display it
void PluginPropertyEditorFrame::DisplayCurrentPropertyValue( QtVariantProperty* property, const shared_ptr<const XPropertyDescriptor>& propertyDesc )
{
    XVariant propertyValue;

    if ( mPlugin->GetProperty( GetPropertyIndex( property ), propertyValue ) == SuccessCode )
    {
        DisplayCurrentPropertyValue( property, propertyDesc, propertyValue );
    }
}

// Display given value as the current value of certain plug-in's property
void PluginPropertyEditorFrame::DisplayCurrentPropertyValue( QtVariantProperty* property,
                                                             const shared_ptr<const XPropertyDescriptor>& propertyDesc,
                                                             const XVariant& propertyValue )
{
    mData->acceptValueChange = false;

    if ( !propertyDesc->IsSelectionByValue( ) )
    {
        QVariant qvalue =  XVariantConverter::ToQVariant( propertyValue );
        property->setValue( qvalue );
    }
    else
    {
        int32_t valueIndex = propertyDesc->GetChoiceIndex( propertyValue );

        if ( valueIndex == -1 )
        {
            SetPluginProperty( property, propertyDesc->GetDefaultValue( ) );
        }
        else
        {
            property->setValue( valueIndex );
        }
    }

    mData->acceptValueChange = true;
}

// Set value of the specified property
void PluginPropertyEditorFrame::SetPluginProperty( QtVariantProperty* property, const XVariant& value, bool forceUiUpdate )
{
    if ( !value.IsNullOrEmpty( ) )
    {
        int32_t propertyIndex = GetPropertyIndex( property );

        if ( propertyIndex != -1 )
        {
            shared_ptr<const XPropertyDescriptor> propertyDesc = mPluginDesc->GetPropertyDescriptor( propertyIndex );

            // set property to the plug-in
            // XErrorCode error =
            mPlugin->SetProperty( propertyIndex, value );

            // read the property back to get actual value of it
            XVariant actualValue;

            mPlugin->GetProperty( propertyIndex, actualValue );

            if ( ( actualValue != value ) || ( forceUiUpdate ) )
            {
                // update property value
                DisplayCurrentPropertyValue( property, propertyDesc, actualValue );
            }

            // update all properties which may depend on this one
            UpdateDependentProperties( property, actualValue );

            emit PluginUpdated( );
            // --------------------------
            /*
            qDebug( "   setting %d: %s", propertyIndex, value.ToString( ).c_str( ) );
            qDebug( "   set prop code: %d", error );
            qDebug( "   prop type: %d", value.Type( ) );
            qDebug( "   actual value: %s", actualValue.ToString( ).c_str( ) );
            qDebug( "" );
            */
        }
    }
}

// Update configuration of all dependent properties for the given parent property
void PluginPropertyEditorFrame::UpdateDependentProperties( QtVariantProperty* parentProperty, const XVariant& parentValue )
{
    int32_t parentIndex = GetPropertyIndex( parentProperty );

    if ( parentIndex != -1 )
    {
        for ( QMap<const QtVariantProperty*, int32_t>::const_iterator it = mData->PropertiesIndexMap.begin( );
              it != mData->PropertiesIndexMap.end( ); ++it )
        {
            QtVariantProperty*                    propertyItem = const_cast<QtVariantProperty*>( it.key( ) );
            shared_ptr<const XPropertyDescriptor> propertyDesc = mPluginDesc->GetPropertyDescriptor( it.value( ) );

            if ( ( propertyDesc->IsDependent( ) ) && ( propertyDesc->GetParentPropertyID( ) == parentIndex ) )
            {
                propertyDesc->Update( parentValue );

                if ( propertyDesc->IsSelection( ) )
                {
                    ConfigureEnumProperty( propertyItem, propertyDesc );
                }
                else
                {
                    ConfigureProperty( propertyItem, propertyDesc );
                }

                propertyItem->setEnabled( !propertyDesc->IsDisabled( ) );

                // now update properties, which depend on this one
                XVariant propertyValue;
                mPlugin->GetProperty( GetPropertyIndex( propertyItem ), propertyValue );
                UpdateDependentProperties( propertyItem, propertyValue );
            }
        }
    }
}

// Index of selected item has changed
void PluginPropertyEditorFrame::on_variantEditor_currentItemChanged( QtBrowserItem* item )
{
    if ( ( item != 0 ) && ( item->property( ) != 0 ) )
    {
        const QtVariantProperty* property      = static_cast<const QtVariantProperty*>( item->property( ) );
        int32_t                  propertyIndex = GetPropertyIndex( property );
        QString                  description;

        if ( propertyIndex == -1 )
        {
            if ( item->parent( ) != 0 )
            {
                // try parent
                property = static_cast<const QtVariantProperty*>( item->parent( )->property( ) );
                propertyIndex = GetPropertyIndex( property );
            }
            else
            {
                description = QString::fromUtf8( mPluginDesc->Description( ).c_str( ) );
            }
        }

        if ( propertyIndex != -1 )
        {
            shared_ptr<const XPropertyDescriptor> propertyDesc = mPluginDesc->GetPropertyDescriptor( propertyIndex );
            description = QString::fromUtf8( propertyDesc->Description( ).c_str( ) );
        }

        ui->descriptionLabel->setText( description );
    }
}

// Value of property was changed
void PluginPropertyEditorFrame::on_variantManager_valueChanged( QtProperty* property, QVariant value )
{
    if ( mData->acceptValueChange )
    {
        QtVariantProperty* variantProperty = static_cast<QtVariantProperty*>( property );
        int32_t            propertyIndex   = GetPropertyIndex( variantProperty );

        if ( propertyIndex != -1 )
        {
            shared_ptr<const XPropertyDescriptor> propertyDesc = mPluginDesc->GetPropertyDescriptor( propertyIndex );

            if ( propertyDesc->IsSelectionByValue( ) )
            {
                uint16_t choiceIndex = (uint16_t) value.toInt( );

                if ( choiceIndex < propertyDesc->ChoicesCount( ) )
                {
                    SetPluginProperty( variantProperty, propertyDesc->GetPossibleChoice( choiceIndex ), false );
                }
            }
            else
            {
                SetPluginProperty( variantProperty, XVariantConverter::FromQVariant( value ), false );
            }

            emit ConfigurationUpdated( );
        }
    }
}
