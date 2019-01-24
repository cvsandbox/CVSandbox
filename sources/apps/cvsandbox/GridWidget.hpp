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
#ifndef CVS_GRID_WIDGET_HPP
#define CVS_GRID_WIDGET_HPP

#include <memory>
#include <QWidget>
#include <QUrl>

class QImage;

// -------------------------------------------------------------------
// ======= A class providing info of the dropTargetCheck event =======
// -------------------------------------------------------------------
class GridDropTargetCheckEvent
{
public:
    GridDropTargetCheckEvent( int row, int column, const QUrl& url );

    int Row( ) const;
    int Column( ) const;
    const QUrl& Url( ) const;
    bool CanTargetAccept( ) const;
    bool IsUrlAcceptable( ) const;

    void SetCanTargetAccept( bool canTargetAccept );
    void SetIsUrlAcceptable( bool isUrlAcceptable );

private:
    int     mRow;
    int     mColumn;
    QUrl    mUrl;
    bool    mCanTargetAccept;
    bool    mIsUrlAcceptable;
};

// ---------------------------------------------------------------
// ======= A class providing info of the itemDropped event =======
// ---------------------------------------------------------------
class GridItemDroppedEvent
{
public:
    GridItemDroppedEvent( int row, int column, const QUrl& url );

    int Row( ) const;
    int Column( ) const;
    const QUrl& Url( ) const;

private:
    int     mRow;
    int     mColumn;
    QUrl    mUrl;
};

namespace Private
{
    class GridWidgetData;
}

class GridWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GridWidget( QWidget* parent = 0 );
    ~GridWidget( );

    // Enable/disable set highlight on mouse over
    void EnableCellHighlight( bool enable );

    // Get/set rows/columns count
    int RowsCount( ) const;
    int ColumnsCount( ) const;
    void SetRowsCount( int rowsCount );
    void SetColumnsCount( int columnsCount );

    // Check if a cell can be merge or split
    bool CanMergeDown( int row, int column ) const;
    bool CanMergeRight( int row, int column ) const;
    bool CanMergeDownAndRight( int row, int column ) const;
    bool CanSplit( int row, int column ) const;

    // Perform cell merging/splitting
    void MergeDown( int row, int column );
    void MergeRight( int row, int column );
    void MergeDownAndRight( int row, int column );
    void Split( int row, int column );

    // Set cells' background color when no highlighted
    void SetCellsDefaultBackgroundColor( const QColor& color );

    // Set image to display in the grid's cell
    void SetCellImage( int row, int column, const std::shared_ptr<QImage>& image );
    // Set text to display in the grid's cell
    void SetCellText( int row, int column, const QString& text );
    // Set color of the border displayed around the cell
    void SetCellBorderColor( int row, int column, const QColor& color );
    // Set row/columns span of the grid's cell
    void SetCellSpan( int row, int column, int rowSpan, int columnSpan );

signals:
    void cellContextMenuRequested( int row, int column, const QPoint& gloablPos );
    void dropTargetCheck( GridDropTargetCheckEvent* event );
    void itemDropped( GridItemDroppedEvent* event );

protected:
    virtual void resizeEvent( QResizeEvent* event );
    virtual void paintEvent( QPaintEvent* event );
    virtual void mouseMoveEvent( QMouseEvent* event );
    virtual void leaveEvent( QEvent* event );
    virtual void contextMenuEvent ( QContextMenuEvent* event );
    virtual void dragEnterEvent( QDragEnterEvent* event );
    virtual void dragMoveEvent( QDragMoveEvent* event );
    virtual void dropEvent( QDropEvent* event );

private:
    void CheckDropTarget( QDragMoveEvent* event );

private:
    Private::GridWidgetData* mData;
};

#endif // CVS_GRID_WIDGET_HPP
