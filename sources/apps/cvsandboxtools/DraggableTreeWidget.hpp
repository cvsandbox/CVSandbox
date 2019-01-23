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
#ifndef CVS_DRAGGABLE_TREE_WIDGET_HPP
#define CVS_DRAGGABLE_TREE_WIDGET_HPP

#include <QTreeWidget>
#include <QUrl>
#include "cvsandboxtools_global.h"

// ------------------------------------------------------------------------
// ======= A class providing info of the draggingAboutToStart event =======
// ------------------------------------------------------------------------
class CVS_SHARED_EXPORT TreeDraggingStartEvent
{
public:
    TreeDraggingStartEvent( const QTreeWidgetItem* treeItem );

    const QTreeWidgetItem* TreeItem( ) const;
    const QUrl& Url( ) const;
    bool CanStartDragging( ) const;

    void SetUrl( const QUrl& url );
    void SetCanStartDragging( bool canStartDragging );

private:
    const QTreeWidgetItem*  mTreeItem;
    QUrl                    mUrl;
    bool                    mCanStartDragging;
};

// -------------------------------------------------------------------
// ======= A class providing info of the dropTargetCheck event =======
// -------------------------------------------------------------------
class CVS_SHARED_EXPORT TreeDropTargetCheckEvent
{
public:
    TreeDropTargetCheckEvent( const QTreeWidgetItem* targetTreeItem, const QUrl& url );

    const QTreeWidgetItem* TargetTreeItem( ) const;
    const QUrl& Url( ) const;
    bool CanTargetAccept( ) const;
    bool IsUrlAcceptable( ) const;

    void SetCanTargetAccept( bool canTargetAccept );
    void SetIsUrlAcceptable( bool isUrlAcceptable );

private:
    const QTreeWidgetItem*  mTargetTreeItem;
    QUrl                    mUrl;
    bool                    mCanTargetAccept;
    bool                    mIsUrlAcceptable;
};

// ---------------------------------------------------------------
// ======= A class providing info of the itemDropped event =======
// ---------------------------------------------------------------
class CVS_SHARED_EXPORT TreeItemDroppedEvent
{
public:
    TreeItemDroppedEvent( QTreeWidgetItem* targetTreeItem, const QUrl& url );

    QTreeWidgetItem* TargetTreeItem( );
    const QUrl& Url( ) const;

private:
    QTreeWidgetItem*    mTargetTreeItem;
    QUrl                mUrl;
};

// -----------------------------------------------------------
// ======= A QTreeWidget with custom drag-n-drop logic =======
// -----------------------------------------------------------
class CVS_SHARED_EXPORT DraggableTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    explicit DraggableTreeWidget( QWidget* parent = 0 );

signals:
    void draggingAboutToStart( TreeDraggingStartEvent* event );
    void dropTargetCheck( TreeDropTargetCheckEvent* event );
    void itemDropped( TreeItemDroppedEvent* event );

protected:
    virtual void mousePressEvent( QMouseEvent* event );
    virtual void mouseMoveEvent( QMouseEvent* event );
    virtual void dragEnterEvent( QDragEnterEvent* event );
    virtual void dragMoveEvent( QDragMoveEvent* event );
    virtual void dropEvent( QDropEvent* event );

private:
    void CheckDropTarget( QDragMoveEvent* event );

private:
    const QTreeWidgetItem* mLastClickedItem;
};

#endif // CVS_DRAGGABLE_TREE_WIDGET_HPP
