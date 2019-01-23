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
#ifndef CVS_GOTOLINEDIALOG_HPP
#define CVS_GOTOLINEDIALOG_HPP

#include <QDialog>

namespace Ui
{
    class GoToLineDialog;
}

class GoToLineDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GoToLineDialog( int linesCount, int currentLine, QWidget *parent = 0 );
    ~GoToLineDialog( );

    int LineNumber( ) const;

private slots:
    void on_pushButton_clicked( );

private:
    Ui::GoToLineDialog* ui;
};

#endif // CVS_GOTOLINEDIALOG_HPP
