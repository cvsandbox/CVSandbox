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
#ifndef CVS_PROJECT_OBJECT_ICON_HPP
#define CVS_PROJECT_OBJECT_ICON_HPP

#include <QString>
#include "ProjectObject.hpp"

class ProjectObjectIcon
{
private:
    ProjectObjectIcon( ) { }

public:
    // Get resource name for the icon corresponding to the project type
    static QString GetResourceName( const ProjectObjectType& poType, bool largeIcon = false );
};

#endif // CVS_PROJECT_OBJECT_ICON_HPP
