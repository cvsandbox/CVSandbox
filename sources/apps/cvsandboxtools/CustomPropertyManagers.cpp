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

#include "CustomPropertyManagers.hpp"
#include "XVariantConverter.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QLineEdit>
#include <QToolButton>
#include <QKeyEvent>
#include <QFileDialog>
#include <QIcon>
#include <QPainter>
#include "UITools.hpp"

#include "ScriptEditorDialog.hpp"
#include "HuePickerDialog.hpp"
#include "MorphologyStructuringElementDialog.hpp"
#include "ConvolutionKernelDialog.hpp"

#include "GlobalServiceManager.hpp"

#include <XGuid.hpp>
#include <ximaging.h>

using namespace CVSandbox;

static const QString STR_LAST_FOLDER               ( "LastFolder" );
static const QString STR_FOLDER_NAME_PROPERTY      ( "FolderNameProperty" );
static const QString STR_FILE_NAME_PROPERTY        ( "FileNameProperty" );
static const QString STR_SCRIPT_FILE_NAME_PROPERTY ( "ScriptFileNameProperty" );

// Update value and notify derived class
void VariantPropertyCustomEditorWidget::setValue( const QVariant& value )
{
    if ( mValue != value )
    {
        mValue = value;
        onValueUpdated( );
    }
}

// Notify derived class of the attribute change
void VariantPropertyCustomEditorWidget::setAttribute( const QString& attribute, const QVariant& value )
{
    onAttributeChanged( attribute, value );
}

/* ===== Base factory for variant property custom editors ===== */

VariantPropertyCustomEditorFactoryBase::VariantPropertyCustomEditorFactoryBase( QObject* parent ) :
    QtAbstractEditorFactory<QtVariantPropertyManager>( parent )
{

}

VariantPropertyCustomEditorFactoryBase::~VariantPropertyCustomEditorFactoryBase( )
{
    const EditorToPropertyMap::Iterator ecend = mEditorToProperty.end( );

    for ( EditorToPropertyMap::Iterator itEditor = mEditorToProperty.begin( ); itEditor != ecend; ++itEditor )
    {
        delete itEditor.key( );
    }
}

void VariantPropertyCustomEditorFactoryBase::connectPropertyManager( QtVariantPropertyManager* manager )
{
    connect( manager, SIGNAL( valueChanged(QtProperty*, const QVariant&) ),
             this, SLOT( slotPropertyChanged(QtProperty*, const QVariant&) ) );

    connect( manager, SIGNAL( propertyDestroyed(QtProperty*) ),
             this, SLOT( slotPropertyDestroyed(QtProperty*) ) );
}

void VariantPropertyCustomEditorFactoryBase::disconnectPropertyManager( QtVariantPropertyManager* manager )
{
    disconnect( manager, SIGNAL( valueChanged(QtProperty*, const QVariant&) ),
                this, SLOT( slotPropertyChanged(QtProperty*, const QVariant&) ) );

    disconnect( manager, SIGNAL( propertyDestroyed(QtProperty*) ),
               this, SLOT( slotPropertyDestroyed(QtProperty*) ) );
}

// Create editor for the specified property
QWidget* VariantPropertyCustomEditorFactoryBase::createEditor( QtVariantPropertyManager* manager, QtProperty* property, QWidget* parent )
{
    VariantPropertyCustomEditorWidget* editor = createEditorWidget( parent );

    // add the editor to the list of editors for the property
    PropertyToEditorListMap::iterator it = mCreatedEditors.find( property );
    if ( it == mCreatedEditors.end( ) )
    {
        it = mCreatedEditors.insert( property, EditorList( ) );
    }
    it.value( ).append( editor );
    mEditorToProperty.insert( editor, property );

    // set current property value to the editor
    editor->setValue( manager->value( property ) );

    // connect with it
    connect( editor, SIGNAL( valueChanged(QVariant) ), this, SLOT( slotSetValue(QVariant) ) );
    connect( editor, SIGNAL( destroyed(QObject*) ), this, SLOT( slotEditorDestroyed(QObject*) ) );

    // tell the editor about all custom attributes the property may have
    PropertyToAttributesMap::iterator itAttrMap = mAttributes.find( property );

    if ( itAttrMap != mAttributes.end( ) )
    {
        const AttributesMap& attributes = itAttrMap.value( );

        for ( AttributesMap::ConstIterator itAttr = attributes.begin( ); itAttr != attributes.end( ); ++itAttr )
        {
            editor->setAttribute( itAttr.key( ), itAttr.value( ) );
        }
    }

    return editor;
}

// Value of the specified property has changed, so update editors responsible for it
void VariantPropertyCustomEditorFactoryBase::slotPropertyChanged( QtProperty* property, const QVariant& value )
{
    const PropertyToEditorListMap::iterator it = mCreatedEditors.find( property );

    if ( it != mCreatedEditors.end( ) )
    {
        QListIterator<VariantPropertyCustomEditorWidget*> itEditor( it.value( ) );

        while ( itEditor.hasNext( ) )
        {
            itEditor.next( )->setValue( value );
        }
    }
}

// Property is destroyed, so remove all associate attributes with it
void VariantPropertyCustomEditorFactoryBase::slotPropertyDestroyed( QtProperty* property )
{
    mAttributes.remove( property );
}

// Set custom attribute for the specified property
void VariantPropertyCustomEditorFactoryBase::setCustomAttribute( QtProperty* property, const QString& attribute, const QVariant& value )
{
    PropertyToAttributesMap::iterator it = mAttributes.find( property );

    if ( it == mAttributes.end( ) )
    {
        it = mAttributes.insert( property, AttributesMap( ) );
    }
    it.value( )[attribute] = value;
}

// Editor's value has changed, so update corresponding property
void VariantPropertyCustomEditorFactoryBase::slotSetValue( const QVariant& value )
{
    QObject* object = sender( );
    const EditorToPropertyMap::ConstIterator ecend = mEditorToProperty.constEnd( );

    for ( EditorToPropertyMap::ConstIterator itEditor = mEditorToProperty.constBegin( );
          itEditor != ecend; ++itEditor )
    {
        if ( itEditor.key( ) == object )
        {
            QtProperty*               property = itEditor.value( );
            QtVariantPropertyManager* manager  = propertyManager( property );

            if ( manager )
            {
                manager->setValue( property, value );
            }

            break;
        }
    }
}

