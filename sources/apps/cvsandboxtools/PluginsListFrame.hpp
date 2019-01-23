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
#ifndef CVS_PLUGINS_LIST_FRAME_HPP
#define CVS_PLUGINS_LIST_FRAME_HPP

#include <QFrame>
#include <QTreeWidgetItem>
#include <QMenu>
#include <XPluginsEngine.hpp>
#include "cvsandboxtools_global.h"

namespace Ui
{
    class PluginsListFrame;
}

// Frame to display list of available plug-ins of the certain type
class CVS_SHARED_EXPORT PluginsListFrame : public QFrame
{
    Q_OBJECT

public:
    explicit PluginsListFrame( QWidget* parent = 0 );
    ~PluginsListFrame( );

    // Set collection of modules modules to gather plug-ins from
    void SetModulesCollection( const std::shared_ptr<const XFamiliesCollection>& familiesCollection,
                               const std::shared_ptr<const XModulesCollection>& modulesCollection,
                               const std::shared_ptr<XPluginsCollection>& favouritePlugins,
                               PluginType typesMask );

    // Set context menu to display for plug-ins
    void SetPluginsContextMenu( QMenu* menu, QAction* addPluginToFavouritesAction = nullptr,
                                             QAction* removePluginFromFavouritesAction = nullptr );

    // Get ID of the currently selected plug-in
    const CVSandbox::XGuid GetSelectedPluginID( ) const;

private:
    // Perform clean-up
    void Cleanup( );
    // Show available modules
    void ShowModulesList( );
    // Update plug-ins list after module selection was done
    void UpdatePluginsList( );

    // Create tree item for the family with specified ID
    QTreeWidgetItem* CtreateFamilyItem( const std::shared_ptr<const XFamily>& familyID );
    // Get collection of plug-ins to show based on currently selected module
    const std::shared_ptr<const XPluginsCollection> GetPluginsToShow( ) const;

    // Family was selected - show its description
    void OnFamilySelected( const CVSandbox::XGuid& id );
    // Plug-in was selected - show its description
    void OnPluginSelected( const CVSandbox::XGuid& id );

signals:
    // Selected item has changed in the plug-ins tree
    void SelectedPluginChanged( );

protected:
    void focusInEvent( QFocusEvent* event );

private slots:
    void on_modulesCombo_currentIndexChanged( int );
    void on_pluginsTree_itemSelectionChanged( );
    void on_pluginsTree_customContextMenuRequested( const QPoint &pos );
    void on_pluginsTree_itemDoubleClicked( QTreeWidgetItem *item, int column );
    void on_PluginsContextMenu_aboutToShow( );
    void on_AddPluginToFavouritesAction_triggered( );
    void on_RemovePluginFromFavouritesAction_triggered( );

private:
    Ui::PluginsListFrame* ui;

    std::shared_ptr<const XFamiliesCollection>  mFamiliesCollection;
    std::shared_ptr<const XModulesCollection>   mModulesCollection;
    std::shared_ptr<XPluginsCollection>         mFavouritePlugins;
    PluginType                                  mPluginTypesMask;
    QMenu*                                      mPluginsContextMenu;
    QAction*                                    mAddPluginToFavouritesAction;
    QAction*                                    mRemovePluginFromFavouritesAction;

    int                                         mSelectedComboIndex;
    const QTreeWidgetItem*                      mSelectedTreeItem;
};

#endif // CVS_PLUGINS_LIST_FRAME_HPP
