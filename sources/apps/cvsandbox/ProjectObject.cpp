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

#include <assert.h>
#include <xtypes.h>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

#include "ProjectObject.hpp"
#include "XGuidGenerator.hpp"

using namespace std;
using namespace CVSandbox;

// ================ ProjectObjectType ================

static const char* PO_STRS[]       = { "None", "Folder", "Sandbox", "Camera" };
static const char* PO_TYPE_ANY     = "Any";
static const char* PO_TYPE_UNKNOWN = "Unknown";

// Convert project object type to string
string ProjectObjectType::ToString( ) const
{
    string ret = PO_STRS[0];

    if ( mValue != 0 )
    {
        if ( mValue == Any )
        {
            ret = PO_TYPE_ANY;
        }
        else
        {
            if ( XBitsCount( (uint32_t) mValue ) != 1 )
            {
                ret = PO_TYPE_UNKNOWN;
            }
            else
            {
                ret = PO_STRS[XBitsGetLowestSet( (uint32_t) mValue )];
            }
        }
    }

    return ret;
}

// Convert string into project object type
ProjectObjectType ProjectObjectType::FromString( const string& str )
{
    ProjectObjectType poType;

    for ( unsigned int i = 1; i < sizeof( PO_STRS ) / sizeof( char* ); i++ )
    {
        if ( str == PO_STRS[i] )
        {
            poType = static_cast<Enum>( 1 << ( i - 1 ) );
            break;
        }
    }

    return poType;
}

// ================ ProjectObject ================

static const QString STR_ID          = QString::fromUtf8( "Id" );
static const QString STR_PID         = QString::fromUtf8( "PId" );
static const QString STR_NAME        = QString::fromUtf8( "Name" );
static const QString STR_DESCRIPTION = QString::fromUtf8( "Description" );

ProjectObject::ProjectObject( ProjectObjectType type, const XGuid& id, const string& name,
                              const string& description, const XGuid& parentId ) :
    mType( type ),
    mParentId( parentId ), mId( id ),
    mName( name ), mDescription( description )
{
}

ProjectObject::~ProjectObject( )
{
}

// Get type of the project object
ProjectObjectType ProjectObject::Type( ) const
{
    return mType;
}

// Get Parent ID of the project object
const XGuid ProjectObject::ParentId( ) const
{
    return mParentId;
}

// Get ID of the project object
const XGuid ProjectObject::Id( ) const
{
    return mId;
}

// Get name of the project object
const string ProjectObject::Name( ) const
{
    return mName;
}

// Get description of the project object
const string ProjectObject::Description( ) const
{
    return mDescription;
}

// Set name of the project object
void ProjectObject::SetName( const string& name )
{
    mName = name;
}

// Set description of the project object
void ProjectObject::SetDescription( const string& description )
{
    mDescription = description;
}

// Save project object into XML writer
void ProjectObject::Save( QXmlStreamWriter& xmlWriter ) const
{
    if ( !mParentId.IsEmpty( ) )
    {
        xmlWriter.writeAttribute( STR_PID, mParentId.ToString( ).c_str( ) );
    }

    xmlWriter.writeAttribute( STR_ID, mId.ToString( ).c_str( ) );
    xmlWriter.writeAttribute( STR_NAME, QString::fromUtf8( mName.c_str( ) ) );
    xmlWriter.writeAttribute( STR_DESCRIPTION, QString::fromUtf8( mDescription.c_str( ) ) );
}

// Load project object from XML reader
bool ProjectObject::Load( QXmlStreamReader& xmlReader )
{
    QXmlStreamAttributes xmlAttrs = xmlReader.attributes( );
    QStringRef           strRef;
    bool                 ret      = true;

    // name
    strRef = xmlAttrs.value( STR_NAME );

    if ( strRef.isEmpty( ) )
    {
        ret = false;
    }
    else
    {
        mName = strRef.toString( ).toUtf8( ).data( );
    }

    // description
    strRef = xmlAttrs.value( STR_DESCRIPTION );

    if ( !strRef.isEmpty( ) )
    {
        mDescription = strRef.toString( ).toUtf8( ).data( );
    }

    // id
    strRef = xmlAttrs.value( STR_ID );

    if ( strRef.isEmpty( ) )
    {
        ret = false;
    }
    else
    {
        mId  = XGuid::FromString( strRef.toString( ).toStdString( ) );
        ret &= ( !mId.IsEmpty( ) );
    }

    // parent ID
    strRef = xmlAttrs.value( STR_PID );

    if ( !strRef.isEmpty( ) )
    {
        mParentId = XGuid::FromString( strRef.toString( ).toStdString( ) );
        // don't care if error on parsing or the attribute does not present at all - put it into root then
    }

    return ret;
}