// An editor created by the factory was destroyed, so remove it from internal collections
void VariantPropertyCustomEditorFactoryBase::slotEditorDestroyed( QObject* object )
{
    const EditorToPropertyMap::Iterator ecend = mEditorToProperty.end( );

    for ( EditorToPropertyMap::Iterator itEditor = mEditorToProperty.begin( ); itEditor != ecend; ++itEditor )
    {
        if ( static_cast<QObject*>( itEditor.key( ) ) == object )
        {
            VariantPropertyCustomEditorWidget* editor   = itEditor.key( );
            QtProperty*                        property = itEditor.value( );

            const PropertyToEditorListMap::iterator pit = mCreatedEditors.find( property );

            if ( pit != mCreatedEditors.end( ) )
            {
                pit.value( ).removeAll( editor );
                if ( pit.value( ).empty( ) )
                {
                    mCreatedEditors.erase( pit );
                }
            }
            mEditorToProperty.erase( itEditor );

            break;
        }
    }
}

/* ============================================================================
   Custom property editor for string properties which provide a button for custom editor
   ============================================================================
 */

CustomDialogPropertyEditWidget::CustomDialogPropertyEditWidget( QWidget* parent,
                                                          const QString& buttonIconResource,
                                                          const QString& buttonToolTip ) :
    VariantPropertyCustomEditorWidget( parent ),
    mEdit( new QLineEdit ),
    mButton( new QToolButton )
{
    QHBoxLayout* layout = new QHBoxLayout( this );

    layout->setContentsMargins( 4, 0, 0, 0 );
    layout->setSpacing( 0 );

    // add edit box
    mEdit->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Ignored );
    mEdit->installEventFilter( this );
    layout->addWidget( mEdit );

    // add button
    mButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Ignored );
    mButton->setFixedWidth( 20 );
    mButton->setText( tr( "..." ) );
    if ( !buttonIconResource.isEmpty( ) )
    {
        mButton->setIcon( QIcon( buttonIconResource ) );
    }
    mButton->setToolTip( buttonToolTip );
    mButton->installEventFilter( this );
    layout->addWidget( mButton );

    // let edit box take focus when editor is focused
    setFocusProxy( mEdit );
    setFocusPolicy( mEdit->focusPolicy( ) );

    connect( mButton, SIGNAL( clicked() ), this, SLOT( buttonClicked() ) );
}

// Set new value to the editting widget
void CustomDialogPropertyEditWidget::onValueUpdated( )
{
    mEdit->setText( mValue.toString( ) );
}

// Editor's button is clicked - let derived class handle it
void CustomDialogPropertyEditWidget::buttonClicked( )
{
    HandleButtonClick( );
}

// Filter some event of the edit box and button
bool CustomDialogPropertyEditWidget::eventFilter( QObject* obj, QEvent* ev )
{
    bool ret = false;

    if ( obj == mButton )
    {
        if ( ( ev->type( ) == QEvent::KeyPress ) || ( ev->type( ) == QEvent::KeyRelease ) )
        {
            switch ( static_cast<const QKeyEvent*>( ev )->key( ) )
            {
            case Qt::Key_Escape:
            case Qt::Key_Enter:
            case Qt::Key_Return:

                mEdit->setFocus( );
                ev->ignore( );
                ret = true;

            case Qt::Key_Tab:

                // don't want to leave the button on Tab, so don't let parent widget
                // to process the event and don't let the button to do anything either
                ret = true;

            default:
                break;
            }
        }
    }
    else if ( obj == mEdit )
    {
        if ( ( ev->type( ) == QEvent::KeyPress ) || ( ev->type( ) == QEvent::KeyRelease ) )
        {
            switch ( static_cast<const QKeyEvent*>( ev )->key( ) )
            {
            case Qt::Key_Enter:
            case Qt::Key_Return:
                {
                    QString directory = mEdit->text( ).trimmed( );

                    if ( mValue != directory )
                    {
                        setValue( directory );
                        emit valueChanged( directory );
                    }
                }

                break;

            case Qt::Key_Backtab:

                // similar to button - ignoring Shift+Tab so focus does not leave the editing widget
                ret = true;
                break;
            }
        }
    }

    if ( !ret )
    {
        ret = QWidget::eventFilter( obj, ev );
    }

    return ret;
}

/* ============================================================================
   Widget to edit properties only through custom editor provided on button click
   ============================================================================
 */

CustomDialogPropertyWidget::CustomDialogPropertyWidget( QWidget* parent,
                                                        const QString& buttonIconResource,
                                                        const QString& buttonToolTip ) :
    VariantPropertyCustomEditorWidget( parent ),
    mLabel( new QLabel ),
    mButton( new QToolButton )
{
    QHBoxLayout* layout = new QHBoxLayout( this );

    layout->setContentsMargins( 4, 0, 0, 0 );
    layout->setSpacing( 0 );

    // add edit box
    mLabel->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Ignored );
    //mLabel->installEventFilter( this );
    layout->addWidget( mLabel );

    // add button
    mButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Ignored );
    mButton->setFixedWidth( 20 );
    mButton->setText( tr( "..." ) );
    if ( !buttonIconResource.isEmpty( ) )
    {
        mButton->setIcon( QIcon( buttonIconResource ) );
    }
    mButton->setToolTip( buttonToolTip );
    //mButton->installEventFilter( this );
    layout->addWidget( mButton );

    // let edit box take focus when editor is focused
    setFocusProxy( mButton );
    setFocusPolicy( mButton->focusPolicy( ) );

    connect( mButton, SIGNAL( clicked() ), this, SLOT( buttonClicked() ) );
}

// Set new value to the editting widget
void CustomDialogPropertyWidget::onValueUpdated( )
{
    mLabel->setText( GetValueText( ) );
}

// Editor's button is clicked - let derived class handle it
void CustomDialogPropertyWidget::buttonClicked( )
{
    HandleButtonClick( );
}

/* ============================================================================
   Custom property editor for string properties with folder browsing support
   ============================================================================
 */

FolderPropertyEditWidget::FolderPropertyEditWidget( QWidget* parent ) :
    CustomDialogPropertyEditWidget( parent, ":/images/icons/folder.png", "Select folder" )
{
}

