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
#ifndef CVS_ADD_SCRIPTING_THREAD_DIALOG_HPP
#define CVS_ADD_SCRIPTING_THREAD_DIALOG_HPP

#include <QDialog>
#include <set>
#include "ScriptingThreadDesc.hpp"

namespace Ui
{
    class EditScriptingThreadDialog;
}

namespace Private
{
    class EditScriptingThreadDialogData;
}

class EditScriptingThreadDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditScriptingThreadDialog( const ScriptingThreadDesc& threadDesc, QWidget* parent = 0 );
    ~EditScriptingThreadDialog( );

    // Get description of the thread after dialog has been accepted
    ScriptingThreadDesc ThreadDescription( ) const;

    // Set names of existing threads, which can not be used
    void SetExistingNames( const std::set<std::string>& names );

private slots:
    void on_nameEdit_textChanged( const QString &arg1 );
    void on_cancelButton_clicked( );
    void on_okButton_clicked( );

private:
    Ui::EditScriptingThreadDialog* ui;
    Private::EditScriptingThreadDialogData* mData;
};

#endif // CVS_ADD_SCRIPTING_THREAD_DIALOG_HPP
