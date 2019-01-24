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

#include <ErrorProviderHelper.hpp>

#include "EditFolderDialog.hpp"
#include "ui_EditFolderDialog.h"
#include "ServiceManager.hpp"
#include "ProjectObjectIcon.hpp"

using namespace std;
using namespace CVSandbox;

EditFolderDialog::EditFolderDialog( const XGuid& parentId, const string& name, const string& description, QWidget* parent ) :
    QDialog( parent ),
    ui( new Ui::EditFolderDialog )
{
    ui->setupUi( this );

    ui->projectObjectNamePage->SetObjectProperties( parentId, name, description );
    ui->projectObjectNamePage->SetObjectIcon( ProjectObjectIcon::GetResourceName( ProjectObjectType::Folder, true ) );
    ui->projectObjectNamePage->SetObjectTypeName( QString::fromUtf8( "Folder" ) );

    // disable "?" button on title bar
    setWindowFlags( windowFlags( ) & ~Qt::WindowContextHelpButtonHint );

    // check if we are adding or editing the folder
    if ( name.empty( ) )
    {
        setWindowTitle( QString( "Add new folder" ) );
    }
    else
    {
        setWindowTitle( QString( "Edit folder" ) );
    }

    // restore size/position
    ServiceManager::Instance( ).GetUiPersistenceService( )->RestoreWidget( this );
}

EditFolderDialog::~EditFolderDialog( )
{
    delete ui;
}

// Get name of the group entered by user
const string EditFolderDialog::GetFolderName( ) const
{
    return string( ui->projectObjectNamePage->GetObjectName( ) );
}

// Get description of the group entered by user
const string EditFolderDialog::GetFolderDescription( ) const
{
    return string( ui->projectObjectNamePage->GetObjectDescription( ) );
}

// Widget is closed
void EditFolderDialog::closeEvent( QCloseEvent* )
{
    ServiceManager::Instance( ).GetUiPersistenceService( )->SaveWidget( this );
}

// Dialog is accepted
void EditFolderDialog::accept( )
{
    ServiceManager::Instance( ).GetUiPersistenceService( )->SaveWidget( this );
    QDialog::accept( );
}

// Dialog is rejected
void EditFolderDialog::reject( )
{
    ServiceManager::Instance( ).GetUiPersistenceService( )->SaveWidget( this );
    QDialog::reject( );
}
