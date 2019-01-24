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

#include "WizardDialog.hpp"
#include "ui_WizardDialog.h"

#include "WizardPageFrame.hpp"
#include "ServiceManager.hpp"

namespace Private
{
    class WizardDialogData
    {
    public:
        WizardDialogData( ) :
            WizardPages( ),
            CurrentPageIndex( -1 )
        {
        }

    public:
        QList<WizardPageFrame*> WizardPages;
        int                     CurrentPageIndex;
    };
}

WizardDialog::WizardDialog( const QString& wizardName, QWidget* parent ) :
    QDialog( parent) ,
    ui( new Ui::WizardDialog ),
    mData( new Private::WizardDialogData )

{
    ui->setupUi( this );

    // disable "?" button on title bar
    setWindowFlags( windowFlags( ) & ~Qt::WindowContextHelpButtonHint );

    // set name of the form so position is saved/loaded correctly
    setObjectName( wizardName );

    // restore size/position
    ServiceManager::Instance( ).GetUiPersistenceService( )->RestoreWidget( this );
}

WizardDialog::~WizardDialog( )
{
    while ( mData->WizardPages.size( ) != 0 )
    {
        WizardPageFrame* page = mData->WizardPages.first( );

        RemovePage( page );
        delete page;
    }

    delete ui;
    delete mData;
}

// Add page to the wizard
void WizardDialog::AddPage( WizardPageFrame* page )
{
    if ( page != 0 )
    {
        mData->WizardPages.append( page );
        connect( page, SIGNAL( CompleteStatusChanged( bool ) ),
                 this, SLOT( on_wizardPage_CompleteStatusChanged( bool ) ) );

        if ( mData->CurrentPageIndex == -1 )
        {
            ShowNewPage( 0 );
        }
        else
        {
            page->setVisible( false );
        }
    }
}

// Remove page from the wizard
void WizardDialog::RemovePage( WizardPageFrame* page )
{
    if ( page != 0 )
    {
        int pageIndex = mData->WizardPages.indexOf( page );

        if ( pageIndex != -1 )
        {
            if ( pageIndex == mData->CurrentPageIndex )
            {
                int newPageIndex = ( mData->WizardPages.size( ) == 1 ) ? -1 :
                                 ( ( pageIndex == 0 ) ? 1 : pageIndex - 1 );
                ShowNewPage( newPageIndex );
            }

            // no longer interested in notifications from that page
            disconnect( page, SIGNAL( CompleteStatusChanged( bool ) ),
                        this, SLOT( on_wizardPage_CompleteStatusChanged( bool ) ) );
            // remove from collection
            mData->WizardPages.removeAt( pageIndex );
            // update current selection index in the case deleted page was preceding it
            if ( mData->CurrentPageIndex > pageIndex )
            {
                mData->CurrentPageIndex--;
            }
        }
    }
}

// Widget is closed
void WizardDialog::closeEvent( QCloseEvent* )
{
    ServiceManager::Instance( ).GetUiPersistenceService( )->SaveWidget( this );
}

// Dialog is accepted
void WizardDialog::accept( )
{
    ServiceManager::Instance( ).GetUiPersistenceService( )->SaveWidget( this );
    QDialog::accept( );
}

// Dialog is rejected
void WizardDialog::reject( )
{
    ServiceManager::Instance( ).GetUiPersistenceService( )->SaveWidget( this );
    QDialog::reject( );
}

// On "Cancel" button clicked
void WizardDialog::on_cancelButton_clicked( )
{
    this->reject( );
}

// On "Finish" button clicked
void WizardDialog::on_finishButton_clicked( )
{
    this->accept( );
}

// On "Next" button clicked
void WizardDialog::on_nextButton_clicked( )
{
    if ( mData->CurrentPageIndex + 1 < mData->WizardPages.count( ) )
    {
        ShowNewPage( mData->CurrentPageIndex + 1 );
    }
}

// On "Previous" button clicked
void WizardDialog::on_previousButton_clicked( )
{
    if ( mData->CurrentPageIndex > 0 )
    {
        ShowNewPage( mData->CurrentPageIndex - 1 );
    }
}

// On "Help" button clicked
void WizardDialog::on_helpButton_clicked()
{
    if ( ( mData->CurrentPageIndex >= 0 ) && ( mData->CurrentPageIndex < mData->WizardPages.count( ) ) )
    {
        mData->WizardPages[mData->CurrentPageIndex]->ShowHelp( );
    }
}

// Show page with the given index
void WizardDialog::ShowNewPage( int newPageIndex )
{
    if ( mData->CurrentPageIndex != -1 )
    {
        WizardPageFrame* oldPage = mData->WizardPages[mData->CurrentPageIndex];

        ui->mainFrame->layout( )->removeWidget( oldPage );
        oldPage->setVisible( false );

        mData->CurrentPageIndex = -1;
    }

    if ( ( newPageIndex >= 0 ) && ( newPageIndex < mData->WizardPages.count( ) ) )
    {
        OnChangingPage( newPageIndex );

        WizardPageFrame* newPage = mData->WizardPages[newPageIndex];

        ui->mainFrame->layout( )->addWidget( newPage );
        newPage->setFocus( );
        newPage->setVisible( true );

        mData->CurrentPageIndex = newPageIndex;
    }

    UpdateDialogButtonsStatus( );
}

// Update status of Previous/Next/Finish buttons
void WizardDialog::UpdateDialogButtonsStatus( )
{
    ui->previousButton->setEnabled( mData->CurrentPageIndex > 0 );
    ui->nextButton->setEnabled(
                ( mData->CurrentPageIndex >= 0 ) &&
                ( mData->CurrentPageIndex < mData->WizardPages.count( ) - 1 ) &&
                ( mData->WizardPages[mData->CurrentPageIndex]->CanGoNext( ) ) );
    ui->helpButton->setVisible(
                ( mData->CurrentPageIndex >= 0 ) &&
                ( mData->CurrentPageIndex < mData->WizardPages.count( ) ) &&
                ( mData->WizardPages[mData->CurrentPageIndex]->ProvidesHelp( ) ) );

    bool canFinishWizard = true;

    for ( QList<WizardPageFrame*>::const_iterator it = mData->WizardPages.begin( );
          it != mData->WizardPages.end( ); it++ )
    {
        canFinishWizard &= (*it)->CanGoNext( );
    }

    ui->finishButton->setEnabled( canFinishWizard );
}

// On page "completeness" notification from one of the pages
void WizardDialog::on_wizardPage_CompleteStatusChanged( bool )
{
    UpdateDialogButtonsStatus( );
}

// Virtual method to allow child classes to do something when page is changing
void WizardDialog::OnChangingPage( int )
{

}
