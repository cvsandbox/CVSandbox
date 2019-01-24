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

#include "ConfigureCamerasViewPage.hpp"
#include "ui_ConfigureCamerasViewPage.h"

#include <map>
#include <QMenu>
#include <ErrorProviderHelper.hpp>

#include "ServiceManager.hpp"
#include "ProjectObjectIcon.hpp"
#include "XGuidGenerator.hpp"
#include "DeviceListWidgetItem.hpp"

using namespace std;
using namespace CVSandbox;

// Defines maximum number of views in a sandbox
#define MAX_VIEWS_NUMBER 9

namespace Private
{
    class ConfigureCamerasViewPageData
    {
    public:
        ConfigureCamerasViewPageData( Ui::ConfigureCamerasViewPage* ui ) :
            Ui( ui ),
            SandboxCameras( ), ViewConfigurations( ), ViewConfiguration( ), DefaultViewId( ),
            RowForContextMenu( -1 ), ColumnForContextMenu( -1 ),

            GridContextMenu( 0 ), MergeDownAndRightAction( 0 ),
            MergeDownAction( 0 ), MergeRightAction( 0 ),
            SplitAction( 0 )
        {
        }

        ~ConfigureCamerasViewPageData( )
        {
            delete GridContextMenu;
        }

        // Get index of the specified camera's ID in the list of available cameras
        int GetCameraIndex( const XGuid& id )
        {
            int counter = 0;
            int ret     = -1;

            for ( vector<XGuid>::const_iterator it = SandboxCameras.begin( ); it != SandboxCameras.end( ); it++ )
            {
                if ( id == *it )
                {
                    ret = counter;
                    break;
                }
                counter++;
            }

            return ret;
        }

        // Make sure Dafault View Id exists in the collection of views
        void EnsureValidDefaultViewId( )
        {
            if ( !ViewConfigurations.empty( ) )
            {
                bool found = false;

                for ( map<XGuid, CamerasViewConfiguration>::const_iterator it = ViewConfigurations.begin( ); it != ViewConfigurations.end( ); it++ )
                {
                    if ( it->first == DefaultViewId )
                    {
                        found = true;
                        break;
                    }
                }

                if ( !found )
                {
                    // assign default view to anything which is valid
                    DefaultViewId = ViewConfigurations.begin( )->first;
                }
            }
        }

        // Remove camera ID from any hidden cell
        void CleanHiddenCells( )
        {
            for ( int row = 0, rows = ViewConfiguration.RowsCount( ); row < rows; row++ )
            {
                for ( int col = 0, columns = ViewConfiguration.ColumnsCount( ); col < columns; col++ )
                {
                    if ( !ViewConfiguration.IsCellVisible( row, col ) )
                    {
                        CleanCell( row, col );
                    }
                }
            }
        }

        // Clear the specified cell
        void CleanCell( int row, int column )
        {
            CameraCellConfiguration* cellConfig = ViewConfiguration.GetCellConfiguration( row, column );

            if ( cellConfig != 0 )
            {
                cellConfig->SetCameraId( XGuid( ) );
                Ui->gridWidget->SetCellText( row, column, QString::null );
            }
        }

    public:
        Ui::ConfigureCamerasViewPage*        Ui;
        vector<XGuid>                        SandboxCameras;
        map<XGuid, CamerasViewConfiguration> ViewConfigurations;
        CamerasViewConfiguration             ViewConfiguration;
        XGuid                                DefaultViewId;

        int         RowForContextMenu;
        int         ColumnForContextMenu;

        QMenu*      GridContextMenu;
        QAction*    MergeDownAndRightAction;
        QAction*    MergeDownAction;
        QAction*    MergeRightAction;
        QAction*    SplitAction;
        QAction*    ClearCellAction;

        ErrorProviderHelper ErrorProvider;
    };
}

