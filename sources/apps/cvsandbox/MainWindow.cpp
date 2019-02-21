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

#include <QDir>
#include <QFileInfo>
#include <QCloseEvent>
#include <QLabel>

#include <UIPersistenceService.hpp>
#include <HelpService.hpp>
#include <ApplicationSettingsService.hpp>
#include <ImageFileService.hpp>
#include <UITools.hpp>
#include <ScriptEditorDialog.hpp>

#include "MainWindow.hpp"
#include "ui_MainWindow.h"

#include "ProjectTreeFrame.hpp"
#include "SandboxVariablesMonitorFrame.hpp"
#include "ProjectManager.hpp"
#include "ServiceManager.hpp"
#include "PerformanceMonitorService.hpp"
#include "UserCommands.hpp"

#include "ProjectObjectOpener.hpp"

#include "AboutDialog.hpp"
#include "version.hpp"

#include "VideoSnapshotDialog.hpp"

#include <XScriptingEnginePlugin.hpp>

#ifdef _WIN32
    #include <windows.h>
#endif

using namespace std;
using namespace std::chrono;
using namespace CVSandbox;
using namespace CVSandbox::Automation;

static const QString APPLICATION_TITLE( "Computer Vision Sandbox" );
static const QString SETTINGS_FOLDER( "CVSandbox/CVS" );

namespace Private
{
    // Group some command handlers together
    class MainWindowCommands
    {
    public:
        AddFolderCommand            AddFolder;
        AddCameraCommand            AddCamera;
        AddSandboxCommand           AddSandbox;
        EditPropertiesCommand       EditProperties;
        DeleteProjectObjectCommand  DeleteProjectObject;

        OpenProjectObjectCommand    OpenProjectObject;
        OpenSandboxWizardCommand    OpenSandboxWizard;
    };

    class ApplicationOptions
    {
    public:
        QString     DefaultSettingsFolder;
        QString     ProjectFileName;
        QString     UiPersistenceFileName;
        QString     ApplicationSettingsFileName;
        QString     ProjectObjectToStart;

        bool        StartFullScreen;
        bool        FitCamerasToScreen;
        bool        HideProjectObjectTree;
        bool        NoSystemSleep;

    public:
        ApplicationOptions( ) :
            DefaultSettingsFolder( QFileInfo( QDir::home( ), SETTINGS_FOLDER ).filePath( ) ),
            ProjectFileName( QFileInfo( DefaultSettingsFolder, "project.xml" ).filePath( ) ),
            UiPersistenceFileName( QFileInfo( DefaultSettingsFolder, "ui.ini" ).filePath( ) ),
            ApplicationSettingsFileName( QFileInfo( DefaultSettingsFolder, "app.ini" ).filePath( ) ),
            ProjectObjectToStart( ),
            StartFullScreen( false ), FitCamerasToScreen( false ),
            HideProjectObjectTree( false ), NoSystemSleep( false )
        {

        }

        void ParseCommandLineArguments( );

    private:

        void TryFileName( const QString& fileName, const QString& expectedExt, QString& target );
    };

    // Private data of the MainWindow class
    class MainWindowData
    {
    public:
        MainWindowData( ) :
            commands( ), appOptions( ),
            projectTreeFrame( new ProjectTreeFrame( ) ),
            variablesMonitorFrame( new SandboxVariablesMonitorFrame( ) ),
            uptimeLabel( nullptr ), fpsLabel( nullptr ), cpuLabel( nullptr ),
            scriptEditor( nullptr ), snapshotDialog( nullptr )
        {
        }

    public:
        MainWindowCommands            commands;
        ApplicationOptions            appOptions;
        ProjectTreeFrame*             projectTreeFrame;
        SandboxVariablesMonitorFrame* variablesMonitorFrame;

        QLabel*               uptimeLabel;
        QLabel*               fpsLabel;
        QLabel*               cpuLabel;

        ScriptEditorDialog*   scriptEditor;
        VideoSnapshotDialog*  snapshotDialog;
    };
}

