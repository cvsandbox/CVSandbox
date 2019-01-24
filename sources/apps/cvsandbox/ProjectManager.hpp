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
#ifndef CVS_PROJECT_MANAGER_HPP
#define CVS_PROJECT_MANAGER_HPP

#include <memory>
#include <QObject>
#include <QMap>
#include <QList>
#include <QQueue>
#include <XInterfaces.hpp>
#include "IProjectManager.hpp"

class QXmlStreamWriter;
class QXmlStreamReader;

class ProjectManager : public QObject, public IProjectManager, private CVSandbox::Uncopyable
{
    Q_OBJECT

private:
    ProjectManager( const QString& projectFileName );

public:
    static const std::shared_ptr<ProjectManager> Create( const QString& projectFileName );

    bool Save( );
    bool Load( );

public: // IProjectManager implementation

    // Check if the specified string is a valid project object's name (if does not contain invalid characters)
    virtual bool CheckIfObjectNameIsValid( const std::string& name ) const;

    // Add/Update/Delete project object
    virtual bool AddProjectObject( const std::shared_ptr<ProjectObject>& po );
    virtual bool UpdateProjectObject( const std::shared_ptr<ProjectObject>& po );
    virtual bool DeleteProjectObject( const std::shared_ptr<ProjectObject>& po );

    // Get project object by its ID
    virtual const std::shared_ptr<ProjectObject> GetProjectObject( const CVSandbox::XGuid& id ) const;
    // Get project object by its path
    virtual const std::shared_ptr<ProjectObject> GetProjectObject( const std::string& path ) const;
    // Get type of the project object with the given ID
    virtual ProjectObjectType GetProjectObjectType( const CVSandbox::XGuid& id ) const;

    // Find child object with the specified name
    virtual const std::shared_ptr<ProjectObject>
        FindChildObject( const CVSandbox::XGuid& parent, const std::string& childName ) const;
    // Get full name of the specified project object, which includes names of its parent folders
    virtual const std::string GetProjectObjectFullName( const std::shared_ptr<const ProjectObject>& po ) const;
    // Get path of the specified project object
    virtual const std::string GetProjectObjectPath( const std::shared_ptr<const ProjectObject>& po ) const;

    // Get children count for the specified parent
    virtual int GetChildrenCount( const CVSandbox::XGuid& parent ) const;

    // Fills the specified list with children IDs of the certain type starting from the specified parent
    virtual void GetChildrenId( const CVSandbox::XGuid& parent, ProjectObjectType type, bool recursive,
                                std::vector<CVSandbox::XGuid>& children ) const;

    // Fills the specified list with children of the certain type starting from the specified parent
    virtual void GetChildren( const CVSandbox::XGuid& parent, ProjectObjectType type, bool recursive,
                              std::vector<std::shared_ptr<ProjectObject> >& children ) const;

    // Check it the specified project object can be moved to new parent
    virtual bool CheckIfObectCanMoveToNewParent( const CVSandbox::XGuid& idToMove, const CVSandbox::XGuid& newParentId );
    // Move the specified project object to the new parent
    virtual bool MoveObjectToNewParent( const CVSandbox::XGuid& idToMove, const CVSandbox::XGuid& newParentId );

signals:
    void ProjectObjectAdded( const std::shared_ptr<ProjectObject>& po );
    void ProjectObjectUpdated( const std::shared_ptr<ProjectObject>& po );
    void ProjectObjectDeleted( const std::shared_ptr<ProjectObject>& po );

private:
    void SaveObjectsOfType( QXmlStreamWriter& xmlWriter, ProjectObjectType type );
    void EnqueueChildren( const CVSandbox::XGuid& parent, QQueue<CVSandbox::XGuid>& queue ) const;
    bool ReadObjectsCollection( QXmlStreamReader& xmlReader );
    void SaveIfNeeded( );

private:
    const QString                                          mProjectFileName;
    QMap<CVSandbox::XGuid, std::shared_ptr<ProjectObject>> mObjectsMap;
    QMap<CVSandbox::XGuid, QList<CVSandbox::XGuid> >       mChildrenMap;
    bool                                                   mSaveOnChange;
};

#endif // CVS_PROJECT_MANAGER_HPP
