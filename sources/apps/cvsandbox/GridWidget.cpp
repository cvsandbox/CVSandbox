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

#include "GridWidget.hpp"
#include <memory>
#include <QPainter>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QImage>
#include <xtypes.h>

using namespace std;

// Gap size between grid's cells
#define CELL_GAP_SIZE (1)

// -------------------------------------------------------
// ======= GridDropTargetCheckEvent implementation =======
// -------------------------------------------------------

GridDropTargetCheckEvent::GridDropTargetCheckEvent( int row, int column, const QUrl& url ) :
    mRow( row ), mColumn( column ), mUrl( url ), mCanTargetAccept( false ), mIsUrlAcceptable( true )
{
}

int GridDropTargetCheckEvent::Row( ) const
{
    return mRow;
}

int GridDropTargetCheckEvent::Column( ) const
{
    return mColumn;
}

const QUrl& GridDropTargetCheckEvent::Url( ) const
{
    return mUrl;
}

bool GridDropTargetCheckEvent::CanTargetAccept( ) const
{
    return mCanTargetAccept;
}

bool GridDropTargetCheckEvent::IsUrlAcceptable( ) const
{
    return mIsUrlAcceptable;
}

void GridDropTargetCheckEvent::SetCanTargetAccept( bool canTargetAccept )
{
    mCanTargetAccept = canTargetAccept;
}

void GridDropTargetCheckEvent::SetIsUrlAcceptable( bool isUrlAcceptable )
{
    mIsUrlAcceptable = isUrlAcceptable;
}

// ---------------------------------------------------
// ======= GridItemDroppedEvent implementation =======
// ---------------------------------------------------

GridItemDroppedEvent::GridItemDroppedEvent( int row, int column, const QUrl& url ) :
    mRow( row ), mColumn( column ), mUrl( url )
{
}

int GridItemDroppedEvent::Row( ) const
{
    return mRow;
}

int GridItemDroppedEvent::Column( ) const
{
    return mColumn;
}

const QUrl& GridItemDroppedEvent::Url( ) const
{
    return mUrl;
}

// -----------------------------------------
// ======= GridWidget's private data =======
// -----------------------------------------
namespace Private
{
    class GridCellInfo
    {
    public:
        GridCellInfo( ) :
            RowSpan( 1 ), ColSpan( 1 ), Text( ), ImageToDisplay( ), BorderColor( Qt::black )
        {
        }

    public:
        int                RowSpan;
        int                ColSpan;
        QString            Text;
        shared_ptr<QImage> ImageToDisplay;
        QColor             BorderColor;
    };

    class GridWidgetData
    {
    public:
        GridWidgetData( ) :
            EnableCellHighlight( false ),
            RowsCount( 1 ), ColumnsCount( 1 ), NeedToCalculateCellSizes( false ), DefaultCellBackgroundColor( 248, 248, 248 ),
            RowUnderCursor( -1 ), ColumnUnderCursor( -1 ),
            RowForContextMenu( -1 ), ColumnForContextMenu( -1 ),
            CellX( 0 ), CellY( 0 ), CellSize( 0 ),
            CellInfo( 0 )
        {
            CellInfo = new GridCellInfo[1];
        }

        ~GridWidgetData( )
        {
            CleanUp( );
            delete [] CellInfo;
        }

        // Set size of the grid
        void SetGridSize( int rows, int columns )
        {
            if ( rows * columns > 0 )
            {
                GridCellInfo* newCellInfo = new GridCellInfo[ rows * columns ];
                int           rowsToCopy  = XMIN( rows, RowsCount );
                int           colsToCopy  = XMIN( columns, ColumnsCount );

                for ( int trow = 0; trow < rowsToCopy; trow++ )
                {
                    for ( int tcol = 0; tcol < colsToCopy; tcol++ )
                    {
                        newCellInfo[trow * columns + tcol] = CellInfo[trow * ColumnsCount + tcol];

                        // make sure the cell is not bigger than it can be
                        if ( newCellInfo[trow * columns + tcol].ColSpan + tcol > columns )
                        {
                            newCellInfo[trow * columns + tcol].ColSpan = columns - tcol;
                        }
                        if ( newCellInfo[trow * columns + tcol].RowSpan + trow > rows )
                        {
                            newCellInfo[trow * columns + tcol].RowSpan = rows - trow;
                        }
                    }
                }

                RowsCount    = rows;
                ColumnsCount = columns;
                NeedToCalculateCellSizes = true;

                delete [] CellInfo;
                CellInfo = newCellInfo;
            }
        }

