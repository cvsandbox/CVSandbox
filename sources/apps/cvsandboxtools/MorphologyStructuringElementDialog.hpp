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
#ifndef CVS_MORPHOLOGY_STRUCTURING_ELEMENT_DIALOG_HPP
#define CVS_MORPHOLOGY_STRUCTURING_ELEMENT_DIALOG_HPP

#include <QDialog>
#include <XVariantArray2d.hpp>

namespace Ui
{
    class MorphologyStructuringElementDialog;
}

namespace Private
{
    class MorphologyStructuringElementDialogData;
}

class MorphologyStructuringElementDialog : public QDialog
{
    Q_OBJECT

public:
    enum class Mode
    {
        SimpleOperator,
        HitAndMiss
    };

public:
    explicit MorphologyStructuringElementDialog( Mode mode = Mode::SimpleOperator, QWidget* parent = 0 );
    ~MorphologyStructuringElementDialog( );

    CVSandbox::XVariantArray2d StructuringElement( ) const;
    void SetStructuringElement( const CVSandbox::XVariantArray2d& element );

protected:
    void closeEvent( QCloseEvent* event );

public slots:
    virtual void accept( );
    virtual void reject( );

private slots:
    void on_elementTable_cellChanged( int row, int column );
    void on_sizeBox_valueChanged( int arg1 );

private:
    bool eventFilter( QObject* obj, QEvent* ev );

private:
    Ui::MorphologyStructuringElementDialog* ui;
    Private::MorphologyStructuringElementDialogData* mData;
};

#endif // CVS_MORPHOLOGY_STRUCTURING_ELEMENT_DIALOG_HPP
