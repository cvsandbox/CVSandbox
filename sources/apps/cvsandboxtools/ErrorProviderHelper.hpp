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
#ifndef CVS_ERRORPROVIDERHELPER_HPP
#define CVS_ERRORPROVIDERHELPER_HPP

#include "cvsandboxtools_global.h"
#include <QLabel>
#include <QWidget>
#include <QDialogButtonBox>

// Helper class to show user input errors
class CVS_SHARED_EXPORT ErrorProviderHelper
{
public:
    ErrorProviderHelper( QLabel* errorLabel = 0, QDialogButtonBox* dialogButtons = 0 );

    // Set label to show error message in
    void SetLabel( QLabel* errorLabel );
    // Set dialog buttons to disable/enable
    void SetDialogButtons( QDialogButtonBox* dialogButtons );

    // Set error message
    void SetError( const QString& message, QWidget* invalidContentWidget = 0 );
    // Clear error message
    void ClearError( );

private:
    void RestoreWidget( );

private:
    QLabel* mErrorLabel;
    QDialogButtonBox* mDialogButtons;
    QWidget* mLastInvalidWidget;

    QColor mLabelColor;
    bool mLabelAutoFill;

    QColor mWidgetColor;
    bool mWidgetAutoFill;
};

#endif // CVS_ERRORPROVIDERHELPER_HPP