MainWindow::MainWindow( QWidget* parent ) :
    QMainWindow( parent ),
    ui( new Ui::MainWindow ),
    mData( new ::Private::MainWindowData )
{
    mData->appOptions.ParseCommandLineArguments( );

    shared_ptr<XPluginsEngine>  pluginsEngine  = XPluginsEngine::Create( );
    shared_ptr<ProjectManager>  pm             = ProjectManager::Create( mData->appOptions.ProjectFileName );
    ServiceManager&             smi            = ServiceManager::Instance( );
    auto                        perfMonitor    = shared_ptr<PerformanceMonitorService>( new PerformanceMonitorService( ) );

    // set/initialize all services
    smi.SetMainWindowService( this );
    smi.SetUiPersistenceService( shared_ptr<IUIPersistenceService>( new UIPersistenceService( mData->appOptions.UiPersistenceFileName ) ) );
    smi.SetSettingsService( shared_ptr<IApplicationSettingsService>( new ApplicationSettingsService( mData->appOptions.ApplicationSettingsFileName ) ) );
    smi.SetHelpService( shared_ptr<IHelpService>( new HelpService( ) ) );
    smi.SetImageFileService( shared_ptr<IImageFileService>( new ImageFileService( pluginsEngine ) ) );
    smi.SetPerformanceMonitor( perfMonitor );
    smi.SetProjectTreeUI( mData->projectTreeFrame );
    smi.SetProjectManager( pm );
    smi.SetPluginsEngine( pluginsEngine );
    smi.SetAutomationServer( XAutomationServer::Create( pluginsEngine, AppTitle, AppVersion ) );
    smi.SetFavouritePluginsManager( shared_ptr<FavouritePluginsManager>( new FavouritePluginsManager( ) ) );

    // some UI initialization
    ui->setupUi( this );
    ui->projectTreeDockWidget->setWidget( mData->projectTreeFrame );
    SetupContextMenus( );
    SetupStatuBar( );
    SetupSandboxVariablesUi( );
    SetActionsSignalHandlers( );
    SetSubTitle( QString::null );

    connect( perfMonitor.get( ), SIGNAL( Updated() ), this, SLOT( on_performanceMonitor_update() ));

    // set background color of the main desktop frame
    QPalette palette = ui->mainFrame->palette( );
    palette.setColor( backgroundRole( ), QColor( 128, 128, 128 ) );
    ui->mainFrame->setPalette( palette );
    ui->mainFrame->setAutoFillBackground( true );

    // collect plug-ins from their folder
    QString pluginsPath = QFileInfo( QCoreApplication::applicationDirPath( ), "cvsplugins/" ).filePath( );

    pluginsEngine->CollectModules( pluginsPath.toUtf8( ).data( ),
                                   PluginType_VideoSource |
                                   PluginType_ImageProcessingFilter |
                                   PluginType_ImageProcessingFilter2 |
                                   PluginType_ImageProcessing |
                                   PluginType_VideoProcessing |
                                   PluginType_ScriptingEngine |
                                   PluginType_Device |
                                   PluginType_CommunicationDevice |
                                   PluginType_ImageImporter |
                                   PluginType_ImageExporter |
                                   PluginType_ScriptingApi );
    smi.GetFavouritePluginsManager( )->Load( pluginsEngine->GetModules( ) );

    // if user does not have scripting engine plug-ins, hide built in scripting editor
    if ( pluginsEngine->GetPluginsOfType( PluginType_ScriptingEngine )->Count( ) == 0 )
    {
        ui->menuTools->menuAction( )->setVisible( false );
    }

    // connect project manager to UI
    connect( pm.get( ), SIGNAL( ProjectObjectAdded( const std::shared_ptr<ProjectObject>& ) ),
             mData->projectTreeFrame, SLOT( AddObject( const std::shared_ptr<ProjectObject>& ) ) );
    connect( pm.get( ), SIGNAL( ProjectObjectUpdated( const std::shared_ptr<ProjectObject>& ) ),
             mData->projectTreeFrame, SLOT( UpdateObject( const std::shared_ptr<ProjectObject>& ) ) );
    connect( pm.get( ), SIGNAL( ProjectObjectDeleted( const std::shared_ptr<ProjectObject>& ) ),
             mData->projectTreeFrame, SLOT( DeleteObject( const std::shared_ptr<ProjectObject>& ) ) );

    connect( pm.get( ), SIGNAL( ProjectObjectUpdated( const std::shared_ptr<ProjectObject>& ) ),
             this, SLOT( on_ProjectObjectUpdated( const std::shared_ptr<ProjectObject>& ) ) );
    connect( pm.get( ), SIGNAL( ProjectObjectDeleted( const std::shared_ptr<ProjectObject>& ) ),
             this, SLOT( on_ProjectObjectDeleted( const std::shared_ptr<ProjectObject>& ) ) );

    // restore size/position
    ServiceManager::Instance( ).GetUiPersistenceService( )->RestoreWidget( this );
    if ( isFullScreen( ) )
    {
        ui->actionFullScreen->setChecked( true );
        ui->menuBar->setVisible( false );
    }

    // start automation server
    smi.GetAutomationServer( )->Start( );

    // read project file
    pm->Load( );

    // complete initialization
    mData->projectTreeFrame->SetTriggerDefaultActionOnDoubleClick( true );
    mData->projectTreeFrame->SetSelectionOfNewItems( true );
    mData->projectTreeFrame->ExpandRootItem( true );
    mData->projectTreeFrame->setFocus( );

    HandleStartOptions( );
}

