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

#include "ApplicationSettingsService.hpp"
#include <QMutex>
#include <QMutexLocker>
#include <QSettings>
#include <QDir>

namespace Private
{
    class ApplicationSettingsServiceData
    {
    public:
        ApplicationSettingsServiceData( const QString& settingsFileName ) :
            mSettings( settingsFileName, QSettings::IniFormat ),
            mSync( )
        {
        }

    public:
        QSettings mSettings;
        QMutex mSync;
    };
}

ApplicationSettingsService::ApplicationSettingsService( const QString& settingsFileName ) :
    mData( new Private::ApplicationSettingsServiceData( settingsFileName ) )
{

}

ApplicationSettingsService::~ApplicationSettingsService( )
{
    delete mData;
}

void ApplicationSettingsService::SetValue( const QString& prefix, const QString& key, const QVariant& value )
{
    QMutexLocker lock( &mData->mSync );
    mData->mSettings.setValue( QString( "%1/%2" ).arg( prefix ).arg( key ), value );
}

const QVariant ApplicationSettingsService::GetValue( const QString& prefix, const QString& key )
{
    QMutexLocker lock( &mData->mSync );
    return mData->mSettings.value( QString( "%1/%2" ).arg( prefix ).arg( key ) );
}
