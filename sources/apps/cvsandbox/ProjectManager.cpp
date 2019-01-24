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

#include "ProjectManager.hpp"
#include "ProjectObjectFactory.hpp"
#include <QFile>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QRegExp>

#include "ProjectObjectSerializationHelper.hpp"

using namespace std;
using namespace CVSandbox;

static const QString STR_CVS_PROJECT  = QString::fromUtf8( "CVSProject" );
static const QString STR_COLLECTION   = QString::fromUtf8( "Collection" );
static const QString STR_TYPE         = QString::fromUtf8( "Type" );

ProjectManager::ProjectManager( const QString& projectFileName ) :
    mProjectFileName( projectFileName ),
    mObjectsMap( ), mChildrenMap( ), mSaveOnChange( true )
{
}

const shared_ptr<ProjectManager> ProjectManager::Create( const QString& projectFileName )
{
    return shared_ptr<ProjectManager>( new (nothrow) ProjectManager( projectFileName ) );
}

// Check if the specified string is a valid project object's name (if does not contain invalid characters)
bool ProjectManager::CheckIfObjectNameIsValid( const string& name ) const
{
    return ( name.find( '/' ) == string::npos ) && ( name.find( '\\' ) == string::npos );
}

// Add the specified project object into the project
bool ProjectManager::AddProjectObject( const shared_ptr<ProjectObject>& po )
{
    bool ret = false;

    if ( ( !po->Id( ).IsEmpty( ) ) &&
         ( po->Type( ) != ProjectObjectType::None ) &&
         ( CheckIfObjectNameIsValid( po->Name( ) ) ) )
    {
        if ( !mObjectsMap.contains( po->Id( ) ) )
        {
            mObjectsMap.insert( po->Id( ), po );
            mChildrenMap[po->ParentId( )].append( po->Id( ) );

            SaveIfNeeded( );

            // inform listeners about the new object
            emit ProjectObjectAdded( po );

            ret = true;
        }
    }

    return ret;
}

// Update the specified project object - just re-valide object's name and notify listeners
bool ProjectManager::UpdateProjectObject( const shared_ptr<ProjectObject>& po )
{
    bool ret = false;

    if ( ( !po->Id( ).IsEmpty( ) ) &&
         ( po->Type( ) != ProjectObjectType::None ) &&
         ( CheckIfObjectNameIsValid( po->Name( ) ) ) )
    {
        if ( mObjectsMap.contains( po->Id( ) ) )
        {
            // check if parent object contains another object with the same name
            const XGuid parentId   = po->ParentId( );
            bool        isNameBusy = false;

            if ( mChildrenMap.contains( parentId ) )
            {
                const QList<XGuid>& children = mChildrenMap[parentId];

                for ( QList<XGuid>::ConstIterator it = children.begin( ); it != children.end( ); it++ )
                {
                    const shared_ptr<ProjectObject> child = mObjectsMap[*it];

                    // below should not normally happen if object's name is checked before updating it
                    if ( ( child->Name( ) == po->Name( ) ) &&
                         ( child->Id( )   != po->Id( ) ) )
                    {
                        // set name to ID, since there is no way for now to completely ignore the update
                        po->SetName( po->Id( ).ToString( ) );
                        isNameBusy = true;
                        break;
                    }
                }
            }

            ret = !isNameBusy;

            // inform listeners about the new updated object
            ProjectObjectUpdated( po );

            SaveIfNeeded( );
        }
    }
    return ret;
}

// Delete the specified project object
bool ProjectManager::DeleteProjectObject( const shared_ptr<ProjectObject>& po )
{
    bool ret = false;

    if ( ( !po->Id( ).IsEmpty( ) ) &&
         ( po->Type( ) != ProjectObjectType::None ) )
    {
        XGuid poId     = po->Id( );
        XGuid parentId = po->ParentId( );

        // delete all children of the folder first
        if ( po->Type( ) == ProjectObjectType::Folder )
        {
            bool oldSaveOnChange = mSaveOnChange;

            // don't need to save for every deleted child
            mSaveOnChange = false;

            if ( mChildrenMap.contains( poId ) )
            {
                QList<XGuid> children( mChildrenMap[poId] );

                for ( QList<XGuid>::ConstIterator it = children.begin( ); it != children.end( ); it++ )
                {
                    if ( mObjectsMap.contains( *it ) )
                    {
                        // Making a copy on purpose, before passing it to deletion.
                        // If copy is not done, but a reference is passed, the object is
                        // destroyed when it is removed from objects' map. So passing
                        // it to notifier will cause a crash - reference to non-existing
                        // object. We could notify before deleting from collection,
                        // but we better make sure the object is not part of project by
                        // the time client is notified.
                        shared_ptr<ProjectObject> poToDelete = mObjectsMap[*it];
                        DeleteProjectObject( poToDelete );
                    }
                }
            }

            mSaveOnChange = oldSaveOnChange;
        }

        mObjectsMap.remove( poId );
        mChildrenMap[parentId].removeOne( poId );

        // delete children's list from parent's map if it is epmpty
        if ( mChildrenMap[parentId].empty( ) )
        {
            mChildrenMap.remove( parentId );
        }

        SaveIfNeeded( );

        // notify clients about object deletion
        ProjectObjectDeleted( po );

        ret = true;
    }
    return ret;
}

