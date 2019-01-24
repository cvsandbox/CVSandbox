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
#ifndef CVS_PROJECTTREEFRAME_HPP
#define CVS_PROJECTTREEFRAME_HPP

#include <memory>
#include <QFrame>
#include <QMenu>
#include <QTreeWidgetItem>
#include "IProjectTreeUI.hpp"
#include "DraggableTreeWidget.hpp"

namespace Ui
{
    class ProjectTreeFrame;
}

namespace Private
{
    class ProjectTreeFrameData;
}

// UI frame showing tree of available cameras and their groups
class ProjectTreeFrame : public QFrame, public IProjectTreeUI
{
    Q_OBJECT
    
public:
    explicit ProjectTreeFrame( QWidget *parent = 0 );
    ~ProjectTreeFrame( );
    
    // Set default context menu for items which don't have their own menu
    void SetDefaultContextMenu( QMenu* menu );
    // Set context menu to display for project objects of the specified type
    void SetProjectObjectContextMenu( ProjectObjectType poType, QMenu* menu );

    // Sets if newly added items should be automatically selected or
    void SetSelectionOfNewItems( bool set );
    // Sets if drag and drop is allowed inside the control
    void SetAllowDragAndDrop( bool set );
    // Sets if default action should be triggered on double click (taken from item's context menu)
    void SetTriggerDefaultActionOnDoubleClick( bool set );
    // Sets if project objects' description should be visible or not
    void ShowProjectObjectsDescription( bool show );

    // Expand or collapse root item
    void ExpandRootItem( bool expand );
    // Set selection on the root item
    void SelectRootItem( );
    // Highlight item with the specified ID
    void SetItemHighlight( const CVSandbox::XGuid& objectId, bool highlight );

public: // IProjectTreeUI interface implementation

    // Check if root item is selected
    bool IsRootSelected( ) const;

    // Get ID of the selected item
    CVSandbox::XGuid GetSelectedItem( ) const;

    // Get ID of the selected folder or the folder containing selected item
    CVSandbox::XGuid GetSelectedOrParentFolder( ) const;

    // Set selected item ID
    virtual void SetSelectedItem( const CVSandbox::XGuid& id );

    // Get type of the selected item
    ProjectObjectType GetSelectedItemType( ) const;

public slots:
    void AddObject( const std::shared_ptr<ProjectObject>& po );
    void UpdateObject( const std::shared_ptr<ProjectObject>& po );
    void DeleteObject( const std::shared_ptr<ProjectObject>& po );

signals:
    void ObjectSelected( const CVSandbox::XGuid& objectId );
    void ObjectDoubleClicked( const CVSandbox::XGuid& objectId );

private slots:
    void on_projectTreeWidget_customContextMenuRequested( const QPoint& pos );
    void on_projectTreeWidget_itemSelectionChanged( );
    void on_projectTreeWidget_draggingAboutToStart( TreeDraggingStartEvent* event );
    void on_projectTreeWidget_dropTargetCheck( TreeDropTargetCheckEvent* event );
    void on_projectTreeWidget_itemDropped( TreeItemDroppedEvent* event );
    void on_projectTreeWidget_itemDoubleClicked( QTreeWidgetItem* item, int column );

protected:
    // Handle focus event
    virtual void focusInEvent( QFocusEvent* event );

private:
    static ProjectObjectType GetProjectObjectType( const CVSandbox::XGuid& id );
    void CreateBlankProjectTree( );
    void TriggerItemsDefaultAction( QTreeWidgetItem* item );

private:
    Ui::ProjectTreeFrame*           ui;
    Private::ProjectTreeFrameData*  mData;
};

#endif // CVS_PROJECTTREEFRAME_HPP
