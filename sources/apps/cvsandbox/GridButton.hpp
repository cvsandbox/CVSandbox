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
#ifndef CVS_GRID_BUTTON_HPP
#define CVS_GRID_BUTTON_HPP

#include <QFrame>
class GridWidget;

namespace Ui
{
    class GridButton;
}

class GridButton : public QFrame
{
    Q_OBJECT

public:
    explicit GridButton( QWidget* parent = 0 );
    ~GridButton( );

    // Set title of the button
    void SetTitle( const QString& title );

    // Get/Set button title's font
    const QFont& TitleFont( ) const;
    void SetTitleFont( const QFont& font );

    // Get/Set selection status
    bool IsSelected( ) const;
    void SetIsSelected( bool selected );

    // Get GridWidget control
    GridWidget* GetGridWidget( ) const;

signals:
    void clicked( QObject* sender );

protected:
    virtual void resizeEvent( QResizeEvent* event );
    virtual void enterEvent( QEvent* event );
    virtual void leaveEvent( QEvent* event );
    virtual void mousePressEvent( QMouseEvent* event );
    virtual void mouseReleaseEvent ( QMouseEvent* event );

private:
    Ui::GridButton* ui;
    bool            mIsSelected;
};

#endif // CVS_GRID_BUTTON_HPP