ConfigureCamerasViewPage::ConfigureCamerasViewPage( QWidget* parent ) :
    WizardPageFrame( parent ),
    ui( new Ui::ConfigureCamerasViewPage ),
    mData( new Private::ConfigureCamerasViewPageData( ui ) )
{
    ui->setupUi( this );
    ui->gridWidget->EnableCellHighlight( true );

    // set custom background colors for the items of the available devices list
    ui->availableCameraList->setStyleSheet(
        "QListWidget::item {"
           "background-color: #F6F6F6;"
        "}"
        "QListWidget::item:selected {"
           "background-color: #80D8E9;"
        "}"
        "QListWidget::item:hover {"
           "background-color: #E8F7Fb;"
        "}"
        "QListWidget::item:selected:hover {"
           "background-color: #B9DEEF;"
        "}"
    );

    // create context menu for grid widget
    mData->GridContextMenu = new QMenu( this );
    mData->MergeDownAndRightAction = mData->GridContextMenu->addAction( "Merge down and right" );
    mData->MergeDownAction = mData->GridContextMenu->addAction( "Merge down" );
    mData->MergeRightAction = mData->GridContextMenu->addAction( "Merge right" );
    mData->GridContextMenu->addSeparator( );
    mData->SplitAction = mData->GridContextMenu->addAction( "Split the cell" );
    mData->GridContextMenu->addSeparator( );
    mData->ClearCellAction = mData->GridContextMenu->addAction( "Clear the cell" );

    // disable vertical/horizontal merging for the 1.0.0 version
    mData->MergeDownAction->setVisible( false );
    mData->MergeRightAction->setVisible( false );
    // ---------------------------------------------------------

    // setup action handlers
    connect( mData->GridContextMenu, SIGNAL( aboutToShow( ) ), this, SLOT( on_gridContextMenu_aboutToShow( ) ) );
    connect( mData->MergeDownAndRightAction, SIGNAL( triggered( ) ), this, SLOT( on_mergeDownAndRightAction_triggered( ) ) );
    connect( mData->MergeDownAction, SIGNAL( triggered( ) ), this, SLOT( on_mergeDownAction_triggered( ) ) );
    connect( mData->MergeRightAction, SIGNAL( triggered( ) ), this, SLOT( on_mergeRightAction_triggered( ) ) );
    connect( mData->SplitAction, SIGNAL( triggered( ) ), this, SLOT( on_splitAction_triggered( ) ) );
    connect( mData->ClearCellAction, SIGNAL( triggered( ) ), this, SLOT( on_clearCellAction_triggered( ) ) );
}

ConfigureCamerasViewPage::~ConfigureCamerasViewPage( )
{
    delete mData;
    delete ui;
}

bool ConfigureCamerasViewPage::CanGoNext( ) const
{
    return ( !ui->viewNameEdit->text( ).isEmpty( ) );
}

// Set list of cameras available for view configuration
void ConfigureCamerasViewPage::SetAvalableCameras( const vector<XGuid>& availableCameras )
{
    shared_ptr<IProjectManager> projectManager = ServiceManager::Instance( ).GetProjectManager( );
    int                         counter        = 1;

    mData->SandboxCameras.clear( );
    ui->availableCameraList->clear( );

    for ( vector<XGuid>::const_iterator it = availableCameras.begin( ); it != availableCameras.end( ); it++ )
    {
        const XGuid               objectId      = *it;
        shared_ptr<ProjectObject> projectObject = projectManager->GetProjectObject( objectId );

        if ( ( projectObject ) && ( projectObject->Type( ) == ProjectObjectType::Camera ) )
        {
            string                fullName   = projectManager->GetProjectObjectFullName( projectObject );
            string                objectPath = projectManager->GetProjectObjectPath( projectObject );
            DeviceListWidgetItem* deviceItem = new DeviceListWidgetItem(
                                                    QString::fromUtf8( objectPath.c_str( ) ),
                                                    QString( "%1 : " ).arg( counter ) + QString::fromUtf8( projectObject->Name( ).c_str( ) ),
                                                    QString::fromUtf8( fullName.c_str( ) ) );
            QListWidgetItem*      listItem   = new QListWidgetItem( );
            QString               iconName   = ProjectObjectIcon::GetResourceName( projectObject->Type( ) );

            listItem->setData( Qt::UserRole, objectId.ToString( ).c_str( ) );
            listItem->setSizeHint( QSize( 0, 40 ) );

            if ( !iconName.isEmpty( ) )
            {
                deviceItem->setIcon( QIcon( iconName ) );
            }

            // add to UI and internal lists
            ui->availableCameraList->addItem( listItem );
            ui->availableCameraList->setItemWidget( listItem, deviceItem );
            mData->SandboxCameras.push_back( objectId );

            counter++;
        }
    }

    ShowCurrentViewLayout( );
}

