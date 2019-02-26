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

#include "ScriptEditorDialog.hpp"
#include "ui_ScriptEditorDialog.h"
#include "GlobalServiceManager.hpp"
#include "UITools.hpp"
#include "FindTextDialog.hpp"
#include "GoToLineDialog.hpp"
#include "GlobalServiceManager.hpp"

#include <xtypes.h>
#include <XScriptingEnginePlugin.hpp>

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QStatusTipEvent>
#include <QLabel>
#include <QMessageBox>

#include <QFileDialog>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>

#include <ScintillaEdit.h>
#include <Scintilla.h>
#include <SciLexer.h>

using namespace std;
using namespace CVSandbox;

namespace Private
{
    #define EDITOR_RGB(r, g, b) (((b) << 16) + ((g) << 8) + (r))

    #define EDITOR_BG_COLOR  EDITOR_RGB( 245, 245, 245 )

    #define EDITOR_BG_COLOR1 EDITOR_RGB( 230, 230, 255 )
    #define EDITOR_BG_COLOR2 EDITOR_RGB( 200, 255, 200 )
    #define EDITOR_BG_COLOR3 EDITOR_RGB( 255, 230, 230 )
    #define EDITOR_BG_COLOR4 EDITOR_RGB( 220, 180, 220 )
    #define EDITOR_BG_COLOR5 EDITOR_RGB( 220, 220, 180 )

    static const QString STR_RECENT_FILES ( "ScriptEditorRecentFiles" );
    static const QString STR_SCRIPT_EDITOR( "ScriptEditor" );
    static const QString STR_LAST_FOLDER  ( "LastFolder" );

    class ScriptEditorDialogData
    {
    public:
        ScriptEditorDialogData( ScriptEditorDialog* parent, Ui::ScriptEditorDialog* ui, QWidget* widgetToActivateOnClose ) :
            DefaultExtension( ), ScriptingName( "Scripting" ), ScriptingEngine( nullptr ), PluginForHelp( nullptr ),
            FileName( ), FilePath( ), LastOpenFilePath( ), IsDirty( false ), AboutToShowContextMenu( false ),
            WidgetToActivateOnClose( widgetToActivateOnClose ), Parent( parent ), Ui( ui ),
            Editor( nullptr ), StatusBar( nullptr ),
            RecentFilesMenu( nullptr ), MenuEdit( nullptr ), MenuScript( nullptr ),
            NewFileMenuAction( nullptr ), OpenFileMenuAction( nullptr ),
            SaveFileMenuAction( nullptr ), SaveAsFileMenuAction( nullptr ),
            CloseMenuAction( nullptr ),
            UndoEditAction( nullptr ), RedoEditAction( nullptr ), CutEditAction( nullptr ),
            CopyEditAction( nullptr ), PasteEditAction( nullptr ), SelectAllEditAction( nullptr ),
            VerifyScriptAction( nullptr ),
            AboutScriptingPluginAction( nullptr ), ModuleListAction( nullptr ),
            CursorLabel( nullptr ),
            FindDialog( nullptr ), FirstSearch( true ), LastSearchString( )
        {
            LoadRecentFiles( );
        }

        void InitUi( )
        {
            Editor = Ui->editor;

            // add menu and status bars
            Parent->layout( )->setMenuBar( CreateMenuBar( ) );
            Parent->layout( )->addWidget( CreateStatusBar( ) );

            InitEditor( );
        }

        void SetDirtyFlag( bool isDirty );
        void UpdateEditorTitle( );
        void UpdateCursorPosition( );

        void NewFile( );
        void OpenScript( );

        bool LoadScript( const QString& fileName, bool silent = false );
        bool SaveScript( bool saveAs = false );
        bool SaveIfNeeded( bool silentSave = false );
        void AddToRecentFiles( const QString& fileName );
        void OpenRecentFile( int fileNumber );

        void StyleToken( int style );
        void RemoveStyle( int style );

        const shared_ptr<const XPluginDescriptor> GetPluginDescriptorFromCursor( );
        const shared_ptr<const XPluginDescriptor> GetPluginDescriptorFromCursor( const QPoint &pos );
        const shared_ptr<const XPluginDescriptor> GetPluginDescriptorFromCursor( sptr_t editorPos );

        void FindString( const QString& toFind, bool matchCase, bool wholeWord );

    private:

        void InitEditor( );
        void SetLuaSettings( );
        QMenuBar* CreateMenuBar( );
        QStatusBar* CreateStatusBar( );
        void SetFileName( const QString& fileName );

        void InitFileDialog( QFileDialog* dialog );
        void SaveRecentFiles( );
        void LoadRecentFiles( );

    public:
        // only UI thread is supposed to access it, so not doing any synchronization for it
        static vector<QString> RecentFiles;

        QString                             DefaultExtension;
        QString                             ScriptingName;
        XGuid                               ScriptingPluginId;
        shared_ptr<XScriptingEnginePlugin>  ScriptingEngine;
        shared_ptr<const XPluginDescriptor> PluginForHelp;

        QString                 FileName;
        QString                 FilePath;
        QString                 LastOpenFilePath;
        bool                    IsDirty;
        bool                    AboutToShowContextMenu;

        QWidget*                WidgetToActivateOnClose;
        ScriptEditorDialog*     Parent;
        Ui::ScriptEditorDialog* Ui;
        ScintillaEdit*          Editor;

        QStatusBar*         StatusBar;
        QMenu*              RecentFilesMenu;
        QMenu*              MenuEdit;
        QMenu*              MenuStyleToken;
        QMenu*              MenuRemoveStyle;
        QMenu*              MenuSearch;
        QMenu*              MenuScript;
        QMenu*              MenuHelp;

        QAction*            NewFileMenuAction;
        QAction*            OpenFileMenuAction;
        QAction*            SaveFileMenuAction;
        QAction*            SaveAsFileMenuAction;
        QAction*            CloseMenuAction;

        QAction*            UndoEditAction;
        QAction*            RedoEditAction;
        QAction*            CutEditAction;
        QAction*            CopyEditAction;
        QAction*            PasteEditAction;
        QAction*            SelectAllEditAction;
        QAction*            PluginDescriptionSeparator;
        QAction*            PluginDescriptionAction;

        QAction*            FindTextAction;
        QAction*            FindNextAction;
        QAction*            GoToLineAction;

        QAction*            VerifyScriptAction;

        QAction*            AboutScriptingPluginAction;
        QAction*            ModuleListAction;

        QAction*            StyleToken1Action;
        QAction*            StyleToken2Action;
        QAction*            StyleToken3Action;
        QAction*            StyleToken4Action;
        QAction*            StyleToken5Action;

        QAction*            RemoveStyle1Action;
        QAction*            RemoveStyle2Action;
        QAction*            RemoveStyle3Action;
        QAction*            RemoveStyle4Action;
        QAction*            RemoveStyle5Action;
        QAction*            RemoveStylesAllAction;

        QLabel*             CursorLabel;

        QString             HighlightedWords[5];

        FindTextDialog*     FindDialog;
        bool                FirstSearch;
        QString             LastSearchString;
        bool                LastSearchMatchCase;
        bool                LastSearchWholeWord;
    };

    vector<QString> ScriptEditorDialogData::RecentFiles;
}

