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

#include "SandboxVariablesMonitorFrame.hpp"
#include "ui_SandboxVariablesMonitorFrame.h"

#include <QMutex>
#include <QMap>
#include <QVector>
#include <QPair>
#include <QList>

#include <XVariantArray.hpp>

using namespace std;
using namespace CVSandbox;
using namespace CVSandbox::Automation;

namespace Private
{
    class VariableData
    {
    public:
        XVariant         Value;
        QTreeWidgetItem* TreeItem;

        VariableData( const XVariant& value ) :
            Value( value ), TreeItem( nullptr )
        {
        }
    };

    typedef QMap<QString, VariableData> VarsMap;
    typedef QList<QString>              NamesList;

    class SandboxVariablesMonitorFrameData
    {
    public:
        QMutex    VarsSync;
        VarsMap   Variables;
        NamesList VarNames;

    public:
        void SetChildItems( const QVector<QPair<QString, QString>>& items,
                            QTreeWidgetItem* parent );
    };
}

using namespace Private;

SandboxVariablesMonitorFrame::SandboxVariablesMonitorFrame( QWidget* parent ) :
    QFrame( parent ),
    ui( new Ui::SandboxVariablesMonitorFrame ),
    mData( new Private::SandboxVariablesMonitorFrameData )
{
    ui->setupUi( this );

    ui->variablesTree->setColumnWidth( 0, 100 );
    ui->variablesTree->setColumnWidth( 1, 100 );

    connect( this, SIGNAL( SignalVariableSet(const QString&) ),
             this, SLOT( VariableSet(const QString&) ), Qt::QueuedConnection );
    connect( this, SIGNAL( SignalClearAllVariables() ),
             this, SLOT( ClearAllVariables() ), Qt::QueuedConnection );
}

SandboxVariablesMonitorFrame::~SandboxVariablesMonitorFrame( )
{
    delete ui;
    delete mData;
}

// Notifies a variable has changed
void SandboxVariablesMonitorFrame::OnVariableSet( const string& name, const XVariant& value )
{
    QString           qName = QString::fromUtf8( name.c_str( ) );
    QMutexLocker      locker( &mData->VarsSync );
    VarsMap::iterator it = mData->Variables.find( qName );

    if ( it != mData->Variables.end( ) )
    {
        it->Value = value;
    }
    else
    {
        mData->Variables.insert( qName, VariableData( value ) );
    }

    emit SignalVariableSet( qName );
}

// Notifies all variables have been cleared/removed
void SandboxVariablesMonitorFrame::OnClearAllVariables( )
{
    QMutexLocker locker( &mData->VarsSync );
    emit SignalClearAllVariables( );
}

// Clear all varaibles currently displayed in the tree view
void SandboxVariablesMonitorFrame::ClearAllVariables( )
{
    ui->variablesTree->clear( );

    {
        QMutexLocker locker( &mData->VarsSync );
        mData->Variables.clear( );
        mData->VarNames.clear( );
    }
}

// Helper function to convert XVariant to a string for displaying
static QString VariantToDisplayString( const XVariant& value )
{
    QString strValue;

    if ( value.IsArray( ) )
    {
        strValue = "<array>";
    }
    else if ( value.IsArray2d( ) )
    {
        strValue = "<2d array>";
    }
    else
    {
        switch ( value.Type( ) )
        {
        case XVT_Image:
            strValue = "<image>";
            break;

        case XVT_R4:
            strValue.sprintf( "%0.3f", value.ToFloat( ) );
            break;

        case XVT_R8:
            strValue.sprintf( "%0.4f", value.ToDouble( ) );
            break;

        case XVT_Range:
            {
                XRange range = value.ToRange( );
                strValue.sprintf( "[%d - %d]", range.Min( ), range.Max( ) );
            }
            break;

        case XVT_RangeF:
            {
                XRangeF range = value.ToRangeF( );
                strValue.sprintf( "[%0.3f - %0.3f]", range.Min( ), range.Max( ) );
            }
            break;

        case XVT_Point:
            {
                XPoint point = value.ToPoint( );
                strValue.sprintf( "(%d, %d)", point.X( ), point.Y( ) );
            }
            break;

        case XVT_PointF:
            {
                XPointF point = value.ToPointF( );
                strValue.sprintf( "(%0.3f, %0.3f)", point.X( ), point.Y( ) );
            }
            break;

        case XVT_Size:
            {
                XSize size = value.ToSize( );
                strValue.sprintf( "%d x %d", size.Width( ), size.Height( ) );
            }
            break;

        default:
            strValue = QString::fromUtf8( value.ToString( ).c_str( ) );
            break;
        }
    }

    return strValue;
}

