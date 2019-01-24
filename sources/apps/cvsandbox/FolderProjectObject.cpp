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

#include "FolderProjectObject.hpp"
#include "XGuidGenerator.hpp"
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

using namespace std;
using namespace CVSandbox;

FolderProjectObject::FolderProjectObject( const XGuid& id,
        const string& name, const string& description, const XGuid& parentId ) :
    ProjectObject( ProjectObjectType::Folder, id, name, description, parentId )
{
}

shared_ptr<FolderProjectObject> FolderProjectObject::CreateNew(
        const string& name, const string& description, const XGuid& parentId )
{
    return shared_ptr<FolderProjectObject>( new FolderProjectObject( XGuidGenerator::Generate( ), name, description, parentId ) );
}

// Save the folder project object
void FolderProjectObject::Save( QXmlStreamWriter& xmlWriter ) const
{
    ProjectObject::Save( xmlWriter );
}

// Load the folder project object
bool FolderProjectObject::Load( QXmlStreamReader& xmlReader )
{
    bool ret = ProjectObject::Load( xmlReader );

    return ret;
}