ScriptEditorDialog::ScriptEditorDialog( QWidget* parent, bool allowMinimize, QWidget* widgetToActivateOnClose ) :
    QDialog( parent ),
    ui( new Ui::ScriptEditorDialog ),
    mData( new Private::ScriptEditorDialogData( this, ui, widgetToActivateOnClose ) )
{
    ui->setupUi( this );
    mData->InitUi( );

    // disable "?" button on title bar
    setWindowFlags( windowFlags( ) & ~Qt::WindowContextHelpButtonHint );

    // restore size/position
    GlobalServiceManager::Instance( ).GetUiPersistenceService( )->RestoreWidget( this );

    // connect signals to slots
    connect( mData->RecentFilesMenu, SIGNAL( aboutToShow() ), this, SLOT( on_RecentFilesMenu_aboutToShow() ) );

    connect( mData->SaveFileMenuAction, SIGNAL( triggered() ), this, SLOT( on_SaveFileMenuAction_triggered() ) );
    connect( mData->SaveAsFileMenuAction, SIGNAL( triggered() ), this, SLOT( on_SaveAsFileMenuAction_triggered() ) );
    connect( mData->NewFileMenuAction, SIGNAL( triggered() ), this, SLOT( on_NewFileMenuAction_triggered() ) );
    connect( mData->OpenFileMenuAction, SIGNAL( triggered() ), this, SLOT( on_OpenFileMenuAction_triggered() ) );
    connect( mData->CloseMenuAction, SIGNAL( triggered() ), this, SLOT( close() ) );

    connect( mData->UndoEditAction, SIGNAL( triggered() ), this, SLOT( on_UndoEditAction_triggered() ) );
    connect( mData->RedoEditAction, SIGNAL( triggered() ), this, SLOT( on_RedoEditAction_triggered() ) );
    connect( mData->CutEditAction, SIGNAL( triggered() ), this, SLOT( on_CutEditAction_triggered() ) );
    connect( mData->CopyEditAction, SIGNAL( triggered() ), this, SLOT( on_CopyEditAction_triggered() ) );
    connect( mData->PasteEditAction, SIGNAL( triggered() ), this, SLOT( on_PasteEditAction_triggered() ) );
    connect( mData->SelectAllEditAction, SIGNAL( triggered() ), this, SLOT( on_SelectAllEditAction_triggered() ) );
    connect( mData->PluginDescriptionAction, SIGNAL( triggered() ), this, SLOT( on_PluginDescriptionAction_triggered() ) );
    connect( mData->MenuEdit, SIGNAL( aboutToShow() ), this, SLOT( on_MenuEdit_aboutToShow() ) );

    connect( mData->StyleToken1Action, SIGNAL( triggered() ), this, SLOT( on_StyleToken1Action_triggered() ) );
    connect( mData->StyleToken2Action, SIGNAL( triggered() ), this, SLOT( on_StyleToken2Action_triggered() ) );
    connect( mData->StyleToken3Action, SIGNAL( triggered() ), this, SLOT( on_StyleToken3Action_triggered() ) );
    connect( mData->StyleToken4Action, SIGNAL( triggered() ), this, SLOT( on_StyleToken4Action_triggered() ) );
    connect( mData->StyleToken5Action, SIGNAL( triggered() ), this, SLOT( on_StyleToken5Action_triggered() ) );

    connect( mData->RemoveStyle1Action, SIGNAL( triggered() ), this, SLOT( on_RemoveStyle1Action_triggered() ) );
    connect( mData->RemoveStyle2Action, SIGNAL( triggered() ), this, SLOT( on_RemoveStyle2Action_triggered() ) );
    connect( mData->RemoveStyle3Action, SIGNAL( triggered() ), this, SLOT( on_RemoveStyle3Action_triggered() ) );
    connect( mData->RemoveStyle4Action, SIGNAL( triggered() ), this, SLOT( on_RemoveStyle4Action_triggered() ) );
    connect( mData->RemoveStyle5Action, SIGNAL( triggered() ), this, SLOT( on_RemoveStyle5Action_triggered() ) );
    connect( mData->RemoveStylesAllAction, SIGNAL( triggered() ), this, SLOT( on_RemoveStylesAllAction_triggered() ) );

    connect( mData->FindTextAction, SIGNAL( triggered() ), this, SLOT( on_FindTextAction_triggered() ) );
    connect( mData->FindNextAction, SIGNAL( triggered() ), this, SLOT( on_FindNextAction_triggered() ) );
    connect( mData->GoToLineAction, SIGNAL( triggered() ), this, SLOT( on_GoToLineAction_triggered() ) );
    connect( mData->MenuSearch, SIGNAL( aboutToShow() ), this, SLOT( on_MenuSearch_aboutToShow() ) );

    connect( mData->MenuScript, SIGNAL( aboutToShow() ), this, SLOT( on_MenuScript_aboutToShow() ) );
    connect( mData->VerifyScriptAction, SIGNAL( triggered() ), this, SLOT( on_VerifyScriptAction_triggered() ) );

    connect( mData->AboutScriptingPluginAction, SIGNAL( triggered() ), this, SLOT( on_AboutScriptingPluginAction_triggered() ) );
    connect( mData->ModuleListAction, SIGNAL( triggered() ), this, SLOT( on_ModuleListAction_triggered() ) );

    connect( mData->Editor, SIGNAL(textAreaClicked(int,int)), this, SLOT(on_textAreaClicked(int,int)) );
    connect( mData->Editor, SIGNAL(savePointChanged(bool)), this, SLOT(on_savePointChanged(bool)) );
    connect( mData->Editor, SIGNAL(keyPressed(QKeyEvent*)), this, SLOT(on_keyPressed(QKeyEvent*)) );

    // set-up custom context menu for the editor
    mData->Editor->setContextMenuPolicy( Qt::CustomContextMenu );
    connect( mData->Editor, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(on_editor_customContextMenuRequested(QPoint)) );

    if ( allowMinimize )
    {
        setWindowFlags( windowFlags( ) | Qt::WindowMinimizeButtonHint );
    }

    mData->NewFile( );
}

ScriptEditorDialog::~ScriptEditorDialog( )
{
    delete mData;
    delete ui;
}

// Widget is closed
void ScriptEditorDialog::closeEvent( QCloseEvent* closeEvet )
{
    if ( !mData->SaveIfNeeded( ) )
    {
        closeEvet->ignore( );
    }
    else
    {
        GlobalServiceManager::Instance( ).GetHelpService( )->HidePluginDescription( this );
        GlobalServiceManager::Instance( ).GetUiPersistenceService( )->SaveWidget( this );

        mData->NewFile( );

        mData->LastSearchString.clear( );

        if ( mData->FindDialog != nullptr )
        {
            delete mData->FindDialog;
            mData->FindDialog = nullptr;
        }

        if ( mData->WidgetToActivateOnClose != nullptr )
        {
            if ( mData->WidgetToActivateOnClose->isMinimized( ) )
            {
                mData->WidgetToActivateOnClose->showNormal( );
            }

            mData->WidgetToActivateOnClose->show( );
            mData->WidgetToActivateOnClose->activateWindow( );
        }
    }
}

// Dialog is accepted
void ScriptEditorDialog::accept( )
{
    GlobalServiceManager::Instance( ).GetUiPersistenceService( )->SaveWidget( this );
    QDialog::accept( );
}

// Dialog is rejected
void ScriptEditorDialog::reject( )
{
    GlobalServiceManager::Instance( ).GetUiPersistenceService( )->SaveWidget( this );
    QDialog::reject( );
}

// Check if there is anything to show in status bar
bool ScriptEditorDialog::event( QEvent* e )
{
    bool ret = true;

    if ( e->type( ) == QEvent::StatusTip )
    {
        QStatusTipEvent* ev = static_cast<QStatusTipEvent*>( e );
        mData->StatusBar->showMessage( ev->tip( ) );
    }
    else
    {
        ret = QDialog::event( e );
    }

    return ret;
}

// Set default extension of the script files to edit
void ScriptEditorDialog::SetDefaultExtension( const QString& ext )
{
    mData->DefaultExtension = ext;
}

// Set name of the scripting engine executing the scripts we are editing
void ScriptEditorDialog::SetScriptingName( const QString& scriptingName )
{
    mData->ScriptingName = scriptingName;
    mData->UpdateEditorTitle( );
}