// New value for the variable was set - update it in tree view
void SandboxVariablesMonitorFrame::VariableSet( const QString& name )
{
    XVariant          value;
    VarsMap::iterator varIt;
    bool              found = false;

    {
        QMutexLocker locker( &mData->VarsSync );

        varIt = mData->Variables.find( name );

        if ( varIt != mData->Variables.end( ) )
        {
            value = varIt->Value;
            found = true;
        }
    }

    if ( found )
    {
        QTreeWidgetItem* treeNode = varIt->TreeItem;

        // add new item into the tree if it is not yet there
        if ( varIt->TreeItem == nullptr )
        {
            treeNode = new QTreeWidgetItem( );
            treeNode->setText( 0, name );

            varIt->TreeItem = treeNode;

            // add name to the list and sort it
            mData->VarNames.append( name );
            sort( mData->VarNames.begin( ), mData->VarNames.end( ) );

            ui->variablesTree->insertTopLevelItem( mData->VarNames.indexOf( name ), treeNode );
        }

        QString strValue = VariantToDisplayString( value );
        QVector<QPair<QString, QString>> subItems;

        if ( value.IsArray( ) )
        {
            XVariantArray varArray = value.ToArray( );
            XVariant      varValue;

            for ( uint32_t i = 0, n = varArray.Length( ); i < n; i++ )
            {
                if ( varArray.Get( i, varValue ) == SuccessCode )
                {
                    subItems.push_back( QPair<QString, QString>( QString( ).sprintf( "[%u]", i ),
                                                                 VariantToDisplayString( varValue ) ) );
                }
            }
        }
        else
        {
            switch ( value.Type( ) )
            {
            case XVT_Range:
                {
                    XRange range = value.ToRange( );

                    subItems.push_back( QPair<QString, QString>( "Min", QString( ).sprintf( "%d", range.Min( ) ) ) );
                    subItems.push_back( QPair<QString, QString>( "Max", QString( ).sprintf( "%d", range.Max( ) ) ) );
                }
                break;

            case XVT_RangeF:
                {
                    XRangeF range = value.ToRangeF( );

                    subItems.push_back( QPair<QString, QString>( "Min", QString( ).sprintf( "%0.3f", range.Min( ) ) ) );
                    subItems.push_back( QPair<QString, QString>( "Max", QString( ).sprintf( "%0.3f", range.Max( ) ) ) );
                }
                break;

            case XVT_Point:
                {
                    XPoint point = value.ToPoint( );

                    subItems.push_back( QPair<QString, QString>( "X", QString( ).sprintf( "%d", point.X( ) ) ) );
                    subItems.push_back( QPair<QString, QString>( "Y", QString( ).sprintf( "%d", point.Y( ) ) ) );
                }
                break;

            case XVT_PointF:
                {
                    XPointF point = value.ToPointF( );

                    subItems.push_back( QPair<QString, QString>( "X", QString( ).sprintf( "%0.3f", point.X( ) ) ) );
                    subItems.push_back( QPair<QString, QString>( "Y", QString( ).sprintf( "%0.3f", point.Y( ) ) ) );
                }
                break;

            case XVT_Size:
                {
                    XSize size = value.ToSize( );

                    subItems.push_back( QPair<QString, QString>( "Width", QString( ).sprintf( "%d", size.Width( ) ) ) );
                    subItems.push_back( QPair<QString, QString>( "Height", QString( ).sprintf( "%d", size.Height( ) ) ) );
                }
                break;
            }
        }

        treeNode->setText( 1, strValue );

        // handle sub-items
        if ( subItems.size( ) != 0 )
        {
            mData->SetChildItems( subItems, treeNode );
        }
        else
        {
            while ( treeNode->childCount( ) != 0 )
            {
                delete treeNode->child( 0 );
            }
        }
    }
}

namespace Private
{

void SandboxVariablesMonitorFrameData::SetChildItems( const QVector<QPair<QString, QString>>& items,
                                                      QTreeWidgetItem* parent )
{
    auto itemsIt       = items.begin( );
    int  itemsCount    = static_cast<int>( items.size( ) );
    int  existingCount = parent->childCount( );
    int  i;


    // delete redundant tree items
    if ( existingCount > itemsCount )
    {
        for ( i = parent->childCount( ) - 1; i >= itemsCount; i++ )
        {
            delete parent->child( i );
        }

        existingCount = itemsCount;
    }

    // set text of existing items
    for ( i = 0; i < existingCount; i++ )
    {
        QTreeWidgetItem* itemWidget = parent->child( i );

        itemWidget->setText( 0, itemsIt->first );
        itemWidget->setText( 1, itemsIt->second );

        ++itemsIt;
    }

    // add new items (continue with "i" index)
    for ( ; i < itemsCount; i++ )
    {
        QTreeWidgetItem* itemWidget = new QTreeWidgetItem( );

        itemWidget->setText( 0, itemsIt->first );
        itemWidget->setText( 1, itemsIt->second );

        parent->addChild( itemWidget );

        ++itemsIt;
    }
}

} // namespace Private