// Add item into views' combo box for the specified view configuration
void ConfigureCamerasViewPage::AddViewsComboItem( const CamerasViewConfiguration& view, bool select )
{
    QVariant userData = QVariant::fromValue( QString::fromStdString( view.Id( ).ToString( ) ) );

    ui->viewsComboBox->addItem( QString::fromUtf8( view.Name( ).c_str( ) ), userData );

    if ( select )
    {
        ui->viewsComboBox->setCurrentIndex( ui->viewsComboBox->findData( userData ) );
    }
}

// Set configuration of the view
void ConfigureCamerasViewPage::SetViewConfigurations( const vector<CamerasViewConfiguration>& viewConfigurations )
{
    ui->viewsComboBox->clear( );

    if ( viewConfigurations.empty( ) )
    {
        // should not really happen unless we failed to read saved project file
        CamerasViewConfiguration view( XGuidGenerator::Generate( ), 1, 1 );

        mData->ViewConfigurations.insert( pair<XGuid,CamerasViewConfiguration>( view.Id( ), view ) );
        AddViewsComboItem(view );
    }
    else
    {
        for ( vector<CamerasViewConfiguration>::const_iterator it = viewConfigurations.begin( );
              it != viewConfigurations.end( ); it++ )
        {
            mData->ViewConfigurations.insert( pair<XGuid,CamerasViewConfiguration>( it->Id( ), *it ) );
            AddViewsComboItem( *it );
        }
    }

    mData->EnsureValidDefaultViewId( );

    ui->viewsComboBox->model( )->sort( 0 );
    ui->viewsComboBox->setCurrentIndex( 0 );
    UpdateViewButtonsStatus( );
    UpdateDefaultViewStatus( );
}

// Get configuration of the view
const vector<CamerasViewConfiguration> ConfigureCamerasViewPage::GetViewConfigurations( ) const
{
    if ( !mData->ViewConfiguration.Id( ).IsEmpty( ) )
    {
        mData->ViewConfigurations[mData->ViewConfiguration.Id( )] = mData->ViewConfiguration;
    }

    vector<CamerasViewConfiguration> ret;
    ret.reserve( mData->ViewConfigurations.size( ) );

    for ( map<XGuid, CamerasViewConfiguration>::const_iterator it = mData->ViewConfigurations.begin( );
          it != mData->ViewConfigurations.end( ); it++ )
    {
        ret.push_back( it->second );
    }

    return ret;
}

// Get/Set default view ID
const XGuid ConfigureCamerasViewPage::DefaultViewId( ) const
{
    return mData->DefaultViewId;
}
void ConfigureCamerasViewPage::SetDefaultViewId( const XGuid& viewId )
{
    mData->DefaultViewId = viewId;
    mData->EnsureValidDefaultViewId( );
    UpdateViewButtonsStatus( );
    UpdateDefaultViewStatus( );
}

// Current view has changed
void ConfigureCamerasViewPage::on_viewsComboBox_currentIndexChanged( int index )
{
    if ( !mData->ViewConfiguration.Id( ).IsEmpty( ) )
    {
        mData->ViewConfigurations[mData->ViewConfiguration.Id( )] = mData->ViewConfiguration;
    }

    mData->ViewConfiguration = CamerasViewConfiguration( );

    // get the index of the selected sandbox
    if ( ( index >= 0 ) && ( index < ui->viewsComboBox->count( ) ) )
    {
         XGuid selectedViewId = XGuid::FromString( ui->viewsComboBox->itemData( index ).toString( ).toStdString( ) );
         map<XGuid, CamerasViewConfiguration>::const_iterator it = mData->ViewConfigurations.find( selectedViewId );

         if  ( it != mData->ViewConfigurations.end( ) )
         {
             mData->ViewConfiguration = it->second;
         }
    }

    if ( !mData->ViewConfiguration.Id( ).IsEmpty( ) )
    {
        int rowsCount    = mData->ViewConfiguration.RowsCount( );
        int columnsCount = mData->ViewConfiguration.ColumnsCount( );

        ui->viewNameEdit->setText( QString::fromUtf8( mData->ViewConfiguration.Name( ).c_str( ) ) );
        ui->rowSpinBox->setValue( rowsCount );
        ui->columnsSpinBox->setValue( columnsCount );
        ui->cellWidthSpinBox->setValue( mData->ViewConfiguration.CellWidth( ) );
        ui->cellHeightSpinBox->setValue( mData->ViewConfiguration.CellHeight( ) );

        ShowCurrentViewLayout( );
    }

    UpdateViewButtonsStatus( );
}

