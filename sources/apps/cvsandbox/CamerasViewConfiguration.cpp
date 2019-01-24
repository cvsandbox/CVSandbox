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

#include "CamerasViewConfiguration.hpp"
#include <QObject>
#include <xtypes.h>
#include <vector>

using namespace std;
using namespace CVSandbox;

// Maximum value for cell's row/column span
#define MAX_CELL_SPAN (4)
// Maximum grid size (grid is size X size actually)
#define MAX_GRID_SIZE (4)
// Minimum/Maximum width/height of a view cell
#define MIN_CELL_WIDTH  (160)
#define MAX_CELL_WIDTH  (2400)
#define MIN_CELL_HEIGHT (120)
#define MAX_CELL_HEIGHT (1800)

// Default name of a view
static const char* DEFAULT_VIEW_NAME = "1 - Default";

// ------------------------------------------------------
// ======= CameraCellConfiguration implementation =======
// ------------------------------------------------------

CameraCellConfiguration::CameraCellConfiguration( ) :
    mRowSpan( 1 ), mColumnSpan( 1 ), mCameraId( )
{

}

// Comparison operator
bool CameraCellConfiguration::operator==( const CameraCellConfiguration& rhs ) const
{
    return ( ( mRowSpan    == rhs.mRowSpan ) &&
             ( mColumnSpan == rhs.mColumnSpan ) &&
             ( mCameraId   == rhs.mCameraId ) );
}

// Get/Set row/column span
int CameraCellConfiguration::RowSpan( ) const
{
    return mRowSpan;
}
int CameraCellConfiguration::ColumnSpan( ) const
{
    return mColumnSpan;
}
void CameraCellConfiguration::SetRowSpan( int rowSpan )
{
    mRowSpan = XINRANGE( rowSpan, 1, MAX_CELL_SPAN );
}
void CameraCellConfiguration::SetColumnSpan( int columnSpan )
{
    mColumnSpan = XINRANGE( columnSpan, 1, MAX_CELL_SPAN );
}

// Get/Set camera ID
const XGuid CameraCellConfiguration::CameraId( ) const
{
    return mCameraId;
}
void CameraCellConfiguration::SetCameraId( const XGuid& cameraId )
{
    mCameraId = cameraId;
}

// -------------------------------------------------------
// ======= CamerasViewConfiguration implementation =======
// -------------------------------------------------------

CamerasViewConfiguration::CamerasViewConfiguration( ) :
    mId( ), mViewName( DEFAULT_VIEW_NAME ),
    mRowsCount( 1 ), mColumnsCount( 1 ),
    mCellWidth( 640 ), mCellHeight( 480 ),
    mCells( new CameraCellConfiguration[1] )
{

}

CamerasViewConfiguration::CamerasViewConfiguration( XGuid id, int rows, int columns ) :
    mId( id ), mViewName( DEFAULT_VIEW_NAME ),
    mRowsCount( 0 ), mColumnsCount( 0 ),
    mCellWidth( 640 ), mCellHeight( 480 ),
    mCells( 0 )
{
    if ( rows * columns > 1 )
    {
        mCellWidth  = 320;
        mCellHeight = 240;
    }

    SetViewSize( rows, columns );
}

CamerasViewConfiguration::CamerasViewConfiguration( const CamerasViewConfiguration& rhs ) :
    mId( ), mViewName( ),
    mRowsCount( 0 ), mColumnsCount( 0 ),
    mCellWidth( 0 ), mCellHeight( 0 ),
    mCells( 0 )
{
    CopyFrom( rhs );
}


CamerasViewConfiguration::~CamerasViewConfiguration( )
{
    delete [] mCells;
}

// Assignment operator - copy the right hand side
CamerasViewConfiguration& CamerasViewConfiguration::operator=( const CamerasViewConfiguration& rhs )
{
    CopyFrom( rhs );
    return *this;
}

// Comparison operator
bool CamerasViewConfiguration::operator==( const CamerasViewConfiguration& rhs ) const
{
    bool ret = ( ( mId           == rhs.mId ) &&
                 ( mRowsCount    == rhs.mRowsCount ) &&
                 ( mColumnsCount == rhs.mColumnsCount ) &&
                 ( mCellWidth    == rhs.mCellWidth ) &&
                 ( mCellHeight   == rhs.mCellHeight ) &&
                 ( mViewName     == rhs.mViewName ) );

    if ( ret )
    {
        int cellCount = mRowsCount * mColumnsCount;

        for ( int i = 0; i < cellCount; i++ )
        {
            if ( mCells[i] != rhs.mCells[i] )
            {
                ret = false;
                break;
            }
        }
    }

    return ret;
}

