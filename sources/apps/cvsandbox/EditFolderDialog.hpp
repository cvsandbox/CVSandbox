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

#pragma once
#ifndef CVS_EDITFOLDERDIALOG_HPP
#define CVS_EDITFOLDERDIALOG_HPP

#include <string>
#include <XGuid.hpp>
#include <QDialog>

namespace Ui
{
    class EditFolderDialog;
}

class EditFolderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditFolderDialog( const CVSandbox::XGuid& parentId,
                               const std::string& name = std::string( ),
                               const std::string& description = std::string( ),
                               QWidget* parent = 0 );
    ~EditFolderDialog( );

    const std::string GetFolderName( ) const;
    const std::string GetFolderDescription( ) const;

protected:
    virtual void closeEvent( QCloseEvent* event );

public slots:
    virtual void accept( );
    virtual void reject( );

private:
    Ui::EditFolderDialog* ui;
};

#endif // CVS_EDITFOLDERDIALOG_HPP
