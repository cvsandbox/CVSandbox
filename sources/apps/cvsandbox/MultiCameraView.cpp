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

#include <vector>
#include "MultiCameraView.hpp"
#include "ui_MultiCameraView.h"
#include "CamerasViewConfiguration.hpp"
#include "VideoSourcePlayerFactory.hpp"
#include "VideoSourceInAutomationServer.hpp"

#include "ServiceManager.hpp"

#include <QMouseEvent>
#include <QTimer>

// Gap size between video cells
#define CELL_GAP_SIZE (2)
// Cells' border size (it must video players property actually, but lets keep it here for not)
#define CELL_BORDER_SIZE (4)
// Cells' total border size (from both sides)
#define CELL_TOTAL_BORDER_SIZE (CELL_BORDER_SIZE * 2)
// Total extra spacing around every space
#define CELL_EXTRA_SPACING (CELL_GAP_SIZE + CELL_TOTAL_BORDER_SIZE)

using namespace std;
using namespace CVSandbox;
using namespace CVSandbox::Automation;

namespace Private
{
    class MultiCameraViewData
    {
    public:
        MultiCameraViewData( )
        {
            NotificationTimer.setSingleShot( true );
            NotificationTimer.setInterval( 0 );
        }

        ~MultiCameraViewData( )
        {
            ReleasePlayers( );
        }

        void ReleasePlayers( )
        {
            Players.clear( );
            PlayerCells.clear( );
        }

    public:
        CamerasViewConfiguration   ViewConfiguration;
        vector<VideoSourcePlayer*> Players;
        vector<pair<int, int>>     PlayerCells;
        QTimer                     NotificationTimer;

        int                        LastClickedRow;
        int                        LastClickedColumn;
        bool                       IsLeftButtonClicked;
        QPoint                     ClickedGlobalPos;
    };

    // Calculate size for view cells
    static const QSize CalculateCellSize( const QSize& configuredSize, bool viewAutoSize, const QSize& viewPreferredSize,
                                          int rowsCount, int columnsCount )
    {
        QSize ret = configuredSize;

        if ( !viewAutoSize )
        {
            int totalConfiguredWidth  = columnsCount * ( configuredSize.width( )  + CELL_EXTRA_SPACING );
            int totalConfiguredHeight = rowsCount    * ( configuredSize.height( ) + CELL_EXTRA_SPACING );

            double xRatio = static_cast<double>( viewPreferredSize.width( ) )  / totalConfiguredWidth;
            double yRatio = static_cast<double>( viewPreferredSize.height( ) ) / totalConfiguredHeight;
            double ratio  = XMIN( xRatio, yRatio );

            int totalWidth  = static_cast<int>( ratio * totalConfiguredWidth );
            int totalHeight = static_cast<int>( ratio * totalConfiguredHeight );

            ret = QSize( totalWidth  / columnsCount - CELL_EXTRA_SPACING,
                         totalHeight / rowsCount    - CELL_EXTRA_SPACING );
        }

        return ret;
    }
}

MultiCameraView::MultiCameraView( QWidget* parent ) :
    QFrame( parent ),
    ui( new Ui::MultiCameraView ),
    mData( new ::Private::MultiCameraViewData )
{
    ui->setupUi( this );
    connect( &mData->NotificationTimer, SIGNAL(timeout()), this, SLOT(me_NotifyMouseClick()), Qt::QueuedConnection );
}

MultiCameraView::~MultiCameraView( )
{
    delete mData;
    delete ui;
}

// Set view confguraton
void MultiCameraView::SetView( const CamerasViewConfiguration& view,
                               const map<XGuid, uint32_t>& videoSourceMap,
                               bool autoSizeView, const QSize& preferredSize )
{
    const shared_ptr<XAutomationServer>& server = ServiceManager::Instance( ).GetAutomationServer( );

    int rowsCount    = view.RowsCount( );
    int columnsCount = view.ColumnsCount( );

    QSize cellSize   = ::Private::CalculateCellSize( QSize( view.CellWidth( ), view.CellHeight( ) ),
                                                     autoSizeView, preferredSize, rowsCount, columnsCount );
    int cellWidth    = cellSize.width( );
    int cellHeight   = cellSize.height( );

    int spanIndent   = CELL_BORDER_SIZE + CELL_GAP_SIZE / 2;
    int totalWidth   = columnsCount * ( cellWidth  + CELL_EXTRA_SPACING );
    int totalHeight  = rowsCount    * ( cellHeight + CELL_EXTRA_SPACING );

    mData->ReleasePlayers( );
    mData->ViewConfiguration = view;

    setMaximumSize( totalWidth, totalHeight );
    setMinimumSize( totalWidth, totalHeight );
    resize( totalWidth, totalHeight );

    for ( int row = 0; row < rowsCount; row++ )
    {
        for ( int column = 0; column < columnsCount; column++ )
        {
            if ( view.IsCellVisible( row,column ) )
            {
                const CameraCellConfiguration* cellConfig = view.GetCellConfiguration( row, column );

                if ( cellConfig != 0 )
                {
                    const XGuid& cameraId = cellConfig->CameraId( );

                    map<XGuid, uint32_t>::const_iterator vsIt = videoSourceMap.find( cameraId );

                    if ( ( !cameraId.IsEmpty( ) ) && ( vsIt != videoSourceMap.end( ) ) )
                    {
                        VideoSourcePlayer* player  = VideoSourcePlayerFactory::CreatePlayer( this );
                        int                rowSpan = cellConfig->RowSpan( );
                        int                colSpan = cellConfig->ColumnSpan( );

                        player->PlayerWidget( )->setGeometry(
                                    column * ( cellWidth  + CELL_EXTRA_SPACING ) + spanIndent * ( colSpan - 1 ),
                                    row    * ( cellHeight + CELL_EXTRA_SPACING ) + spanIndent * ( rowSpan - 1 ),
                                    cellWidth  * colSpan + CELL_TOTAL_BORDER_SIZE,
                                    cellHeight * rowSpan + CELL_TOTAL_BORDER_SIZE );
                        player->PlayerWidget( )->setVisible( true );
                        player->PlayerWidget( )->installEventFilter( this );

                        // wrap video source managed by automation server into IVideoSourceInterface
                        shared_ptr<VideoSourceInAutomationServer> videoSource = VideoSourceInAutomationServer::Create( server, vsIt->second );
                        // set it to the player
                        player->SetVideoSource( videoSource );

                        mData->Players.push_back( player );
                        mData->PlayerCells.push_back( pair<int, int>( row, column ) );
                    }
                }
            }
        }
    }
}