// Copy view configuration from another object
void CamerasViewConfiguration::CopyFrom( const CamerasViewConfiguration& rhs )
{
    delete [] mCells;
    mCells = 0;

    mId           = rhs.mId;
    mViewName     = rhs.mViewName;
    mRowsCount    = rhs.mRowsCount;
    mColumnsCount = rhs.mColumnsCount;
    mCellWidth    = rhs.mCellWidth;
    mCellHeight   = rhs.mCellHeight;

    mCells = new CameraCellConfiguration[mRowsCount * mColumnsCount];

    for ( int trow = 0; trow < mRowsCount; trow++ )
    {
        for ( int tcol = 0; tcol < mColumnsCount; tcol++ )
        {
            mCells[trow * mColumnsCount + tcol] = rhs.mCells[trow * mColumnsCount + tcol];
        }
    }
}

// Get/Set view ID
const XGuid CamerasViewConfiguration::Id( ) const
{
    return mId;
}
void CamerasViewConfiguration::SetId( const XGuid& id )
{
    mId = id;
}

// Get/Set view name
string CamerasViewConfiguration::Name( ) const
{
    return mViewName;
}
void CamerasViewConfiguration::SetName( const string& name )
{
    mViewName = name;
}

// Get view size
int CamerasViewConfiguration::RowsCount( ) const
{
    return mRowsCount;
}
int CamerasViewConfiguration::ColumnsCount( ) const
{
    return mColumnsCount;
}

// Set view size
void CamerasViewConfiguration::SetViewSize( int rows, int columns )
{
    if ( ( mRowsCount != rows ) || ( mColumnsCount != columns ) )
    {
        int newRowsCount    = XINRANGE( rows, 1, MAX_GRID_SIZE );
        int newColumnsCount = XINRANGE( columns, 1, MAX_GRID_SIZE );
        int rowsToCopy      = XMIN( newRowsCount, mRowsCount );
        int colsToCopy      = XMIN( newColumnsCount, mColumnsCount );

        CameraCellConfiguration* newCellInfo = new CameraCellConfiguration[newRowsCount * newColumnsCount];

        for ( int trow = 0; trow < rowsToCopy; trow++ )
        {
            for ( int tcol = 0; tcol < colsToCopy; tcol++ )
            {
                newCellInfo[trow * columns + tcol] = mCells[trow * mColumnsCount + tcol];

                // make sure the cell is not bigger than it can be
                if ( newCellInfo[trow * columns + tcol].ColumnSpan( ) + tcol > newColumnsCount )
                {
                    newCellInfo[trow * columns + tcol].SetColumnSpan( newColumnsCount - tcol );
                }
                if ( newCellInfo[trow * columns + tcol].RowSpan( ) + trow > newRowsCount )
                {
                    newCellInfo[trow * columns + tcol].SetRowSpan( newRowsCount - trow );
                }
            }
        }

        mRowsCount    = newRowsCount;
        mColumnsCount = newColumnsCount;

        delete [] mCells;
        mCells = newCellInfo;
    }
}

// Get cell width/height
int CamerasViewConfiguration::CellWidth( ) const
{
    return mCellWidth;
}
int CamerasViewConfiguration::CellHeight( ) const
{
    return mCellHeight;
}

// Set cell width/height
void CamerasViewConfiguration::SetCellWidth( int width )
{
    mCellWidth = XINRANGE( width, MIN_CELL_WIDTH, MAX_CELL_WIDTH );
}
void CamerasViewConfiguration::SetCellHeight( int height )
{
    mCellHeight = XINRANGE( height, MIN_CELL_HEIGHT, MAX_CELL_HEIGHT );
}

// Get cell configuration
CameraCellConfiguration* CamerasViewConfiguration::GetCellConfiguration( int row, int column )
{
    CameraCellConfiguration* ret = 0;

    if ( ( row >= 0 ) && ( row < mRowsCount ) &&
         ( column >= 0 ) && ( column < mColumnsCount ) )
    {
        ret = &( mCells[row * mColumnsCount + column] );
    }

    return ret;
}
const CameraCellConfiguration* CamerasViewConfiguration::GetCellConfiguration( int row, int column ) const
{
    return const_cast<CamerasViewConfiguration*>( this )->GetCellConfiguration( row, column );
}

// Check if the cell is visible or not (overlapped by another left/top cell with row/column span setting)
bool CamerasViewConfiguration::IsCellVisible( int row, int column ) const
{
   bool ret = false;

   if ( ( row >= 0 ) && ( row < mRowsCount ) &&
        ( column >= 0 ) && ( column < mColumnsCount ) )
   {
       ret = true;

       for ( int trow = 0; ( trow <= row ) && ( ret ); trow++ )
       {
           for ( int tcol = 0; tcol <= column; tcol++ )
           {
               if ( ( trow != row ) || ( tcol != column ) )
               {
                   CameraCellConfiguration* cell = &( mCells[trow * mColumnsCount + tcol] );

                   if ( ( trow + cell->RowSpan( ) > row ) &&
                        ( tcol + cell->ColumnSpan( ) > column ) )
                   {
                       ret = false;
                       break;
                   }
               }
           }
       }
   }

   return ret;
}
