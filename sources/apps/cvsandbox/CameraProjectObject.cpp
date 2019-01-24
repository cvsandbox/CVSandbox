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
#include "CameraProjectObject.hpp"
#include "XGuidGenerator.hpp"
#include "ProjectObjectSerializationHelper.hpp"

#include <QXmlStreamWriter>
#include <QXmlStreamReader>

using namespace std;
using namespace CVSandbox;

static const QString STR_PLUGIN   = QString::fromUtf8( "Plugin" );

CameraProjectObject::CameraProjectObject( const XGuid& id, const string& name, const string& description, const XGuid& parentId ) :
    ProjectObject( ProjectObjectType::Camera, id, name, description, parentId ),
    mPluginId( ), mPluginProperties( )
{
}

shared_ptr<CameraProjectObject> CameraProjectObject::CreateNew( const string& name, const string& description, const XGuid& parentId )
{
    return shared_ptr<CameraProjectObject>( new CameraProjectObject( XGuidGenerator::Generate( ), name, description, parentId ) );
}

// Get ID of the video source plug-in
const XGuid CameraProjectObject::PluginId( ) const
{
    return mPluginId;
}

// Get/Set video source plug-in's properties
const map<string, XVariant> CameraProjectObject::PluginProperties( ) const
{
    return mPluginProperties;
}
void CameraProjectObject::SetPluginProperties( const map<string, XVariant>& pluginProperties )
{
    mPluginProperties = pluginProperties;
}
void CameraProjectObject::MergePluginProperties( const map<string, XVariant>& pluginProperties )
{
    for ( auto pluginProperty : pluginProperties )
    {
        mPluginProperties[pluginProperty.first] = pluginProperty.second;
    }
}

// Set both plug-in ID and its properties
void CameraProjectObject::SetPluginConfiguration( const XGuid& pluginId,
                                                  const map<string, XVariant>& pluginProperties )
{
    assert( !pluginId.IsEmpty( ) );

    mPluginId         = pluginId;
    mPluginProperties = pluginProperties;
}

// Save the camera project object
void CameraProjectObject::Save( QXmlStreamWriter& xmlWriter ) const
{
    ProjectObject::Save( xmlWriter );

    // write plug-in configuration
    xmlWriter.writeStartElement( STR_PLUGIN );
    ProjectObjectSerializationHelper::SavePluginConfiguration( xmlWriter, mPluginId, mPluginProperties );
    xmlWriter.writeEndElement( );
}

// Load the camera project object
bool CameraProjectObject::Load( QXmlStreamReader& xmlReader )
{
    bool ret = ProjectObject::Load( xmlReader );

    mPluginProperties.clear( );

    if ( ret )
    {
        bool foundPluginConfiguration = false;

        while ( xmlReader.readNextStartElement( ) )
        {
            if ( xmlReader.name( ) == STR_PLUGIN )
            {
                foundPluginConfiguration = true;
                // ? ignoring errors if failing loading configuration
                ProjectObjectSerializationHelper::LoadPluginConfiguration( xmlReader, mPluginId, mPluginProperties );
            }

            xmlReader.readNext( );
        }

        ret &= foundPluginConfiguration;
    }

    return ret;
}
