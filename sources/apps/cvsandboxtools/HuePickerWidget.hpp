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
#ifndef CVS_HUE_PICKER_WIDGET_HPP
#define CVS_HUE_PICKER_WIDGET_HPP

#include "cvsandboxtools_global.h"
#include <QWidget>
#include <stdint.h>
#include <XRange.hpp>

namespace Private
{
    class HuePickerWidgetData;

    enum class HuePointer
    {
        None,
        MinPointer,
        MaxPointer
    };
}

class CVS_SHARED_EXPORT HuePickerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HuePickerWidget( QWidget* parent = 0 );
    ~HuePickerWidget( );

    void setHueValue( uint16_t hue );
    uint16_t hueValue( ) const;

    void setHueRange( const CVSandbox::XRange& hueRange );
    const CVSandbox::XRange hueRange( ) const;

signals:
    void valueChanged( uint16_t hue );
    void rangeChanged( const CVSandbox::XRange& hueRange );

protected:
    virtual void paintEvent( QPaintEvent* event );
    virtual void mouseMoveEvent( QMouseEvent* event );
    virtual void mousePressEvent( QMouseEvent* event );
    virtual void mouseReleaseEvent( QMouseEvent* event );

private:
    bool IsInHueCircle( int x, int y, float* pAngle = nullptr );
    bool IsInHuePointer( int x, int y, Private::HuePointer pointer );
    bool IsInHuePointer( int x, int y );

private:
    Private::HuePickerWidgetData* mData;
};

#endif // CVS_HUE_PICKER_WIDGET_HPP
