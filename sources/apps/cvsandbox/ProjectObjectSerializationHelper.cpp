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

#include "ProjectObjectSerializationHelper.hpp"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "XGuidGenerator.hpp"

using namespace std;
using namespace CVSandbox;
using namespace CVSandbox::Automation;

static const QString STR_ID          = QString::fromUtf8( "Id" );
static const QString STR_PROPERTY    = QString::fromUtf8( "Property" );
static const QString STR_NAME        = QString::fromUtf8( "Name" );
static const QString STR_DESCRIPTION = QString::fromUtf8( "Description" );
static const QString STR_TYPE        = QString::fromUtf8( "Type" );
static const QString STR_GRAPH       = QString::fromUtf8( "Graph" );
static const QString STR_STEP        = QString::fromUtf8( "Step" );
static const QString STR_PLUGIN      = QString::fromUtf8( "Plugin" );
static const QString STR_THREAD      = QString::fromUtf8( "Thread" );
static const QString STR_INTERVAL    = QString::fromUtf8( "Interval" );

// Find end element with the specified name (returns true if reader was at this element already)
bool ProjectObjectSerializationHelper::FindEndElement( QXmlStreamReader& xmlReader, const QString& elementName )
{
    bool alreadyThere = true;

    if ( xmlReader.atEnd( ) )
    {
        alreadyThere = false;
    }
    else
    {
        while ( ( ( xmlReader.tokenType( ) != QXmlStreamReader::EndElement ) ||
                  ( xmlReader.name( ) != elementName ) ) &&
                ( !xmlReader.atEnd( ) ) )
        {
            xmlReader.skipCurrentElement( );
            alreadyThere = false;
        }
    }

    return alreadyThere;
}

// Save plug-in's ID and configuration into the given XML writer
void ProjectObjectSerializationHelper::SavePluginConfiguration(
                                        QXmlStreamWriter& xmlWriter,
                                        const XGuid& pluginId,
                                        const map<string, XVariant>& pluginProperties )
{
    xmlWriter.writeAttribute( STR_ID, pluginId.ToString( ).c_str( ) );

    for ( map<string, XVariant>::const_iterator it = pluginProperties.begin( );
          it != pluginProperties.end( ); ++it )
    {
        xmlWriter.writeStartElement( STR_PROPERTY );

        xmlWriter.writeAttribute( STR_NAME, it->first.c_str( ) );
        xmlWriter.writeAttribute( STR_TYPE, QString::number( static_cast<int>( it->second.Type( ) ) ) );
        xmlWriter.writeCharacters( QString::fromUtf8( it->second.ToString( ).c_str( ) ) );

        xmlWriter.writeEndElement( );
    }
}

// Read plug-in's ID and configuration from the given XML reader
bool ProjectObjectSerializationHelper::LoadPluginConfiguration(
                                        QXmlStreamReader& xmlReader,
                                        XGuid& pluginId,
                                        map<string, XVariant>& pluginProperties )
{
    bool ret = true;

    pluginProperties.clear( );

    // read ID
    pluginId = XGuid::FromString( xmlReader.attributes( ).value( STR_ID ).toString( ).toStdString( ) );
    ret     &= ( !pluginId.IsEmpty( ) );

    if ( ret )
    {
        // read properties
        while ( xmlReader.readNextStartElement( ) )
        {
            if ( xmlReader.name( ) == STR_PROPERTY )
            {
                QXmlStreamAttributes xmlAttrs = xmlReader.attributes( );
                XVarType             propType = XVT_Empty;
                string               propName;
                QStringRef           strRef;
                bool                 gotType  = false;

                // get property name
                strRef = xmlAttrs.value( STR_NAME );
                if ( !strRef.isEmpty( ) )
                {
                    propName = strRef.toString( ).toStdString( );
                }

                // get property type
                strRef = xmlAttrs.value( STR_TYPE );
                if ( !strRef.isEmpty( ) )
                {
                    propType = static_cast<XVarType>( strRef.toString( ).toInt( &gotType ) );
                }

                XVariant strVariant( xmlReader.readElementText( QXmlStreamReader::SkipChildElements ).toUtf8( ).data( ) );

                if ( gotType )
                {
                    XErrorCode ec;
                    XVariant propValue = strVariant.ChangeType( propType, &ec );

                    if ( propValue.IsEmpty( ) )
                    {
                        qDebug( "Failed converting property type, type = %d, string value = %s, error = %d", propType, strVariant.ToString( ).c_str( ), ec );
                        ret = false;
                    }
                    else
                    {
                        pluginProperties.insert( pair<string, XVariant>( propName, propValue ) );
                    }
                }
                else
                {
                    ret = false;
                }
            }
            else
            {
                xmlReader.skipCurrentElement( );
            }
        }
    }

    return ret;
}

// Save video processing step into the given XML writer
void ProjectObjectSerializationHelper::SaveVideoProcessingStep(
                                        QXmlStreamWriter& xmlWriter,
                                        const XVideoSourceProcessingStep& step )
{
    xmlWriter.writeStartElement( STR_STEP );
    xmlWriter.writeAttribute( STR_NAME, QString::fromUtf8( step.Name( ).c_str( ) ) );

    xmlWriter.writeStartElement( STR_PLUGIN );
    SavePluginConfiguration( xmlWriter, step.PluginId( ), step.PluginConfiguration( ) );
    xmlWriter.writeEndElement( );

    xmlWriter.writeEndElement( );
}

