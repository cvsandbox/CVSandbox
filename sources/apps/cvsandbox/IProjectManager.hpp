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
#ifndef CVS_IPROJECT_MANAGER_HPP
#define CVS_IPROJECT_MANAGER_HPP

#include <memory>
#include <XGuid.hpp>
#include "ProjectObject.hpp"

class IProjectManager
{
public:
    virtual ~IProjectManager( ) { }

public:
    // Check if the specified string is a valid project object's name (if does not contain invalid characters)
    virtual bool CheckIfObjectNameIsValid( const std::string& name ) const = 0;

    // Add/Update/Delete project object
    virtual bool AddProjectObject( const std::shared_ptr<ProjectObject>& po ) = 0;
    virtual bool UpdateProjectObject( const std::shared_ptr<ProjectObject>& po ) = 0;
    virtual bool DeleteProjectObject( const std::shared_ptr<ProjectObject>& po ) = 0;

    // Get project object by its ID
    virtual const std::shared_ptr<ProjectObject> GetProjectObject( const CVSandbox::XGuid& id ) const = 0;
    // Get project object by its path
    virtual const std::shared_ptr<ProjectObject> GetProjectObject( const std::string& path ) const = 0;
    // Get type of the project object with the given ID
    virtual ProjectObjectType GetProjectObjectType( const CVSandbox::XGuid& id ) const = 0;

    // Find child object with the specified name
    virtual const std::shared_ptr<ProjectObject>
        FindChildObject( const CVSandbox::XGuid& parent, const std::string& childName ) const = 0;
    // Get full name of the specified project object, which includes names of its parent folders
    virtual const std::string GetProjectObjectFullName( const std::shared_ptr<const ProjectObject>& po ) const = 0;
    // Get path of the specified project object
    virtual const std::string GetProjectObjectPath( const std::shared_ptr<const ProjectObject>& po ) const = 0;

    // Get children count for the specified parent
    virtual int GetChildrenCount( const CVSandbox::XGuid& parent ) const = 0;

    // Fills the specified list with children IDs of the certain type starting from the specified parent
    virtual void GetChildrenId( const CVSandbox::XGuid& parent, ProjectObjectType type, bool recursive,
                                std::vector<CVSandbox::XGuid>& children ) const = 0;

    // Fills the specified list with children of the certain type starting from the specified parent
    virtual void GetChildren( const CVSandbox::XGuid& parent, ProjectObjectType type, bool recursive,
                              std::vector<std::shared_ptr<ProjectObject> >& children ) const = 0;

    // Check it the specified project object can be moved to new parent
    virtual bool CheckIfObectCanMoveToNewParent( const CVSandbox::XGuid& idToMove, const CVSandbox::XGuid& newParentId ) = 0;
    // Move the specified project object to the new parent
    virtual bool MoveObjectToNewParent( const CVSandbox::XGuid& idToMove, const CVSandbox::XGuid& newParentId ) = 0;

protected:
    // Check object's parent ID to the new one
    void ChangeProjectObjectParentId( const std::shared_ptr<ProjectObject>& po, const CVSandbox::XGuid& newParentId );
};

#endif // CVS_IPROJECT_MANAGER_HPP