MainWindow::~MainWindow( )
{
    SetCentralWidget( nullptr );
    delete ui;

    // free services
    ServiceManager& smi = ServiceManager::Instance( );

    uint32_t notStarted, running, finalizing;
    smi.GetAutomationServer( )->GetVideoSourceCount( &notStarted, &running, &finalizing );
    qDebug( "notStarted = %u, running = %u, finalizing = %u", notStarted, running, finalizing );

    smi.GetAutomationServer( )->SignalToStop( );
    smi.GetAutomationServer( )->WaitForStop( );

    smi.SetFavouritePluginsManager( nullptr );
    smi.SetPerformanceMonitor( nullptr );
    smi.SetUiPersistenceService( nullptr );
    smi.SetSettingsService( nullptr );
    smi.SetProjectManager( nullptr );
    smi.SetPluginsEngine( nullptr );
    smi.SetHelpService( nullptr );
    smi.SetImageFileService( nullptr );
}

// Get main window of the application
QWidget* MainWindow::GetMainAppWindow( ) const
{
    return const_cast<QWidget*>( static_cast<const QWidget*>( this ) );
}

// Set status message displayed in the status bar of the main window
void MainWindow::SetStatusMessage( const QString& statusMessage ) const
{
    ui->statusBar->showMessage( statusMessage );
}

// Set sub-title - name of open file/object/whatever
void MainWindow::SetSubTitle( const QString& subTitle )
{
    QString title = APPLICATION_TITLE;

    if ( ( !subTitle.isNull( ) ) && ( subTitle.length( ) != 0 ) )
    {
        title.append( QString( " : %1" ).arg( subTitle ) );
    }

    setWindowTitle( title );
}

// Save position/size of some child widgets
void MainWindow::SaveWidgetContent( IUIPersistenceService& persistanceService, const QString& groupName )
{
    qDebug( "Need to save content ");

    persistanceService.SaveWidget( mData->projectTreeFrame, groupName );
}

// Restore position/size of some child widgets
void MainWindow::RestoreWidgetContent( IUIPersistenceService& persistanceService, const QString& groupName )
{
    qDebug( "Need to restore content ");

    persistanceService.RestoreWidget( mData->projectTreeFrame, groupName );
}

// Override close event to prompt user if required
void MainWindow::closeEvent( QCloseEvent* event )
{
    if ( ( PrepareForApplicationExit( ) ) &&
         ( ( mData->scriptEditor == nullptr ) || ( mData->scriptEditor->close( ) ) ) )
    {
        ServiceManager::Instance( ).GetUiPersistenceService( )->SaveWidget( this );

        if ( mData->scriptEditor != nullptr )
        {
            delete mData->scriptEditor;
            mData->scriptEditor = nullptr;
        }

        if ( mData->snapshotDialog != nullptr )
        {
            delete mData->snapshotDialog;
            mData->snapshotDialog = nullptr;
        }

        event->accept( );
    }
    else
    {
        event->ignore( );
    }
}

