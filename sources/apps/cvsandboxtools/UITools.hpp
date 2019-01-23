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

#pragma once
#ifndef CVS_UITOOLS_HPP
#define CVS_UITOOLS_HPP

#include <QString>
#include <QColor>
#include "cvsandboxtools_global.h"

class QWidget;
class QDialog;

class CVS_SHARED_EXPORT UITools
{
private:
    UITools( );

public:
    static bool GetUserConfirmation( const QString& message, QWidget* parent = nullptr );
    static void ShowErrorMessage( const QString& message, QWidget* parent = nullptr );
    static void ShowWarningMessage( const QString& message, QWidget* parent = nullptr );
    static void ShowInfoMessage( const QString& message, QWidget* parent = nullptr );

    // Enable mouse tracking on the specified widget up the specified parent
    static void SetWidgetMouseTracking( QWidget* widget, QWidget* parent, bool mouseTracking );

    // Find the first parent dialog or return nullptr
    static QDialog* GetParentDialog( QWidget* widget );
    // Find the first parent dialog or main application's window. Return nullptr if nothing is found.
    static QWidget* GetParentDialogOrMainWindow( QWidget* widget );

    // Set widget's background color
    static void SetWidgetBackgroundColor( QWidget* widget, const QColor& color, bool autoFillBackground = true );
    // Get background color of the specified widget
    static QColor GetWidgetBackgroundColor( QWidget* widget, bool* autoFillBackground = nullptr );
};

#endif // CVS_UITOOLS_HPP
