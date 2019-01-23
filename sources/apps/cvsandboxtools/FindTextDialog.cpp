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

#include "FindTextDialog.hpp"
#include "ui_FindTextDialog.h"
#include "GlobalServiceManager.hpp"

#include <list>

using namespace std;

namespace Private
{
    class FindTextDialogData
    {
    public:
        static list<QString> SearchHistory;

    public:
        FindTextDialogData( )
        {

        }

        void BuildSearchHistory( QComboBox* searchBox, bool setCurrentToBlank = true );
    };

    list<QString> FindTextDialogData::SearchHistory;
}

FindTextDialog::FindTextDialog( QWidget* parent ) :
    QDialog( parent ),
    ui( new Ui::FindTextDialog ),
    mData( new Private::FindTextDialogData )
{
    ui->setupUi( this );

    // disable "?" button on title bar
    setWindowFlags( windowFlags( ) & ~Qt::WindowContextHelpButtonHint );

    mData->BuildSearchHistory( ui->searchBox );

    // restore size/position
    GlobalServiceManager::Instance( ).GetUiPersistenceService( )->RestoreWidget( this );
}

FindTextDialog::~FindTextDialog( )
{
    delete mData;
    delete ui;
}

// Widget is closed
void FindTextDialog::closeEvent( QCloseEvent* /*closeEvet*/ )
{
    GlobalServiceManager::Instance( ).GetUiPersistenceService( )->SaveWidget( this );
}

// The dialog is shown
void FindTextDialog::showEvent( QShowEvent * )
{
    mData->BuildSearchHistory( ui->searchBox );
}

// Dialog is rejected
void FindTextDialog::reject( )
{
    GlobalServiceManager::Instance( ).GetUiPersistenceService( )->SaveWidget( this );
    QDialog::reject( );
}

// On "Close" button
void FindTextDialog::on_closeButton_clicked( )
{
    this->close( );
}

// Find button was clicked - tell owner to perform search
void FindTextDialog::on_findButton_clicked( )
{
    QString toFind = ui->searchBox->currentText( );

    mData->SearchHistory.remove( toFind );
    mData->SearchHistory.insert( mData->SearchHistory.begin( ), toFind );

    mData->BuildSearchHistory( ui->searchBox, false );

    findRequest( toFind, ui->matchCheck->isChecked( ), ui->wholeWordCheck->isChecked( ) );
}

namespace Private
{

void FindTextDialogData::BuildSearchHistory( QComboBox* searchBox, bool setCurrentToBlank )
{
    searchBox->clear( );

    for ( list<QString>::const_iterator it = SearchHistory.begin( ); it != SearchHistory.end( ); ++it )
    {
        searchBox->addItem( *it );
    }

    if ( setCurrentToBlank )
    {
        searchBox->setEditText( "" );
    }
}

}
