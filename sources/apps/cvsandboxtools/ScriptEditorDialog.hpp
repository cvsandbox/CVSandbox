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
#ifndef CVS_SCRIPT_EDITOR_DIALOG_HPP
#define CVS_SCRIPT_EDITOR_DIALOG_HPP

#include <QDialog>
#include <XGuid.hpp>
#include "cvsandboxtools_global.h"

class XScriptingEnginePlugin;

namespace Ui
{
    class ScriptEditorDialog;
}

namespace Private
{
    class ScriptEditorDialogData;
}

class CVS_SHARED_EXPORT ScriptEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ScriptEditorDialog( QWidget* parent = nullptr, bool allowMinimize = false,
                                 QWidget* widgetToActivateOnClose = nullptr );
    ~ScriptEditorDialog( );

    void SetDefaultExtension( const QString& ext );
    void SetScriptingName( const QString& scriptingName );
    void SetScriptingPluging( const CVSandbox::XGuid& scriptingPluginId );

    void LoadScript( const QString& fileName );
    const QString LastOpenFile( ) const;

protected:
    void closeEvent( QCloseEvent* event );
    bool event( QEvent* e );

public slots:
    virtual void accept( );
    virtual void reject( );

private slots:
    void on_RecentFilesMenu_aboutToShow( );

    void on_openRecentFile0_triggered( );
    void on_openRecentFile1_triggered( );
    void on_openRecentFile2_triggered( );
    void on_openRecentFile3_triggered( );
    void on_openRecentFile4_triggered( );
    void on_openRecentFile5_triggered( );
    void on_openRecentFile6_triggered( );
    void on_openRecentFile7_triggered( );
    void on_openRecentFile8_triggered( );
    void on_openRecentFile9_triggered( );

    void on_SaveFileMenuAction_triggered( );
    void on_SaveAsFileMenuAction_triggered( );
    void on_NewFileMenuAction_triggered( );
    void on_OpenFileMenuAction_triggered( );

    void on_UndoEditAction_triggered( );
    void on_RedoEditAction_triggered( );
    void on_CutEditAction_triggered( );
    void on_CopyEditAction_triggered( );
    void on_PasteEditAction_triggered( );
    void on_SelectAllEditAction_triggered( );
    void on_PluginDescriptionAction_triggered( );

    void on_StyleToken1Action_triggered( );
    void on_StyleToken2Action_triggered( );
    void on_StyleToken3Action_triggered( );
    void on_StyleToken4Action_triggered( );
    void on_StyleToken5Action_triggered( );

    void on_RemoveStyle1Action_triggered( );
    void on_RemoveStyle2Action_triggered( );
    void on_RemoveStyle3Action_triggered( );
    void on_RemoveStyle4Action_triggered( );
    void on_RemoveStyle5Action_triggered( );
    void on_RemoveStylesAllAction_triggered( );

    void on_FindTextAction_triggered( );
    void on_FindNextAction_triggered( );
    void on_GoToLineAction_triggered( );

    void on_VerifyScriptAction_triggered( );
    void on_AboutScriptingPluginAction_triggered( );
    void on_ModuleListAction_triggered( );

    void on_MenuEdit_aboutToShow( );
    void on_MenuSearch_aboutToShow( );
    void on_MenuScript_aboutToShow( );

    void on_savePointChanged( bool dirty );
    void on_textAreaClicked( int line, int modifiers );
    void on_keyPressed( QKeyEvent* event );

    void on_editor_customContextMenuRequested( const QPoint &pos );

    void on_FindDialog_finished( int );
    void on_FindDialog_destroyed( QObject* );
    void on_FindDialog_findRequest( const QString& toFind, bool matchCase, bool wholeWord );

private:
    Ui::ScriptEditorDialog*          ui;
    Private::ScriptEditorDialogData* mData;
};

#endif // CVS_SCRIPT_EDITOR_DIALOG_HPP
