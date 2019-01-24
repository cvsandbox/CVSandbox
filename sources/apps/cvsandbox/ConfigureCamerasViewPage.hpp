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
#ifndef CVS_CONFIGURE_CAMERAS_VIEW_PAGE_HPP
#define CVS_CONFIGURE_CAMERAS_VIEW_PAGE_HPP

#include <XGuid.hpp>
#include <DraggableListWidget.hpp>

#include "GridWidget.hpp"
#include "WizardPageFrame.hpp"
#include "CamerasViewConfiguration.hpp"

namespace Ui
{
    class ConfigureCamerasViewPage;
}

namespace Private
{
    class ConfigureCamerasViewPageData;
}

class ConfigureCamerasViewPage : public WizardPageFrame
{
    Q_OBJECT

public:
    explicit ConfigureCamerasViewPage( QWidget *parent = 0 );
    ~ConfigureCamerasViewPage( );

    // Set list of camera available for view configuration
    void SetAvalableCameras( const std::vector<CVSandbox::XGuid>& availableCameras );
    // Set configuration of the view
    void SetViewConfigurations( const std::vector<CamerasViewConfiguration>& viewConfigurations );
    // Get configuration of the view
    const std::vector<CamerasViewConfiguration> GetViewConfigurations( ) const;
    // Get/Set default view ID
    const CVSandbox::XGuid DefaultViewId( ) const;
    void SetDefaultViewId( const CVSandbox::XGuid& viewId );

public:
    virtual bool CanGoNext( ) const;

private slots:
    void on_viewsComboBox_currentIndexChanged( int index );
    void on_rowSpinBox_valueChanged( int arg1 );
    void on_columnsSpinBox_valueChanged( int arg1 );
    void on_cellWidthSpinBox_valueChanged( int arg1 );
    void on_cellHeightSpinBox_valueChanged( int arg1 );
    void on_availableCameraList_draggingAboutToStart( ListDraggingStartEvent* event );
    void on_gridWidget_cellContextMenuRequested( int row, int column, const QPoint& globalPos );
    void on_gridWidget_dropTargetCheck( GridDropTargetCheckEvent* event );
    void on_gridWidget_itemDropped( GridItemDroppedEvent* event );
    void on_gridContextMenu_aboutToShow( );
    void on_mergeDownAndRightAction_triggered( );
    void on_mergeDownAction_triggered( );
    void on_mergeRightAction_triggered( );
    void on_splitAction_triggered( );
    void on_clearCellAction_triggered( );
    void on_viewNameEdit_textChanged( const QString& arg1 );
    void on_addViewButton_clicked( );
    void on_deleteViewButton_clicked( );
    void on_setDefaultViewButton_clicked( );

private:
    void AddViewsComboItem( const CamerasViewConfiguration& view, bool select = false );
    void UpdateViewButtonsStatus( );
    void UpdateDefaultViewStatus( );
    void ShowCurrentViewLayout( );

private:
    Ui::ConfigureCamerasViewPage*           ui;
    Private::ConfigureCamerasViewPageData*  mData;
};

#endif // CVS_CONFIGURE_CAMERAS_VIEW_PAGE_HPP