// Editor's button is clicked - show folder browser
void FolderPropertyEditWidget::HandleButtonClick( )
{
    QFileDialog dialog( UITools::GetParentDialog( this ) );
    QString     folderName = mValue.toString( );

    dialog.setFileMode( QFileDialog::DirectoryOnly );
    dialog.setOption( QFileDialog::ShowDirsOnly );
    dialog.setAcceptMode( QFileDialog::AcceptOpen );
    dialog.setWindowTitle( "Select folder" );
    dialog.setWindowIcon( QIcon( ":/images/icons/folder.png" ) );
    dialog.setWindowFlags( dialog.windowFlags( ) & ~Qt::WindowContextHelpButtonHint );

    if ( !folderName.isEmpty( ) )
    {
        dialog.setDirectory( folderName );
    }
    else
    {
        // try to use the same folder as was used before
        dialog.setDirectory( GlobalServiceManager::Instance( ).GetSettingsService( )->GetValue(
                             STR_LAST_FOLDER, STR_FOLDER_NAME_PROPERTY ).toString( ) );
    }

    if ( dialog.exec( ) == QDialog::Accepted )
    {
        QStringList selectedList = dialog.selectedFiles( );

        if ( selectedList.size( ) == 1 )
        {
            QString directory = selectedList[0];

            if ( mValue != directory )
            {
                setValue( directory );
                emit valueChanged( directory );

                GlobalServiceManager::Instance( ).GetSettingsService( )->SetValue( STR_LAST_FOLDER, STR_FOLDER_NAME_PROPERTY,
                                                                                   QFileInfo( directory ).absoluteFilePath( ) );
            }
        }
    }
}

/* ============================================================================
   Custom property editor for string properties with file browsing support
   ============================================================================
 */

FilePropertyEditWidget::FilePropertyEditWidget( QWidget* parent ) :
    CustomDialogPropertyEditWidget( parent, ":/images/icons/folder.png", "Select file" )
{
}

// Editor's button is clicked - show file browser
void FilePropertyEditWidget::HandleButtonClick( )
{
    QString     defaultFilter = "All files (*.*)";
    QString     fileName      = mValue.toString( );
    QFileDialog dialog( UITools::GetParentDialog( this ) );

    dialog.setFileMode( QFileDialog::ExistingFile );
    dialog.setAcceptMode( QFileDialog::AcceptOpen );
    dialog.setWindowTitle( "Select file" );
    dialog.setWindowIcon( QIcon( ":/images/icons/folder.png" ) );
    dialog.setWindowFlags( dialog.windowFlags( ) & ~Qt::WindowContextHelpButtonHint );
    dialog.setNameFilter( defaultFilter );

    if ( !fileName.isEmpty( ) )
    {
        dialog.setDirectory( QFileInfo( fileName ).dir( ) );
        dialog.selectFile( fileName );
    }
    else
    {
        // try to use the same folder as was used before
        dialog.setDirectory( GlobalServiceManager::Instance( ).GetSettingsService( )->GetValue(
                             STR_LAST_FOLDER, STR_FILE_NAME_PROPERTY ).toString( ) );
    }

    if ( dialog.exec( ) == QDialog::Accepted )
    {
        QStringList selectedList = dialog.selectedFiles( );

        if ( selectedList.size( ) == 1 )
        {
            QString fileName = selectedList[0];

            if ( mValue != fileName )
            {
                setValue( fileName );
                emit valueChanged( fileName );

                GlobalServiceManager::Instance( ).GetSettingsService( )->SetValue( STR_LAST_FOLDER, STR_FILE_NAME_PROPERTY,
                                                                                   QFileInfo( fileName ).absolutePath( ) );
            }
        }
    }
}

/* ============================================================================
   Custom property editor for string properties which represent script file name
   ============================================================================
 */

const QString ScriptFilePropertyEditWidget::ATTR_DEFAULT_EXT          = "DefExt";
const QString ScriptFilePropertyEditWidget::ATTR_SCRIPTING_NAME       = "ScriptingName";
const QString ScriptFilePropertyEditWidget::ATTR_SCRIPTING_PLUGIN_ID  = "PluginID";

ScriptFilePropertyEditWidget::ScriptFilePropertyEditWidget( QWidget* parent ) :
    VariantPropertyCustomEditorWidget( parent ),
    mEdit( new QLineEdit ),
    mButtonEdit( new QToolButton ), mButtonBrowse( new QToolButton ),
    mDefaultExt( ), mScriptingName( ), mPluginId( )
{
    QHBoxLayout* layout = new QHBoxLayout( this );

    layout->setContentsMargins( 4, 0, 0, 0 );
    layout->setSpacing( 0 );

    // add edit box
    mEdit->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Ignored );
    mEdit->installEventFilter( this );
    layout->addWidget( mEdit );

    // add button which opens script editor
    mButtonEdit->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Ignored );
    mButtonEdit->setFixedWidth( 20 );
    mButtonEdit->setIcon( QIcon( ":/images/icons/edit_file.png" ) );
    mButtonEdit->setToolTip( "Edit script file" );
    mButtonEdit->installEventFilter( this );
    layout->addWidget( mButtonEdit );

    // add button which opens file browser
    mButtonBrowse->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Ignored );
    mButtonBrowse->setFixedWidth( 20 );
    mButtonBrowse->setIcon( QIcon( ":/images/icons/open_script_file.png" ) );
    mButtonBrowse->setToolTip( "Select script file" );
    mButtonBrowse->installEventFilter( this );
    layout->addWidget( mButtonBrowse );

    // let edit box take focus when editor is focused
    setFocusProxy( mEdit );
    setFocusPolicy( mEdit->focusPolicy( ) );

    connect( mButtonBrowse, SIGNAL( clicked() ), this, SLOT( browseButtonClicked() ) );
    connect( mButtonEdit, SIGNAL( clicked() ), this, SLOT( editButtonClicked() ) );
}

// Set new value to the editting widget
void ScriptFilePropertyEditWidget::onValueUpdated( )
{
    mEdit->setText( mValue.toString( ) );
}

// Set attribute of the editing widget
void ScriptFilePropertyEditWidget::onAttributeChanged( const QString& attribute, const QVariant& value )
{
    if ( attribute == ATTR_DEFAULT_EXT )
    {
        mDefaultExt = value.toString( );
    }
    else if ( attribute == ATTR_SCRIPTING_NAME )
    {
        mScriptingName = value.toString( );
    }
    else if ( attribute == ATTR_SCRIPTING_PLUGIN_ID )
    {
        mPluginId = value.toString( );
    }
}