        bool CanMergeDown( int row, int column )
        {
            bool ret  = false;

            if ( CheckIfCellExists( row, column ) )
            {
                const GridCellInfo& cell     = CellInfo[row * ColumnsCount + column];
                int                 rowUnder = row + cell.RowSpan;

                if ( rowUnder < RowsCount )
                {
                    ret = true;

                    // check all cells under this one
                    for ( int tcol = column; tcol < column + cell.ColSpan; tcol++ )
                    {
                        const GridCellInfo& tempCell = CellInfo[rowUnder * ColumnsCount + tcol];

                        ret &= ( ( tempCell.ColSpan == 1 ) && ( tempCell.RowSpan == 1 ) );
                    }
                }
            }
            return ret;
        }

        bool CanMergeRight( int row, int column )
        {
            bool ret = false;

            if ( CheckIfCellExists( row, column ) )
            {
                const GridCellInfo& cell     = CellInfo[row * ColumnsCount + column];
                int                 rightCol = column + cell.ColSpan;

                if ( rightCol < ColumnsCount )
                {
                    ret = true;

                    // check all cells on the right of this one
                    for ( int trow = row; trow < row + cell.RowSpan; trow++ )
                    {
                        const GridCellInfo& tempCell = CellInfo[trow * ColumnsCount + rightCol];

                        ret &= ( ( tempCell.ColSpan == 1 ) && ( tempCell.RowSpan == 1 ) );
                    }
                }
            }
            return ret;
        }

        bool CanMergeDownAndRight( int row, int column )
        {
            bool ret = ( CanMergeDown( row, column ) && CanMergeRight( row, column ) );

            if ( ret )
            {
                const GridCellInfo& cell     = CellInfo[row * ColumnsCount + column];
                int                 rowUnder = row + cell.RowSpan;
                int                 rightCol = column + cell.ColSpan;

                const GridCellInfo& tempCell = CellInfo[rowUnder * ColumnsCount + rightCol];

                ret &= ( ( tempCell.ColSpan == 1 ) && ( tempCell.RowSpan == 1 ) );
            }
            return ret;
        }

        bool CanSplit( int row, int column )
        {
            bool ret = false;

            if ( CheckIfCellExists( row, column ) )
            {
                GridCellInfo& cell = CellInfo[row * ColumnsCount + column];

                ret = ( ( cell.ColSpan != 1 ) || ( cell.RowSpan != 1 ) );
            }

            return ret;
        }

        void MergeDown( int row, int column )
        {
            if ( CanMergeDown( row, column ) )
            {
                CellInfo[row * ColumnsCount + column].RowSpan++;
                NeedToCalculateCellSizes = true;
            }
        }

        void MergeRight( int row, int column )
        {
            if ( CanMergeRight( row, column ) )
            {
                CellInfo[row * ColumnsCount + column].ColSpan++;
                NeedToCalculateCellSizes = true;
            }
        }

        void MergeDownAndRight( int row, int column )
        {
            if ( CanMergeDownAndRight( row, column ) )
            {
                CellInfo[row * ColumnsCount + column].RowSpan++;
                CellInfo[row * ColumnsCount + column].ColSpan++;
                NeedToCalculateCellSizes = true;
            }
        }

        void Split( int row, int column )
        {
            if ( CanSplit( row, column ) )
            {
                GridCellInfo& cellInfo = CellInfo[row * ColumnsCount + column];

                cellInfo.ColSpan = 1;
                cellInfo.RowSpan = 1;

                NeedToCalculateCellSizes = true;
            }
        }

