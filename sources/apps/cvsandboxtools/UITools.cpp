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

#include <QMessageBox>
#include <QMainWindow>
#include "UITools.hpp"

bool UITools::GetUserConfirmation( const QString& message, QWidget* parent )
{
    return ( QMessageBox::question( parent, "Confirmation", message,
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) == QMessageBox::Yes );

    /*
    QMessageBox msgBox;

    msgBox.setText( message );
    msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
    msgBox.setDefaultButton( QMessageBox::No );
    msgBox.setIcon( QMessageBox::Question );
    msgBox.setWindowTitle( "Question" );

    return ( msgBox.exec( ) == QMessageBox::Yes );*/
}

void UITools::ShowErrorMessage( const QString& message, QWidget* parent )
{
    QMessageBox::critical( parent, "Error", message );
}

void UITools::ShowWarningMessage( const QString& message, QWidget* parent )
{
    QMessageBox::warning( parent, "Warning", message );
}

void UITools::ShowInfoMessage( const QString& message, QWidget* parent )
{
    QMessageBox::information( parent, "Information", message );
}

// Enable mouse tracking on the specified widget up the specified parent
void UITools::SetWidgetMouseTracking( QWidget* widget, QWidget* parent, bool mouseTracking )
{
    QWidget* w = widget;

    while ( w != 0 )
    {
        w->setMouseTracking( mouseTracking );

        if ( w == parent )
        {
            break;
        }

        w = w->parentWidget( );
    }
}

// Find the first parent dialog or return nullptr
QDialog* UITools::GetParentDialog( QWidget* widget )
{
    QDialog* parentDialog = nullptr;
    QWidget* parentWidget = widget->parentWidget( );

    while ( ( parentWidget != nullptr ) && ( parentDialog == nullptr ) )
    {
        parentDialog = qobject_cast<QDialog*>( parentWidget );
        parentWidget = parentWidget->parentWidget( );
    }

    return parentDialog;
}

// Find the first parent dialog or main application's window. Return nullptr if nothing is found.
QWidget* UITools::GetParentDialogOrMainWindow( QWidget* widget )
{
    QWidget* parent       = nullptr;
    QWidget* parentWidget = widget->parentWidget( );

    while ( ( parentWidget != nullptr ) && ( parent == nullptr ) )
    {
        parent = qobject_cast<QDialog*>( parentWidget );
        if ( parent == nullptr )
        {
            parent = qobject_cast<QMainWindow*>( parentWidget );
        }
        parentWidget = parentWidget->parentWidget( );
    }

    return parent;
}


// Set widget's background color
void UITools::SetWidgetBackgroundColor( QWidget* widget, const QColor& color, bool autoFillBackground )
{
    if ( widget != 0 )
    {
        QPalette palette = widget->palette( );
        palette.setColor( widget->backgroundRole( ), color );
        widget->setPalette( palette );
        widget->setAutoFillBackground( autoFillBackground );
    }
}

// Get background color of the specified widget
QColor UITools::GetWidgetBackgroundColor( QWidget* widget, bool* autoFillBackground )
{
    QColor ret;

    if ( widget != nullptr )
    {
        QPalette palette = widget->palette( );
        ret = palette.color( widget->backgroundRole( ) );

        if ( autoFillBackground != nullptr )
        {
            *autoFillBackground = widget->autoFillBackground( );
        }
    }

    return ret;
}

