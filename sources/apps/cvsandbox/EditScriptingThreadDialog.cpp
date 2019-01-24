/*
    Computer Vision Sandbox

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

#include "EditScriptingThreadDialog.hpp"
#include "ui_EditScriptingThreadDialog.h"
#include <QPushButton>

#include <UITools.hpp>
#include <XImageInterface.hpp>
#include "ServiceManager.hpp"
#include "XGuidGenerator.hpp"

using namespace std;
using namespace CVSandbox;

namespace Private
{
    class EditScriptingThreadDialogData
    {
    public:
        ScriptingThreadDesc ThreadDesc;
        bool                EditMode;
        set<string>         ExistingNames;

    public:
        EditScriptingThreadDialogData( const ScriptingThreadDesc& threadDesc ) :
            ThreadDesc( threadDesc ),
            EditMode( !threadDesc.PluginId( ).IsEmpty( ) ),
            ExistingNames( )
        {

        }

        void CollectScriptingEnginePlugins( QComboBox* scriptsCombo );
    };
}

EditScriptingThreadDialog::EditScriptingThreadDialog( const ScriptingThreadDesc& threadDesc, QWidget* parent ) :
    QDialog( parent ),
    ui( new Ui::EditScriptingThreadDialog ),
    mData( new Private::EditScriptingThreadDialogData( threadDesc ) )
{
    ui->setupUi( this );

    // disable "?" button on title bar
    setWindowFlags( windowFlags( ) & ~Qt::WindowContextHelpButtonHint );

    // set dialog' title
    this->setWindowTitle( ( mData->EditMode ) ? "Edit Scripting Thread" : "Add Scripting Thread" );

    // set labels' buddies for quick navigation
    ui->nameLabel->setBuddy( ui->nameEdit );
    ui->intervalLabel->setBuddy( ui->intervalSpinBox );
    ui->engineLabel->setBuddy( ui->scriptsComboBox );
    ui->descriptionLabel->setBuddy( ui->descriptionEdit );

    mData->CollectScriptingEnginePlugins( ui->scriptsComboBox );

    QString name = QString::fromUtf8( mData->ThreadDesc.Name( ).c_str( ) );

    if ( mData->EditMode )
    {
        ui->nameEdit->setText( name );
        ui->descriptionEdit->setPlainText( QString::fromUtf8( mData->ThreadDesc.Description( ).c_str( ) ) );
        ui->intervalSpinBox->setValue( mData->ThreadDesc.Interval( ) );
    }

    on_nameEdit_textChanged( name );
}

EditScriptingThreadDialog::~EditScriptingThreadDialog( )
{
    delete ui;
}

// Get description of the thread after dialog has been accepted
ScriptingThreadDesc EditScriptingThreadDialog::ThreadDescription( ) const
{
    return mData->ThreadDesc;
}

// Set names of existing threads, which can not be used
void EditScriptingThreadDialog::SetExistingNames( const std::set<std::string>& names )
{
    mData->ExistingNames = names;
}

// Thread name was changed
void EditScriptingThreadDialog::on_nameEdit_textChanged( const QString& str )
{
    QString threadName = str.trimmed( );

    ui->okButton->setEnabled( !threadName.isEmpty( ) );
}

// Cancel button was clicked
void EditScriptingThreadDialog::on_cancelButton_clicked( )
{
    reject( );
}

// OK button was clicked
void EditScriptingThreadDialog::on_okButton_clicked( )
{
    string   name        = ui->nameEdit->text( ).trimmed( ).toUtf8( ).data( );
    string   description = ui->descriptionEdit->toPlainText( ).trimmed( ).toUtf8( ).data( );
    uint32_t interval    = static_cast<uint32_t>( ui->intervalSpinBox->value( ) );

    if ( mData->ExistingNames.find( name ) == mData->ExistingNames.end( ) )
    {
        if ( mData->EditMode )
        {
            mData->ThreadDesc.SetName( name );
            mData->ThreadDesc.SetDescription( description );
            mData->ThreadDesc.SetInterval( interval );
        }
        else
        {
            XGuid pluginId( ui->scriptsComboBox->itemData( ui->scriptsComboBox->currentIndex( ) ).toString( ).toStdString( ) );

            mData->ThreadDesc = ScriptingThreadDesc( XGuidGenerator::Generate( ), name, interval,
                                                     pluginId, description );
        }

        accept( );
    }
    else
    {
        UITools::ShowErrorMessage( "A scripting thread with specified name already exists.", this );
        ui->nameEdit->setFocus( );
    }
}

namespace Private
{

// Get index of the scripting plug-in in the combo box
static int GetPluginComboIndex( QComboBox* scriptsCombo, XGuid id )
{
    int index = -1;

    for ( int i = 0, n = scriptsCombo->count( ); i < n; i++ )
    {
        XGuid pluginId( scriptsCombo->itemData( i ).toString( ).toStdString( ) );

        if ( pluginId == id )
        {
            index = i;
            break;
        }
    }

    return index;
}

// Collect available scripting engine plug-ins
void EditScriptingThreadDialogData::CollectScriptingEnginePlugins( QComboBox* scriptsCombo )
{
    shared_ptr<const XPluginsCollection> scriptingPlugins = ServiceManager::Instance( ).GetPluginsEngine( )->
                                                            GetPluginsOfType( PluginType_ScriptingEngine );

    scriptsCombo->clear( );

    for ( XPluginsCollection::ConstIterator it = scriptingPlugins->begin( );
          it != scriptingPlugins->end( ); ++it )
    {
        shared_ptr<const XPluginDescriptor> pluginDesc = *it;

        shared_ptr<const XImage> icon = pluginDesc->Icon( );

        QString itemTitle = pluginDesc->Name( ).c_str( );
        QVariant itemData = QString( pluginDesc->ID( ).ToString( ).c_str( ) );

        if ( icon )
        {
            shared_ptr<QImage> qimage = XImageInterface::XtoQimage( icon );
            scriptsCombo->addItem( QIcon( QPixmap::fromImage( *qimage ) ), itemTitle, itemData );
        }
        else
        {
            scriptsCombo->addItem( itemTitle, itemData );
        }
    }

    // sort in alphabetic order
    scriptsCombo->model( )->sort( 0 );

    int indexToSelect = 0;

    if ( EditMode )
    {
        indexToSelect = GetPluginComboIndex( scriptsCombo, ThreadDesc.PluginId( ) );

        if ( indexToSelect == -1 )
        {
            scriptsCombo->clear( );
            scriptsCombo->addItem( "Not found" );
        }

        scriptsCombo->setEnabled( false );
    }

    scriptsCombo->setCurrentIndex( indexToSelect );
}

} // namespace Private