// Get project object by its ID
const shared_ptr<ProjectObject> ProjectManager::GetProjectObject( const XGuid& id ) const
{
    shared_ptr<ProjectObject> ret;

    if ( ( !id.IsEmpty( ) ) && ( mObjectsMap.contains( id ) ) )
    {
        ret = mObjectsMap[id];
    }

    return ret;
}

// Get project object by its path
const shared_ptr<ProjectObject> ProjectManager::GetProjectObject( const string& path ) const
{
    shared_ptr<ProjectObject> ret;
    shared_ptr<ProjectObject> parent;
    XGuid                     parentId;
    string                    currentPath = path;

    for ( ; ; )
    {
        size_t foundAt = currentPath.find( '/' );

        if ( foundAt == string::npos )
        {
            ret = FindChildObject( parentId, currentPath );
            break;
        }
        else if ( foundAt == 0 )
        {
            currentPath = currentPath.substr( 1 );
        }
        else
        {
            string parentName = currentPath.substr( 0, foundAt );

            parent = FindChildObject( parentId, parentName );

            if ( parent )
            {
                currentPath = currentPath.substr( foundAt + 1 );
                parentId    = parent->Id( );
            }
            else
            {
                break;
            }
        }

        if ( currentPath.size( ) == 0 )
        {
            ret = parent;
            break;
        }
    }

    return ret;
}

// Get type of the project object with the given ID
ProjectObjectType ProjectManager::GetProjectObjectType( const XGuid& id ) const
{
    ProjectObjectType ret = ProjectObjectType::None;

    if ( mObjectsMap.contains( id ) )
    {
        ret = mObjectsMap[id]->Type( );
    }

    return ret;
}

// Find child object with the specified name
const shared_ptr<ProjectObject> ProjectManager::FindChildObject( const XGuid& parent, const string& childName ) const
{
    shared_ptr<ProjectObject> child;

    if ( mChildrenMap.contains( parent ) )
    {
        const QList<XGuid>& children = mChildrenMap[parent];

        for ( QList<XGuid>::ConstIterator it = children.begin( ); it != children.end( ); it++ )
        {
            const shared_ptr<ProjectObject> tempChild = mObjectsMap[*it];

            if ( tempChild->Name( ) == childName )
            {
                child = tempChild;
                break;
            }
        }
    }

    return child;
}

// Get full name of the specified project object, which includes names of its parent folders
const string ProjectManager::GetProjectObjectFullName( const shared_ptr<const ProjectObject>& po ) const
{
    string  path   = po->Name( );
    auto    parent = GetProjectObject( po->ParentId( ) );

    while ( parent )
    {
        path   = parent->Name( ) + " / " + path;
        parent = GetProjectObject( parent->ParentId( ) );
    }

    return path;
}

// Get path of the specified project object
const string ProjectManager::GetProjectObjectPath( const shared_ptr<const ProjectObject>& po ) const
{
    string  path   = "/";
    auto    parent = GetProjectObject( po->ParentId( ) );

    while ( parent )
    {
        path   =  "/ " + parent->Name( ) + " " + path;
        parent = GetProjectObject( parent->ParentId( ) );
    }

    return path;
}

// Get children count for the specified parent
int ProjectManager::GetChildrenCount( const XGuid& parent ) const
{
    int count = 0;

    if ( mChildrenMap.contains( parent ) )
    {
        count = mChildrenMap[parent].count( );
    }

    return count;
}

// Fills the specified list with children IDs of the certain type starting from the specified parent
void ProjectManager::GetChildrenId( const XGuid& parent, ProjectObjectType type, bool recursive, vector<XGuid>& children ) const
{
    QList<XGuid> myChildren = mChildrenMap[parent];

    for ( QList<XGuid>::ConstIterator it = myChildren.begin( ); it != myChildren.end( ); it++ )
    {
        const shared_ptr<ProjectObject> child     = mObjectsMap[*it];
        ProjectObjectType               childType = child->Type( );
        XGuid                           childId   = child->Id( );

        if ( ( childType.Value( ) & type.Value( ) ) != ProjectObjectType::None )
        {
            children.push_back( childId );
        }

        if ( ( childType == ProjectObjectType::Folder ) && ( recursive ) )
        {
            GetChildrenId( childId, type, true, children );
        }
    }
}