// Set number of rows in the view's grid
void ConfigureCamerasViewPage::on_rowSpinBox_valueChanged( int arg1 )
{
    ui->gridWidget->SetRowsCount( arg1 );
    mData->ViewConfiguration.SetViewSize( arg1, mData->ViewConfiguration.ColumnsCount( ) );
}

// Set number of columns in the view's grid
void ConfigureCamerasViewPage::on_columnsSpinBox_valueChanged( int arg1 )
{
    ui->gridWidget->SetColumnsCount( arg1 );
    mData->ViewConfiguration.SetViewSize( mData->ViewConfiguration.RowsCount( ), arg1 );
}

// Set view's cell width
void ConfigureCamerasViewPage::on_cellWidthSpinBox_valueChanged( int arg1 )
{
    mData->ViewConfiguration.SetCellWidth( arg1 );
}

// Set view's cell height
void ConfigureCamerasViewPage::on_cellHeightSpinBox_valueChanged( int arg1 )
{
    mData->ViewConfiguration.SetCellHeight( arg1 );
}

// On starting dragging in the list of available cameras
void ConfigureCamerasViewPage::on_availableCameraList_draggingAboutToStart( ListDraggingStartEvent* event )
{
    event->SetUrl( QUrl( event->ListItem( )->data( Qt::UserRole ).toString( ) ) );
    event->SetCanStartDragging( true );
}

// Context menu is requested for the view's grid
void ConfigureCamerasViewPage::on_gridWidget_cellContextMenuRequested( int row, int column, const QPoint& globalPos )
{
    mData->RowForContextMenu    = row;
    mData->ColumnForContextMenu = column;
    mData->GridContextMenu->popup( globalPos );
}

// An item is dragged into layout grid
void ConfigureCamerasViewPage::on_gridWidget_dropTargetCheck( GridDropTargetCheckEvent* event )
{
    XGuid             itemId = XGuid( event->Url( ).toString( ).toStdString( ) );
    ProjectObjectType type   = ServiceManager::Instance( ).GetProjectManager( )->GetProjectObjectType( itemId );

    event->SetIsUrlAcceptable( ( type != ProjectObjectType::None ) ? true : false );
    event->SetCanTargetAccept( ( type == ProjectObjectType::Camera ) ? true : false );
}

// An item is dropped in layout grid
void ConfigureCamerasViewPage::on_gridWidget_itemDropped( GridItemDroppedEvent* event )
{
    XGuid                    itemId     = XGuid( event->Url( ).toString( ).toStdString( ) );
    int                      index      = mData->GetCameraIndex( itemId );
    CameraCellConfiguration* cellConfig = mData->ViewConfiguration.GetCellConfiguration( event->Row( ), event->Column( ) );

    if ( ( index != -1 ) && ( cellConfig != 0 ) )
    {
        cellConfig->SetCameraId( itemId );

        ui->gridWidget->SetCellText( event->Row( ), event->Column( ),
                                     QString( "%1" ).arg( index + 1 ) );
    }
}

// Grid cell's context menu is about to show - update items
void ConfigureCamerasViewPage::on_gridContextMenu_aboutToShow( )
{
    int                      row        = mData->RowForContextMenu;
    int                      column     = mData->ColumnForContextMenu;
    CameraCellConfiguration* cellConfig = mData->ViewConfiguration.GetCellConfiguration( row, column );

    mData->MergeDownAction->setEnabled( ui->gridWidget->CanMergeDown( row, column ) );
    mData->MergeRightAction->setEnabled( ui->gridWidget->CanMergeRight( row, column ) );
    mData->MergeDownAndRightAction->setEnabled( ui->gridWidget->CanMergeDownAndRight( row, column ) );
    mData->SplitAction->setEnabled( ui->gridWidget->CanSplit( row, column ) );
    mData->ClearCellAction->setEnabled( ( cellConfig != 0 ) && ( !cellConfig->CameraId( ).IsEmpty( ) ) );
}

// Merge selected cell with the bottom and right ones
void ConfigureCamerasViewPage::on_mergeDownAndRightAction_triggered( )
{
    int                      row        = mData->RowForContextMenu;
    int                      column     = mData->ColumnForContextMenu;
    CameraCellConfiguration* cellConfig = mData->ViewConfiguration.GetCellConfiguration( row, column );

    if ( ( cellConfig != 0 ) && ( ui->gridWidget->CanMergeDownAndRight( row, column ) ) )
    {
        cellConfig->SetRowSpan( cellConfig->RowSpan( ) + 1 );
        cellConfig->SetColumnSpan( cellConfig->ColumnSpan( ) + 1 );
        ui->gridWidget->MergeDownAndRight( row, column );
        mData->CleanHiddenCells( );
    }
}