// Check if user is happy to exit and do clean-up if so
bool MainWindow::PrepareForApplicationExit( )
{
    static_pointer_cast<ProjectManager>( ServiceManager::Instance( ).GetProjectManager( ) )->Save( );

    if ( isMinimized( ) )
    {
        showNormal( );
    }

    show( );
    activateWindow( );

    // for now just ask user, later we'll check if there are unsaved documents, etc.
    return UITools::GetUserConfirmation( "Exit from the application ?", this );
}

// Connect some signals to slots, where UI designer did not let to do
void MainWindow::SetActionsSignalHandlers( )
{
    connect( ui->menuProject, SIGNAL( aboutToShow( ) ), this, SLOT( on_menuProject_aboutToShow( ) ) );

    // setup action handlers
    connect( ui->actionAddFolder, SIGNAL( triggered( ) ), &mData->commands.AddFolder, SLOT( Exec( ) ) );
    connect( ui->actionAddCamera, SIGNAL( triggered( ) ), &mData->commands.AddCamera, SLOT( Exec( ) ) );
    connect( ui->actionAddSandbox, SIGNAL( triggered( ) ), &mData->commands.AddSandbox, SLOT( Exec( ) ) );
    connect( ui->actionProjectItemProperties, SIGNAL( triggered( ) ), &mData->commands.EditProperties, SLOT( Exec( ) ) );
    connect( ui->actionEditCamerasProperties, SIGNAL( triggered( ) ), &mData->commands.EditProperties, SLOT( Exec( ) ) );
    connect( ui->actionEditSandbox, SIGNAL( triggered( ) ), &mData->commands.EditProperties, SLOT( Exec( ) ) );

    connect( ui->actionDeleteProjectObject, SIGNAL( triggered( ) ), &mData->commands.DeleteProjectObject, SLOT( Exec( ) ) );
    connect( ui->actionDeleteCamera, SIGNAL( triggered( ) ), &mData->commands.DeleteProjectObject, SLOT( Exec( ) ) );
    connect( ui->actionDeleteSandbox, SIGNAL( triggered( ) ), &mData->commands.DeleteProjectObject, SLOT( Exec( ) ) );

    connect( ui->actionOpenCamera, SIGNAL( triggered( ) ), &mData->commands.OpenProjectObject, SLOT( Exec( ) ) );
    connect( ui->actionOpenSandbox, SIGNAL( triggered( ) ), &mData->commands.OpenProjectObject, SLOT( Exec( ) ) );

    connect( ui->actionOpenSandboxWizard, SIGNAL( triggered( ) ), &mData->commands.OpenSandboxWizard, SLOT( Exec( ) ) );
}

// Configure docking widget for sandbox variables
void MainWindow::SetupSandboxVariablesUi( )
{
    ui->sandboxVariablesDockWidget->setWidget( mData->variablesMonitorFrame );
    ui->sandboxVariablesDockWidget->setVisible( false );
}

// Configure context menus for different UI elements
void MainWindow::SetupContextMenus( )
{
    // hide context menus from user
    ui->menuContextMenus->menuAction( )->setVisible( false );

    ui->menuCameraContextMenu->setDefaultAction( ui->actionOpenCamera );
    ui->menuSandboxContextMenu->setDefaultAction( ui->actionOpenSandbox );

    mData->projectTreeFrame->SetDefaultContextMenu( ui->menuProject );
    mData->projectTreeFrame->SetProjectObjectContextMenu( ProjectObjectType::Camera, ui->menuCameraContextMenu );
    mData->projectTreeFrame->SetProjectObjectContextMenu( ProjectObjectType::Sandbox, ui->menuSandboxContextMenu );
}

