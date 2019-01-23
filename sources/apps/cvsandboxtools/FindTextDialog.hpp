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
#ifndef CVS_FIND_TEXT_DIALOG_HPP
#define CVS_FIND_TEXT_DIALOG_HPP

#include <QDialog>

namespace Ui
{
    class FindTextDialog;
}

namespace Private
{
    class FindTextDialogData;
}

class FindTextDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindTextDialog( QWidget* parent = 0 );
    ~FindTextDialog( );

signals:
    void findRequest( const QString& toFind, bool matchCase, bool wholeWord );

protected:
    void closeEvent( QCloseEvent* event );
    void showEvent( QShowEvent * );

public slots:
    virtual void reject( );

private slots:
    void on_closeButton_clicked( );
    void on_findButton_clicked( );

private:
    Ui::FindTextDialog*          ui;
    Private::FindTextDialogData* mData;
};

#endif // CVS_FIND_TEXT_DIALOG_HPP