// Fills the specified list with children of the certain type starting from the specified parent
void ProjectManager::GetChildren( const XGuid& parent, ProjectObjectType type, bool recursive, vector<shared_ptr<ProjectObject> >& children ) const
{
    QList<XGuid> myChildren = mChildrenMap[parent];

    for ( QList<XGuid>::ConstIterator it = myChildren.begin( ); it != myChildren.end( ); it++ )
    {
        const shared_ptr<ProjectObject> child     = mObjectsMap[*it];
        ProjectObjectType               childType = child->Type( );

        if ( ( childType.Value( ) & type.Value( ) ) != ProjectObjectType::None )
        {
            children.push_back( child );
        }

        if ( ( childType == ProjectObjectType::Folder ) && ( recursive ) )
        {
            GetChildren( child->Id( ), type, true, children );
        }
    }
}

// Check it the specified project object can be moved to new parent
bool ProjectManager::CheckIfObectCanMoveToNewParent( const XGuid& idToMove, const XGuid& newParentId )
{
    auto poToMove    = GetProjectObject( idToMove );
    auto poNewParent = GetProjectObject( newParentId );
    bool ret         = false;

    // new paremt can not be the item itself or its current paretn
    if ( ( ( newParentId.IsEmpty( ) ) || ( poNewParent ) ) &&
         ( poToMove ) && ( idToMove != newParentId ) && ( poToMove->ParentId( ) != newParentId ) )
    {
        // new parent can be either root or some other folder
        if ( ( newParentId.IsEmpty( ) ) || ( GetProjectObjectType( newParentId ) == ProjectObjectType::Folder ) )
        {
            ret = true;

            if ( poNewParent )
            {
                // finally make sure that the moving item is not in the parent chain of the new parent
                auto poParent = poNewParent;

                while ( !poParent->ParentId( ).IsEmpty( ) )
                {
                    if ( poParent->ParentId( ) == idToMove )
                    {
                        ret = false;
                        break;
                    }

                    poParent = GetProjectObject( poParent->ParentId( ) );
                }
            }
        }
    }

    return ret;
}

// Move the specified project object to the new parent
bool ProjectManager::MoveObjectToNewParent( const XGuid& idToMove, const XGuid& newParentId )
{
    bool ret = CheckIfObectCanMoveToNewParent( idToMove, newParentId );

    if ( ret )
    {
        shared_ptr<ProjectObject> poToMove    = GetProjectObject( idToMove );
        XGuid                     oldParentId = poToMove->ParentId( );

        // remove from old parent's children list
        mChildrenMap[oldParentId].removeOne( idToMove );

        // delete children's list from old parent's map if it is empty
        if ( mChildrenMap[oldParentId].empty( ) )
        {
            mChildrenMap.remove( oldParentId );
        }

        // put the object ID into the new parent's list
        mChildrenMap[newParentId].append( idToMove );

        // change object's parent
        ChangeProjectObjectParentId( poToMove, newParentId );

        // finally inform listeners about the updated object
        ProjectObjectUpdated( poToMove );
    }

    return ret;
}

// Private helper to save
void ProjectManager::SaveIfNeeded( )
{
    if ( mSaveOnChange )
    {
        Save( );
    }
}

// Save the project into XML document
bool ProjectManager::Save( )
{
    QFile projectFile( mProjectFileName );
    bool ret = false;

    if ( projectFile.open( QIODevice::WriteOnly ) )
    {
        QXmlStreamWriter xmlWriter( &projectFile );

        // configure and start XML document
        xmlWriter.setAutoFormatting( true );
        xmlWriter.writeStartDocument( );
        xmlWriter.writeStartElement( STR_CVS_PROJECT );

        SaveObjectsOfType( xmlWriter, ProjectObjectType::Folder );
        SaveObjectsOfType( xmlWriter, ProjectObjectType::Camera );
        SaveObjectsOfType( xmlWriter, ProjectObjectType::Sandbox );

        // complete the document
        xmlWriter.writeEndElement( );
        xmlWriter.writeEndDocument( );

        ret = true;
    }

    return ret;
}

// Load a project from XML document
bool ProjectManager::Load( )
{
    QFile projectFile( mProjectFileName );
    bool  ret = true;
    bool  oldSaveOnChange  = mSaveOnChange;

    // disable saving project while it is being loaded
    mSaveOnChange = false;

    if ( projectFile.open( QIODevice::ReadOnly ) )
    {
        QXmlStreamReader xmlReader( &projectFile );

        if ( xmlReader.readNext( ) == QXmlStreamReader::StartDocument )
        {
            xmlReader.readNext( );

            if ( ( xmlReader.name( ) != STR_CVS_PROJECT ) || ( xmlReader.tokenType( ) != QXmlStreamReader::StartElement ) )
            {
                ret = false;
            }
            else
            {
                while ( xmlReader.readNextStartElement( ) )
                {
                    if ( xmlReader.name( ) == STR_COLLECTION )
                    {
                        ReadObjectsCollection( xmlReader );
                    }
                }
            }
        }
    }

    // restore auto saving option
    mSaveOnChange = oldSaveOnChange;

    return ret;
}

