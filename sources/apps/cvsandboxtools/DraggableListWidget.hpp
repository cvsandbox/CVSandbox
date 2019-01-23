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
#ifndef CVS_DRAGGABLE_LIST_WIDGET_HPP
#define CVS_DRAGGABLE_LIST_WIDGET_HPP

#include <QListWidget>
#include <QUrl>
#include "cvsandboxtools_global.h"

// ------------------------------------------------------------------------
// ======= A class providing info of the draggingAboutToStart event =======
// ------------------------------------------------------------------------
class CVS_SHARED_EXPORT ListDraggingStartEvent
{
public:
    ListDraggingStartEvent( const QListWidgetItem* listItem );

    const QListWidgetItem* ListItem( ) const;
    const QUrl& Url( ) const;
    bool CanStartDragging( ) const;

    void SetUrl( const QUrl& url );
    void SetCanStartDragging( bool canStartDragging );

private:
    const QListWidgetItem*  mListItem;
    QUrl                    mUrl;
    bool                    mCanStartDragging;
};

// -------------------------------------------------------------------
// ======= A class providing info of the dropTargetCheck event =======
// -------------------------------------------------------------------
class CVS_SHARED_EXPORT ListDropTargetCheckEvent
{
public:
    ListDropTargetCheckEvent( const QListWidgetItem* targetListItem, const QUrl& url );

    const QListWidgetItem* TargetListItem( ) const;
    const QUrl& Url( ) const;
    bool CanTargetAccept( ) const;
    bool IsUrlAcceptable( ) const;

    void SetCanTargetAccept( bool canTargetAccept );
    void SetIsUrlAcceptable( bool isUrlAcceptable );

private:
    const QListWidgetItem*  mTargetListItem;
    QUrl                    mUrl;
    bool                    mCanTargetAccept;
    bool                    mIsUrlAcceptable;
};

// ---------------------------------------------------------------
// ======= A class providing info of the itemDropped event =======
// ---------------------------------------------------------------
class CVS_SHARED_EXPORT ListItemDroppedEvent
{
public:
    ListItemDroppedEvent( QListWidgetItem* targetListItem, const QUrl& url );

    QListWidgetItem* TargetListItem( );
    const QUrl& Url( ) const;

private:
    QListWidgetItem*    mTargetListItem;
    QUrl                mUrl;
};

// -----------------------------------------------------------
// ======= A QListWidget with custom drag-n-drop logic =======
// -----------------------------------------------------------
class CVS_SHARED_EXPORT DraggableListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit DraggableListWidget( QWidget* parent = 0 );

signals:
    void draggingAboutToStart( ListDraggingStartEvent* event );
    void dropTargetCheck( ListDropTargetCheckEvent* event );
    void itemDropped( ListItemDroppedEvent* event );

protected:
    virtual void mousePressEvent( QMouseEvent* event );
    virtual void mouseMoveEvent( QMouseEvent* event );
    virtual void dragEnterEvent( QDragEnterEvent* event );
    virtual void dragMoveEvent( QDragMoveEvent* event );
    virtual void dropEvent( QDropEvent* event );

private:
    void CheckDropTarget( QDragMoveEvent* event );

private:
    const QListWidgetItem* mLastClickedItem;
};

#endif // CVS_DRAGGABLE_LIST_WIDGET_HPP
