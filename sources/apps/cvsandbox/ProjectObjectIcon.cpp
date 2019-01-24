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

#include "ProjectObjectIcon.hpp"

static const QString ICON_FOLDER  = QString::fromUtf8( ":/images/icons/folder_blue.png" );
static const QString ICON_SANDBOX = QString::fromUtf8( ":/images/icons/sandbox.png" );
static const QString ICON_CAMERA  = QString::fromUtf8( ":/images/icons/camera.png" );

static const QString ICON_LARGE_FOLDER  = QString::fromUtf8( ":/images/icons/folder_blue_32.png" );
static const QString ICON_LARGE_SANDBOX = QString::fromUtf8( ":/images/icons/sandbox_32.png" );
static const QString ICON_LARGE_CAMERA  = QString::fromUtf8( ":/images/icons/camera_32.png" );

// Get resource name for the icon corresponding to the project type
QString ProjectObjectIcon::GetResourceName( const ProjectObjectType& poType, bool largeIcon )
{
    QString iconName;

    if ( largeIcon )
    {
        switch ( poType.Value( ) )
        {
        case ProjectObjectType::Folder:
            iconName = ICON_LARGE_FOLDER;
            break;
        case ProjectObjectType::Sandbox:
            iconName = ICON_LARGE_SANDBOX;
            break;
        case ProjectObjectType::Camera:
            iconName = ICON_LARGE_CAMERA;
            break;

        default:
            break;
        }
    }
    else
    {
        switch ( poType.Value( ) )
        {
        case ProjectObjectType::Folder:
            iconName = ICON_FOLDER;
            break;
        case ProjectObjectType::Sandbox:
            iconName = ICON_SANDBOX;
            break;
        case ProjectObjectType::Camera:
            iconName = ICON_CAMERA;
            break;

        default:
            break;
        }
    }

    return iconName;
}
