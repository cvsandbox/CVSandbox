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

#include "SandboxProjectObject.hpp"
#include "XGuidGenerator.hpp"
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

#include "CamerasViewConfiguration.hpp"
#include "ProjectObjectSerializationHelper.hpp"

using namespace std;
using namespace CVSandbox;
using namespace CVSandbox::Automation;

static const QString STR_DEVICES      = QString::fromUtf8( "Devices" );
static const QString STR_DEVICE       = QString::fromUtf8( "Device" );
static const QString STR_THREADS      = QString::fromUtf8( "Threads" );
static const QString STR_VIEWS        = QString::fromUtf8( "Views" );
static const QString STR_VIEW         = QString::fromUtf8( "View" );
static const QString STR_DEFAULT_VIEW = QString::fromUtf8( "DefaultView" );
static const QString STR_ID           = QString::fromUtf8( "Id" );
static const QString STR_REPORT_FPS   = QString::fromUtf8( "ReportFps" );
static const QString STR_NAME         = QString::fromUtf8( "Name" );
static const QString STR_ROW          = QString::fromUtf8( "Row" );
static const QString STR_COLUMN       = QString::fromUtf8( "Column" );
static const QString STR_ROWS         = QString::fromUtf8( "Rows" );
static const QString STR_COLUMNS      = QString::fromUtf8( "Columns" );
static const QString STR_CELL_WIDTH   = QString::fromUtf8( "CellWidth" );
static const QString STR_CELL_HEIGHT  = QString::fromUtf8( "CellHeight" );
static const QString STR_CELL         = QString::fromUtf8( "Cell" );
static const QString STR_ROW_SPAN     = QString::fromUtf8( "RowSpan" );
static const QString STR_COL_SPAN     = QString::fromUtf8( "ColSpan" );

SandboxProjectObject::SandboxProjectObject( const XGuid& id,
        const string& name, const string& description, const XGuid& parentId ) :
    ProjectObject( ProjectObjectType::Sandbox, id, name, description, parentId ),
    mDevices( ), mDevicesFpsReport( ), mViews( ), mDefaultViewId( ), mSandboxSettings( ),
    mScriptingThreads( ), mProcessingGraphs( )
{
}

shared_ptr<SandboxProjectObject> SandboxProjectObject::CreateNew(
        const string& name, const string& description, const XGuid& parentId )
{
    return shared_ptr<SandboxProjectObject>( new SandboxProjectObject( XGuidGenerator::Generate( ), name, description, parentId ) );
}

// Get/Set device list for the sandbox
vector<XGuid> SandboxProjectObject::GetSandboxDevices( ) const
{
    return mDevices;
}
void SandboxProjectObject::SetSandboxDevices( const vector<XGuid>& devices )
{
    mDevices = devices;
}

// Get/Set FPS report state for the devices of the sandbox
map<XGuid, bool> SandboxProjectObject::GetDeviceFpsReportState( ) const
{
    return mDevicesFpsReport;
}
void SandboxProjectObject::SetDeviceFpsReportState( map<XGuid, bool> fpsState )
{
    mDevicesFpsReport = fpsState;
}

// Get/Set list of views
vector<CamerasViewConfiguration> SandboxProjectObject::GetViews( ) const
{
    return mViews;
}
void SandboxProjectObject::SetViews( const vector<CamerasViewConfiguration>& views )
{
    mViews = views;
}

// Get/Set default view ID
XGuid SandboxProjectObject::DefaultViewId( ) const
{
    return mDefaultViewId;
}
void SandboxProjectObject::SetDefaultViewId( const XGuid& viewId )
{
    mDefaultViewId = viewId;
}

// Get/Set sanbox settings
SandboxSettings SandboxProjectObject::Settings( ) const
{
    return mSandboxSettings;
}
void SandboxProjectObject::SetSettings( const SandboxSettings& settings )
{
    mSandboxSettings = settings;
}

// Get/Set video processing graphs
map<XGuid, XVideoSourceProcessingGraph> SandboxProjectObject::GetCamerasProcessingGraphs( ) const
{
    return mProcessingGraphs;
}
void SandboxProjectObject::SetCamerasProcessingGraphs( const map<XGuid, XVideoSourceProcessingGraph>& processingGraphs )
{
    mProcessingGraphs = processingGraphs;
}