// Set/clear specified cell highlighting
void MultiCameraView::HighlightCell( int row, int column, bool highlight )
{
    if ( ( row >= 0 ) && ( column >= 0 ) &&
         ( row < mData->ViewConfiguration.RowsCount( ) ) &&
         ( column < mData->ViewConfiguration.ColumnsCount( ) ) )
    {
        int counter = 0;

        for ( vector<pair<int, int>>::const_iterator it = mData->PlayerCells.begin( ); it != mData->PlayerCells.end( ); ++it )
        {
            if ( ( row == (*it).first ) && ( column == (*it).second ) )
            {
                mData->Players[counter]->SetHighlighted( highlight );
            }

            counter++;
        }
    }
}

// Set preferred size of the total area available for cameras' view
void MultiCameraView::SetPreferredContentSize( bool autoSize, const QSize& size )
{
    if ( !mData->Players.empty( ) )
    {
        int rowsCount    = mData->ViewConfiguration.RowsCount( );
        int columnsCount = mData->ViewConfiguration.ColumnsCount( );

        QSize cellSize   = ::Private::CalculateCellSize( QSize( mData->ViewConfiguration.CellWidth( ), mData->ViewConfiguration.CellHeight( ) ),
                                                         autoSize, size, rowsCount, columnsCount );
        int cellWidth    = cellSize.width( );
        int cellHeight   = cellSize.height( );

        int spanIndent   = CELL_BORDER_SIZE + CELL_GAP_SIZE / 2;
        int totalWidth   = columnsCount * ( cellWidth  + CELL_EXTRA_SPACING );
        int totalHeight  = rowsCount    * ( cellHeight + CELL_EXTRA_SPACING );

        int counter      = 0;

        setMaximumSize( totalWidth, totalHeight );
        setMinimumSize( totalWidth, totalHeight );
        resize( totalWidth, totalHeight );

        for ( vector<pair<int, int>>::const_iterator it = mData->PlayerCells.begin( ); it != mData->PlayerCells.end( ); ++it )
        {
            int row    = (*it).first;
            int column = (*it).second;

            const CameraCellConfiguration* cellConfig = mData->ViewConfiguration.GetCellConfiguration( row, column );

            if ( cellConfig != 0 )
            {
                VideoSourcePlayer* player  = mData->Players[counter];
                int                rowSpan = cellConfig->RowSpan( );
                int                colSpan = cellConfig->ColumnSpan( );

                player->PlayerWidget( )->setGeometry(
                            column * ( cellWidth  + CELL_EXTRA_SPACING ) + spanIndent * ( colSpan - 1 ),
                            row    * ( cellHeight + CELL_EXTRA_SPACING ) + spanIndent * ( rowSpan - 1 ),
                            cellWidth  * colSpan + CELL_TOTAL_BORDER_SIZE,
                            cellHeight * rowSpan + CELL_TOTAL_BORDER_SIZE );
            }

            counter++;
        }
    }
}

// Handle mouse press in video player widgets
bool MultiCameraView::eventFilter( QObject* target, QEvent* event )
{
    bool ret = false;

    if ( event->type( ) == QEvent::MouseButtonPress )
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>( event );

        if ( ( mouseEvent->button( ) == Qt::LeftButton ) ||
             ( mouseEvent->button( ) == Qt::RightButton ) )
        {
            int counter = 0;

            mData->LastClickedRow      = -1;
            mData->LastClickedColumn   = -1;
            mData->IsLeftButtonClicked = ( mouseEvent->button( ) == Qt::LeftButton );
            mData->ClickedGlobalPos    = mouseEvent->globalPos( );

            for ( vector<pair<int, int>>::const_iterator it = mData->PlayerCells.begin( ); it != mData->PlayerCells.end( ); ++it )
            {
                if ( static_cast<QObject*>( mData->Players[counter]->PlayerWidget( ) ) == target )
                {
                    mData->LastClickedRow    = (*it).first;
                    mData->LastClickedColumn = (*it).second;
                    break;
                }

                counter++;
            }

            ret = true;

            if ( mData->LastClickedRow != -1 )
            {
                mData->NotificationTimer.start( );
            }
        }
    }

    return ret;
}

// Notify anyone interested about mouse click in one of the cells
void MultiCameraView::me_NotifyMouseClick( )
{
    if ( mData->LastClickedRow != -1 )
    {
       emit CameraMousePress( mData->LastClickedRow, mData->LastClickedColumn,
                              mData->IsLeftButtonClicked, mData->ClickedGlobalPos );
    }
}