// Editor's button is clicked - show folder browser
void ScriptFilePropertyEditWidget::browseButtonClicked( )
{
    QString     scriptingName = ( mScriptingName.isEmpty( ) ) ? "Scripting" : mScriptingName;
    QString     defaultFilter = "All files (*.*)";
    QString     fileName      = mValue.toString( );
    QFileDialog dialog( UITools::GetParentDialog( this ) );

    if ( !mDefaultExt.isEmpty( ) )
    {
        dialog.setDefaultSuffix( mDefaultExt );
        defaultFilter = QString( "%0 files (*.%1);;").arg( scriptingName ).arg( mDefaultExt ) + defaultFilter;
    }

    dialog.setFileMode( QFileDialog::ExistingFile );
    dialog.setAcceptMode( QFileDialog::AcceptOpen );
    dialog.setWindowTitle( "Select script file" );
    dialog.setWindowIcon( QIcon( ":/images/icons/open_script_file.png" ) );
    dialog.setWindowFlags( dialog.windowFlags( ) & ~Qt::WindowContextHelpButtonHint );
    dialog.setNameFilter( defaultFilter );

    if ( !fileName.isEmpty( ) )
    {
        dialog.setDirectory( QFileInfo( fileName ).dir( ) );
        dialog.selectFile( fileName );
    }
    else
    {
        // try to use the same folder as was used before
        dialog.setDirectory( GlobalServiceManager::Instance( ).GetSettingsService( )->GetValue(
                             STR_LAST_FOLDER, STR_SCRIPT_FILE_NAME_PROPERTY ).toString( ) );
    }

    if ( dialog.exec( ) == QDialog::Accepted )
    {
        QStringList selectedList = dialog.selectedFiles( );

        if ( selectedList.size( ) == 1 )
        {
            QString fileName = selectedList[0];

            if ( mValue != fileName )
            {
                setValue( fileName );
                emit valueChanged( fileName );

                GlobalServiceManager::Instance( ).GetSettingsService( )->SetValue( STR_LAST_FOLDER, STR_SCRIPT_FILE_NAME_PROPERTY,
                                                                                   QFileInfo( fileName ).absolutePath( ) );
            }
        }
    }
}

// Open script editor for script file editing
void ScriptFilePropertyEditWidget::editButtonClicked( )
{
    ScriptEditorDialog editorDialog( UITools::GetParentDialog( this ) );

    editorDialog.SetDefaultExtension( mDefaultExt );
    editorDialog.SetScriptingName( mScriptingName );
    editorDialog.SetScriptingPluging( XGuid::FromString( mPluginId.toStdString( ) ) );

    if ( !mValue.toString( ).isEmpty( ) )
    {
        editorDialog.LoadScript( mValue.toString( ) );
    }

    editorDialog.exec( );

    // check if user changed to a different file (or re-saved it using different name)
    const QString lastOpenFile = editorDialog.LastOpenFile( );

    if ( ( mValue != lastOpenFile ) && ( !lastOpenFile.isEmpty( ) ) )
    {
        if ( ( mValue.toString( ).isEmpty( ) ) ||
             ( UITools::GetUserConfirmation( QString( "The last opened file is different:<br /><b>%0</b><br /><br />Change the property to its name?" ).arg( lastOpenFile ),
                                             UITools::GetParentDialog( this ) ) ) )
        {
            setValue( lastOpenFile );
            emit valueChanged( lastOpenFile );
        }
    }
}

// Filter some event of the edit box and button
bool ScriptFilePropertyEditWidget::eventFilter( QObject* obj, QEvent* ev )
{
    bool ret = false;

    if ( ( obj == mButtonEdit ) || (  obj == mButtonBrowse ) )
    {
        if ( ( ev->type( ) == QEvent::KeyPress ) || ( ev->type( ) == QEvent::KeyRelease ) )
        {
            switch ( static_cast<const QKeyEvent*>( ev )->key( ) )
            {
            case Qt::Key_Escape:
            case Qt::Key_Enter:
            case Qt::Key_Return:

                mEdit->setFocus( );
                ev->ignore( );
                ret = true;

            case Qt::Key_Tab:

                if ( obj == mButtonBrowse )
                {
                    // don't want to leave the button on Tab, so don't let parent widget
                    // to process the event and don't let the button to do anything either
                    ret = true;
                }

            default:
                break;
            }
        }
    }
    else if ( obj == mEdit )
    {
        if ( ( ev->type( ) == QEvent::KeyPress ) || ( ev->type( ) == QEvent::KeyRelease ) )
        {
            switch ( static_cast<const QKeyEvent*>( ev )->key( ) )
            {
            case Qt::Key_Enter:
            case Qt::Key_Return:
                {
                    QString directory = mEdit->text( ).trimmed( );

                    if ( mValue != directory )
                    {
                        setValue( directory );
                        emit valueChanged( directory );
                    }
                }

                break;

            case Qt::Key_Backtab:

                // similar to button - ignoring Shift+Tab so focus does not leave the editing widget
                ret = true;
                break;
            }
        }
    }

    if ( !ret )
    {
        ret = QWidget::eventFilter( obj, ev );
    }

    return ret;
}

/* ============================================================================
   Custom property editor for properties which represent hue value
   ============================================================================
 */

HuePickerPropertyEditWidget::HuePickerPropertyEditWidget( QWidget* parent ) :
    VariantPropertyCustomEditorWidget( parent ),
    mLabel( new QLabel ),
    mSpinBox( new QSpinBox ),
    mButtonEdit( new QToolButton )
{
    QHBoxLayout* layout = new QHBoxLayout( this );

    layout->setContentsMargins( 4, 0, 0, 0 );
    layout->setSpacing( 0 );

    // add label
    mLabel->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Ignored );
    mLabel->setFixedWidth( 16 );
    mLabel->setFixedHeight( 16 );
    mLabel->setMargin( 2 );
    mLabel->setFrameShape( QFrame::Box );
    layout->addWidget( mLabel );

    // spacer
    QLabel* spacer = new QLabel( );
    spacer->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Ignored );
    spacer->setFixedWidth( 4 );
    layout->addWidget( spacer );

    // add spin box
    mSpinBox->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Ignored );
    mSpinBox->setMinimum( 0 );
    mSpinBox->setMaximum( 359 );
    mSpinBox->installEventFilter( this );
    layout->addWidget( mSpinBox );

    // add button which opens script editor
    mButtonEdit->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Ignored );
    mButtonEdit->setFixedWidth( 20 );
    mButtonEdit->setText( "... " );
    mButtonEdit->setToolTip( "Pick hue value" );
    mButtonEdit->installEventFilter( this );
    layout->addWidget( mButtonEdit );

    // let spin box take focus when editor is focused
    setFocusProxy( mSpinBox );
    setFocusPolicy( mSpinBox->focusPolicy( ) );

    connect( mButtonEdit, SIGNAL( clicked() ), this, SLOT( editButtonClicked() ) );
    connect( mSpinBox, SIGNAL( valueChanged(int) ), this, SLOT( spinBoxValueChanged(int) ) );
}