// Configure status bar so it could display all info
void MainWindow::SetupStatuBar( )
{
    // up-time label
    mData->uptimeLabel = new QLabel( this );

    mData->uptimeLabel->setMinimumWidth( 150 );
    mData->uptimeLabel->setAlignment( Qt::AlignRight );
    mData->uptimeLabel->setFrameShape( QFrame::Panel );
    mData->uptimeLabel->setFrameShadow( QFrame::Sunken );
    mData->uptimeLabel->setToolTip( "Up-time of the camera/sandbox" );
    mData->uptimeLabel->setVisible( false );

    // cpu label
    mData->cpuLabel = new QLabel( this );

    mData->cpuLabel->setMinimumWidth( 100 );
    mData->cpuLabel->setFrameShape( QFrame::Panel );
    mData->cpuLabel->setFrameShadow( QFrame::Sunken );
    mData->cpuLabel->setToolTip( "Application CPU load" );
    mData->cpuLabel->setVisible( false );

    // cpu label
    mData->fpsLabel = new QLabel( this );

    mData->fpsLabel->setMinimumWidth( 100 );
    mData->fpsLabel->setFrameShape( QFrame::Panel );
    mData->fpsLabel->setFrameShadow( QFrame::Sunken );
    mData->fpsLabel->setToolTip( "Total frame rate from all running cameras" );
    mData->fpsLabel->setVisible( false );

    // add them all
    ui->statusBar->addPermanentWidget( mData->uptimeLabel );
    ui->statusBar->addPermanentWidget( mData->cpuLabel );
    ui->statusBar->addPermanentWidget( mData->fpsLabel );
}

// Handle different command line options
void MainWindow::HandleStartOptions( )
{
    // check if full screen must be forced
    if ( ( mData->appOptions.StartFullScreen ) && ( !isFullScreen( ) ) )
    {
        ui->actionFullScreen->setChecked( true );
        ui->menuBar->setVisible( false );
        showFullScreen( );
    }

    // check if camera must be fit the screen
    if ( mData->appOptions.FitCamerasToScreen )
    {
        ui->actionFitTheWindow->setChecked( true );
        ui->mainFrame->EnableContentFitting( true );
    }

    // check if project object tree must be hidden
    if ( mData->appOptions.HideProjectObjectTree )
    {
        ui->actionShowProjectView->setChecked( false );
        ui->projectTreeDockWidget->hide( );
    }

    // check if screen saver and system sleep must be prevented
    if ( mData->appOptions.NoSystemSleep )
    {
        ui->actionPreventScreenSaver->setChecked( true );
    }

    // check if we need to start a project object
    if ( !mData->appOptions.ProjectObjectToStart.isEmpty( ) )
    {
        auto                      pm = ServiceManager::Instance( ).GetProjectManager( );
        shared_ptr<ProjectObject> po = pm->GetProjectObject( string( mData->appOptions.ProjectObjectToStart.toUtf8( ).data( ) ) );

        if ( po )
        {
            ProjectObjectOpener::Open( po );
            mData->projectTreeFrame->SetSelectedItem( po->Id( ) );
        }
    }
}

// Set widget to display in the main window
void MainWindow::SetCentralWidget( QWidget* widget )
{
    ServiceManager::Instance( ).GetPerformanceMonitor( )->Stop( );

    ui->mainFrame->SetContentWidget( widget );

    XGuid id = ui->mainFrame->DisplayedProjectObjectId( );
    auto  pm = ServiceManager::Instance( ).GetProjectManager( );
    auto  po = pm->GetProjectObject( id );

    ui->actionCloseObject->setEnabled( !id.IsEmpty( ) );

    SetSubTitle( ( po ) ? QString::fromUtf8( pm->GetProjectObjectFullName( po ).c_str( ) ) : QString::null );
}

// Get project object's ID for the central widget
const XGuid MainWindow::CentralWidgetsObjectId( ) const
{
    return ui->mainFrame->DisplayedProjectObjectId( );
}

