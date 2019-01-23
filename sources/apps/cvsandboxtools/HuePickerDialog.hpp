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
#ifndef CVS_HUEPICKERDIALOG_HPP
#define CVS_HUEPICKERDIALOG_HPP

#include <QDialog>
#include <stdint.h>
#include <XRange.hpp>

namespace Ui
{
    class HuePickerDialog;
}

class HuePickerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HuePickerDialog( QWidget* parent = 0 );
    ~HuePickerDialog( );

    void setHueValue( uint16_t hue );
    uint16_t hueValue( ) const;

    void setHueRange( const CVSandbox::XRange& hueRange );
    const CVSandbox::XRange hueRange( ) const;

private slots:
    void on_hueValueSpin1_valueChanged( int value );
    void on_hueValueSpin2_valueChanged( int value );
    void on_huePicker_valueChanged( uint16_t hue );
    void on_huePicker_rangeChanged( const CVSandbox::XRange& hueRange );

private:
    Ui::HuePickerDialog* ui;
    bool rangeMode;
};

#endif // CVS_HUEPICKERDIALOG_HPP