// Get/set scripting threads configuration
map<XGuid, ScriptingThreadDesc> SandboxProjectObject::GetScriptingThreads( ) const
{
    return mScriptingThreads;
}
void SandboxProjectObject::SetScriptingThreads( const map<XGuid, ScriptingThreadDesc>& threads )
{
    mScriptingThreads = threads;
}

// Save the sandbox project object
void SandboxProjectObject::Save( QXmlStreamWriter& xmlWriter ) const
{
    ProjectObject::Save( xmlWriter );

    xmlWriter.writeAttribute( STR_DEFAULT_VIEW, mDefaultViewId.ToString( ).c_str( ) );

    // write all sandbox devices
    xmlWriter.writeStartElement( STR_DEVICES );
    SaveDevices( xmlWriter );
    xmlWriter.writeEndElement( );

    // write all configured threads, if any
    if ( !mScriptingThreads.empty( ) )
    {
        xmlWriter.writeStartElement( STR_THREADS );
        ProjectObjectSerializationHelper::SaveScriptingThreads( xmlWriter, mScriptingThreads );
        xmlWriter.writeEndElement( );
    }

    // write all views
    xmlWriter.writeStartElement( STR_VIEWS );
    for ( vector<CamerasViewConfiguration>::const_iterator it = mViews.begin( ); it != mViews.end( ); it++ )
    {
        const CamerasViewConfiguration& view         = *it;
        int                             rowsCount    = view.RowsCount( );
        int                             columnsCount = view.ColumnsCount( );

        xmlWriter.writeStartElement( STR_VIEW );

        xmlWriter.writeAttribute( STR_ID, view.Id( ).ToString( ).c_str( ) );
        xmlWriter.writeAttribute( STR_NAME, QString::fromUtf8( view.Name( ).c_str( ) ) );
        xmlWriter.writeAttribute( STR_ROWS, QString( "%1" ).arg( rowsCount ) );
        xmlWriter.writeAttribute( STR_COLUMNS, QString( "%1" ).arg( columnsCount ) );
        xmlWriter.writeAttribute( STR_CELL_WIDTH, QString( "%1" ).arg( view.CellWidth( ) ) );
        xmlWriter.writeAttribute( STR_CELL_HEIGHT, QString( "%1" ).arg( view.CellHeight( ) ) );

        for ( int trow = 0; trow < rowsCount; trow++ )
        {
            for ( int tcol = 0; tcol < columnsCount; tcol++ )
            {
                const CameraCellConfiguration* cell = view.GetCellConfiguration( trow, tcol );

                // write cell info
                xmlWriter.writeStartElement( STR_CELL );
                xmlWriter.writeAttribute( STR_ROW, QString( "%1" ).arg( trow ) );
                xmlWriter.writeAttribute( STR_COLUMN, QString( "%1" ).arg( tcol ) );

                if ( cell != 0 )
                {
                    xmlWriter.writeAttribute( STR_ROW_SPAN, QString( "%1" ).arg( cell->RowSpan( ) ) );
                    xmlWriter.writeAttribute( STR_COL_SPAN, QString( "%1" ).arg( cell->ColumnSpan( ) ) );
                    xmlWriter.writeAttribute( STR_DEVICE, QString( "%1" ).arg( cell->CameraId( ).ToString( ).c_str( ) ) );
                }

                xmlWriter.writeEndElement( );
            }
        }

        xmlWriter.writeEndElement( );
    }
    xmlWriter.writeEndElement( );

    // save sandbox settings
    mSandboxSettings.Save( xmlWriter );
}