        // Set cell's image
        void SetCellImage( int row, int column, const shared_ptr<QImage>& image )
        {
            if ( CheckIfCellExists( row, column ) )
            {
                CellInfo[row * ColumnsCount + column].ImageToDisplay = image;
            }
        }

        // Set cell's text
        void SetCellText( int row, int column, const QString& text )
        {
            if ( CheckIfCellExists( row, column ) )
            {
                CellInfo[row * ColumnsCount + column].Text = text;
            }
        }

        // Set cell's border color
        void SetCellBorderColor( int row, int column, const QColor& color )
        {
            if ( CheckIfCellExists( row, column ) )
            {
                CellInfo[row * ColumnsCount + column].BorderColor = color;
            }
        }

        // Set row/columns span of the grif's cell
        void SetCellSpan( int row, int column, int rowSpan, int columnSpan )
        {
            if ( CheckIfCellExists( row, column ) )
            {
                CellInfo[row * ColumnsCount + column].RowSpan = XINRANGE( rowSpan, 1, RowsCount - row );
                CellInfo[row * ColumnsCount + column].ColSpan = XINRANGE( columnSpan, 1, ColumnsCount - column );
                NeedToCalculateCellSizes = true;
            }
        }

        // Calculate position and size of grid's cells
        void CalculateCellSizes( int totalWidth, int totalHeight, int gapSize )
        {
            int* CellWidth  = new int[ColumnsCount];
            int* CellHeight = new int[RowsCount];

            CleanUp( );

            CellX      = new int[ColumnsCount];
            CellY      = new int[RowsCount];
            CellSize   = new QSize[RowsCount * ColumnsCount];

            DevideIntoPieces( RowsCount, totalHeight, gapSize, CellHeight, CellY );
            DevideIntoPieces( ColumnsCount, totalWidth, gapSize, CellWidth, CellX );

            // set default size for cells without considering row/column span
            for ( int trow = 0; trow < RowsCount; trow++ )
            {
                for ( int tcol = 0; tcol < ColumnsCount; tcol++ )
                {
                    CellSize[trow * ColumnsCount + tcol] = QSize( CellWidth[tcol], CellHeight[trow] );
                }
            }

            // handle row/column span
            for ( int trow = 0; trow < RowsCount; trow++ )
            {
                for ( int tcol = 0; tcol < ColumnsCount; tcol++ )
                {
                    GridCellInfo& cellInfo = CellInfo[trow * ColumnsCount + tcol];

                    if ( ( cellInfo.ColSpan != 1 ) || ( cellInfo.RowSpan != 1 ) )
                    {
                        QSize& cellSize = CellSize[trow * ColumnsCount + tcol];

                        // increase width of the cell
                        for ( int span = 1; span < cellInfo.ColSpan; span++ )
                        {
                            cellSize.setWidth( cellSize.width( ) +
                                               gapSize +
                                               CellSize[trow * ColumnsCount + tcol + span].width( ) );
                        }
                        // increase height of the cell
                        for ( int span = 1; span < cellInfo.RowSpan; span++ )
                        {
                            cellSize.setHeight( cellSize.height( ) +
                                                gapSize +
                                                CellSize[( trow + span ) * ColumnsCount + tcol].height( ) );
                        }
                        // hide cells covered by the one with row/column span
                        for ( int rowSpan = 0; rowSpan < cellInfo.RowSpan; rowSpan++ )
                        {
                            for ( int colSpan = 0; colSpan < cellInfo.ColSpan; colSpan++ )
                            {
                                if ( ( rowSpan != 0 ) || ( colSpan != 0 ) )
                                {
                                    QSize& hiddenCellSize = CellSize[( trow + rowSpan ) * ColumnsCount + tcol + colSpan];
                                    hiddenCellSize.setHeight( 0 );
                                    hiddenCellSize.setWidth( 0 );
                                }
                            }
                        }
                    }
                }
            }

            delete [] CellWidth;
            delete [] CellHeight;
        }