// Set new value to the editting widget
void HuePickerPropertyEditWidget::onValueUpdated( )
{
    uint16_t hueValue = XINRANGE( mValue.toUInt( ), 0, 359 );
    xhsl  hsl = { hueValue, 1.0f, 0.5f };
    xargb rgb = { 0 };

    mSpinBox->setValue( hueValue );

    Hsl2Rgb( &hsl, &rgb );

    UITools::SetWidgetBackgroundColor( mLabel, QColor::fromRgb( rgb.components.r,
                                                                rgb.components.g,
                                                                rgb.components.b ) );

    emit valueChanged( hueValue );
}


// Open hue picker dialog
void HuePickerPropertyEditWidget::editButtonClicked( )
{
    HuePickerDialog editorDialog( UITools::GetParentDialog( this ) );
    uint16_t hueValue = XINRANGE( mValue.toUInt( ), 0, 359 );

    editorDialog.setHueValue( hueValue );

    if ( editorDialog.exec( ) == QDialog::Accepted )
    {
        uint16_t newHueValue = editorDialog.hueValue( );

        if ( newHueValue != hueValue )
        {
            setValue( newHueValue );
            emit valueChanged( newHueValue );
            mSpinBox->selectAll( );
        }
    }
}

// Hue spin box has changed its value, so update the property
void HuePickerPropertyEditWidget::spinBoxValueChanged( int i )
{
    setValue( QVariant( i ) );
}

// Filter some events of the sping box and button
bool HuePickerPropertyEditWidget::eventFilter( QObject* obj, QEvent* ev )
{
    bool ret = false;

    if ( obj == mButtonEdit )
    {
        if ( ( ev->type( ) == QEvent::KeyPress ) || ( ev->type( ) == QEvent::KeyRelease ) )
        {
            switch ( static_cast<const QKeyEvent*>( ev )->key( ) )
            {
            case Qt::Key_Escape:
            case Qt::Key_Enter:
            case Qt::Key_Return:

                mSpinBox->setFocus( );
                ev->ignore( );
                ret = true;

            case Qt::Key_Tab:

                // don't want to leave the button on Tab, so don't let parent widget
                // to process the event and don't let the button to do anything either
                ret = true;

            default:
                break;
            }
        }
    }
    else if ( obj == mSpinBox )
    {
        if ( ( ev->type( ) == QEvent::KeyPress ) || ( ev->type( ) == QEvent::KeyRelease ) )
        {
            switch ( static_cast<const QKeyEvent*>( ev )->key( ) )
            {
            case Qt::Key_Enter:
                /*{
                    if ( mValue != directory )
                    {
                        setValue( directory );
                        emit valueChanged( directory );
                    }
                }*/

                break;

            case Qt::Key_Backtab:

                // similar to button - ignoring Shift+Tab so focus does not leave the editing widget
                ret = true;
                break;
            }
        }
    }

    if ( !ret )
    {
        ret = QWidget::eventFilter( obj, ev );
    }

    return ret;
}

/* ============================================================================
   Custom property manager for hue picker
   ============================================================================
 */

// Create pixmap for the specified brush
QPixmap HuePickerPropertyManager::brushValuePixmap( const QBrush& b )
{
    QImage img( 16, 16, QImage::Format_ARGB32_Premultiplied );
    QPainter painter( &img );

    painter.setCompositionMode( QPainter::CompositionMode_Source );
    painter.fillRect( 0, 0, 16, 16, b );
    painter.end( );

    return QPixmap::fromImage( img );
}

HuePickerPropertyManager::HuePickerPropertyManager( QObject* parent ) :
    QtVariantPropertyManager( parent )
{

}

// Provide value of the specified property
QVariant HuePickerPropertyManager::value( const QtProperty* property ) const
{
    return QVariant( m_values.value( property, 0 ) );
}

// Set value for the specified property
void HuePickerPropertyManager::setValue( QtProperty *property, const QVariant& val )
{
    uint16_t hueValue = static_cast<uint16_t>( val.toUInt( ) );

    const PropertyValueMap::iterator it = m_values.find( property );

    if ( ( it != m_values.end( ) ) && ( it.value( ) != hueValue ) )
    {
        it.value( ) = hueValue;

        emit propertyChanged( property );
        emit valueChanged( property, val );
    }
}

// Get display text for the property
QString HuePickerPropertyManager::valueText( const QtProperty* property ) const
{
    return QString( "%0" ).arg( m_values.value( property, 0 ) );
}

// Get display icon for the property
QIcon HuePickerPropertyManager::valueIcon( const QtProperty* property ) const
{
    const PropertyValueMap::const_iterator it = m_values.constFind( property );
    QIcon icon;

    if ( it != m_values.constEnd( ) )
    {
        xhsl  hsl = { it.value( ), 1.0f, 0.5f };
        xargb rgb = { 0 };

        Hsl2Rgb( &hsl, &rgb );

        icon = QIcon( brushValuePixmap( QBrush( QColor::fromRgb( rgb.components.r, rgb.components.g, rgb.components.b ) ) ) );
    }

    return icon;
}

void HuePickerPropertyManager::initializeProperty( QtProperty* property )
{
    m_values[property] = 0;
}

void HuePickerPropertyManager::uninitializeProperty( QtProperty* property )
{
    m_values.remove( property );
}

/* ============================================================================
   Custom property editor for properties which represent hue range
   ============================================================================
 */

