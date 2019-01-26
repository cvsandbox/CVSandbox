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

#include "AboutDialog.hpp"
#include "ui_AboutDialog.h"
#include "version.hpp"

AboutDialog::AboutDialog( QWidget* parent ) :
    QDialog( parent ),
    ui( new Ui::AboutDialog )
{
    ui->setupUi(this);

    // disable "?" button on title bar
    setWindowFlags( windowFlags( ) & ~Qt::WindowContextHelpButtonHint );
    // disable resize
    setFixedSize( size( ) );

    ui->versionLabel->setText( QString( "Version %0.%1.%2" ).arg( AppVersion.major ).arg( AppVersion.minor ).arg( AppVersion.revision ) );

    ui->websiteLabel->setText( "<a href=\"http://www.cvsandbox.com/\">www.cvsandbox.com</a>" );
    ui->websiteLabel->setOpenExternalLinks( true );

    ui->emailLabel->setText( "<a href=\"mailto:cvsandbox@gmail.com\">cvsandbox@gmail.com</a>" );
    ui->emailLabel->setOpenExternalLinks( true );
}

AboutDialog::~AboutDialog( )
{
    delete ui;
}

void AboutDialog::on_closeButton_clicked( )
{
    accept( );
}
