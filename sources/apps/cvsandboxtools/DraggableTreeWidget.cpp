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

#include "DraggableTreeWidget.hpp"
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>

// -----------------------------------------------------
// ======= TreeDraggingStartEvent implementation =======
// -----------------------------------------------------

TreeDraggingStartEvent::TreeDraggingStartEvent( const QTreeWidgetItem* treeItem ) :
    mTreeItem( treeItem ), mUrl( ), mCanStartDragging( false )
{
}

const QTreeWidgetItem* TreeDraggingStartEvent::TreeItem( ) const
{
    return mTreeItem;
}

const QUrl& TreeDraggingStartEvent::Url( ) const
{
    return mUrl;
}

bool TreeDraggingStartEvent::CanStartDragging( ) const
{
    return mCanStartDragging;
}

void TreeDraggingStartEvent::SetUrl( const QUrl& url )
{
    mUrl = url;
}

void TreeDraggingStartEvent::SetCanStartDragging( bool canStartDragging )
{
    mCanStartDragging = canStartDragging;
}

// -------------------------------------------------------
// ======= TreeDropTargetCheckEvent implementation =======
// -------------------------------------------------------

TreeDropTargetCheckEvent::TreeDropTargetCheckEvent( const QTreeWidgetItem* targetTreeItem, const QUrl& url ) :
    mTargetTreeItem( targetTreeItem ), mUrl( url ), mCanTargetAccept( false ), mIsUrlAcceptable( true )
{
}

const QTreeWidgetItem* TreeDropTargetCheckEvent::TargetTreeItem( ) const
{
    return mTargetTreeItem;
}

const QUrl& TreeDropTargetCheckEvent::Url( ) const
{
    return mUrl;
}

bool TreeDropTargetCheckEvent::CanTargetAccept( ) const
{
    return mCanTargetAccept;
}

bool TreeDropTargetCheckEvent::IsUrlAcceptable( ) const
{
    return mIsUrlAcceptable;
}

void TreeDropTargetCheckEvent::SetCanTargetAccept( bool canTargetAccept )
{
    mCanTargetAccept = canTargetAccept;
}

void TreeDropTargetCheckEvent::SetIsUrlAcceptable( bool isUrlAcceptable )
{
    mIsUrlAcceptable = isUrlAcceptable;
}

// ---------------------------------------------------
// ======= TreeItemDroppedEvent implementation =======
// ---------------------------------------------------

TreeItemDroppedEvent::TreeItemDroppedEvent( QTreeWidgetItem* targetTreeItem, const QUrl& url ) :
    mTargetTreeItem( targetTreeItem ), mUrl( url )
{
}

QTreeWidgetItem* TreeItemDroppedEvent::TargetTreeItem( )
{
    return mTargetTreeItem;
}

const QUrl& TreeItemDroppedEvent::Url( ) const
{
    return mUrl;
}

// --------------------------------------------------
// ======= DraggableTreeWidget implementation =======
// --------------------------------------------------

DraggableTreeWidget::DraggableTreeWidget( QWidget* parent ) :
    QTreeWidget( parent ), mLastClickedItem( 0 )
{
}

// Mouse button was clicked - remember tree item uder mouse cursor
void DraggableTreeWidget::mousePressEvent( QMouseEvent* event )
{
    mLastClickedItem = itemAt( event->pos( ) );
    QTreeWidget::mousePressEvent( event );
}

// Check if drag-n-drop needs to start while mouse is moved in the control
void DraggableTreeWidget::mouseMoveEvent( QMouseEvent* event )
{
    bool handled = false;

    // if not left button - return
    if ( event->buttons( ) & Qt::LeftButton )
    {
        QList<QTreeWidgetItem*> items = selectedItems( );

        // do nothing, if no item selected
        if ( items.count( ) == 1 )
        {
            QTreeWidgetItem* item = items.at( 0 );

            if ( item == mLastClickedItem )
            {
                TreeDraggingStartEvent draggingEvent( item );

                emit draggingAboutToStart( &draggingEvent );

                if ( draggingEvent.CanStartDragging( ) )
                {
                    QDrag*      drag = new QDrag( this );
                    QMimeData*  mimeData = new QMimeData;
                    QList<QUrl> list;
                    QIcon       icon = item->icon( 0 );

                    list.append( draggingEvent.Url( ) );

                    mimeData->setUrls( list );
                    drag->setMimeData( mimeData );

                    if ( !icon.isNull( ) )
                    {
                        // set icon if it is available
                        drag->setPixmap( icon.pixmap( 16, 16 ) );
                    }

                    drag->start( Qt::MoveAction );
                }

                handled = true;
            }
        }
    }

    if ( !handled )
    {
        QTreeWidget::mouseMoveEvent( event );
    }
}

// Drag-n-drop entered the control
void DraggableTreeWidget::dragEnterEvent( QDragEnterEvent* event )
{
    CheckDropTarget( event );
}

// Drag-n-drop is moving over the control
void DraggableTreeWidget::dragMoveEvent( QDragMoveEvent* event )
{
    CheckDropTarget( event );
}

// An item was dropped in the control
void DraggableTreeWidget::dropEvent( QDropEvent* event )
{
    QList<QUrl> urlList  = event->mimeData( )->urls( );

    if ( urlList.count( ) == 1 )
    {
        QTreeWidgetItem* targetItem = itemAt( event->pos( ) );

        if ( targetItem != 0 )
        {
            TreeItemDroppedEvent droppedEvent( targetItem, urlList[0] );

            emit itemDropped( &droppedEvent );
        }
    }
}

// Internal helper for checking if list item under mouse cursor can accept the draggable item
void DraggableTreeWidget::CheckDropTarget( QDragMoveEvent* event )
{
    QList<QUrl> urlList  = event->mimeData( )->urls( );
    bool        accepted = false;

    if ( urlList.count( ) == 1 )
    {
        QTreeWidgetItem* targetItem = itemAt( event->pos( ) );

        if ( targetItem != 0 )
        {
            TreeDropTargetCheckEvent dropCheckEvent( targetItem, urlList[0] );

            emit dropTargetCheck( &dropCheckEvent );

            accepted = dropCheckEvent.IsUrlAcceptable( );
            event->setDropAction( ( dropCheckEvent.CanTargetAccept( ) ) ? Qt::MoveAction : Qt::IgnoreAction );
        }
    }

    event->setAccepted( accepted );
}