HueRangePickerPropertyEditWidget::HueRangePickerPropertyEditWidget( QWidget* parent ) :
    VariantPropertyCustomEditorWidget( parent ),
    mHueLabel1( new QLabel ),
    mHueLabel2( new QLabel ),
    mValueLabel( new QLabel ),
    mButtonEdit( new QToolButton )
{
    QHBoxLayout* layout = new QHBoxLayout( this );

    layout->setContentsMargins( 4, 0, 0, 0 );
    layout->setSpacing( 0 );

    // add min hue label
    mHueLabel1->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Ignored );
    mHueLabel1->setFixedWidth( 16 );
    mHueLabel1->setFixedHeight( 16 );
    mHueLabel1->setMargin( 2 );
    mHueLabel1->setFrameShape( QFrame::Box );
    layout->addWidget( mHueLabel1 );

    // 1st spacer
    QLabel* spacer = new QLabel( );
    spacer->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Ignored );
    spacer->setFixedWidth( 2 );
    layout->addWidget( spacer );

    // add max hue label
    mHueLabel2->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Ignored );
    mHueLabel2->setFixedWidth( 16 );
    mHueLabel2->setFixedHeight( 16 );
    mHueLabel2->setMargin( 2 );
    mHueLabel2->setFrameShape( QFrame::Box );
    layout->addWidget( mHueLabel2 );

    // 2nd spacer
    spacer = new QLabel( );
    spacer->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Ignored );
    spacer->setFixedWidth( 4 );
    layout->addWidget( spacer );

    // add value label
    mValueLabel->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Ignored );
    mValueLabel->setMargin( 2 );
    layout->addWidget( mValueLabel );

    // add button which opens script editor
    mButtonEdit->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Ignored );
    mButtonEdit->setFixedWidth( 20 );
    mButtonEdit->setText( "... " );
    mButtonEdit->setToolTip( "Pick hue rane" );
    mButtonEdit->installEventFilter( this );
    layout->addWidget( mButtonEdit );

    // let button take focus when editor is focused
    setFocusProxy( mButtonEdit );
    setFocusPolicy( mButtonEdit->focusPolicy( ) );

    connect( mButtonEdit, SIGNAL( clicked() ), this, SLOT( editButtonClicked() ) );
}

// Set new value to the editting widget
void HueRangePickerPropertyEditWidget::onValueUpdated( )
{
    XVariant   hueRangeVar = XVariantConverter::FromQVariant( mValue );
    xhsl       hsl         = { 0, 1.0f, 0.5f };
    xargb      rgb         = { 0 };
    XErrorCode ecode       = SuccessCode;
    XRange     hueRange    = hueRangeVar.ToRange( &ecode );

    if ( ecode == SuccessCode )
    {
        mValueLabel->setText( QString( "[%0, %1]" ).arg( hueRange.Min( ) ).arg( hueRange.Max( ) ) );

        // min
        hsl.Hue = hueRange.Min( );

        Hsl2Rgb( &hsl, &rgb );

        UITools::SetWidgetBackgroundColor( mHueLabel1, QColor::fromRgb( rgb.components.r,
                                                                        rgb.components.g,
                                                                        rgb.components.b ) );

        // min
        hsl.Hue = hueRange.Max( );

        Hsl2Rgb( &hsl, &rgb );

        UITools::SetWidgetBackgroundColor( mHueLabel2, QColor::fromRgb( rgb.components.r,
                                                                        rgb.components.g,
                                                                        rgb.components.b ) );

        emit valueChanged( mValue );
    }
}

// Open hue picker dialog
void HueRangePickerPropertyEditWidget::editButtonClicked( )
{
    XVariant   hueRangeVar = XVariantConverter::FromQVariant( mValue );
    XRange     hueRange    = hueRangeVar.ToRange( );

    HuePickerDialog editorDialog( UITools::GetParentDialog( this ) );

    editorDialog.setHueRange( hueRange );

    if ( editorDialog.exec( ) == QDialog::Accepted )
    {
        XRange newHueRange = editorDialog.hueRange( );

        if ( newHueRange != hueRange )
        {
            QVariant varHueRange = XVariantConverter::ToQVariant( XVariant( newHueRange ) );
            setValue( varHueRange );
            emit valueChanged( varHueRange );
        }
    }
}

// Filter some events of the sping box and button
bool HueRangePickerPropertyEditWidget::eventFilter( QObject* obj, QEvent* ev )
{
    bool ret = false;

    if ( obj == mButtonEdit )
    {
        if ( ( ev->type( ) == QEvent::KeyPress ) || ( ev->type( ) == QEvent::KeyRelease ) )
        {
            switch ( static_cast<const QKeyEvent*>( ev )->key( ) )
            {
            case Qt::Key_Tab:

                // don't want to leave the button on Tab, so don't let parent widget
                // to process the event and don't let the button to do anything either
                ret = true;

            default:
                break;
            }
        }
    }

    if ( !ret )
    {
        ret = QWidget::eventFilter( obj, ev );
    }

    return ret;
}

/* ============================================================================
   Custom property manager for hue range picker
   ============================================================================
 */

// Create pixmap for the specified brush
QPixmap HueRangePickerPropertyManager::brushValuePixmap( const QBrush& b1, const QBrush& b2 )
{
    QImage img( 16, 16, QImage::Format_ARGB32_Premultiplied );
    QPainter painter( &img );

    painter.setCompositionMode( QPainter::CompositionMode_Source );
    painter.fillRect( 0, 0, 8, 16, b1 );
    painter.fillRect( 8, 0, 8, 16, b2 );
    painter.end( );

    return QPixmap::fromImage( img );
}

HueRangePickerPropertyManager::HueRangePickerPropertyManager( QObject* parent ) :
    QtVariantPropertyManager( parent )
{
    m_huePickerPropertyManager = new HuePickerPropertyManager( this );

    connect( m_huePickerPropertyManager, SIGNAL( valueChanged(QtProperty*, const QVariant&) ),
             this, SLOT( hueChanged(QtProperty *, const QVariant&) ) );
    connect( m_huePickerPropertyManager, SIGNAL( propertyDestroyed(QtProperty*) ),
             this, SLOT( huePropertyDestroyed(QtProperty *) ) );
}

// Check if property type is supported by the manager
bool HueRangePickerPropertyManager::isPropertyTypeSupported( int propertyType ) const
{
    return ( propertyType == XVariantConverter::ToQVariantType( XVT_Range ) );
}

// Provide value of the specified property
QVariant HueRangePickerPropertyManager::value( const QtProperty* property ) const
{
    return QVariant( m_values.value( property, XVariantConverter::ToQVariant( XVariant( XRange( 0, 359 ) ) ) ) );
}

