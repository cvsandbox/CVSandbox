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
#ifndef CVS_PROJECT_OBJECT_HPP
#define CVS_PROJECT_OBJECT_HPP

#include <string>
#include <XGuid.hpp>
#include <XInterfaces.hpp>
#include <QObject>

class QXmlStreamWriter;
class QXmlStreamReader;

class IProjectManager;
class ProjectObject;

// Wrapper structure for project type enumeration
struct ProjectObjectType
{
public:
    enum Enum
    {
        None    = 0,
        Folder  = 1,
        Sandbox = 2,
        Camera  = 4,

        Any     = 0xFFFFFFFF
    };

public:
    ProjectObjectType( Enum value = None ) : mValue( value ) { }

    ProjectObjectType& operator= ( const Enum& rhs )
    {
        this->mValue = rhs;
        return *this;
    }

    // Comparison operators
    bool operator==( const ProjectObjectType& rhs ) const
    {
        return ( this->mValue == rhs.mValue );
    }
    bool operator==( const ProjectObjectType::Enum& rhs ) const
    {
        return ( this->mValue == rhs );
    }
    bool operator!=( const ProjectObjectType& rhs  ) const
    {
        return ( !( (*this) == rhs ) ) ;
    }
    bool operator!=( const ProjectObjectType::Enum& rhs  ) const
    {
        return ( !( (*this) == rhs ) ) ;
    }

    // Check if one PO type is less than another (required for maps)
    bool operator<( const ProjectObjectType& rhs ) const
    {
        return ( static_cast<int>( mValue) < static_cast<int>( rhs.mValue ) );
    }

    Enum Value( ) const { return mValue; }
    std::string ToString( ) const;

    static ProjectObjectType FromString( const std::string& str );

private:
    Enum    mValue;
};

// Base project object class
class ProjectObject : public QObject, private CVSandbox::Uncopyable
{
    Q_OBJECT

    friend class IProjectManager;

protected:
    ProjectObject( ProjectObjectType type, const CVSandbox::XGuid& id, const std::string& name,
                   const std::string& description = std::string( ), const CVSandbox::XGuid& parentId = CVSandbox::XGuid( ) );
public:

    virtual ~ProjectObject( );

    // Get different project object properties
    ProjectObjectType Type( ) const;
    const CVSandbox::XGuid ParentId( ) const;
    const CVSandbox::XGuid Id( ) const;
    const std::string Name( ) const;
    const std::string Description( ) const;

    // Project object's name/description
    void SetName( const std::string& name );
    void SetDescription( const std::string& description );

public:
    virtual void Save( QXmlStreamWriter& xmlWriter ) const;
    virtual bool Load( QXmlStreamReader& xmlReader );

private:
    ProjectObjectType   mType;
    CVSandbox::XGuid    mParentId;
    CVSandbox::XGuid    mId;
    std::string         mName;
    std::string         mDescription;
};

#endif // CVS_PROJECT_OBJECT_HPP