// Set scripting engine
void ScriptEditorDialog::SetScriptingPluging( const XGuid& scriptingPluginId )
{
    mData->ScriptingPluginId = XGuid( );
    mData->ScriptingEngine.reset( );

    if ( !scriptingPluginId.IsEmpty( ) )
    {
        auto pluginDesc = GlobalServiceManager::Instance( ).GetPluginsEngine( )->GetPlugin( scriptingPluginId );

        if ( pluginDesc )
        {
            shared_ptr<XPlugin> plugin = pluginDesc->CreateInstance( );

            if ( plugin )
            {
                mData->ScriptingPluginId = scriptingPluginId;
                mData->ScriptingEngine   = static_pointer_cast<XScriptingEnginePlugin>( plugin );

                mData->ScriptingEngine->Init( );
            }
        }
    }

    mData->MenuScript->menuAction( )->setVisible( mData->ScriptingEngine != nullptr );
    mData->AboutScriptingPluginAction->setEnabled( !mData->ScriptingPluginId.IsEmpty( ) );
}

// Load script from the specified file
void ScriptEditorDialog::LoadScript( const QString& fileName )
{
    mData->LoadScript( fileName );
}

// Get the name of the last opened file
const QString ScriptEditorDialog::LastOpenFile( ) const
{
    return mData->LastOpenFilePath;
}

// Recent file menu is about to open
void ScriptEditorDialog::on_RecentFilesMenu_aboutToShow( )
{
    int counter = 0;

    mData->RecentFilesMenu->clear( );

    for ( auto it = mData->RecentFiles.begin( ); ( it != mData->RecentFiles.end( ) ) && ( counter < 10 ); ++it, ++counter )
    {
        QAction* recentFileAction = new QAction( QString( "&%0: " ).arg( ( counter + 1 ) % 10 ) + *it,
                                                 mData->RecentFilesMenu);
        recentFileAction->setStatusTip( QString( "Open the recent file #%0" ).arg( ( counter + 1 ) % 10 ) );

        mData->RecentFilesMenu->addAction( recentFileAction );

        connect( recentFileAction, SIGNAL( triggered() ), this,
                 QString( "1on_openRecentFile%0_triggered()" ).arg( counter ).toStdString( ).c_str( ) );
    }
}

// Open one of the recent files
void ScriptEditorDialog::on_openRecentFile0_triggered( )
{
    mData->OpenRecentFile( 0 );
}
void ScriptEditorDialog::on_openRecentFile1_triggered( )
{
    mData->OpenRecentFile( 1 );
}
void ScriptEditorDialog::on_openRecentFile2_triggered( )
{
    mData->OpenRecentFile( 2 );
}
void ScriptEditorDialog::on_openRecentFile3_triggered( )
{
    mData->OpenRecentFile( 3 );
}
void ScriptEditorDialog::on_openRecentFile4_triggered( )
{
    mData->OpenRecentFile( 4 );
}
void ScriptEditorDialog::on_openRecentFile5_triggered( )
{
    mData->OpenRecentFile( 5 );
}
void ScriptEditorDialog::on_openRecentFile6_triggered( )
{
    mData->OpenRecentFile( 6 );
}
void ScriptEditorDialog::on_openRecentFile7_triggered( )
{
    mData->OpenRecentFile( 7 );
}
void ScriptEditorDialog::on_openRecentFile8_triggered( )
{
    mData->OpenRecentFile( 8 );
}
void ScriptEditorDialog::on_openRecentFile9_triggered( )
{
    mData->OpenRecentFile( 9 );
}

// Save current file
void ScriptEditorDialog::on_SaveFileMenuAction_triggered( )
{
    mData->SaveScript( );
}

// Save current file using different name
void ScriptEditorDialog::on_SaveAsFileMenuAction_triggered( )
{
    mData->SaveScript( true );
}

// Start new file
void ScriptEditorDialog::on_NewFileMenuAction_triggered( )
{
    if ( mData->SaveIfNeeded( ) )
    {
        mData->NewFile( );
    }
}

// Show file open dialog and open a file then
void ScriptEditorDialog::on_OpenFileMenuAction_triggered( )
{
    if ( mData->SaveIfNeeded( ) )
    {
        mData->OpenScript( );
    }
}

// Undo recent change
void ScriptEditorDialog::on_UndoEditAction_triggered( )
{
    mData->Editor->undo( );
}

// Redo recently undone change
void ScriptEditorDialog::on_RedoEditAction_triggered( )
{
    mData->Editor->redo( );
}

// Cut current selection putting it to clipboard
void ScriptEditorDialog::on_CutEditAction_triggered( )
{
    mData->Editor->cut( );
}

// Copy current selection into clipboard
void ScriptEditorDialog::on_CopyEditAction_triggered( )
{
    mData->Editor->copy( );
}

// Paste from clipboard
void ScriptEditorDialog::on_PasteEditAction_triggered( )
{
    mData->Editor->paste( );
}

// Select all text available in the editor
void ScriptEditorDialog::on_SelectAllEditAction_triggered( )
{
    mData->Editor->selectAll( );
}

// Show help for the plug-in name inder cursor
void ScriptEditorDialog::on_PluginDescriptionAction_triggered( )
{
   shared_ptr<const XPluginDescriptor> pluginDesc = mData->PluginForHelp;

   if ( !pluginDesc )
   {
       // somehow we got here without plug-in descriptor, which must be a hot key
       // (the action is not visible otherwise, so can not be invoked from UI)
       pluginDesc = mData->GetPluginDescriptorFromCursor( );
   }
   else
   {
       mData->PluginForHelp.reset( );
   }

   if ( pluginDesc )
   {
       GlobalServiceManager::Instance( ).GetHelpService( )->ShowPluginDescription( this, pluginDesc->ID( ) );
   }
}

// Set style for a token (identifier)
void ScriptEditorDialog::on_StyleToken1Action_triggered( )
{
    mData->StyleToken( 0 );
}
void ScriptEditorDialog::on_StyleToken2Action_triggered( )
{
    mData->StyleToken( 1 );
}
void ScriptEditorDialog::on_StyleToken3Action_triggered( )
{
    mData->StyleToken( 2 );
}
void ScriptEditorDialog::on_StyleToken4Action_triggered( )
{
    mData->StyleToken( 3 );
}
void ScriptEditorDialog::on_StyleToken5Action_triggered( )
{
    mData->StyleToken( 4 );
}

// Clear token's style
void ScriptEditorDialog::on_RemoveStyle1Action_triggered( )
{
    mData->RemoveStyle( 0 );
}
void ScriptEditorDialog::on_RemoveStyle2Action_triggered( )
{
    mData->RemoveStyle( 1 );
}
void ScriptEditorDialog::on_RemoveStyle3Action_triggered( )
{
    mData->RemoveStyle( 2 );
}
void ScriptEditorDialog::on_RemoveStyle4Action_triggered( )
{
    mData->RemoveStyle( 3 );
}
void ScriptEditorDialog::on_RemoveStyle5Action_triggered( )
{
    mData->RemoveStyle( 4 );
}
void ScriptEditorDialog::on_RemoveStylesAllAction_triggered( )
{
    mData->RemoveStyle( -1 );
}

// Find text string in the currently open file
void ScriptEditorDialog::on_FindTextAction_triggered( )
{
    if ( mData->FindDialog == nullptr )
    {
        mData->FindDialog = new FindTextDialog( this );

        connect( mData->FindDialog, SIGNAL(finished(int)), this, SLOT(on_FindDialog_finished(int)) );
        connect( mData->FindDialog, SIGNAL(destroyed(QObject*)), this, SLOT(on_FindDialog_destroyed(QObject*)) );
        connect( mData->FindDialog, SIGNAL(findRequest( const QString&, bool, bool)),
                 this, SLOT(on_FindDialog_findRequest(const QString&, bool, bool)) );
    }

    mData->FindDialog->show( );
    mData->FindDialog->raise( );
    mData->FindDialog->activateWindow( );

    mData->FirstSearch = true;
}