// Merge selected cell with the bottom one
void ConfigureCamerasViewPage::on_mergeDownAction_triggered( )
{
    int                      row        = mData->RowForContextMenu;
    int                      column     = mData->ColumnForContextMenu;
    CameraCellConfiguration* cellConfig = mData->ViewConfiguration.GetCellConfiguration( row, column );

    if ( ( cellConfig != 0 ) && ( ui->gridWidget->CanMergeDown( row, column ) ) )
    {
        cellConfig->SetRowSpan( cellConfig->RowSpan( ) + 1 );
        ui->gridWidget->MergeDown( row, column );
        mData->CleanHiddenCells( );
    }
}

// Merge selected cell with the right one
void ConfigureCamerasViewPage::on_mergeRightAction_triggered( )
{
    int                      row        = mData->RowForContextMenu;
    int                      column     = mData->ColumnForContextMenu;
    CameraCellConfiguration* cellConfig = mData->ViewConfiguration.GetCellConfiguration( row, column );

    if ( ( cellConfig != 0 ) && ( ui->gridWidget->CanMergeRight( row, column ) ) )
    {
        cellConfig->SetColumnSpan( cellConfig->ColumnSpan( ) + 1 );
        ui->gridWidget->MergeRight( row, column );
        mData->CleanHiddenCells( );
    }
}

// Split cell removing any merging on it
void ConfigureCamerasViewPage::on_splitAction_triggered( )
{
    int                      row        = mData->RowForContextMenu;
    int                      column     = mData->ColumnForContextMenu;
    CameraCellConfiguration* cellConfig = mData->ViewConfiguration.GetCellConfiguration( row, column );

    if ( ( cellConfig != 0 ) && ( ui->gridWidget->CanSplit( row, column ) ) )
    {
        cellConfig->SetRowSpan( 1 );
        cellConfig->SetColumnSpan( 1 );
        ui->gridWidget->Split( mData->RowForContextMenu, mData->ColumnForContextMenu );
    }
}

// Clear the cell - remove camera set to it
void ConfigureCamerasViewPage::on_clearCellAction_triggered( )
{
    mData->CleanCell( mData->RowForContextMenu, mData->ColumnForContextMenu );
}

// View name has changed
void ConfigureCamerasViewPage::on_viewNameEdit_textChanged( const QString& arg1 )
{
    string viewName( arg1.trimmed( ).toUtf8( ).data( ) );
    int    currenIndex = ui->viewsComboBox->currentIndex( );

    emit CompleteStatusChanged( !viewName.empty( ) );

    // update view name in the combo box
    if ( ( currenIndex >= 0 ) && ( currenIndex < ui->viewsComboBox->count( ) ) )
    {
        ui->viewsComboBox->setItemText( ui->viewsComboBox->currentIndex( ), QString::fromUtf8( viewName.c_str( ) ) );
    }
    // update view name in the configuration
    if ( !mData->ViewConfiguration.Id( ).IsEmpty( ) )
    {
        mData->ViewConfiguration.SetName( viewName );
    }

    // signal error on empty name
    if ( viewName.empty( ) )
    {
        mData->ErrorProvider.SetError( "", ui->viewNameEdit );
    }
    else
    {
        mData->ErrorProvider.ClearError( );
    }
}

// Add new view to the sandbox
void ConfigureCamerasViewPage::on_addViewButton_clicked( )
{
    int viewsCount   = mData->ViewConfigurations.size( );
    int rowsCount    = 2;
    int columnsCount = 2;
    int cellWidth    = 320;
    int cellHeight   = 240;

    if ( !mData->ViewConfiguration.Id( ).IsEmpty( ) )
    {
        rowsCount    = mData->ViewConfiguration.RowsCount( );
        columnsCount = mData->ViewConfiguration.ColumnsCount( );
        cellWidth    = mData->ViewConfiguration.CellWidth( );
        cellHeight   = mData->ViewConfiguration.CellHeight( );
    }

    if ( viewsCount < MAX_VIEWS_NUMBER )
    {
        CamerasViewConfiguration newView( XGuidGenerator::Generate( ), rowsCount, columnsCount );
        char                     viewName[16] = { 0 };

        sprintf( viewName, "%X - View", viewsCount + 1 );

        newView.SetCellWidth( cellWidth );
        newView.SetCellHeight( cellHeight );
        newView.SetName( viewName );

        mData->ViewConfigurations.insert( pair<XGuid, CamerasViewConfiguration>( newView.Id( ), newView ) );
        AddViewsComboItem( newView, true );
        UpdateViewButtonsStatus( );
    }
}

