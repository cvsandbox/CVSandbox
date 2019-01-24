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

#include <QList>

#include "PropertyPagesDialog.hpp"
#include "ui_PropertyPagesDialog.h"

#include "WizardPageFrame.hpp"
#include "ServiceManager.hpp"

namespace Private
{
    class PropertyPagesDialogData
    {
    public:
        QList<WizardPageFrame*> WizardPages;
    };
}


PropertyPagesDialog::PropertyPagesDialog( const QString& propertiesCollectionName, QWidget* parent ) :
    QDialog( parent ),
    ui( new Ui::PropertyPagesDialog ),
    mData( new Private::PropertyPagesDialogData( ) )
{
    ui->setupUi( this );

    // disable "?" button on title bar
    setWindowFlags( windowFlags( ) & ~Qt::WindowContextHelpButtonHint );

    // set name of the form so position is saved/loaded correctly
    setObjectName( propertiesCollectionName );

    // restore size/position
    ServiceManager::Instance( ).GetUiPersistenceService( )->RestoreWidget( this );
}

PropertyPagesDialog::~PropertyPagesDialog( )
{
    delete ui;
    delete mData;
}

// Add page to the properties dialog
void PropertyPagesDialog::AddPage( WizardPageFrame* page )
{
    if ( page != 0 )
    {
        mData->WizardPages.append( page );
        connect( page, SIGNAL( CompleteStatusChanged( bool ) ),
                 this, SLOT( on_wizardPage_CompleteStatusChanged( bool ) ) );

        // create helper frame to get some padding around the page
        QFrame*      frame       = new QFrame( );
        QVBoxLayout* frameLayout = new QVBoxLayout( );
        frame->setLayout( frameLayout );
        frame->layout( )->addWidget( page );

        // add the new page to tab widget
        ui->tabWidget->addTab( frame, page->windowTitle( ) );

        // check the new page to see if affect "OK" button
        UpdateDialogButtonsStatus( );
    }
}

// Widget is closed
void PropertyPagesDialog::closeEvent( QCloseEvent* )
{
    ServiceManager::Instance( ).GetUiPersistenceService( )->SaveWidget( this );
}

// Dialog is accepted
void PropertyPagesDialog::accept( )
{
    ServiceManager::Instance( ).GetUiPersistenceService( )->SaveWidget( this );
    QDialog::accept( );
}

// Dialog is rejected
void PropertyPagesDialog::reject( )
{
    ServiceManager::Instance( ).GetUiPersistenceService( )->SaveWidget( this );
    QDialog::reject( );
}

// On "Help" button clicked
void PropertyPagesDialog::on_helpButton_clicked()
{
    int index = ui->tabWidget->currentIndex( );

    if ( ( index >= 0 ) && ( index < mData->WizardPages.count( ) ) )
    {
        mData->WizardPages[index]->ShowHelp( );
    }
}

// On "Apply" button clicked
void PropertyPagesDialog::on_applyButton_clicked( )
{
    ApplyCurrentPage( ui->tabWidget->currentIndex( ) );
}

// On "Ok" button clicked
void PropertyPagesDialog::on_okButton_clicked( )
{
    ApplyAllPages( );
    this->accept( );
}

// On "Cancel" button clicked
void PropertyPagesDialog::on_cancelButton_clicked( )
{
    this->reject( );
}

// Current tab page has changed
void PropertyPagesDialog::on_tabWidget_currentChanged( int index )
{
    if ( ( index >= 0 ) && ( index < mData->WizardPages.count( ) ) )
    {
        UpdateDialogButtonsStatus( );
        mData->WizardPages[ui->tabWidget->currentIndex( )]->setFocus( );
    }
}

// Update status of Apply and Ok buttons
void PropertyPagesDialog::UpdateDialogButtonsStatus( )
{
    bool canApplyDialog  = mData->WizardPages[ui->tabWidget->currentIndex( )]->CanGoNext( );
    bool canFinishDialog = true;

    for ( QList<WizardPageFrame*>::const_iterator it = mData->WizardPages.begin( );
          it != mData->WizardPages.end( ); it++ )
    {
        canFinishDialog &= (*it)->CanGoNext( );
    }

    ui->applyButton->setEnabled( canApplyDialog );
    ui->okButton->setEnabled( canFinishDialog );
    ui->helpButton->setVisible( mData->WizardPages[ui->tabWidget->currentIndex( )]->ProvidesHelp( ) );
}

// On page "completeness" notification from one of the pages
void PropertyPagesDialog::on_wizardPage_CompleteStatusChanged( bool )
{
    UpdateDialogButtonsStatus( );
}

// Virtual method to allow child classes to do something to apply page changes
void PropertyPagesDialog::ApplyCurrentPage( int )
{
}

// Virtual method to allow child classes to do something to apply all pages
void PropertyPagesDialog::ApplyAllPages( )
{
}