void ScriptEditorDialog::on_FindDialog_finished( int )
{
    mData->FindDialog = nullptr;
}

void ScriptEditorDialog::on_FindDialog_destroyed( QObject* )
{
    mData->FindDialog = nullptr;
}

void ScriptEditorDialog::on_FindDialog_findRequest( const QString& toFind, bool matchCase, bool wholeWord )
{
    mData->FindString( toFind, matchCase, wholeWord );
}

// Find next occurrence of the last searched string
void ScriptEditorDialog::on_FindNextAction_triggered( )
{
    if ( !mData->LastSearchString.isEmpty( ) )
    {
        mData->FindString( mData->LastSearchString, mData->LastSearchMatchCase, mData->LastSearchWholeWord );
    }
}

// Perform search of the given string
void Private::ScriptEditorDialogData::FindString( const QString& toFind, bool matchCase, bool wholeWord )
{
    if ( !toFind.trimmed( ).isEmpty( ) )
    {
        sptr_t searchFlags = 0;
        sptr_t editorPos   = Editor->currentPos( );
        sptr_t targetStart = Editor->wordStartPosition( editorPos, true );
        sptr_t targetEnd   = Editor->textLength( );
        sptr_t selStart    = Editor->selectionStart( );
        sptr_t selEnd      = Editor->selectionEnd( );

        QByteArray utfToFind = toFind.toUtf8( );
        char*      strToFind = utfToFind.data( );
        size_t     searchLen = strlen( strToFind );

        if ( matchCase ) searchFlags |= SCFIND_MATCHCASE;
        if ( wholeWord ) searchFlags |= SCFIND_WHOLEWORD;

        // check if there is selection of the search string (can be left by previous search)
        if ( ( selEnd - selStart == toFind.length( ) ) && ( !FirstSearch ) &&
             ( toFind == LastSearchString ) )
        {
            targetStart = selEnd;
        }

        Editor->setSearchFlags( searchFlags );
        Editor->setTargetRange( targetStart, targetEnd );

        sptr_t foundAt = Editor->searchInTarget( searchLen, strToFind );

        if ( ( foundAt == -1 ) && ( targetStart != 0 ) )
        {
            // repeat search from start
            Editor->setTargetRange( 9, targetEnd );
            foundAt = Editor->searchInTarget( searchLen, strToFind );
        }

        if ( foundAt != -1 )
        {
            Editor->gotoPos( foundAt );
            Editor->setSelectionStart( foundAt );
            Editor->setSelectionEnd( foundAt + toFind.length( ) );

            FirstSearch = false;

            StatusBar->showMessage( "" );
            UpdateCursorPosition( );
        }
        else
        {
            StatusBar->showMessage( QString( "Search string not found: " ) + toFind );
        }

        LastSearchString    = toFind;
        LastSearchMatchCase = matchCase;
        LastSearchWholeWord = wholeWord;
    }
}

// Navigate to the specified line number
void ScriptEditorDialog::on_GoToLineAction_triggered( )
{
    GoToLineDialog goToDialog( mData->Editor->lineCount( ),
                               mData->Editor->lineFromPosition( mData->Editor->currentPos( ) + 1 ),
                               this );

    if ( goToDialog.exec( ) == QDialog::Accepted )
    {
        mData->Editor->gotoLine( goToDialog.LineNumber( ) - 1 );
        mData->UpdateCursorPosition( );
    }
}

// Verify the script compiles
void ScriptEditorDialog::on_VerifyScriptAction_triggered( )
{
    if ( mData->SaveIfNeeded( true ) )
    {
        if ( mData->ScriptingEngine )
        {
            mData->ScriptingEngine->SetScriptFile( mData->FilePath.toUtf8( ).data( ) );
            if ( mData->ScriptingEngine->LoadScript( ) == SuccessCode )
            {
                mData->StatusBar->showMessage( "The script compiles successfully." );
            }
            else
            {
                UITools::ShowErrorMessage( QString::fromUtf8( mData->ScriptingEngine->GetLastErrorMessage( ).c_str( ) ), this );
            }
        }
    }
}

// Show help for the scripting plug-in used for the script
void ScriptEditorDialog::on_AboutScriptingPluginAction_triggered( )
{
    if ( !mData->ScriptingPluginId.IsEmpty( ) )
    {
        GlobalServiceManager::Instance( ).GetHelpService( )->ShowPluginDescription( this, mData->ScriptingPluginId );
    }
}

// Show list of available modules
void ScriptEditorDialog::on_ModuleListAction_triggered( )
{
    GlobalServiceManager::Instance( ).GetHelpService( )->ShowPluginDescription( this, XGuid( ) );
}

// Edit menu is about to show - update its items
void ScriptEditorDialog::on_MenuEdit_aboutToShow( )
{
    bool showPluginHelpAction = ( ( mData->AboutToShowContextMenu ) &&
                                  ( mData->PluginForHelp ) );

    mData->UndoEditAction->setEnabled( mData->Editor->canUndo( ) );
    mData->RedoEditAction->setEnabled( mData->Editor->canRedo( ) );
    mData->PasteEditAction->setEnabled( mData->Editor->canPaste( ) );

    mData->CutEditAction->setEnabled( !mData->Editor->selectionEmpty( ) );
    mData->CopyEditAction->setEnabled( !mData->Editor->selectionEmpty( ) );

    mData->PluginDescriptionSeparator->setVisible( showPluginHelpAction );
    mData->PluginDescriptionAction->setVisible( showPluginHelpAction );
}

// Search menu is about to show
void ScriptEditorDialog::on_MenuSearch_aboutToShow( )
{
    mData->FindNextAction->setEnabled( !mData->LastSearchString.isEmpty( ) );
}

// Script menu is about to show
void ScriptEditorDialog::on_MenuScript_aboutToShow( )
{
    mData->VerifyScriptAction->setEnabled( mData->Editor->length( ) != 0 );
}

// Notification of a change to the edited file
void ScriptEditorDialog::on_savePointChanged( bool dirty )
{
    mData->SetDirtyFlag( dirty );
    if ( true )
    {
        mData->StatusBar->showMessage( "" );
    }
}

// Text clicked with mouse - update cursor position
void ScriptEditorDialog::on_textAreaClicked( int line, int modifiers )
{
    XUNREFERENCED_PARAMETER( line )
    XUNREFERENCED_PARAMETER( modifiers )

    mData->UpdateCursorPosition( );
}

// Keyboard key pressed - update cursor position
void ScriptEditorDialog::on_keyPressed( QKeyEvent* event )
{
    XUNREFERENCED_PARAMETER( event )

    mData->UpdateCursorPosition( );
}

// Show custom context menu for the editor
void ScriptEditorDialog::on_editor_customContextMenuRequested( const QPoint &pos )
{
    mData->AboutToShowContextMenu = true;
    mData->PluginForHelp = mData->GetPluginDescriptorFromCursor( pos );
    mData->MenuEdit->popup( mData->Editor->mapToGlobal( pos ) );
}