// Set value for the specified property
void HueRangePickerPropertyManager::setValue( QtProperty* property, const QVariant& val )
{
    const PropertyValueMap::iterator it = m_values.find( property );

    if ( ( it != m_values.end( ) ) && ( it.value( ) != val ) )
    {
        XVariant varHueRange = XVariantConverter::FromQVariant( val );
        XRange   hueRange    = varHueRange.ToRange( );

        it.value( ) = val;

        m_huePickerPropertyManager->setValue( m_propertyToMin[property], QVariant( hueRange.Min( ) ) );
        m_huePickerPropertyManager->setValue( m_propertyToMax[property], QVariant( hueRange.Max( ) ) );

        emit propertyChanged( property );
        emit valueChanged( property, val );
    }
}

// Provide manager of the hue sub properties
HuePickerPropertyManager* HueRangePickerPropertyManager::subHuePropertyManager( ) const
{
    return m_huePickerPropertyManager;
}

// Get display text for the property
QString HueRangePickerPropertyManager::valueText( const QtProperty* property ) const
{
    XVariant   hueRangeVar = XVariantConverter::FromQVariant( value( property ) );
    XErrorCode ecode       = SuccessCode;
    XRange     hueRange    = hueRangeVar.ToRange( &ecode );

    return ( ecode == SuccessCode ) ? QString( "[%0, %1]" ).arg( hueRange.Min( ) ).arg( hueRange.Max( ) ) : QString( );
}

// Get display icon for the property
QIcon HueRangePickerPropertyManager::valueIcon( const QtProperty* property ) const
{
    const PropertyValueMap::const_iterator it = m_values.constFind( property );
    QIcon icon;

    if ( it != m_values.constEnd( ) )
    {
        XVariant hueRangeVar = XVariantConverter::FromQVariant( it.value( ) );
        XRange   hueRange    = hueRangeVar.ToRange( );

        xhsl  hsl  = { 0, 1.0f, 0.5f };
        xargb rgb1 = { 0 };
        xargb rgb2 = { 0 };

        hsl.Hue = hueRange.Min( );
        Hsl2Rgb( &hsl, &rgb1 );

        hsl.Hue = hueRange.Max( );
        Hsl2Rgb( &hsl, &rgb2 );

        icon = QIcon( brushValuePixmap( QBrush( QColor::fromRgb( rgb1.components.r, rgb1.components.g, rgb1.components.b ) ),
                                        QBrush( QColor::fromRgb( rgb2.components.r, rgb2.components.g, rgb2.components.b ) ) ) );
    }

    return icon;
}

// Initialize the specified property
void HueRangePickerPropertyManager::initializeProperty( QtProperty* property )
{
    m_values[property] = XVariantConverter::ToQVariant( XVariant( XRange( 0, 359 ) ) );

    QtProperty* minProp = m_huePickerPropertyManager->addProperty( QVariant::Int, tr( "Min" ) );
    m_huePickerPropertyManager->setValue( minProp, QVariant( 0 ) );
    m_propertyToMin[property] = minProp;
    m_minToProperty[minProp] = property;
    property->addSubProperty( minProp );

    QtProperty* maxProp = m_huePickerPropertyManager->addProperty( QVariant::Int, tr( "Max" ) );
    m_huePickerPropertyManager->setValue( maxProp, QVariant( 359 ) );
    m_propertyToMax[property] = maxProp;
    m_maxToProperty[maxProp] = property;
    property->addSubProperty( maxProp );
}

// Uninitialize the specified property
void HueRangePickerPropertyManager::uninitializeProperty( QtProperty* property )
{
    QtProperty* minProp = m_propertyToMin[property];
    if ( minProp )
    {
        m_minToProperty.remove( minProp );
        delete minProp;
    }
    m_propertyToMin.remove( property );

    QtProperty* maxProp = m_propertyToMax[property];
    if ( maxProp )
    {
        m_maxToProperty.remove( maxProp );
        delete maxProp;
    }
    m_propertyToMax.remove( property );

    m_values.remove( property );
}

// Child hue property value has changed
void HueRangePickerPropertyManager::hueChanged( QtProperty* property, const QVariant& value )
{
    if ( QtProperty *hueRangeProp = m_minToProperty.value( property, nullptr ) )
    {
        XVariant hueRangeVar = XVariantConverter::FromQVariant( m_values[hueRangeProp] );
        XRange   hueRange    = hueRangeVar.ToRange( );

        hueRange.SetMin( value.toInt( ) );

        setValue( hueRangeProp, XVariantConverter::ToQVariant( XVariant( hueRange ) ) );
    }
    else if ( QtProperty *hueRangeProp = m_maxToProperty.value( property, nullptr ) )
    {
        XVariant hueRangeVar = XVariantConverter::FromQVariant( m_values[hueRangeProp] );
        XRange   hueRange    = hueRangeVar.ToRange( );

        hueRange.SetMax( value.toInt( ) );

        setValue( hueRangeProp, XVariantConverter::ToQVariant( XVariant( hueRange ) ) );
    }
}

// Child property was destroyed
void HueRangePickerPropertyManager::huePropertyDestroyed( QtProperty* property )
{
    if ( QtProperty *hueRangeProp = m_minToProperty.value( property, nullptr ) )
    {
        m_propertyToMin[hueRangeProp] = nullptr;
        m_minToProperty.remove( property );
    }
    else if ( QtProperty *hueRangeProp = m_maxToProperty.value( property, nullptr ) )
    {
        m_propertyToMax[hueRangeProp] = nullptr;
        m_maxToProperty.remove( property );
    }
}

/* ============================================================================
   Custom property editor for structuring elements of morphology operators
   ============================================================================
 */

const QString StructuringElementPropertyWidget::ATTR_MODE = "Mode";

static QString GetTextForStructurinElement( const XVariantArray2d& se )
{
    return ( se.IsAllocated( ) ) ? QString( "%0x%0 structuring element" ).arg( se.Rows( ) ) :
                                   QString( "not set" );
}

StructuringElementPropertyWidget::StructuringElementPropertyWidget( QWidget* parent ) :
    CustomDialogPropertyWidget( parent, ":/images/icons/table.png", "Edit structuring elemet" ),
    mMode( 0 )
{
}

// Get text to display current value
QString StructuringElementPropertyWidget::GetValueText( ) const
{
    XVariant        valueVar   = XVariantConverter::FromQVariant( mValue );
    XVariantArray2d arrayValue = valueVar.ToArray2d( );

    return GetTextForStructurinElement( arrayValue );
}