        // Divide line into pieces of equal/similar size
        void DevideIntoPieces( int piecesCount, int totalSize, int gapSize, int* sizes, int* indents )
        {
            if ( piecesCount == 1 )
            {
                indents[0] = 0;
                sizes[0]   = totalSize;
            }
            else
            {
                int sizeLeft   = totalSize + gapSize;
                int piecesLeft = piecesCount;
                int nextIndent = 0;

                for ( int piece = 0; piece < piecesCount; piece++ )
                {
                    indents[piece] = nextIndent;
                    sizes[piece]   = sizeLeft / piecesLeft;

                    sizeLeft      -= sizes[piece];
                    piecesLeft    -= 1;

                    nextIndent    += sizes[piece];
                    sizes[piece]  -= gapSize;
                }
            }
        }

        // Get cell at the specified coordinates
        void GetCellAt( int x, int y, int* row, int* column )
        {
            if ( ( row != 0 ) && ( column != 0 ) )
            {
                *row    = -1;
                *column = -1;

                if ( CellX != 0 )
                {
                    for ( int trow = 0; trow < RowsCount; trow++ )
                    {
                        for ( int tcol = 0; tcol < ColumnsCount; tcol++ )
                        {
                            QSize cellSize = CellSize[trow * ColumnsCount + tcol];

                            if ( ( x >= CellX[tcol] ) &&
                                 ( y >= CellY[trow] ) &&
                                 ( x <  CellX[tcol] + cellSize.width( ) ) &&
                                 ( y <  CellY[trow] + cellSize.height( ) ) )
                            {
                                *row    = trow;
                                *column = tcol;
                                break;
                            }
                        }
                    }
                }
            }
        }

    private:
        void CleanUp( )
        {
            delete [] CellX;
            delete [] CellY;
            delete [] CellSize;
        }

        bool CheckIfCellExists( int row, int column )
        {
            bool ret = false;

            if ( ( row >= 0 ) && ( column >= 0 ) &&
                 ( row < RowsCount ) && ( column < ColumnsCount ) )
            {
                ret = true;
            }

            return ret;
        }

    public:
        bool    EnableCellHighlight;
        int     RowsCount;
        int     ColumnsCount;
        bool    NeedToCalculateCellSizes;
        QColor  DefaultCellBackgroundColor;

        int     RowUnderCursor;
        int     ColumnUnderCursor;

        int     RowForContextMenu;
        int     ColumnForContextMenu;

        int*    CellX;
        int*    CellY;
        QSize*  CellSize;

        GridCellInfo* CellInfo;
    };
}

// -----------------------------------------
// ======= GridWidget implementation =======
// -----------------------------------------

GridWidget::GridWidget( QWidget* parent ) :
    QWidget( parent ),
    mData( new Private::GridWidgetData )
{
    this->setMouseTracking( true );
}

GridWidget::~GridWidget( )
{
    delete mData;
}

// Enable/disable set highlight on mouse over
void GridWidget::EnableCellHighlight( bool enable )
{
    mData->EnableCellHighlight = enable;
}

// Get rows/columns count
int GridWidget::RowsCount( ) const
{
    return mData->RowsCount;
}
int GridWidget::ColumnsCount( ) const
{
    return mData->ColumnsCount;
}

// Get rows/columns count
void GridWidget::SetRowsCount( int rowsCount )
{
    mData->SetGridSize( XMAX( 1, rowsCount ), mData->ColumnsCount );
    update( );
}
void GridWidget::SetColumnsCount( int columnsCount )
{
    mData->SetGridSize( mData->RowsCount, XMAX( 1, columnsCount ) );
    update( );
}

// The widget was resized
void GridWidget::resizeEvent( QResizeEvent* )
{
    mData->NeedToCalculateCellSizes = true;
    update( );
}