namespace Private
{

// Initialize code editor's widget
void ScriptEditorDialogData::InitEditor( )
{
    static const sptr_t markerOptions[] =
    {
        SC_MARKNUM_FOLDEROPEN, SC_MARKNUM_FOLDER, SC_MARKNUM_FOLDERSUB, SC_MARKNUM_FOLDERTAIL,
        SC_MARKNUM_FOLDEREND, SC_MARKNUM_FOLDEROPENMID, SC_MARKNUM_FOLDERMIDTAIL
    };
    static const sptr_t markerSettings[] =
    {
        SC_MARK_BOXMINUS, SC_MARK_BOXPLUS, SC_MARK_VLINE, SC_MARK_LCORNER,
        SC_MARK_BOXPLUSCONNECTED, SC_MARK_BOXMINUSCONNECTED, SC_MARK_TCORNER
    };

    Editor->setCodePage( SC_CP_UTF8 );

    // some editor settings
    Editor->setScrollWidth( 200 );
    Editor->setScrollWidthTracking( true );
    Editor->setTabWidth( 4 );
    Editor->setUseTabs( false );

    // margin settings
    Editor->setMarginWidthN( 0, 30 );
    Editor->setMarginWidthN( 1, 0 );
    Editor->setMarginWidthN( 2, 16 );
    Editor->setMarginWidthN( 3, 0 );
    Editor->setMarginWidthN( 4, 0 );

    // code folding settings
    Editor->setModEventMask( Editor->modEventMask( ) | SC_MOD_CHANGEFOLD );
    Editor->setFoldFlags( SC_FOLDFLAG_LINEAFTER_CONTRACTED );
    Editor->setMarginTypeN( 2, SC_MARGIN_SYMBOL );
    Editor->setMarginMaskN( 2, SC_MASK_FOLDERS );
    Editor->setMarginSensitiveN( 2, 1 );
    Editor->setAutomaticFold( SC_AUTOMATICFOLD_SHOW | SC_AUTOMATICFOLD_CLICK | SC_AUTOMATICFOLD_CHANGE );

    for ( size_t i = 0; i < sizeof( markerOptions ) / sizeof( markerOptions[0] ); i++ )
    {
        Editor->markerDefine( markerOptions[i], markerSettings[i] );
        Editor->markerSetFore( markerOptions[i], EDITOR_RGB( 255, 255, 255 ) );
        Editor->markerSetBack( markerOptions[i], EDITOR_RGB( 0, 0, 0 ) );
    }

    // default style
    Editor->styleSetFont( STYLE_DEFAULT, "Courier" );
    Editor->styleSetSize( STYLE_DEFAULT, 10 );
    Editor->styleSetBack( STYLE_DEFAULT, EDITOR_BG_COLOR );

    // Editor->setViewWS( 1 );

    // Hard code Lua settings for now - need to change though
    SetLuaSettings( );

    // finally enable folding
    Editor->setProperty( "fold", "1" );
}

// Set Lua configuration
void ScriptEditorDialogData::SetLuaSettings( )
{
    Editor->setLexer( SCLEX_LUA );

    Editor->setKeyWords( 0, "and break do else elseif end false for function if "
                            "in local nil not or repeat return then true until while" );

    Editor->styleSetFore( SCE_LUA_COMMENT,     EDITOR_RGB( 0, 128, 0 ) );
    Editor->styleSetFore( SCE_LUA_COMMENTLINE, EDITOR_RGB( 0, 128, 0 ) );
    Editor->styleSetFore( SCE_LUA_COMMENTDOC,  EDITOR_RGB( 0, 128, 0 ) );
    Editor->styleSetFore( SCE_LUA_WORD,        EDITOR_RGB( 0, 0, 255 ) );
    Editor->styleSetFore( SCE_LUA_NUMBER,      EDITOR_RGB( 255, 128, 40 ) );
    Editor->styleSetFore( SCE_LUA_STRING,      EDITOR_RGB( 0, 162, 232 ) );
    Editor->styleSetFore( SCE_LUA_CHARACTER,   EDITOR_RGB( 0, 162, 232 ) );
    Editor->styleSetFore( SCE_LUA_OPERATOR,    EDITOR_RGB( 64, 0, 128 ) );

    Editor->styleSetBold( SCE_LUA_COMMENT,     true );
    Editor->styleSetBold( SCE_LUA_COMMENTLINE, true );
    Editor->styleSetBold( SCE_LUA_COMMENTDOC,  true );
    Editor->styleSetBold( SCE_LUA_WORD,        true );

    for ( int style = SCE_LUA_DEFAULT; style <= SCE_LUA_LABEL; style++ )
    {
        Editor->styleSetBack( style, EDITOR_BG_COLOR );
    }

    Editor->styleSetBack( SCE_LUA_WORD2, EDITOR_BG_COLOR1 );
    Editor->styleSetBack( SCE_LUA_WORD3, EDITOR_BG_COLOR2 );
    Editor->styleSetBack( SCE_LUA_WORD4, EDITOR_BG_COLOR3 );
    Editor->styleSetBack( SCE_LUA_WORD5, EDITOR_BG_COLOR4 );
    Editor->styleSetBack( SCE_LUA_WORD6, EDITOR_BG_COLOR5 );
}

// Create menu bar of the editor
QMenuBar* ScriptEditorDialogData::CreateMenuBar( )
{
    QMenuBar* menuBar    = new QMenuBar( Parent );
    QMenu*    menuFile   = new QMenu( "&File", menuBar );

    // File menu
    menuBar->addAction( menuFile->menuAction( ) );

    NewFileMenuAction = new QAction( "&New", menuFile );
    NewFileMenuAction->setStatusTip( "Create new script file" );
    NewFileMenuAction->setIcon( QIcon( QPixmap( ":/images/icons/file_new.png" ) ) );

    OpenFileMenuAction = new QAction( "&Open", menuFile );
    OpenFileMenuAction->setStatusTip( "Open script file" );
    OpenFileMenuAction->setIcon( QIcon( QPixmap( ":/images/icons/open_script_file.png" ) ) );
    OpenFileMenuAction->setShortcut( QKeySequence( "Ctrl+O" ) );

    SaveFileMenuAction = new QAction( "&Save", menuFile );
    SaveFileMenuAction->setStatusTip( "Save current script file" );
    SaveFileMenuAction->setIcon( QIcon( QPixmap( ":/images/icons/file_save.png" ) ) );
    SaveFileMenuAction->setShortcut( QKeySequence( "Ctrl+S" ) );

    SaveAsFileMenuAction = new QAction( "Save &As", menuFile );
    SaveAsFileMenuAction->setStatusTip( "Save current script file using a different name" );

    RecentFilesMenu = new QMenu( "&Recent files", menuFile );

    CloseMenuAction = new QAction( "&Close", menuFile );
    CloseMenuAction->setStatusTip( "Close the script editor" );
    CloseMenuAction->setIcon( QIcon( QPixmap( ":/images/icons/file_close.png" ) ) );

    menuFile->addAction( NewFileMenuAction );
    menuFile->addAction( OpenFileMenuAction );
    menuFile->addSeparator( );
    menuFile->addAction( SaveFileMenuAction );
    menuFile->addAction( SaveAsFileMenuAction );
    menuFile->addSeparator( );
    menuFile->addMenu( RecentFilesMenu );
    menuFile->addSeparator( );
    menuFile->addAction( CloseMenuAction );

    // Edit menu
    MenuEdit = new QMenu( "&Edit", menuBar );
    menuBar->addAction( MenuEdit->menuAction( ) );

    UndoEditAction = new QAction( "&Undo", MenuEdit );
    UndoEditAction->setStatusTip( "Undo last edit" );
    UndoEditAction->setIcon( QIcon( QPixmap( ":/images/icons/undo.png" ) ) );
    UndoEditAction->setShortcut( QKeySequence( "Ctrl+Z" ) );

    RedoEditAction = new QAction( "&Redo", MenuEdit );
    RedoEditAction->setStatusTip( "Redo previously undone edit" );
    RedoEditAction->setIcon( QIcon( QPixmap( ":/images/icons/redo.png" ) ) );
    RedoEditAction->setShortcut( QKeySequence( "Ctrl+Y" ) );

    CutEditAction = new QAction( "Cu&t", MenuEdit );
    CutEditAction->setStatusTip( "Cut currently selected text" );
    CutEditAction->setIcon( QIcon( QPixmap( ":/images/icons/cut.png" ) ) );
    CutEditAction->setShortcut( QKeySequence( "Ctrl+X" ) );

    CopyEditAction = new QAction( "&Copy", MenuEdit );
    CopyEditAction->setStatusTip( "Copy currently selected text" );
    CopyEditAction->setIcon( QIcon( QPixmap( ":/images/icons/copy.png" ) ) );
    CopyEditAction->setShortcut( QKeySequence( "Ctrl+C" ) );

    PasteEditAction = new QAction( "&Paste", MenuEdit );
    PasteEditAction->setStatusTip( "Paste from clipboard" );
    PasteEditAction->setIcon( QIcon( QPixmap( ":/images/icons/paste.png" ) ) );
    PasteEditAction->setShortcut( QKeySequence( "Ctrl+V" ) );

    SelectAllEditAction = new QAction( "Select &All", MenuEdit );
    SelectAllEditAction->setStatusTip( "Select all text" );
    SelectAllEditAction->setShortcut( QKeySequence( "Ctrl+A" ) );

    PluginDescriptionSeparator = new QAction( MenuEdit );
    PluginDescriptionSeparator->setSeparator( true );

    PluginDescriptionAction = new QAction( "Plug-in description", MenuEdit );
    PluginDescriptionAction->setStatusTip( "Get description for the plug-in name under cursor" );
    PluginDescriptionAction->setIcon( QIcon( QPixmap( ":/images/icons/idea.png" ) ) );
    PluginDescriptionAction->setShortcut( QKeySequence( "Ctrl+H" ) );

    // add/remove style sub menus
    MenuStyleToken  = new QMenu( "Style token" );
    MenuRemoveStyle = new QMenu( "Remove style" );

    StyleToken1Action = new QAction( "Using &1st style", MenuStyleToken );
    StyleToken1Action->setStatusTip( "Highlight token using style #1" );
    StyleToken1Action->setShortcut( QKeySequence( "Ctrl+1" ) );

    StyleToken2Action = new QAction( "Using &2nd style", MenuStyleToken );
    StyleToken2Action->setStatusTip( "Highlight token using style #2" );
    StyleToken2Action->setShortcut( QKeySequence( "Ctrl+2" ) );

    StyleToken3Action = new QAction( "Using &3rd style", MenuStyleToken );
    StyleToken3Action->setStatusTip( "Highlight token using style #3" );
    StyleToken3Action->setShortcut( QKeySequence( "Ctrl+3" ) );

    StyleToken4Action = new QAction( "Using &4th style", MenuStyleToken );
    StyleToken4Action->setStatusTip( "Highlight token using style #4" );
    StyleToken4Action->setShortcut( QKeySequence( "Ctrl+4" ) );

    StyleToken5Action = new QAction( "Using &5th style", MenuStyleToken );
    StyleToken5Action->setStatusTip( "Highlight token using style #5" );
    StyleToken5Action->setShortcut( QKeySequence( "Ctrl+5" ) );

    MenuStyleToken->addAction( StyleToken1Action );
    MenuStyleToken->addAction( StyleToken2Action );
    MenuStyleToken->addAction( StyleToken3Action );
    MenuStyleToken->addAction( StyleToken4Action );
    MenuStyleToken->addAction( StyleToken5Action );

    RemoveStyle1Action = new QAction( "Clear &1st style", MenuStyleToken );
    RemoveStyle1Action->setStatusTip( "Clear highlighting with style #1" );
    RemoveStyle1Action->setShortcut( QKeySequence( "Ctrl+Shift+1" ) );

    RemoveStyle2Action = new QAction( "Clear &2nd style", MenuStyleToken );
    RemoveStyle2Action->setStatusTip( "Clear highlighting with style #2" );
    RemoveStyle2Action->setShortcut( QKeySequence( "Ctrl+Shift+2" ) );

    RemoveStyle3Action = new QAction( "Clear &3rd style", MenuStyleToken );
    RemoveStyle3Action->setStatusTip( "Clear highlighting with style #3" );
    RemoveStyle3Action->setShortcut( QKeySequence( "Ctrl+Shift+3" ) );

    RemoveStyle4Action = new QAction( "Clear &4th style", MenuStyleToken );
    RemoveStyle4Action->setStatusTip( "Clear highlighting with style #4" );
    RemoveStyle4Action->setShortcut( QKeySequence( "Ctrl+Shift+4" ) );

    RemoveStyle5Action = new QAction( "Clear &5th style", MenuStyleToken );
    RemoveStyle5Action->setStatusTip( "Clear highlighting with style #5" );
    RemoveStyle5Action->setShortcut( QKeySequence( "Ctrl+Shift+5" ) );

    RemoveStylesAllAction = new QAction( "Clear &all styles", MenuStyleToken );
    RemoveStylesAllAction->setStatusTip( "Clear all styles highlighting" );
    RemoveStylesAllAction->setShortcut( QKeySequence( "Ctrl+Shift+6" ) );

    MenuRemoveStyle->addAction( RemoveStyle1Action );
    MenuRemoveStyle->addAction( RemoveStyle2Action );
    MenuRemoveStyle->addAction( RemoveStyle3Action );
    MenuRemoveStyle->addAction( RemoveStyle4Action );
    MenuRemoveStyle->addAction( RemoveStyle5Action );
    MenuRemoveStyle->addSeparator( );
    MenuRemoveStyle->addAction( RemoveStylesAllAction );

    MenuEdit->addAction( UndoEditAction );
    MenuEdit->addAction( RedoEditAction );
    MenuEdit->addSeparator( );
    MenuEdit->addAction( CutEditAction );
    MenuEdit->addAction( CopyEditAction );
    MenuEdit->addAction( PasteEditAction );
    MenuEdit->addSeparator( );
    MenuEdit->addAction( SelectAllEditAction );
    MenuEdit->addSeparator( );
    MenuEdit->addMenu( MenuStyleToken );
    MenuEdit->addMenu( MenuRemoveStyle );
    MenuEdit->addAction( PluginDescriptionSeparator );
    MenuEdit->addAction( PluginDescriptionAction );

    // Search menu
    MenuSearch = new QMenu( "&Search", menuBar );
    menuBar->addAction( MenuSearch->menuAction( ) );

    FindTextAction = new QAction( "&Find", MenuEdit );
    FindTextAction->setStatusTip( "Find text in the current file" );
    FindTextAction->setIcon( QIcon( QPixmap( ":/images/icons/find.png" ) ) );
    FindTextAction->setShortcut( QKeySequence( "Ctrl+F" ) );

    FindNextAction = new QAction( "Find &next", MenuEdit );
    FindNextAction->setStatusTip( "Find next occurrence of the last search" );
    FindNextAction->setShortcut( QKeySequence( "F3" ) );

    GoToLineAction = new QAction( "&Go to line", MenuEdit );
    GoToLineAction->setStatusTip( "Navigate to specified line number" );
    GoToLineAction->setIcon( QIcon( QPixmap( ":/images/icons/goto.png" ) ) );
    GoToLineAction->setShortcut( QKeySequence( "Ctrl+G" ) );

    MenuSearch->addAction( FindTextAction );
    MenuSearch->addAction( FindNextAction );
    MenuSearch->addSeparator( );
    MenuSearch->addAction( GoToLineAction );

    // Script menu
    MenuScript = new QMenu( "Sc&ript", menuBar );
    menuBar->addAction( MenuScript->menuAction( ) );

    VerifyScriptAction = new QAction( "&Verify", MenuEdit );
    VerifyScriptAction->setStatusTip( "Verifies script by compiling it (requires saving)" );
    VerifyScriptAction->setIcon( QIcon( QPixmap( ":/images/icons/compile.png" ) ) );
    VerifyScriptAction->setShortcut( QKeySequence( "Ctrl+T" ) );

    MenuScript->addAction( VerifyScriptAction );

    // Help menu
    MenuHelp = new QMenu( "&Help", menuBar );
    menuBar->addAction( MenuHelp->menuAction( ) );

    AboutScriptingPluginAction = new QAction( "&About scripting plug-in", MenuEdit );
    AboutScriptingPluginAction->setStatusTip( "Shows information about the scripting plug-in in use" );
    AboutScriptingPluginAction->setIcon( QIcon( QPixmap( ":/images/icons/idea.png" ) ) );

    ModuleListAction = new QAction( "&Modules list", MenuEdit );
    ModuleListAction->setStatusTip( "Shows information about availabe modules and plug-ins" );
    ModuleListAction->setIcon( QIcon( QPixmap( ":/images/icons/block.png" ) ) );

    MenuHelp->addAction( AboutScriptingPluginAction );
    MenuHelp->addSeparator( );
    MenuHelp->addAction( ModuleListAction );

    // disable scripting related items until scripting plug-in is set
    MenuScript->menuAction( )->setVisible( false );
    AboutScriptingPluginAction->setEnabled( false );

    return menuBar;
}

// Create status bar of the editor
QStatusBar* ScriptEditorDialogData::CreateStatusBar( )
{
    StatusBar = new QStatusBar( Parent );

    // cursor label
    CursorLabel = new QLabel( StatusBar );

    CursorLabel->setMinimumWidth( 120 );
    CursorLabel->setFrameShape( QFrame::Panel );
    CursorLabel->setFrameShadow( QFrame::Sunken );
    CursorLabel->setToolTip( "Cursor position" );

    // add them all
    StatusBar->addPermanentWidget( CursorLabel );

    return StatusBar;
}

// Update title bar of the script editor window, showing file name and dirty status
void ScriptEditorDialogData::UpdateEditorTitle( )
{
    Parent->setWindowTitle( QString( "%0 : %1 %2" ).arg( ScriptingName )
                                                   .arg( ( FileName.isEmpty( ) ) ? "<new>" : FileName )
                                                   .arg( ( IsDirty ) ? "*" : "" ) );
}

// Update cursor position shown in the status bar
void ScriptEditorDialogData::UpdateCursorPosition( )
{
    sptr_t pos = Editor->currentPos( );
    sptr_t row = Editor->lineFromPosition( pos );
    sptr_t col = Editor->column( pos );

    CursorLabel->setText( QString( "Ln: %0  Col: %1" ).arg( static_cast<int>( row ) + 1 )
                                                      .arg( static_cast<int>( col ) + 1 ) );
}

// Set current file name
void ScriptEditorDialogData::SetFileName( const QString& fileName )
{
    if ( !fileName.isEmpty( ) )
    {
        QFileInfo fileInfo( fileName );

        FilePath = fileName; // use it as specified by user
        FileName = fileInfo.fileName( );

        // set last open file only if it is not empty
        LastOpenFilePath = fileName;
    }
    else
    {
        FilePath = FileName = "";
    }
}

// Set "dirty" flag indicating status of the current file
void ScriptEditorDialogData::SetDirtyFlag( bool dirty )
{
    bool oldDirty = IsDirty;

    IsDirty = dirty;
    SaveFileMenuAction->setEnabled( dirty );
    UpdateEditorTitle( );

    if ( ( !dirty ) && ( dirty != oldDirty ) )
    {
        Editor->setSavePoint( );
    }
}

// Set common properties for file dialog
void ScriptEditorDialogData::InitFileDialog( QFileDialog* dialog )
{
    QString defaultFilter = "All files (*.*)";

    if ( !DefaultExtension.isEmpty( ) )
    {
        dialog->setDefaultSuffix( DefaultExtension );
        defaultFilter = QString( "%0 files (*.%1);;").arg( ScriptingName ).arg( DefaultExtension ) + defaultFilter;
    }

    dialog->setWindowFlags( dialog->windowFlags( ) & ~Qt::WindowContextHelpButtonHint );
    dialog->setNameFilter( defaultFilter );
}

// Create new empty document
void ScriptEditorDialogData::NewFile( )
{
    Editor->setText( "" );

    Editor->setEOLMode( SC_EOL_LF );
    Editor->emptyUndoBuffer( );

    SetFileName( "" );
    SetDirtyFlag( false );
    UpdateCursorPosition( );

    // remove any highlighting
    RemoveStyle( -1 );
}

// Select and open script
void ScriptEditorDialogData::OpenScript( )
{
    QFileDialog dialog( Parent );

    InitFileDialog( &dialog );

    dialog.setFileMode( QFileDialog::ExistingFile );
    dialog.setAcceptMode( QFileDialog::AcceptOpen );
    dialog.setWindowTitle( "Select script file" );
    dialog.setWindowIcon( QIcon( ":/images/icons/open_script_file.png" ) );

    if ( !FilePath.isEmpty( ) )
    {
        dialog.setDirectory( QFileInfo( FilePath ).dir( ) );
    }
    else
    {
        dialog.setDirectory( GlobalServiceManager::Instance( ).GetSettingsService( )->GetValue( STR_LAST_FOLDER, STR_SCRIPT_EDITOR ).toString( ) );
    }

    if ( dialog.exec( ) == QDialog::Accepted )
    {
        QStringList selectedList = dialog.selectedFiles( );

        if ( selectedList.size( ) == 1 )
        {
            LoadScript( selectedList[0] );

            GlobalServiceManager::Instance( ).GetSettingsService( )->SetValue( STR_LAST_FOLDER, STR_SCRIPT_EDITOR, QFileInfo( selectedList[0] ).absolutePath( ) );
        }
    }
}

// Load script from the specified file
bool ScriptEditorDialogData::LoadScript( const QString& fileName, bool silent )
{
    QFile file( fileName );
    bool  ret = true;

    if ( file.open( QIODevice::ReadOnly ) )
    {
        QTextStream textStream( &file );

        // remove any highlighting
        RemoveStyle( -1 );

        // let text stream to detect Unicode - we'll get it as UTF8 later anyway
        textStream.setCodec( "UTF-8" );
        textStream.setAutoDetectUnicode( true );

        Editor->setText( textStream.readAll( ).toUtf8( ).data( ) );

        Editor->convertEOLs( SC_EOL_LF );
        Editor->setEOLMode( SC_EOL_LF );
        Editor->emptyUndoBuffer( );

        SetFileName( fileName );
        SetDirtyFlag( false );
        UpdateCursorPosition( );

        AddToRecentFiles( fileName );
    }
    else
    {
        ret = false;

        if ( !silent )
        {
            UITools::ShowErrorMessage( QString( "Failed opening the file specified:<br><br><b>%0</b>" ).arg( fileName ), Parent );
        }
    }

    return ret;
}

// Save current file
bool ScriptEditorDialogData::SaveScript( bool saveAs )
{
    QString saveName = FilePath;
    bool    ret      = false;
    bool    canceled = false;

    if ( saveName.isEmpty( ) )
    {
        saveAs = true;
    }

    if ( saveAs )
    {
        QFileDialog dialog( Parent );

        InitFileDialog( &dialog );

        dialog.setFileMode( QFileDialog::AnyFile );
        dialog.setAcceptMode( QFileDialog::AcceptSave );
        dialog.setWindowTitle( "Select file to write to" );
        dialog.setWindowIcon( QIcon( ":/images/icons/file_save.png" ) );

        if ( !saveName.isEmpty( ) )
        {
            dialog.selectFile( saveName );
        }
        else
        {
            dialog.setDirectory( GlobalServiceManager::Instance( ).GetSettingsService( )->GetValue( STR_LAST_FOLDER, STR_SCRIPT_EDITOR ).toString( ) );
        }

        if ( dialog.exec( ) == QDialog::Accepted )
        {
            QStringList selectedList = dialog.selectedFiles( );

            if ( selectedList.size( ) == 1 )
            {
                saveName = selectedList[0];

                GlobalServiceManager::Instance( ).GetSettingsService( )->SetValue( STR_LAST_FOLDER, STR_SCRIPT_EDITOR, QFileInfo( saveName ).absolutePath( ) );
            }
        }
        else
        {
            canceled = true;
        }
    }

    if ( !canceled )
    {
        QFile file( saveName );

        if ( file.open( QIODevice::WriteOnly ) )
        {
            sptr_t  textLength   = Editor->textLength( );
            qint64  bytesWritten = file.write( Editor->getText( textLength + 1 ), textLength );

            file.close( );

            if ( bytesWritten == static_cast<qint64>( textLength ) )
            {
                ret = true;

                SetFileName( saveName );
                SetDirtyFlag( false );
                AddToRecentFiles( saveName );
            }
            else
            {
                UITools::ShowErrorMessage( QString( "Failed while writing the file:<br><br><b>%0</b>" ).append( saveName ), Parent );
            }
        }
        else
        {
            UITools::ShowErrorMessage( QString( "Failed opening the file for writing:<br><br><b>%0</b>" ).append( saveName ) , Parent );
        }
    }

    return ret;
}

// Check if the current needs to be saved
bool ScriptEditorDialogData::SaveIfNeeded( bool silentSave )
{
    bool ret = true;

    if ( IsDirty )
    {
        if ( silentSave )
        {
            ret = SaveScript( );
        }
        else
        {
            switch ( QMessageBox::question( Parent, "File modified", "The current file was modified.\n\nWould you like saving it before continue ?",
                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes ) )
            {
                case QMessageBox::Cancel:
                    ret = false;
                    break;

                case QMessageBox::Yes:
                    ret = SaveScript( );
                    break;

                default:
                    break;
            }
        }
    }

    return ret;
}

// Add new script file to the list of recently opened/saved files
void ScriptEditorDialogData::AddToRecentFiles( const QString& fileName )
{
    // remove the file from the recent list
    vector<QString>::iterator foundAt = std::find( RecentFiles.begin( ), RecentFiles.end( ), fileName );
    if ( foundAt != RecentFiles.end( ) )
    {
        RecentFiles.erase( foundAt );
    }
    // add the new file to the top
    RecentFiles.insert( RecentFiles.begin( ), fileName );
    // shrink it
    if ( RecentFiles.size( ) > 10 )
    {
        RecentFiles.pop_back( );
    }

    SaveRecentFiles( );
}

// Open recent file with the specified index
void ScriptEditorDialogData::OpenRecentFile( int fileNumber )
{
    if ( fileNumber < static_cast<int>( RecentFiles.size( ) ) )
    {
        QString fileName = RecentFiles[fileNumber];

        if ( !LoadScript( fileName, true ) )
        {
            if ( QMessageBox::question( Parent, "Failed loading file", QString( "Failed loading the file:<br><b>%0</b><br><br>Would you like to remove it from the recent lit?" ).arg( fileName ),
                 QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes ) == QMessageBox::Yes )
            {
                vector<QString>::iterator foundAt = std::find( RecentFiles.begin( ), RecentFiles.end( ), fileName );
                if ( foundAt != RecentFiles.end( ) )
                {
                    RecentFiles.erase( foundAt );
                    SaveRecentFiles( );
                }
            }
        }
    }
}

// Save list of recent files
void ScriptEditorDialogData::SaveRecentFiles( )
{
    shared_ptr<IApplicationSettingsService> settingsService = GlobalServiceManager::Instance( ).GetSettingsService( );
    int counter = 0;

    for ( auto it = RecentFiles.begin( ); it != RecentFiles.end( ); ++it, ++counter )
    {
        settingsService->SetValue( STR_RECENT_FILES, QString( "%0" ).arg( counter ), it->toUtf8( ).data( ) );
    }
}

// Load list of recently used files
void ScriptEditorDialogData::LoadRecentFiles( )
{
    shared_ptr<IApplicationSettingsService> settingsService = GlobalServiceManager::Instance( ).GetSettingsService( );

    for ( int i = 0; i < 10; ++i )
    {
        QString value = QString::fromUtf8(
                        settingsService->GetValue( STR_RECENT_FILES, QString( "%0" ).arg( i ) ).
                        toString( ).toStdString( ).data( ) );

        if ( ( !value.isEmpty( ) ) &&
             ( std::find( RecentFiles.begin( ), RecentFiles.end( ), value ) == RecentFiles.end( ) ) )
        {
            RecentFiles.push_back( value );
        }
    }
}

// Try getting plug-in descriptor for the Lua string currently under cursor
const shared_ptr<const XPluginDescriptor> ScriptEditorDialogData::GetPluginDescriptorFromCursor( )
{
    return GetPluginDescriptorFromCursor( Editor->currentPos( ) );
}
const shared_ptr<const XPluginDescriptor> ScriptEditorDialogData::GetPluginDescriptorFromCursor( const QPoint &pos )
{
    return GetPluginDescriptorFromCursor( Editor->positionFromPoint( pos.x( ), pos.y( ) ) );
}
const shared_ptr<const XPluginDescriptor> ScriptEditorDialogData::GetPluginDescriptorFromCursor( sptr_t editorPos )
{
    shared_ptr<const XPluginDescriptor> ret;

    sptr_t style = Editor->styleAt( editorPos );

    // check if we have string at the cursor position
    if ( ( style == SCE_LUA_STRING ) || ( style == SCE_LUA_CHARACTER ) )
    {
        sptr_t  before = Editor->wordStartPosition( editorPos, true );
        sptr_t  after  = Editor->wordEndPosition( editorPos, true );
        QString string = QString( Editor->textRange( before, after ) );

        sptr_t  charBefore = Editor->charAt( before - 1 );
        sptr_t  charAfter  = Editor->charAt( after );

        if ( ( ( charBefore == '\'') || ( charBefore == '"' ) ) && ( charAfter == charBefore ) )
        {
            ret = GlobalServiceManager::Instance( ).GetPluginsEngine( )->GetPlugin( string.toStdString( ) );
        }
    }

    return ret;
}

// Set background highlighting for a token
void ScriptEditorDialogData::StyleToken( int style )
{
    sptr_t  editorPos  = Editor->currentPos( );
    sptr_t  before     = Editor->wordStartPosition( editorPos, true );
    sptr_t  after      = Editor->wordEndPosition( editorPos, true );
    sptr_t  tokenStyle = Editor->styleAt( before );
    QString token      = QString( Editor->textRange( before, after ) ).trimmed( );

    if ( ( tokenStyle == SCE_LUA_IDENTIFIER ) || ( tokenStyle == SCE_LUA_WORD ) ||
       ( ( tokenStyle >= SCE_LUA_WORD2 ) && ( tokenStyle <= SCE_LUA_WORD6 ) ) )
    {
        // remove token from any of the list
        for ( int i = 0; i < 5; i++ )
        {
            HighlightedWords[i].replace( token, "" );
        }

        // add the token to required highlight list
        if ( !HighlightedWords[style].isEmpty( ) )
        {
            HighlightedWords[style].append( ' ' );
        }
        HighlightedWords[style].append( token );

        // set key words back
        for ( int i = 0; i < 5; i++ )
        {
            Editor->setKeyWords( i + 1, HighlightedWords[i].toUtf8( ).data( ) );
        }

        // re-colour entire document
        Editor->colourise( 0, Editor->textLength( ) );
    }
}

// Remove highlighting with the specified style
void ScriptEditorDialogData::RemoveStyle( int style )
{
    if ( style == -1 )
    {
        for ( int i = 0; i < 5; i++ )
        {
            HighlightedWords[i].clear( );
            Editor->setKeyWords( i + 1, "" );
        }
    }
    else
    {
        HighlightedWords[style].clear( );
        Editor->setKeyWords( style + 1, "" );
    }

    Editor->colourise( 0, Editor->textLength( ) );
}

} // namespace Private