// Read video processing step from the given XML reader
bool ProjectObjectSerializationHelper::LoadVideoProcessingStep(
                                        QXmlStreamReader& xmlReader,
                                        XVideoSourceProcessingStep& step )
{
    bool ret = true;

    if ( xmlReader.name( ) == STR_STEP )
    {
        string stepName = xmlReader.attributes( ).value( STR_NAME ).toString( ).toUtf8( ).data( );

        // read plug-in
        if ( ( xmlReader.readNextStartElement( ) ) && ( xmlReader.name( ) == STR_PLUGIN ) )
        {
            map<string, XVariant> pluginConfiguration;
            XGuid                 pluginId;

            // if some configuration failed to load, ignore it - get as much as possible
            LoadPluginConfiguration( xmlReader, pluginId, pluginConfiguration );

            step = XVideoSourceProcessingStep( stepName, pluginId );
            step.SetPluginConfiguration( pluginConfiguration );

            FindEndElement( xmlReader, STR_PLUGIN );
        }

        FindEndElement( xmlReader, STR_STEP );
    }

    return ret;
}

// Save video processing graph into the given XML writer
void ProjectObjectSerializationHelper::SaveVideoProcessingGraph(
                                        QXmlStreamWriter& xmlWriter,
                                        const XVideoSourceProcessingGraph& graph )
{
    xmlWriter.writeStartElement( STR_GRAPH );

    for ( XVideoSourceProcessingGraph::ConstIterator stepIt = graph.begin( ); stepIt != graph.end( ); ++stepIt )
    {
        SaveVideoProcessingStep( xmlWriter, *stepIt );
    }

    xmlWriter.writeEndElement( );
}

// Read video processing graph from the given XML reader
bool ProjectObjectSerializationHelper::LoadVideoProcessingGraph(
                                        QXmlStreamReader& xmlReader,
                                        XVideoSourceProcessingGraph& graph )
{
    bool ret = false;

    if ( xmlReader.name( ) == STR_GRAPH )
    {
        // read steps
        while ( xmlReader.readNextStartElement( ) )
        {
            XVideoSourceProcessingStep processingStep;

            if ( LoadVideoProcessingStep( xmlReader, processingStep ) )
            {
                graph.AddStep( processingStep );
            }
        }

        FindEndElement( xmlReader, STR_GRAPH );

        ret = ( graph.StepsCount( ) != 0 );
    }

    return ret;
}

// Save scripting threads into the given XML writer
void ProjectObjectSerializationHelper::SaveScriptingThreads(
                                        QXmlStreamWriter& xmlWriter,
                                        const map<XGuid, ScriptingThreadDesc>& threads )
{
    for ( auto pair : threads )
    {
        const ScriptingThreadDesc thread = pair.second;

        xmlWriter.writeStartElement( STR_THREAD );
        xmlWriter.writeAttribute( STR_ID, thread.Id( ).ToString( ).c_str( ) );
        xmlWriter.writeAttribute( STR_NAME, thread.Name( ).c_str( ) );
        xmlWriter.writeAttribute( STR_DESCRIPTION, thread.Description( ).c_str( ) );
        xmlWriter.writeAttribute( STR_INTERVAL, QString( "%0" ).arg( thread.Interval( ) ) );

        xmlWriter.writeStartElement( STR_PLUGIN );
        SavePluginConfiguration( xmlWriter, thread.PluginId( ), thread.PluginConfiguration( ) );
        xmlWriter.writeEndElement( );

        xmlWriter.writeEndElement( );
    }
}

// Read scripting threads configuration from the given XML reader
bool ProjectObjectSerializationHelper::LoadScriptingThreads(
                                        QXmlStreamReader& xmlReader,
                                        map<XGuid, ScriptingThreadDesc>& threads )
{
    bool ret = true;

    threads.clear( );

    // read steps
    while ( xmlReader.readNextStartElement( ) )
    {
        if ( xmlReader.name( ) == STR_THREAD )
        {
            QXmlStreamAttributes  xmlAttrs = xmlReader.attributes( );
            map<string, XVariant> pluginConfiguration;
            XGuid                 pluginId;
            XGuid                 threadId;
            string                name, description;
            uint32_t              interval = 50;
            QStringRef            strRef;

            // get thread ID
            threadId = XGuid::FromString( xmlReader.attributes( ).value( STR_ID ).toString( ).toStdString( ) );
            if ( threadId.IsEmpty( ) )
            {
                threadId = XGuidGenerator::Generate( );
            }

            // get thread name
            strRef = xmlAttrs.value( STR_NAME );
            if ( !strRef.isEmpty( ) )
            {
                name = strRef.toString( ).toStdString( );
            }

            // get thread description
            strRef = xmlAttrs.value( STR_DESCRIPTION );
            if ( !strRef.isEmpty( ) )
            {
                description = strRef.toString( ).toStdString( );
            }

            // get thread interval
            strRef = xmlAttrs.value( STR_INTERVAL );
            if ( !strRef.isEmpty( ) )
            {
                interval = strRef.toString( ).toUInt( );
            }

            // read plug-in
            if ( ( xmlReader.readNextStartElement( ) ) && ( xmlReader.name( ) == STR_PLUGIN ) )
            {
                // if some configuration failed to load, ignore it - get as much as possible
                LoadPluginConfiguration( xmlReader, pluginId, pluginConfiguration );

                FindEndElement( xmlReader, STR_PLUGIN );
            }

            ScriptingThreadDesc threadDesc( threadId, name, interval, pluginId, description );
            threadDesc.SetPluginConfiguration( pluginConfiguration );

            threads.insert( pair<XGuid, ScriptingThreadDesc>( threadId, threadDesc ) );

            FindEndElement( xmlReader, STR_THREAD );
        }
        else
        {
            ret = false;
            break;
        }
    }

    return ret;
}