// Paint the control
void GridWidget::paintEvent( QPaintEvent* )
{
    QPainter painter;
    QRect    myRect( rect( ) );
    QBrush   cellBrush( mData->DefaultCellBackgroundColor, Qt::SolidPattern );
    QBrush   cellUnderCursorBrush( QColor( 220, 220, 255 ), Qt::SolidPattern );
    QBrush   cellWithContextMenuBrush( QColor( 220, 255, 220 ), Qt::SolidPattern );

    if ( mData->NeedToCalculateCellSizes )
    {
        mData->CalculateCellSizes( myRect.width( ), myRect.height( ), CELL_GAP_SIZE );
        mData->NeedToCalculateCellSizes = false;
    }

    painter.begin( this );
    painter.setRenderHint( QPainter::Antialiasing, false );
    painter.setFont( font( ) );

    for ( int row = 0; row < mData->RowsCount; row++ )
    {
        for ( int column = 0; column < mData->ColumnsCount; column++ )
        {
            QSize                  cellSize = mData->CellSize[row * mData->ColumnsCount + column];
            Private::GridCellInfo& cellInfo = mData->CellInfo[row * mData->ColumnsCount + column];

            if ( ( cellSize.width( ) > 0 ) && ( cellSize.height( ) > 0 ) )
            {
                QBrush brush = cellBrush;

                if ( ( row == mData->RowUnderCursor ) && ( column == mData->ColumnUnderCursor ) && ( mData->EnableCellHighlight ) )
                {
                    brush = cellUnderCursorBrush;
                }
                if ( ( row == mData->RowForContextMenu ) && ( column == mData->ColumnForContextMenu ) )
                {
                    brush = cellWithContextMenuBrush;
                }

                painter.setPen( QPen( cellInfo.BorderColor ) );

                painter.fillRect( mData->CellX[column], mData->CellY[row],
                                  cellSize.width( ) - 1, cellSize.height( ) - 1,
                                  brush );
                painter.drawRect( mData->CellX[column], mData->CellY[row],
                                  cellSize.width( ) - 1, cellSize.height( ) - 1 );

                {
                    if ( cellInfo.ImageToDisplay )
                    {
                        painter.setCompositionMode( QPainter::CompositionMode_Source );
                        painter.drawImage( QRect( mData->CellX[column] + 1, mData->CellY[row] + 1,
                                                  cellSize.width( ) - 2, cellSize.height( ) - 2 ),
                                           *cellInfo.ImageToDisplay );
                    }

                    if ( !cellInfo.Text.isEmpty( ) )
                    {
                        painter.setPen( QPen( Qt::black ) );
                        painter.drawText ( mData->CellX[column], mData->CellY[row],
                                           cellSize.width( ) - 1, cellSize.height( ) - 1,
                                           Qt::AlignCenter, cellInfo.Text );
                    }
                }
            }
        }
    }

    painter.end( );
}

// On mouse moving in the widget
void GridWidget::mouseMoveEvent( QMouseEvent* event )
{
    int newRowUnderCursor    = -1;
    int newColumnUnderCursor = -1;

    mData->GetCellAt( event->x( ), event->y( ), &newRowUnderCursor, &newColumnUnderCursor );

    if ( ( mData->RowUnderCursor != newRowUnderCursor ) ||
         ( mData->ColumnUnderCursor != newColumnUnderCursor ) )
    {
        mData->RowUnderCursor    = newRowUnderCursor;
        mData->ColumnUnderCursor = newColumnUnderCursor;

        if ( mData->EnableCellHighlight )
        {
            update( );
        }
    }
}

// On mouse left the widget
void GridWidget::leaveEvent( QEvent* )
{
    mData->RowUnderCursor    = -1;
    mData->ColumnUnderCursor = -1;

    update( );
}

// Context menu is requested for the widget
void GridWidget::contextMenuEvent( QContextMenuEvent* event )
{
    mData->GetCellAt( event->x( ), event->y( ),
                      &(mData->RowForContextMenu), &(mData->ColumnForContextMenu) );

    if ( ( mData->RowForContextMenu != -1 ) && ( mData->ColumnForContextMenu != -1 ) )
    {
        update( );

        emit cellContextMenuRequested( mData->RowForContextMenu, mData->ColumnForContextMenu, event->globalPos( ) );

        mData->RowForContextMenu    = -1;
        mData->ColumnForContextMenu = -1;
        update( );
    }
}