// Load the sandbox project object
bool SandboxProjectObject::Load( QXmlStreamReader& xmlReader )
{
    QXmlStreamAttributes xmlAttrs = xmlReader.attributes( );
    bool                 ret      = ProjectObject::Load( xmlReader );

    mDevices.clear( );
    mViews.clear( );

    if ( ret )
    {
        QStringRef strRef;

        // default view id
        strRef = xmlAttrs.value( STR_DEFAULT_VIEW );
        if ( !strRef.isEmpty( ) )
        {
            XGuid defaultViewId = XGuid::FromString( strRef.toString( ).toStdString( ) );
            SetDefaultViewId( defaultViewId );
        }

        while ( xmlReader.readNextStartElement( ) )
        {
            if ( xmlReader.name( ) == STR_DEVICES )
            {
                ret &= LoadDevices( xmlReader );
            }
            else if ( xmlReader.name( ) == STR_THREADS )
            {
                if ( !ProjectObjectSerializationHelper::LoadScriptingThreads( xmlReader, mScriptingThreads ) )
                {
                    ProjectObjectSerializationHelper::FindEndElement( xmlReader, STR_THREADS );
                }
            }
            else if ( xmlReader.name( ) == STR_VIEWS )
            {
                ret &= LoadViews( xmlReader );
            }
            else if ( xmlReader.name( ) == SandboxSettings::XmlTagName( ) )
            {
                ret &= mSandboxSettings.Load( xmlReader );
            }

            xmlReader.readNext( );
        }
    }

    return ret;
}

// Load devices of the sandbox and any related configuration
void SandboxProjectObject::SaveDevices( QXmlStreamWriter& xmlWriter ) const
{
    for ( vector<XGuid>::const_iterator deviceIt = mDevices.begin( ); deviceIt != mDevices.end( ); ++deviceIt )
    {
        map<XGuid, bool>::const_iterator itFpsState = mDevicesFpsReport.find( *deviceIt );
        bool fpsReportState = true;

        if ( itFpsState != mDevicesFpsReport.end( ) )
        {
            fpsReportState = itFpsState->second;
        }

        xmlWriter.writeStartElement( STR_DEVICE );
        xmlWriter.writeAttribute( STR_ID, deviceIt->ToString( ).c_str( ) );
        xmlWriter.writeAttribute( STR_REPORT_FPS, ( fpsReportState ) ? "1" : "0" );

        // save video processing graph
        map<XGuid, XVideoSourceProcessingGraph>::const_iterator graphIt = mProcessingGraphs.find( *deviceIt );

        if ( ( graphIt != mProcessingGraphs.end( ) ) && ( graphIt->second.StepsCount( ) != 0 ) )
        {
            ProjectObjectSerializationHelper::SaveVideoProcessingGraph( xmlWriter, graphIt->second );
        }

        xmlWriter.writeEndElement( );
    }
}

// Load IDs of devices used in the sandbox
bool SandboxProjectObject::LoadDevices( QXmlStreamReader& xmlReader )
{
    bool ret = true;

    while ( xmlReader.readNextStartElement( ) )
    {
        if ( xmlReader.name( ) == STR_DEVICE )
        {
            QStringRef strRefId        = xmlReader.attributes( ).value( STR_ID );
            QStringRef strRefReportFps = xmlReader.attributes( ).value( STR_REPORT_FPS );
            XGuid      deviceId        = XGuid::FromString( strRefId.toString( ).toStdString( ) );
            bool       reportFps       = true;

            if ( !deviceId.IsEmpty( ) )
            {
                if ( !strRefReportFps.isEmpty( ) )
                {
                    reportFps = ( strRefReportFps.toString( ) == "1" );
                }

                mDevices.push_back( deviceId );
                mDevicesFpsReport.insert( pair<XGuid, bool>( deviceId, reportFps ) );

                // read graph
                if ( xmlReader.readNextStartElement( ) )
                {
                    XVideoSourceProcessingGraph processingGraph;

                    if ( ProjectObjectSerializationHelper::LoadVideoProcessingGraph( xmlReader, processingGraph ) )
                    {
                        mProcessingGraphs.insert( pair<XGuid, XVideoSourceProcessingGraph>( deviceId, processingGraph ) );
                    }
                }
            }
            ProjectObjectSerializationHelper::FindEndElement( xmlReader, STR_DEVICE );
        }
        else
        {
            ret = false;
            ProjectObjectSerializationHelper::FindEndElement( xmlReader, STR_DEVICE );
        }
    }

    return ret;
}