// Remove currently selected view from the sandbox
void ConfigureCamerasViewPage::on_deleteViewButton_clicked( )
{
    int viewsCount    = mData->ViewConfigurations.size( );
    int selectedIndex = ui->viewsComboBox->currentIndex( );

    if ( ( viewsCount > 1 ) && ( selectedIndex != - 1 ) )
    {
        XGuid selectedViewId = XGuid::FromString( ui->viewsComboBox->itemData( selectedIndex ).toString( ).toStdString( ) );

        ui->viewsComboBox->removeItem( selectedIndex );
        mData->ViewConfigurations.erase( selectedViewId );
        UpdateViewButtonsStatus( );
    }
}

// Set selected view as default
void ConfigureCamerasViewPage::on_setDefaultViewButton_clicked( )
{
    int selectedIndex = ui->viewsComboBox->currentIndex( );

    if ( selectedIndex != -1 )
    {
        mData->DefaultViewId = XGuid::FromString( ui->viewsComboBox->itemData( selectedIndex ).toString( ).toStdString( ) );

        UpdateViewButtonsStatus( );
        UpdateDefaultViewStatus( );
    }
}

// Update status of buttons for Adding/Deleting/MakingDefault view
void ConfigureCamerasViewPage::UpdateViewButtonsStatus( )
{
    int viewsCount    = mData->ViewConfigurations.size( );
    int selectedIndex = ui->viewsComboBox->currentIndex( );

    ui->addViewButton->setEnabled( viewsCount < MAX_VIEWS_NUMBER );
    ui->deleteViewButton->setEnabled( viewsCount > 1 );

    if ( selectedIndex != -1 )
    {
        XGuid selectedViewId = XGuid::FromString( ui->viewsComboBox->itemData( selectedIndex ).toString( ).toStdString( ) );

        ui->setDefaultViewButton->setEnabled( selectedViewId != mData->DefaultViewId );
    }
}

// Update views' combo box items to show which view is default
void ConfigureCamerasViewPage::UpdateDefaultViewStatus( )
{
    int     itemsCount = ui->viewsComboBox->count( );
    QFont   normalFont = ui->viewsComboBox->font( );
    QFont   boldFont   = ui->viewsComboBox->font( );

    boldFont.setBold( true );

    for ( int i = 0; i < itemsCount; i++ )
    {
        XGuid viewId = XGuid::FromString( ui->viewsComboBox->itemData( i ).toString( ).toStdString( ) );

         ui->viewsComboBox->setItemData( i,
                                         QVariant( ( viewId == mData->DefaultViewId ) ? boldFont : normalFont ),
                                         Qt::FontRole );
    }
}

// Show layout of the currently selected view configuration
void ConfigureCamerasViewPage::ShowCurrentViewLayout( )
{
    if ( !mData->ViewConfiguration.Id( ).IsEmpty( ) )
    {
        int rowsCount    = mData->ViewConfiguration.RowsCount( );
        int columnsCount = mData->ViewConfiguration.ColumnsCount( );

        mData->ViewConfiguration.SetViewSize( rowsCount, columnsCount );

        for ( int trow = 0; trow < rowsCount; trow++ )
        {
            for ( int tcol = 0; tcol < columnsCount; tcol++ )
            {
                CameraCellConfiguration* cell  = mData->ViewConfiguration.GetCellConfiguration( trow, tcol );
                int                      index = -1;

                if ( cell != 0 )
                {
                    index = mData->GetCameraIndex( cell->CameraId( ) );
                    ui->gridWidget->SetCellSpan( trow, tcol, cell->RowSpan( ), cell->ColumnSpan( ) );
                }

                ui->gridWidget->SetCellText( trow, tcol, ( index == -1 ) ? QString::null :
                                             QString( "%1" ).arg( index + 1 ) );

                if ( index == -1 )
                {
                    // the object is not there, so clean the cell
                    cell->SetCameraId( XGuid( ) );
                }
            }
        }
    }
}
