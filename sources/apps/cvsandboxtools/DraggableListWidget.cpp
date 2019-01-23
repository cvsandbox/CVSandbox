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

#include "DraggableListWidget.hpp"
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>

// -----------------------------------------------------
// ======= ListDraggingStartEvent implementation =======
// -----------------------------------------------------

ListDraggingStartEvent::ListDraggingStartEvent( const QListWidgetItem* listItem ) :
    mListItem( listItem ), mUrl( ), mCanStartDragging( false )
{
}

const QListWidgetItem* ListDraggingStartEvent::ListItem( ) const
{
    return mListItem;
}

const QUrl& ListDraggingStartEvent::Url( ) const
{
    return mUrl;
}

bool ListDraggingStartEvent::CanStartDragging( ) const
{
    return mCanStartDragging;
}

void ListDraggingStartEvent::SetUrl( const QUrl& url )
{
    mUrl = url;
}

void ListDraggingStartEvent::SetCanStartDragging( bool canStartDragging )
{
    mCanStartDragging = canStartDragging;
}

// -------------------------------------------------------
// ======= ListDropTargetCheckEvent implementation =======
// -------------------------------------------------------

ListDropTargetCheckEvent::ListDropTargetCheckEvent( const QListWidgetItem* targetListItem, const QUrl& url ) :
    mTargetListItem( targetListItem ), mUrl( url ), mCanTargetAccept( false ), mIsUrlAcceptable( true )
{
}

const QListWidgetItem* ListDropTargetCheckEvent::TargetListItem( ) const
{
    return mTargetListItem;
}

const QUrl& ListDropTargetCheckEvent::Url( ) const
{
    return mUrl;
}

bool ListDropTargetCheckEvent::CanTargetAccept( ) const
{
    return mCanTargetAccept;
}

bool ListDropTargetCheckEvent::IsUrlAcceptable( ) const
{
    return mIsUrlAcceptable;
}

void ListDropTargetCheckEvent::SetCanTargetAccept( bool canTargetAccept )
{
    mCanTargetAccept = canTargetAccept;
}

void ListDropTargetCheckEvent::SetIsUrlAcceptable( bool isUrlAcceptable )
{
    mIsUrlAcceptable = isUrlAcceptable;
}

// ---------------------------------------------------
// ======= ListItemDroppedEvent implementation =======
// ---------------------------------------------------

ListItemDroppedEvent::ListItemDroppedEvent( QListWidgetItem* targetListItem, const QUrl& url ) :
    mTargetListItem( targetListItem ), mUrl( url )
{
}

QListWidgetItem* ListItemDroppedEvent::TargetListItem( )
{
    return mTargetListItem;
}

const QUrl& ListItemDroppedEvent::Url( ) const
{
    return mUrl;
}

// --------------------------------------------------
// ======= DraggableListWidget implementation =======
// --------------------------------------------------

DraggableListWidget::DraggableListWidget( QWidget* parent ) :
    QListWidget( parent )
{
}

// Mouse button was clicked - remember list item uder mouse cursor
void DraggableListWidget::mousePressEvent( QMouseEvent* event )
{
    mLastClickedItem = itemAt( event->pos( ) );
    QListWidget::mousePressEvent( event );
}

// Check if drag-n-drop needs to start while mouse is moved in the control
void DraggableListWidget::mouseMoveEvent( QMouseEvent* event )
{
    bool handled = false;

    // if not left button - return
    if ( event->buttons( ) & Qt::LeftButton )
    {
        QList<QListWidgetItem*> items = selectedItems( );

        // do nothing, if no item selected
        if ( items.count( ) == 1 )
        {
            QListWidgetItem* item = items.at( 0 );

            if ( item == mLastClickedItem )
            {
                ListDraggingStartEvent draggingEvent( item );

                emit draggingAboutToStart( &draggingEvent );

                if ( draggingEvent.CanStartDragging( ) )
                {
                    QDrag*      drag = new QDrag( this );
                    QMimeData*  mimeData = new QMimeData;
                    QList<QUrl> list;
                    QIcon       icon = item->icon( );

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
        QListWidget::mouseMoveEvent( event );
    }
}

// Drag-n-drop entered the control
void DraggableListWidget::dragEnterEvent( QDragEnterEvent* event )
{
    CheckDropTarget( event );
}

// Drag-n-drop is moving over the control
void DraggableListWidget::dragMoveEvent( QDragMoveEvent* event )
{
    CheckDropTarget( event );
}

// An item was dropped in the control
void DraggableListWidget::dropEvent( QDropEvent* event )
{
    QList<QUrl> urlList  = event->mimeData( )->urls( );

    if ( urlList.count( ) == 1 )
    {
        QListWidgetItem* targetItem = itemAt( event->pos( ) );

        if ( targetItem != 0 )
        {
            ListItemDroppedEvent droppedEvent( targetItem, urlList[0] );

            emit itemDropped( &droppedEvent );
        }
    }
}

// Internal helper for checking if list item under mouse cursor can accept the draggable item
void DraggableListWidget::CheckDropTarget( QDragMoveEvent* event )
{
    QList<QUrl> urlList  = event->mimeData( )->urls( );
    bool        accepted = false;

    if ( urlList.count( ) == 1 )
    {
        QListWidgetItem* targetItem = itemAt( event->pos( ) );

        if ( targetItem != 0 )
        {
            ListDropTargetCheckEvent dropCheckEvent( targetItem, urlList[0] );

            emit dropTargetCheck( &dropCheckEvent );

            accepted = dropCheckEvent.IsUrlAcceptable( );
            event->setDropAction( ( dropCheckEvent.CanTargetAccept( ) ) ? Qt::MoveAction : Qt::IgnoreAction );
        }
    }

    event->setAccepted( accepted );
}