// Save all project objects of the specified type into the XML writer
void ProjectManager::SaveObjectsOfType( QXmlStreamWriter& xmlWriter, ProjectObjectType type )
{
    QQueue<XGuid> saveQueue;
    QString       strType = QString::fromStdString( type.ToString( ) );

    xmlWriter.writeStartElement( STR_COLLECTION );
    xmlWriter.writeAttribute( STR_TYPE, strType );

    // first put all children of the root node into the saving queue
    EnqueueChildren( XGuid( ), saveQueue );

    while ( !saveQueue.isEmpty( ) )
    {
        XGuid itemId = saveQueue.dequeue( );

        if ( mObjectsMap.contains( itemId ) )
        {
            shared_ptr<ProjectObject>& item = mObjectsMap[itemId];

            // save only objects of the specified type
            if ( item->Type( ) == type )
            {
                xmlWriter.writeStartElement( strType );

                item->Save( xmlWriter );

                xmlWriter.writeEndElement( );
            }

            // put children of a folder into the saving queue
            if ( item->Type( ) == ProjectObjectType::Folder )
            {
                EnqueueChildren( item->Id( ), saveQueue );
            }
        }
    }

    xmlWriter.writeEndElement( );
}

// Add children of the specified parent into given queue
void ProjectManager::EnqueueChildren( const XGuid& parent, QQueue<XGuid>& queue ) const
{
    if ( mChildrenMap.contains( parent ) )
    {
        queue += mChildrenMap[parent];
    }
}

// Read object collection
bool ProjectManager::ReadObjectsCollection( QXmlStreamReader& xmlReader )
{
    QStringRef startElementName = xmlReader.name( );
    QStringRef strRef           = xmlReader.attributes( ).value( STR_TYPE );
    bool       ret              = true;

    //qDebug( "start element: %s, type: %d", xmlReader.name( ).toString().toStdString().c_str(), xmlReader.tokenType( ));

    if ( !strRef.isEmpty( ) )
    {
        ProjectObjectType poType = ProjectObjectType::FromString( strRef.toString( ).toStdString( ) );

        if ( poType != ProjectObjectType::None )
        {
            while ( xmlReader.readNextStartElement( ) )
            {
                // check that name of current element corresponds to its type
                ProjectObjectType type = ProjectObjectType::FromString( xmlReader.name( ).toString( ).toStdString( ) );

                //qDebug( "found element: %s, type: %d", xmlReader.name( ).toString().toStdString().c_str(), xmlReader.tokenType( ));

                if ( type == poType )
                {
                    QStringRef                currentElementName = xmlReader.name( );
                    shared_ptr<ProjectObject> po = ProjectObjectFactory::Create( poType );

                    if ( po )
                    {
                        if ( po->Load( xmlReader ) )
                        {
                            //qDebug( "object loaded: %s", po->Name( ).c_str( ) );
                            AddProjectObject( po );
                        }
                        else
                        {
                            ret = false;
                        }
                    }

                    //qDebug( "before reading next element: %s, type: %d", xmlReader.name( ).toString().toStdString().c_str(), xmlReader.tokenType( ));

                    if ( !ProjectObjectSerializationHelper::FindEndElement( xmlReader, currentElementName.toString( ) ) )
                    {
                        ret = false;
                        //qDebug( "after alligning element: %s, type: %d", xmlReader.name( ).toString().toStdString().c_str(), xmlReader.tokenType( ));
                    }

                    xmlReader.readNext( );

                    //qDebug( "after reading next element: %s, type: %d", xmlReader.name( ).toString().toStdString().c_str(), xmlReader.tokenType( ));
                }
                else
                {
                    xmlReader.skipCurrentElement( );

                    //qDebug( "after skipping next element: %s, type: %d", xmlReader.name( ).toString().toStdString().c_str(), xmlReader.tokenType( ));

                    ret = false;
                }
            }
        }
    }

    //qDebug( "end element: %s, type: %d", xmlReader.name( ).toString().toStdString().c_str(), xmlReader.tokenType( ));
    //qDebug( "============" );

    // make sure we read everything up to the end of the element
    if ( !ProjectObjectSerializationHelper::FindEndElement( xmlReader, startElementName.toString( ) ) )
    {
        ret = false;
    }

    return ret;
}
