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

#include "GridButton.hpp"
#include "ui_GridButton.h"
#include <QMouseEvent>

static const QString DEFAULT_STYLE     = QString::fromAscii( "GridButton { border: 1px solid black; background-color:#B0B0B0; }" );
static const QString SELECTED_STYLE    = QString::fromAscii( "GridButton { border: 1px solid black; background-color:#84FFA3; }" );
static const QString HIGHLIGHTED_STYLE = QString::fromAscii( "GridButton { border: 1px solid black; background-color:#A6E2FF; }" );
static const QString PRESSED_STYLE     = QString::fromAscii( "GridButton { border: 1px solid black; background-color:#64CDFF; }" );

GridButton::GridButton( QWidget* parent ) :
    QFrame( parent ),
    ui( new Ui::GridButton ),
    mIsSelected( false )
{
    ui->setupUi( this );

    setStyleSheet( DEFAULT_STYLE );
}

GridButton::~GridButton( )
{
    delete ui;
}

// Set title of the button
void GridButton::SetTitle( const QString& title )
{
    ui->titleLabel->setText( title );
}

// Get/Set button title's font
const QFont& GridButton::TitleFont( ) const
{
    return ui->titleLabel->font( );
}
void GridButton::SetTitleFont( const QFont& font )
{
    ui->titleLabel->setFont( font );
}

// Get/Set selection status
bool GridButton::IsSelected( ) const
{
    return mIsSelected;
}
void GridButton::SetIsSelected( bool selected )
{
    mIsSelected = selected;
    setStyleSheet( ( mIsSelected ) ? SELECTED_STYLE : DEFAULT_STYLE );
}

// Get GridWidget control
GridWidget* GridButton::GetGridWidget( ) const
{
    return ui->gridWidget;
}

// On control resize
void GridButton::resizeEvent( QResizeEvent* )
{
    int gridWidth    = width( ) - 10;
    int gridHeight   = ( gridWidth * 3 ) / 4;
    int rowsCount    = ui->gridWidget->RowsCount( );
    int columnsCount = ui->gridWidget->ColumnsCount( );

    if ( rowsCount != columnsCount )
    {
        if ( columnsCount > rowsCount )
        {
            gridHeight = gridHeight * rowsCount / columnsCount;
        }
        else
        {
            gridWidth = gridWidth * columnsCount / rowsCount;
        }
    }

    ui->gridWidget->setMinimumSize( gridWidth, gridHeight );
    ui->gridWidget->setMaximumSize( gridWidth, gridHeight );
}

// Mouse entered the control
void GridButton::enterEvent( QEvent* )
{
    setStyleSheet( HIGHLIGHTED_STYLE );
}

// Mouse left the control
void GridButton::leaveEvent( QEvent* )
{
    setStyleSheet( ( mIsSelected ) ? SELECTED_STYLE : DEFAULT_STYLE );
}

// Mouse pressed in the control
void GridButton::mousePressEvent( QMouseEvent* )
{
    setStyleSheet( PRESSED_STYLE );
}

// Mouse released, might be outside
void GridButton::mouseReleaseEvent ( QMouseEvent* event )
{
    int x = event->x( );
    int y = event->y( );

    if ( ( x >= 0 ) && ( y >= 0 ) && ( x < width( ) ) && ( y < height( ) ) )
    {
        emit clicked( this );
    }

    setStyleSheet( HIGHLIGHTED_STYLE );
}