// Check if it is possible to increase cell's row span
bool GridWidget::CanMergeDown( int row, int column ) const
{
    return mData->CanMergeDown( row, column );
}

// Check if it is possible to increase cell's column span
bool GridWidget::CanMergeRight( int row, int column ) const
{
    return mData->CanMergeRight( row, column );
}

// Check if it is possible to increase cell's row and column span
bool GridWidget::CanMergeDownAndRight( int row, int column ) const
{
    return mData->CanMergeDownAndRight( row, column );
}

// Check if the cell can be split
bool GridWidget::CanSplit( int row, int column ) const
{
    return mData->CanSplit( row, column );
}

// Increase cell's row span
void GridWidget::MergeDown( int row, int column )
{
    mData->MergeDown( row, column );
    update( );
}

// Increase cell's column span
void GridWidget::MergeRight( int row, int column )
{
    mData->MergeRight( row, column );
    update( );
}

// Increase cell's row and column spans
void GridWidget::MergeDownAndRight( int row, int column )
{
    mData->MergeDownAndRight( row, column );
    update( );
}

// Split the cell removing any merging on it
void GridWidget::Split( int row, int column )
{
    mData->Split( row, column );
    update( );
}

// Set cells' background color when no highlighted
void GridWidget::SetCellsDefaultBackgroundColor( const QColor& color )
{
    mData->DefaultCellBackgroundColor = color;
    update( );
}

// Set image to display in the grid's cell
void GridWidget::SetCellImage( int row, int column, const std::shared_ptr<QImage>& image )
{
    mData->SetCellImage( row, column, image );
    update( );
}

// Set text to display in the grid's cell
void GridWidget::SetCellText( int row, int column, const QString& text )
{
    mData->SetCellText( row, column, text );
    update( );
}

// Set color of the border displayed around the cell
void GridWidget::SetCellBorderColor( int row, int column, const QColor& color )
{
    mData->SetCellBorderColor( row, column, color );
    update( );
}

// Set row/columns span of the grid's cell
void GridWidget::SetCellSpan( int row, int column, int rowSpan, int columnSpan )
{
    mData->SetCellSpan( row, column, rowSpan, columnSpan );
    update( );
}

// Drag-n-drop entered the control
void GridWidget::dragEnterEvent( QDragEnterEvent* event )
{
    CheckDropTarget( event );
}

// Drag-n-drop is moving over the control
void GridWidget::dragMoveEvent( QDragMoveEvent* event )
{
    CheckDropTarget( event );
}

// An item was dropped in the control
void GridWidget::dropEvent( QDropEvent* event )
{
    QList<QUrl> urlList  = event->mimeData( )->urls( );

    if ( urlList.count( ) == 1 )
    {
        int row    = -1;
        int column = -1;

        mData->GetCellAt( event->pos( ).x( ), event->pos( ).y( ), &row, &column );

        if ( ( row != -1 ) && ( column != -1 ) )
        {
            GridItemDroppedEvent droppedEvent( row, column, urlList[0] );

            emit itemDropped( &droppedEvent );
        }
    }
}

// Internal helper for checking if list item under mouse cursor can accept the draggable item
void GridWidget::CheckDropTarget( QDragMoveEvent* event )
{
    QList<QUrl> urlList  = event->mimeData( )->urls( );
    bool        accepted = false;

    if ( urlList.count( ) == 1 )
    {
        int row    = -1;
        int column = -1;

        mData->GetCellAt( event->pos( ).x( ), event->pos( ).y( ), &row, &column );

        if ( ( row != -1 ) && ( column != -1 ) )
        {
            GridDropTargetCheckEvent dropCheckEvent( row, column, urlList[0] );

            emit dropTargetCheck( &dropCheckEvent );

            accepted = dropCheckEvent.IsUrlAcceptable( );
            event->setDropAction( ( dropCheckEvent.CanTargetAccept( ) ) ? Qt::LinkAction : Qt::IgnoreAction );
        }
    }

    event->setAccepted( accepted );
}
