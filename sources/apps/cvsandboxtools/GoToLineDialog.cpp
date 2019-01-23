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

#include "GoToLineDialog.hpp"
#include "ui_GoToLineDialog.h"

GoToLineDialog::GoToLineDialog( int linesCount, int currentLine, QWidget *parent ) :
    QDialog( parent ),
    ui( new Ui::GoToLineDialog )
{
    ui->setupUi( this);

    // disable "?" button on title bar
    setWindowFlags( windowFlags( ) & ~Qt::WindowContextHelpButtonHint );

    ui->lineNumberBox->setMaximum( linesCount );
    ui->lineNumberBox->setValue( currentLine );
    ui->lineNumberBox->selectAll( );
}

GoToLineDialog::~GoToLineDialog( )
{
    delete ui;
}

void GoToLineDialog::on_pushButton_clicked( )
{
    accept( );
}

int GoToLineDialog::LineNumber( ) const
{
    return ui->lineNumberBox->value( );
}