// Custom attribute has changed - check for editor mode
void StructuringElementPropertyWidget::onAttributeChanged( const QString& attribute, const QVariant& value )
{
    if ( attribute == ATTR_MODE )
    {
        mMode = value.toInt( );
    }
}

// Editor's button is clicked - show file browser
void StructuringElementPropertyWidget::HandleButtonClick( )
{
    MorphologyStructuringElementDialog editorDialog( static_cast<MorphologyStructuringElementDialog::Mode>( mMode ),
                                                     UITools::GetParentDialog( this ) );
    XVariant seVar = XVariantConverter::FromQVariant( mValue );
    XVariantArray2d structuringElement = seVar.ToArray2d( );

    if ( structuringElement.IsAllocated( ) )
    {
        editorDialog.SetStructuringElement( structuringElement );
    }

    if ( editorDialog.exec( ) == QDialog::Accepted )
    {
        XVariantArray2d newStructuringElement = editorDialog.StructuringElement( );

        if ( newStructuringElement != structuringElement )
        {
            QVariant seQVar = XVariantConverter::ToQVariant( XVariant( newStructuringElement ) );
            setValue( seQVar );
            emit valueChanged( seQVar );
        }
    }
}

/* ============================================================================
   Custom property manager for structuring elements' editors
   ============================================================================
 */

StructuringElementPropertyManager::StructuringElementPropertyManager( QObject* parent ) :
    QtVariantPropertyManager( parent )
{

}

// Check if property type is supported by the manager
bool StructuringElementPropertyManager::isPropertyTypeSupported( int propertyType ) const
{
    return ( propertyType == XVariantConverter::ToQVariantType( XVT_Array2d ) );
}

// Provide value of the specified property
QVariant StructuringElementPropertyManager::value( const QtProperty* property ) const
{
    return XVariantConverter::ToQVariant( XVariant( m_values.value( property, XVariantArray2d( ) ) ) );
}

// Set value for the specified property
void StructuringElementPropertyManager::setValue( QtProperty *property, const QVariant& val )
{
    XVariant        var        = XVariantConverter::FromQVariant( val );
    XVariantArray2d arrayValue = var.ToArray2d( );

    const PropertyValueMap::iterator it = m_values.find( property );

    if ( ( it != m_values.end( ) ) && ( it.value( ) != arrayValue ) )
    {
        it.value( ) = arrayValue;

        emit propertyChanged( property );
        emit valueChanged( property, val );
    }
}

// Get display text for the property
QString StructuringElementPropertyManager::valueText( const QtProperty* property ) const
{
    return GetTextForStructurinElement( m_values[property] );
}

void StructuringElementPropertyManager::initializeProperty( QtProperty* property )
{
    m_values[property] = XVariantArray2d( );
}

void StructuringElementPropertyManager::uninitializeProperty( QtProperty* property )
{
    m_values.remove( property );
}

/* ============================================================================
   Custom property editor for convolution kernels
   ============================================================================
 */

static QString GetTextForConvolutionKernel( const XVariantArray2d& se )
{
    return ( se.IsAllocated( ) ) ? QString( "%0x%0 convolution kernel" ).arg( se.Rows( ) ) :
                                   QString( "not set" );
}

ConvolutionKernelPropertyWidget::ConvolutionKernelPropertyWidget( QWidget* parent ) :
    CustomDialogPropertyWidget( parent, ":/images/icons/table.png", "Edit convolution kernel" )
{
}

// Get text to display current value
QString ConvolutionKernelPropertyWidget::GetValueText( ) const
{
    XVariant        valueVar   = XVariantConverter::FromQVariant( mValue );
    XVariantArray2d arrayValue = valueVar.ToArray2d( );

    return GetTextForConvolutionKernel( arrayValue );
}

// Editor's button is clicked - show file browser
void ConvolutionKernelPropertyWidget::HandleButtonClick( )
{
    ConvolutionKernelDialog editorDialog( UITools::GetParentDialog( this ) );
    XVariant seVar = XVariantConverter::FromQVariant( mValue );
    XVariantArray2d kernel = seVar.ToArray2d( );

    if ( kernel.IsAllocated( ) )
    {
        editorDialog.SetKernel( kernel );
    }

    if ( editorDialog.exec( ) == QDialog::Accepted )
    {
        XVariantArray2d newKernel = editorDialog.Kernel( );

        if ( newKernel != kernel )
        {
            QVariant kernelQVar = XVariantConverter::ToQVariant( XVariant( newKernel ) );
            setValue( kernelQVar );
            emit valueChanged( kernelQVar );
        }
    }
}

/* ============================================================================
   Custom property manager for convolution kernels' editors
   ============================================================================
 */

ConvolutionKernelPropertyManager::ConvolutionKernelPropertyManager( QObject* parent ) :
    QtVariantPropertyManager( parent )
{

}

// Check if property type is supported by the manager
bool ConvolutionKernelPropertyManager::isPropertyTypeSupported( int propertyType ) const
{
    return ( propertyType == XVariantConverter::ToQVariantType( XVT_Array2d ) );
}

// Provide value of the specified property
QVariant ConvolutionKernelPropertyManager::value( const QtProperty* property ) const
{
    return XVariantConverter::ToQVariant( XVariant( m_values.value( property, XVariantArray2d( ) ) ) );
}

// Set value for the specified property
void ConvolutionKernelPropertyManager::setValue( QtProperty *property, const QVariant& val )
{
    XVariant        var        = XVariantConverter::FromQVariant( val );
    XVariantArray2d arrayValue = var.ToArray2d( );

    const PropertyValueMap::iterator it = m_values.find( property );

    if ( ( it != m_values.end( ) ) && ( it.value( ) != arrayValue ) )
    {
        it.value( ) = arrayValue;

        emit propertyChanged( property );
        emit valueChanged( property, val );
    }
}

// Get display text for the property
QString ConvolutionKernelPropertyManager::valueText( const QtProperty* property ) const
{
    return GetTextForConvolutionKernel( m_values[property] );
}

void ConvolutionKernelPropertyManager::initializeProperty( QtProperty* property )
{
    m_values[property] = XVariantArray2d( );
}

void ConvolutionKernelPropertyManager::uninitializeProperty( QtProperty* property )
{
    m_values.remove( property );
}