// Load view configurations used in the sandbox
bool SandboxProjectObject::LoadViews( QXmlStreamReader& xmlReader )
{
    bool ret = true;

    while ( ( xmlReader.readNextStartElement( ) ) && ( ret ) )
    {
        if ( xmlReader.name( ) == STR_VIEW )
        {
            QXmlStreamAttributes     xmlAttrs     = xmlReader.attributes( );
            CamerasViewConfiguration view;
            QStringRef               strRef;
            XGuid                    viewId;
            int                      rowsCount    = 0;
            int                      columnsCount = 0;

            // view id
            strRef = xmlAttrs.value( STR_ID );
            if ( !strRef.isEmpty( ) )
            {
                viewId = XGuid::FromString( strRef.toString( ).toStdString( ) );
            }

            // rows count
            strRef = xmlAttrs.value( STR_ROWS );
            if ( !strRef.isEmpty( ) )
            {
                rowsCount = strRef.toString( ).toInt( );
            }

            // columns count
            strRef = xmlAttrs.value( STR_COLUMNS );
            if ( !strRef.isEmpty( ) )
            {
                columnsCount = strRef.toString( ).toInt( );
            }

            if ( ( viewId.IsEmpty( ) ) || ( rowsCount == 0 ) || ( columnsCount == 0 ) )
            {
                ret = false;
                break;
            }

            // create view
            view = CamerasViewConfiguration( viewId, rowsCount, columnsCount );

            // view's name
            view.SetName( xmlAttrs.value( STR_NAME ).toString( ).toUtf8( ).data( ) );

            // cell width
            strRef = xmlAttrs.value( STR_CELL_WIDTH );
            if ( !strRef.isEmpty( ) )
            {
                view.SetCellWidth( strRef.toString( ).toInt( ) );
            }

            strRef = xmlAttrs.value( STR_CELL_HEIGHT );
            if ( !strRef.isEmpty( ) )
            {
                view.SetCellHeight( strRef.toString( ).toInt( ) );
            }

            // check if we have cells
            xmlReader.readNext( );
            if ( ( xmlReader.name( ) != STR_VIEW ) || ( xmlReader.tokenType( ) != QXmlStreamReader::EndElement ) )
            {
                // read cells of the view
                while ( xmlReader.readNextStartElement( ) )
                {
                    if ( xmlReader.name( ) == STR_CELL )
                    {
                        int   row        = -1;
                        int   column     = -1;
                        int   rowSpan    = 1;
                        int   columnSpan = 1;
                        XGuid deviceId;

                        xmlAttrs = xmlReader.attributes( );

                        // row
                        strRef = xmlAttrs.value( STR_ROW );
                        if ( !strRef.isEmpty( ) )
                        {
                            row = strRef.toString( ).toInt( );
                        }

                        // column
                        strRef = xmlAttrs.value( STR_COLUMN );
                        if ( !strRef.isEmpty( ) )
                        {
                            column = strRef.toString( ).toInt( );
                        }

                        // row span
                        strRef = xmlAttrs.value( STR_ROW_SPAN );
                        if ( !strRef.isEmpty( ) )
                        {
                            rowSpan = strRef.toString( ).toInt( );
                        }

                        // row span
                        strRef = xmlAttrs.value( STR_COL_SPAN );
                        if ( !strRef.isEmpty( ) )
                        {
                            columnSpan = strRef.toString( ).toInt( );
                        }

                        // device id
                        strRef = xmlAttrs.value( STR_DEVICE );
                        if ( !strRef.isEmpty( ) )
                        {
                            deviceId = XGuid::FromString( strRef.toString( ).toStdString( ) );
                        }

                        if ( ( row == -1 ) || ( column == -1 ) || ( rowSpan == 0 ) || ( columnSpan == 0 ) )
                        {
                            ret = false;
                            break;
                        }

                        // set cell configuration
                        CameraCellConfiguration* cell = view.GetCellConfiguration( row, column );
                        if ( cell == 0 )
                        {
                            ret = false;
                        }
                        else
                        {
                            cell->SetRowSpan( rowSpan );
                            cell->SetColumnSpan( columnSpan );
                            cell->SetCameraId( deviceId );
                        }

                        xmlReader.readNext( );
                    }
                    else
                    {
                        xmlReader.skipCurrentElement( );
                    }
                }
            }
            else
            {
                xmlReader.skipCurrentElement( );
            }

            // add the view to the collection
            mViews.push_back( view );
        }
        else
        {
            xmlReader.skipCurrentElement( );
        }
    }

    return ret;
}
