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

#include "UIPersistenceService.hpp"
#include "IPersistentUIContent.hpp"
#include <QMutex>
#include <QMutexLocker>
#include <QSettings>
#include <QDir>
#include <QMainWindow>
#include <QApplication>
#include <QDesktopWidget>
#include <QDockWidget>

namespace Private
{
    const QString STR_SIZE( "size" );
    const QString STR_POS( "pos" );
    const QString STR_IS_MAX( "isMax" );
    const QString STR_IS_VISIBLE( "isVisible" );
    const QString STR_IS_FULLSCREEN( "isFullScreen" );

    class UIPersistenceServiceData
    {
    public:
        UIPersistenceServiceData( const QString& settingsFileName ) :
            mSettings( settingsFileName, QSettings::IniFormat ),
            mSync( QMutex::Recursive )
        {
        }

    public:
        QSettings mSettings;
        QMutex mSync;
    };

    static QString GetGroupName( const QString& prefix, const QString& widgetName )
    {
        return ( prefix.isEmpty( ) ) ? widgetName : QString( "%1@%2" ).arg( prefix ).arg( widgetName );
    }
}


UIPersistenceService::UIPersistenceService( const QString& settingsFileName ) :
    mData( new Private::UIPersistenceServiceData( settingsFileName ) )
{
}

UIPersistenceService::~UIPersistenceService( )
{
    delete mData;
}

// Save widget's size/position into configuration file
void UIPersistenceService::SaveWidget( const QWidget* widget, const QString& prefix,
                                       const QMap<QString, QVariant>* extraValues )
{
    if ( widget != 0 )
    {
        QMutexLocker lock( &mData->mSync );
        QString groupName = Private::GetGroupName( prefix, widget->objectName( ) );

        mData->mSettings.beginGroup( groupName );

        // save full screen status for main window
        if ( qobject_cast<const QMainWindow*>( widget ) != nullptr )
        {
            mData->mSettings.setValue( Private::STR_IS_FULLSCREEN, widget->isFullScreen( ) );
        }

        // save some common widget settings
        mData->mSettings.setValue( Private::STR_SIZE, widget->size( ) );
        mData->mSettings.setValue( Private::STR_POS, widget->pos( ) );
        mData->mSettings.setValue( Private::STR_IS_MAX, widget->isMaximized( ) );

        // check if the widget is QDockWidget
        /*{
            const QDockWidget* dockWidget = qobject_cast<const QDockWidget*>( widget );

            if ( dockWidget != 0 )
            {
                mData->mSettings.setValue( Private::STR_IS_VISIBLE, dockWidget->isVisible( ) );
            }
        }*/

        // save any extra values passed by the caller
        if ( extraValues != nullptr )
        {
            for ( auto it = extraValues->begin( ); it != extraValues->end( ); ++it )
            {
                mData->mSettings.setValue( it.key( ), it.value( ) );
            }
        }

        mData->mSettings.endGroup( );

        // save content if needed
        IPersistentUIContent* persistentContent = qobject_cast<IPersistentUIContent*>( const_cast<QWidget*>( widget ) );

        if ( persistentContent != 0 )
        {
            persistentContent->SaveWidgetContent( *this, groupName );
        }
    }
}

// Restore widget's size/position from configuration file
void UIPersistenceService::RestoreWidget( QWidget* widget, const QString& prefix,
                                          QMap<QString, QVariant>* extraValues )
{
    if ( widget != 0 )
    {
        QMutexLocker    lock( &mData->mSync );
        QString         groupName   = Private::GetGroupName( prefix, widget->objectName( ) );
        QDesktopWidget* desktop     = QApplication::desktop( );
        QSize           desktopSize = desktop->size( );
        QPoint          desktopPos  = desktop->pos( );
        QRect           defaultRect = desktop->screenGeometry( );

        mData->mSettings.beginGroup( groupName );

        QVariant varSize  = mData->mSettings.value( Private::STR_SIZE );
        QVariant varPos   = mData->mSettings.value( Private::STR_POS );
        QVariant varIsMax = mData->mSettings.value( Private::STR_IS_MAX );

        if ( ( varSize.isValid( ) ) && ( varPos.isValid( ) ) && ( varIsMax.isValid( ) ) )
        {
            QSize size = varSize.toSize( );
            QPoint pos = varPos.toPoint( );

            // make sure window is not outside of desktop
            if ( ( size.width( )  <= desktopSize.width( ) ) &&
                 ( size.height( ) <= desktopSize.height( ) ) )
            {
                if ( pos.x( ) < desktopPos.x( ) )
                {
                    pos.setX( desktopPos.x( ) );
                }
                if ( pos.y( ) < desktopPos.y( ) )
                {
                    pos.setY( desktopPos.y( ) );
                }
                if ( pos.x( ) + size.width( ) > desktopPos.x( ) + desktopSize.width( ) )
                {
                    pos.setX( desktopPos.x( ) + desktopSize.width( ) - size.width( ) );
                }
                if ( pos.y( ) + size.height( ) > desktopPos.y( ) + desktopSize.height( ) )
                {
                    pos.setY( desktopPos.y( ) + desktopSize.height( ) - size.height( ) );
                }
            }
            else
            {
                if ( size.width( ) > defaultRect.width( ) )
                {
                    size.setWidth( defaultRect.width( ) * 0.9 );
                }
                if ( size.height( ) > defaultRect.height( ) )
                {
                    size.setHeight( defaultRect.height( ) * 0.9 );
                }

                pos = QPoint( defaultRect.x( ) + ( defaultRect.width( )  - size.width( )  ) / 2,
                              defaultRect.y( ) + ( defaultRect.height( ) - size.height( ) ) / 2 );
            }

            widget->resize( size );
            widget->move( pos );

            if ( varIsMax.toBool( ) )
            {
                widget->showMaximized( );
            }

            if ( qobject_cast<QMainWindow*>( widget ) != 0 )
            {
                QVariant varIsFullscreen = mData->mSettings.value( Private::STR_IS_FULLSCREEN );

                if ( ( varIsFullscreen.isValid( ) ) && ( varIsFullscreen.toBool( ) ) )
                {
                    widget->showFullScreen( );
                }
            }
        }

        // save any extra values passed by the caller
        if ( extraValues != nullptr )
        {
            for ( auto it = extraValues->begin( ); it != extraValues->end( ); ++it )
            {
                *it = mData->mSettings.value( it.key( ), it.value( ) );
            }
        }

        mData->mSettings.endGroup( );

        // load content if needed
        IPersistentUIContent* persistentContent = qobject_cast<IPersistentUIContent*>( widget );

        if ( persistentContent != 0 )
        {
            persistentContent->RestoreWidgetContent( *this, groupName );
        }
    }
}
