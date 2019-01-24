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

#include "MainViewFrame.hpp"
#include "ui_MainViewFrame.h"

#include "ProjectObjectViewFrame.hpp"

using namespace CVSandbox;

namespace Private
{
    class MainViewFrameData
    {
    public:
        MainViewFrameData( ) :
            EnableContentFitting( false ),
            CurrentContent( NULL )
        {

        }

    public:
        bool     EnableContentFitting;
        QWidget* CurrentContent;
    };
}

MainViewFrame::MainViewFrame( QWidget* parent ) :
    QFrame( parent ),
    ui( new Ui::MainViewFrame ),
    mData( new Private::MainViewFrameData )
{
    ui->setupUi( this );
}

MainViewFrame::~MainViewFrame( )
{
    SetContentWidget( NULL );
    delete mData;
    delete ui;
}

// Get/Set content widget of the view
QWidget* MainViewFrame::ContentWidget( ) const
{
    return mData->CurrentContent;
}
void MainViewFrame::SetContentWidget( QWidget* widget )
{
    if ( mData->CurrentContent != NULL )
    {
        ui->scrollAreaWidgetContents->layout( )->removeWidget( mData->CurrentContent );
        delete mData->CurrentContent;

        mData->CurrentContent = NULL;
    }

    if ( widget != NULL )
    {
        mData->CurrentContent = widget;
        ui->scrollAreaWidgetContents->layout( )->addWidget( widget );
    }

    UpdateContentSize( );
}

// Get ID of the project object shown in the view (if any)
const XGuid MainViewFrame::DisplayedProjectObjectId( ) const
{
    ProjectObjectViewFrame* poFrame = qobject_cast<ProjectObjectViewFrame*>( mData->CurrentContent );

    return ( poFrame != NULL ) ? poFrame->ProjectObjectId( ) : XGuid( );
}


// Enable content fitting to the widget
void MainViewFrame::EnableContentFitting( bool enable )
{
    mData->EnableContentFitting = enable;
    UpdateContentSize( );
}

// On widget size changed
void MainViewFrame::resizeEvent( QResizeEvent* )
{
    UpdateContentSize( );
}


// Update preferred size of the content (project object view)
void MainViewFrame::UpdateContentSize( )
{
    ProjectObjectViewFrame* poFrame = qobject_cast<ProjectObjectViewFrame*>( mData->CurrentContent );

    if ( poFrame != NULL )
    {
        poFrame->SetPreferredContentSize( !mData->EnableContentFitting, size( ) );
    }
}
