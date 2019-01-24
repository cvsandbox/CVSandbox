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
#ifndef CVS_VIDEO_PROCESSING_WIZARD_PAGE_HPP
#define CVS_VIDEO_PROCESSING_WIZARD_PAGE_HPP

#include "WizardPageFrame.hpp"
#include <vector>
#include <map>
#include <XGuid.hpp>
#include <XVideoSourceProcessingGraph.hpp>

class QTreeWidgetItem;

namespace Ui
{
    class VideoProcessingWizardPage;
}

namespace Private
{
    class VideoProcessingWizardPageData;
}

class VideoProcessingWizardPage : public WizardPageFrame
{
    Q_OBJECT

    friend class ::Private::VideoProcessingWizardPageData;

public:
    explicit VideoProcessingWizardPage( QWidget* parent = 0 );
    ~VideoProcessingWizardPage( );

    // Set list of cameras to configure video processing graph for
    void SetCamerasToConfigure( const std::vector<CVSandbox::XGuid>& camerasList );

    // Get/Set video processing graphs for all cameras
    const std::map<CVSandbox::XGuid, CVSandbox::Automation::XVideoSourceProcessingGraph> GetCamerasProcessingGraphs( ) const;
    void SetCamerasProcessingGraphs( const std::map<CVSandbox::XGuid, CVSandbox::Automation::XVideoSourceProcessingGraph>& processingGraphs );

public:
    virtual bool CanGoNext( ) const;

private slots:
    void on_SelectedPluginChanged( );
    void on_camerasComboBox_currentIndexChanged( int index );
    void on_addPluginButton_clicked( );
    void on_removeButton_clicked( );
    void on_configureButton_clicked( );
    void on_renameStepAction_clicked( );
    void on_processinGraphTreeView_itemSelectionChanged( );
    void on_moveStepUpButton_clicked( );
    void on_moveStepDownButton_clicked( );
    void on_processinGraphTreeView_itemDoubleClicked( QTreeWidgetItem* item, int column );
    void on_GetPluginHelpAction_triggered( );
    void on_propertiesDialog_HelpRequested( QWidget* sender );
    void on_processinGraphTreeView_customContextMenuRequested( const QPoint& pos );
    void on_GetAddedPluginHelpAction_triggered( );
    void on_GraphPluginsContextMenu_aboutToShow( );

private:
    Ui::VideoProcessingWizardPage*          ui;
    Private::VideoProcessingWizardPageData* mData;
};

#endif // CVS_VIDEO_PROCESSING_WIZARD_PAGE_HPP