// Take snapshot of the specified video source
void MainWindow::TakeSnapshotForVideoSource( uint32_t videoSourceId, const QString& title )
{
    if ( mData->snapshotDialog == nullptr )
    {
        mData->snapshotDialog = new VideoSnapshotDialog( nullptr, true, this );

        connect( mData->snapshotDialog, SIGNAL(finished(int)), this, SLOT(on_snapshotDialog_finished(int)) );
        connect( mData->snapshotDialog, SIGNAL(destroyed(QObject*)), this, SLOT(on_snapshotDialog_destroyed(QObject*)) );
    }

    mData->snapshotDialog->TakeSnapshot( videoSourceId, title );

    if ( mData->snapshotDialog->isMinimized( ) )
    {
        mData->snapshotDialog->showNormal( );
    }

    mData->snapshotDialog->show( );
    mData->snapshotDialog->activateWindow( );
}

void MainWindow::on_snapshotDialog_finished( int )
{
    mData->snapshotDialog = nullptr;
}

void MainWindow::on_snapshotDialog_destroyed( QObject* )
{
    mData->snapshotDialog = nullptr;
}

// ==========================
// <<<<< Action handlers >>>>
// ==========================

// Exit from application
void MainWindow::on_actionExit_triggered( )
{
    this->close( );
}

// Close currently running object
void MainWindow::on_actionCloseObject_triggered( )
{
    if ( !ui->mainFrame->DisplayedProjectObjectId( ).IsEmpty( ) )
    {
        SetCentralWidget( NULL );
    }
}

// Show/hide cameras list
void MainWindow::on_actionShowProjectView_triggered( )
{
    if ( !ui->projectTreeDockWidget->visibleRegion( ).isEmpty( ) )
    {
        ui->projectTreeDockWidget->hide( );
    }
    else
    {
        ui->projectTreeDockWidget->show( );

        if ( !tabifiedDockWidgets( ui->projectTreeDockWidget ).empty( ) )
        {
            ui->projectTreeDockWidget->raise( );
        }

        mData->projectTreeFrame->setFocus( );
    }
}

// Show/hide sandbox variables monitor
void MainWindow::on_actionSandboxVariablesMonitor_triggered( )
{
    if ( !ui->sandboxVariablesDockWidget->visibleRegion( ).isEmpty( ) )
    {
        ui->sandboxVariablesDockWidget->hide( );
    }
    else
    {
        ui->sandboxVariablesDockWidget->show( );

        if ( !tabifiedDockWidgets( ui->sandboxVariablesDockWidget ).empty( ) )
        {
            ui->sandboxVariablesDockWidget->raise( );
        }
    }
}


// Show main view in full screen or normal mode
void MainWindow::on_actionFullScreen_triggered( )
{
    bool showControls = isFullScreen( );

    ui->menuBar->setVisible( showControls );

    if ( showControls )
    {
        showNormal( );
    }
    else
    {
        showFullScreen( );
    }
}

// Enable current view (camera or sandbox) fitting to window size
void MainWindow::on_actionFitTheWindow_triggered( )
{
    ui->mainFrame->EnableContentFitting( ui->actionFitTheWindow->isChecked( ) );
}

// Cameras list's visibility has changed
void MainWindow::on_projectTreeDockWidget_visibilityChanged( bool visible )
{
    ui->actionShowProjectView->setChecked( visible );
}

// Sandbox variables monitor's visibility has changed
void MainWindow::on_sandboxVariablesDockWidget_visibilityChanged( bool visible )
{
    auto automationServer = ServiceManager::Instance( ).GetAutomationServer( );

    ui->actionSandboxVariablesMonitor->setChecked( visible );

    if ( visible )
    {
        automationServer->SetVariablesListener( mData->variablesMonitorFrame, true );
    }
    else
    {
        automationServer->ClearVariablesListener( );
        mData->variablesMonitorFrame->ClearAllVariables( );
    }
}

// Update items of cameras' menu
void MainWindow::on_menuProject_aboutToShow( )
{
    bool isParentFolderSelected = mData->projectTreeFrame->GetSelectedItem( ).IsEmpty( );

    ui->actionDeleteProjectObject->setEnabled( !isParentFolderSelected );
    ui->actionProjectItemProperties->setEnabled( !isParentFolderSelected );
}

// Show About dialog
void MainWindow::on_actionAbout_triggered( )
{
    AboutDialog dialog( this );

    dialog.exec( );
}

