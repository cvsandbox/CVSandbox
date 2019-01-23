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

#include "HuePickerDialog.hpp"
#include "ui_HuePickerDialog.h"

using namespace CVSandbox;

HuePickerDialog::HuePickerDialog( QWidget* parent ) :
    QDialog( parent ),
    ui( new Ui::HuePickerDialog ), rangeMode( false )
{
    ui->setupUi( this );

    // disable "?" button on title bar
    setWindowFlags( windowFlags( ) & ~Qt::WindowContextHelpButtonHint );

    connect( ui->huePicker, SIGNAL( valueChanged(uint16_t) ),
             this, SLOT( on_huePicker_valueChanged(uint16_t) ) );
    connect( ui->huePicker, SIGNAL( rangeChanged(const CVSandbox::XRange&) ),
             this, SLOT( on_huePicker_rangeChanged(const CVSandbox::XRange&) ) );
}

HuePickerDialog::~HuePickerDialog( )
{
    delete ui;
}

void HuePickerDialog::setHueValue( uint16_t hue )
{
    rangeMode = false;

    ui->hueValueSpin2->setVisible( false );
    ui->groupBox->setTitle( "Hue Value" );

    ui->hueValueSpin1->setValue( hue );
    ui->hueValueSpin1->selectAll( );

    ui->huePicker->setHueValue( hue );
}

uint16_t HuePickerDialog::hueValue( ) const
{
    return ui->huePicker->hueValue( );
}

void HuePickerDialog::setHueRange( const XRange& hueRange )
{
    rangeMode = true;

    ui->hueValueSpin2->setVisible( true );
    ui->groupBox->setTitle( "Hue Range" );

    ui->hueValueSpin1->setValue( hueRange.Min( ) );
    ui->hueValueSpin1->selectAll( );

    ui->hueValueSpin2->setValue( hueRange.Max( ) );
    ui->hueValueSpin2->selectAll( );

    ui->huePicker->setHueRange( hueRange );
}

const XRange HuePickerDialog::hueRange( ) const
{
    return ui->huePicker->hueRange( );
}

void HuePickerDialog::on_hueValueSpin1_valueChanged( int value )
{
    if ( !rangeMode )
    {
        ui->huePicker->setHueValue( static_cast<uint16_t>( value ) );
    }
    else
    {
        ui->huePicker->setHueRange( XRange( value, ui->hueValueSpin2->value( ) ) );
    }
}

void HuePickerDialog::on_hueValueSpin2_valueChanged( int value )
{
    ui->huePicker->setHueRange( XRange( ui->hueValueSpin1->value( ), value ) );
}

void HuePickerDialog::on_huePicker_valueChanged( uint16_t hue )
{
    ui->hueValueSpin1->setValue( hue );
    ui->hueValueSpin1->selectAll( );
}

void HuePickerDialog::on_huePicker_rangeChanged( const XRange& hueRange )
{
    ui->hueValueSpin1->setValue( hueRange.Min( ) );
    ui->hueValueSpin2->setValue( hueRange.Max( ) );
    ui->hueValueSpin1->selectAll( );
    ui->hueValueSpin2->selectAll( );
}
