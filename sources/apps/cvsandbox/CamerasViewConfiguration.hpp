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
#ifndef CVS_CAMERAS_VIEW_CONFIGURATION_HPP
#define CVS_CAMERAS_VIEW_CONFIGURATION_HPP

#include <XGuid.hpp>

// Configuration of a view's cell
class CameraCellConfiguration
{
public:
    CameraCellConfiguration( );

    // Comparison operator
    bool operator==( const CameraCellConfiguration& rhs ) const;
    bool operator!=( const CameraCellConfiguration& rhs ) const
    {
        return ( !( (*this) == rhs ) ) ;
    }

    // Get/Set row/column span
    int RowSpan( ) const;
    int ColumnSpan( ) const;
    void SetRowSpan( int rowSpan );
    void SetColumnSpan( int columnSpan );

    // Get/Set camera ID
    const CVSandbox::XGuid CameraId( ) const;
    void SetCameraId( const CVSandbox::XGuid& cameraId );

private:
    int               mRowSpan;
    int               mColumnSpan;
    CVSandbox::XGuid  mCameraId;
};

// Cameras' view configuration which is part of a sandbox
class CamerasViewConfiguration
{
public:
    CamerasViewConfiguration( );
    CamerasViewConfiguration( CVSandbox::XGuid id, int rows, int columns );
    CamerasViewConfiguration( const CamerasViewConfiguration& rhs );

    ~CamerasViewConfiguration( );

    // Assignment operators
    CamerasViewConfiguration& operator= ( const CamerasViewConfiguration& rhs );

    // Comparison operator
    bool operator==( const CamerasViewConfiguration& rhs ) const;
    bool operator!=( const CamerasViewConfiguration& rhs ) const
    {
        return ( !( (*this) == rhs ) ) ;
    }

    // Get/Set view ID
    const CVSandbox::XGuid Id( ) const;
    void SetId( const CVSandbox::XGuid& id );

    // Get/Set view name
    std::string Name( ) const;
    void SetName( const std::string& name );

    // Get/Set view size
    int RowsCount( ) const;
    int ColumnsCount( ) const;
    void SetViewSize( int rows, int columns );

    // Get/Set cell width/height
    int CellWidth( ) const;
    int CellHeight( ) const;
    void SetCellWidth( int width );
    void SetCellHeight( int height );

    // Get cell configuration
    CameraCellConfiguration* GetCellConfiguration( int row, int column );
    const CameraCellConfiguration* GetCellConfiguration( int row, int column ) const;

    // Check if the cell is visible or not (overlapped by another left/top cell with row/column span setting)
    bool IsCellVisible( int row, int column ) const;

private:
    void CopyFrom( const CamerasViewConfiguration& rhs );

private:
    CVSandbox::XGuid         mId;
    std::string              mViewName;
    int                      mRowsCount;
    int                      mColumnsCount;
    int                      mCellWidth;
    int                      mCellHeight;
    CameraCellConfiguration* mCells;
};

#endif // CVS_CAMERAS_VIEW_CONFIGURATION_HPP