// Update status bar with some info coming from performance monitor
void MainWindow::on_performanceMonitor_update( )
{
    auto  perfMonitor = ServiceManager::Instance( ).GetPerformanceMonitor( );
    float totalCpu    = perfMonitor->TotalCpuLoad( );
    float totalFps    = perfMonitor->TotalFrameRate( );

    system_clock::duration upTime = perfMonitor->UpTime( );
    uint32_t upSeconds = static_cast<uint32_t>( duration_cast<std::chrono::seconds>( upTime ).count( ) );
    uint32_t upMinutes = static_cast<uint32_t>( duration_cast<std::chrono::minutes>( upTime ).count( ) );
    uint32_t upHours   = static_cast<uint32_t>( duration_cast<std::chrono::hours>( upTime ).count( ) );

    if ( totalCpu < 0 )
    {
        mData->cpuLabel->setVisible( false );
        mData->cpuLabel->setText( QString::null );
    }
    else
    {
        mData->cpuLabel->setText( QString( "CPU: %0%" ).arg( totalCpu, 0, 'f', 2 ) );
        mData->cpuLabel->setVisible( true );
    }

    if ( totalFps < 0 )
    {
        mData->fpsLabel->setVisible( false );
        mData->fpsLabel->setText( QString::null );
    }
    else
    {
        mData->fpsLabel->setText( QString( "FPS: %0" ).arg( totalFps, 0, 'f', 2 ) );
        mData->fpsLabel->setVisible( true );
    }

    if ( upSeconds == 0 )
    {
        mData->uptimeLabel->setVisible( false );
        mData->uptimeLabel->setText( QString::null );
    }
    else
    {
        QString strUpTime;

        uint32_t upDays = upHours / 24;

        upHours   %= 24;
        upMinutes %= 60;
        upSeconds %= 60;

        if ( upDays != 0 )
        {
            strUpTime = QString( "%0 day%1 " ).arg( upDays ).arg( ( upDays > 1 ) ? "s" : "" );
        }

        if ( ( upHours != 0 ) || ( !strUpTime.isEmpty( ) ) )
        {
            strUpTime += QString( "%0 hour%1 " ).arg( upHours ).arg( ( upHours > 1 ) ? "s" : "" );
        }

        if ( ( upMinutes != 0 ) || ( !strUpTime.isEmpty( ) ) )
        {
            strUpTime += QString( "%0 min " ).arg( upMinutes );
        }

        if ( ( ( upSeconds != 0 ) || ( !strUpTime.isEmpty( ) ) ) && ( upDays == 0 ) )
        {
            strUpTime += QString( "%0 sec " ).arg( upSeconds );
        }

        mData->uptimeLabel->setText( strUpTime.trimmed( ) );
        mData->uptimeLabel->setVisible( true );
    }

    {
        static int threadStateCounter = 0;

        if ( ( ++threadStateCounter % 10 ) == 0 )
        {
            threadStateCounter = 0;

            // check if we don't want screen saver
            if ( ui->actionPreventScreenSaver->isChecked( ) )
            {
                #ifdef _WIN32
                SetThreadExecutionState( ES_DISPLAY_REQUIRED );
                #endif
            }
        }
    }
}

// On project object update
void MainWindow::on_ProjectObjectUpdated( const shared_ptr<ProjectObject>& po )
{
    if ( ( po ) && ( po->Id( ) == CentralWidgetsObjectId( ) ) )
    {
        // update window title if the updated object is currently open
        SetSubTitle( QString::fromUtf8( ServiceManager::Instance( ).GetProjectManager( )->
                                        GetProjectObjectFullName( po ).c_str( ) ) );
    }
}

// On project object deleted
void MainWindow::on_ProjectObjectDeleted( const shared_ptr<ProjectObject>& po )
{
    if ( ( po ) && ( po->Id( ) == CentralWidgetsObjectId( ) ) )
    {
        // Close current view if it displays deleted object
        SetCentralWidget( NULL );
    }
}

