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

#include "ErrorProviderHelper.hpp"
#include <assert.h>
#include <QPushButton>

#include "UITools.hpp"

static const QColor ERROR_COLOR( 255, 200, 200 );

ErrorProviderHelper::ErrorProviderHelper( QLabel* errorLabel, QDialogButtonBox* dialogButtons ) :
    mErrorLabel( 0 ),
    mDialogButtons( dialogButtons ),
    mLastInvalidWidget( 0 )
{
    SetLabel( errorLabel );
}

// Set label to show error message in
void ErrorProviderHelper::SetLabel( QLabel* errorLabel )
{
    mErrorLabel = errorLabel;

    if ( mErrorLabel )
    {
        mLabelColor = UITools::GetWidgetBackgroundColor( mErrorLabel, &mLabelAutoFill );
    }
}

// Set dialog buttons to disable/enable
void ErrorProviderHelper::SetDialogButtons( QDialogButtonBox* dialogButtons )
{
    mDialogButtons = dialogButtons;
}

// Set error message
void ErrorProviderHelper::SetError( const QString& message, QWidget* invalidContentWidget )
{
    if ( mErrorLabel != 0 )
    {
        UITools::SetWidgetBackgroundColor( mErrorLabel, ERROR_COLOR );
        mErrorLabel->setText( message );
    }

    if ( mDialogButtons != 0 )
    {
        mDialogButtons->button( QDialogButtonBox::Ok )->setEnabled( false );
    }

    RestoreWidget( );

    if ( invalidContentWidget != 0 )
    {
        mLastInvalidWidget = invalidContentWidget;
        mWidgetColor = UITools::GetWidgetBackgroundColor( mLastInvalidWidget, &mWidgetAutoFill );
        UITools::SetWidgetBackgroundColor( mLastInvalidWidget, ERROR_COLOR );
    }
}

// Clear error message
void ErrorProviderHelper::ClearError( )
{
    if ( mErrorLabel != 0 )
    {
        UITools::SetWidgetBackgroundColor( mErrorLabel, mLabelColor, mLabelAutoFill );
        mErrorLabel->setText( QString::null );
    }

    if ( mDialogButtons != 0 )
    {
        mDialogButtons->button( QDialogButtonBox::Ok )->setEnabled( true );
    }

    RestoreWidget( );
}

// Restore widgets background color
void ErrorProviderHelper::RestoreWidget( )
{
    if ( mLastInvalidWidget != 0 )
    {
        UITools::SetWidgetBackgroundColor( mLastInvalidWidget, mWidgetColor, mWidgetAutoFill );
        mLastInvalidWidget = 0;
    }
}
