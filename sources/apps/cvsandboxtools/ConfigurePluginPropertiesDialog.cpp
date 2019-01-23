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

#include "ConfigurePluginPropertiesDialog.hpp"
#include "ui_ConfigurePluginPropertiesDialog.h"
#include "GlobalServiceManager.hpp"

using namespace std;
using namespace CVSandbox;

ConfigurePluginPropertiesDialog::ConfigurePluginPropertiesDialog(
        const shared_ptr<const XPluginDescriptor>& pluginDesc,
        const shared_ptr<XPlugin>& plugin,
        bool showDeviceRuntimeProperties, const QWidget* parent ) :
    QDialog( const_cast<QWidget*>( parent ) ),
    ui( new Ui::ConfigurePluginPropertiesDialog ),
    mPropertyEditor( pluginDesc, plugin, showDeviceRuntimeProperties, nullptr )
{
    ui->setupUi( this );

    // disable "?" button on title bar
    setWindowFlags( windowFlags( ) & ~Qt::WindowContextHelpButtonHint );
    EnableHelpButton( false );
    EnableDefaultsButton( false );

    // set title of the group box
    ui->groupBox->setTitle( ( !showDeviceRuntimeProperties ) ? "Plug-in properties" : "Plug-in run time properties" );

    // restore size/position
    GlobalServiceManager::Instance( ).GetUiPersistenceService( )->RestoreWidget( this );

    // add property editor to the form
    QVBoxLayout* vlayout = new QVBoxLayout( ui->groupBox );
    vlayout->addWidget( &mPropertyEditor );
    connect( &mPropertyEditor, SIGNAL( ConfigurationUpdated() ), this, SLOT( on_propertyEditor_ConfigurationUpdated() ) );

    setWindowTitle( QString( "%1 : %2" ).arg( windowTitle( ) )
                                        .arg( QString( pluginDesc->Name( ).c_str( ) ) ) );

    mPropertyEditor.setFocus( );
}

ConfigurePluginPropertiesDialog::~ConfigurePluginPropertiesDialog( )
{
    delete ui;
}

// Widget is closed
void ConfigurePluginPropertiesDialog::closeEvent( QCloseEvent* )
{
    GlobalServiceManager::Instance( ).GetUiPersistenceService( )->SaveWidget( this );
}

// Dialog is accepted
void ConfigurePluginPropertiesDialog::accept( )
{
    GlobalServiceManager::Instance( ).GetUiPersistenceService( )->SaveWidget( this );
    QDialog::accept( );
}

// Dialog is rejected
void ConfigurePluginPropertiesDialog::reject( )
{
    GlobalServiceManager::Instance( ).GetUiPersistenceService( )->SaveWidget( this );
    QDialog::reject( );
}

// Show/hide help button
void ConfigurePluginPropertiesDialog::EnableHelpButton( bool enable )
{
    ui->helpButton->setVisible( enable );
}

// Show/hide restore defaults button
void ConfigurePluginPropertiesDialog::EnableDefaultsButton( bool enable )
{
    ui->defaultsButton->setVisible( enable );
}

// Help button clicked
void ConfigurePluginPropertiesDialog::on_helpButton_clicked( )
{
    emit HelpRequested( this );
}

// Defaults button clicked - restore currently visible properties to their default values
void ConfigurePluginPropertiesDialog::on_defaultsButton_clicked( )
{
    mPropertyEditor.ResetPropertiesToDefaultValues( );
}

// On "Ok" button clicked
void ConfigurePluginPropertiesDialog::on_okButton_clicked( )
{
    this->accept( );
}

// On "Cancel" button clicked
void ConfigurePluginPropertiesDialog::on_cancelButton_clicked( )
{
    this->reject( );
}

// Configuration of the configure plug-in was updated
void ConfigurePluginPropertiesDialog::on_propertyEditor_ConfigurationUpdated( )
{
    emit ConfigurationUpdated( );
}