// Open built-in scripts' editor
void MainWindow::on_actionScriptEditor_triggered( )
{
    if ( mData->scriptEditor == nullptr )
    {
        auto scriptingPlugins = ServiceManager::Instance( ).GetPluginsEngine( )->
                                    GetPluginsOfType( PluginType_ScriptingEngine );

        mData->scriptEditor = new ScriptEditorDialog( nullptr, true, this );

        connect( mData->scriptEditor, SIGNAL(finished(int)), this, SLOT(on_scriptEditor_finished(int)) );
        connect( mData->scriptEditor, SIGNAL(destroyed(QObject*)), this, SLOT(on_scriptEditor_destroyed(QObject*)) );

        if ( scriptingPlugins->Count( ) == 1 )
        {
            auto pluginDesc = *scriptingPlugins->begin( );
            auto plugin     = static_pointer_cast<XScriptingEnginePlugin>( pluginDesc->CreateInstance( ) );

            mData->scriptEditor->SetScriptingPluging( pluginDesc->ID( ) );
            mData->scriptEditor->SetScriptingName( QString::fromUtf8( pluginDesc->Name( ).c_str( ) ) );

            if ( plugin )
            {
                mData->scriptEditor->SetDefaultExtension( QString::fromUtf8( plugin->GetDefaultExtension( ).c_str( ) ) );
            }
        }
    }

    if ( mData->scriptEditor->isMinimized( ) )
    {
        mData->scriptEditor->showNormal( );
    }

    mData->scriptEditor->show( );
    mData->scriptEditor->activateWindow( );
}

// Script editor has finished
void MainWindow::on_scriptEditor_finished( int )
{
    mData->scriptEditor = nullptr;
}

// Script editor was destroyed
void MainWindow::on_scriptEditor_destroyed( QObject* )
{
    mData->scriptEditor = nullptr;
}


// Some private details
namespace Private
{

// Check the specified file name and use instead of target if valid
void ApplicationOptions::TryFileName( const QString& fileName, const QString& expectedExt, QString& target )
{
    QString correctedFileName = fileName;
    QString dottedExtesion    = "." + expectedExt;

    if ( !correctedFileName.endsWith( dottedExtesion ) )
    {
        correctedFileName += dottedExtesion;
    }

    QFileInfo fileInfo( correctedFileName );

    // If the file name represents absolute path, keep it. Othwerwise use default settings folder.
    if ( !fileInfo.isAbsolute( ) )
    {
        correctedFileName = QFileInfo( DefaultSettingsFolder, fileInfo.fileName( ) ).filePath( );
    }
    else
    {
        correctedFileName = fileInfo.filePath( );
    }

    target = correctedFileName;

    qDebug( "target: %s", target.toStdString().c_str());
}

// Check command line arguments and update corresponding application options
void ApplicationOptions::ParseCommandLineArguments( )
{
    QStringList arguments = QCoreApplication::arguments( );

    for ( QStringList::const_iterator it = arguments.begin( ); it != arguments.end( ); it++ )
    {
        if ( ( ( it->startsWith( '/' ) ) || ( it->startsWith( '-' ) ) ) && ( it->size( ) > 1 ) )
        {
            QString option = it->mid( 1 );

            if ( ( option.startsWith( "o:" ) ) && ( option.size( ) > 2 ) )
            {
                ProjectObjectToStart = option.mid( 2 );
            }
            else if ( ( option.startsWith( "pf:" ) ) && ( option.size( ) > 3 ) )
            {
                TryFileName( option.mid( 3 ), "xml", ProjectFileName );
            }
            else if ( ( option.startsWith( "appf:" ) ) && ( option.size( ) > 5 ) )
            {
                TryFileName( option.mid( 5 ), "ini", ApplicationSettingsFileName );
            }
            else if ( ( option.startsWith( "uif:" ) ) && ( option.size( ) > 4 ) )
            {
                TryFileName( option.mid( 4 ), "ini", UiPersistenceFileName );
            }
            else if ( option == "fs" )
            {
                StartFullScreen = true;
            }
            else if ( option == "fit" )
            {
                FitCamerasToScreen = true;
            }
            else if ( option == "hpo" )
            {
                HideProjectObjectTree = true;
            }
            else if ( option == "ns" )
            {
                NoSystemSleep = true;
            }
        }
    }
}

} // namespace Private
