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
#ifndef CVS_MAINWINDOW_HPP
#define CVS_MAINWINDOW_HPP

#include <memory>
#include <QMainWindow>
#include <IPersistentUIContent.hpp>
#include "IMainWindowService.hpp"

class ProjectObject;

namespace Ui
{
    class MainWindow;
}

namespace Private
{
    class MainWindowData;
}

class MainWindow : public QMainWindow, public IMainWindowService, public IPersistentUIContent
{
    Q_OBJECT
    Q_INTERFACES(IPersistentUIContent)

public:
    explicit MainWindow( QWidget *parent = 0 );
    ~MainWindow( );

public: // IMainWindowService implementation

    // Get main window of the application
    virtual QWidget* GetMainAppWindow( ) const;

    // Set status message displayed in the status bar of the main window
    virtual void SetStatusMessage( const QString& statusMessage ) const;

    // Set sub-title - name of open file/object/whatever
    virtual void SetSubTitle( const QString& subTitle );

    // Set widget to be display in the main area
    virtual void SetCentralWidget( QWidget* widget );

    // Get project object's ID for the central widget
    virtual const CVSandbox::XGuid CentralWidgetsObjectId( ) const;

    // Take snapshot of the specified video source
    virtual void TakeSnapshotForVideoSource( uint32_t videoSourceId, const QString& title );

public: // IPersistentUIContent implementation

    void SaveWidgetContent( IUIPersistenceService& persistanceService, const QString& groupName );
    void RestoreWidgetContent( IUIPersistenceService& persistanceService, const QString& groupName );

private:
    bool PrepareForApplicationExit( );
    void SetActionsSignalHandlers( );
    void SetupSandboxVariablesUi( );
    void SetupContextMenus( );
    void SetupStatuBar( );
    void HandleStartOptions( );

protected:
    virtual void closeEvent( QCloseEvent* event );

private slots:
    void on_actionExit_triggered( );
    void on_actionCloseObject_triggered( );
    void on_actionShowProjectView_triggered( );
    void on_actionSandboxVariablesMonitor_triggered( );
    void on_actionFullScreen_triggered( );
    void on_actionFitTheWindow_triggered( );
    void on_projectTreeDockWidget_visibilityChanged( bool visible );
    void on_sandboxVariablesDockWidget_visibilityChanged( bool visible );
    void on_menuProject_aboutToShow( );
    void on_actionAbout_triggered( );
    void on_performanceMonitor_update( );
    void on_ProjectObjectUpdated( const std::shared_ptr<ProjectObject>& po );
    void on_ProjectObjectDeleted( const std::shared_ptr<ProjectObject>& po );
    void on_actionScriptEditor_triggered( );
    void on_scriptEditor_finished( int );
    void on_scriptEditor_destroyed( QObject* );
    void on_snapshotDialog_finished( int );
    void on_snapshotDialog_destroyed( QObject* );

private:
    Ui::MainWindow*                               ui;
    const std::auto_ptr<Private::MainWindowData>  mData;
};

#endif // CVS_MAINWINDOW_HPP
