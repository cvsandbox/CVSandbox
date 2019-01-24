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
#ifndef CVS_PROJECT_OBJECT_SERIALIZATION_HELPER_HPP
#define CVS_PROJECT_OBJECT_SERIALIZATION_HELPER_HPP

#include <string>
#include <vector>
#include <map>
#include <XGuid.hpp>
#include <XVariant.hpp>
#include <XVideoSourceProcessingGraph.hpp>
#include "ScriptingThreadDesc.hpp"

class QString;
class QXmlStreamReader;
class QXmlStreamWriter;

class ProjectObjectSerializationHelper
{
private:
    ProjectObjectSerializationHelper( );

public:
    // Find end element with the specified name (returns true if reader was at this element already)
    static bool FindEndElement( QXmlStreamReader& xmlReader, const QString& elementName );

    // Save plug-in's ID and configuration into the given XML writer
    static void SavePluginConfiguration( QXmlStreamWriter& xmlWriter,
                                         const CVSandbox::XGuid& pluginId,
                                         const std::map<std::string, CVSandbox::XVariant>& pluginProperties );

    // Read plug-in's ID and configuration from the given XML reader
    static bool LoadPluginConfiguration( QXmlStreamReader& xmlReader,
                                         CVSandbox::XGuid& pluginId,
                                         std::map<std::string, CVSandbox::XVariant>& pluginProperties );

    // Save video processing step into the given XML writer
    static void SaveVideoProcessingStep( QXmlStreamWriter& xmlWriter,
                                         const CVSandbox::Automation::XVideoSourceProcessingStep& step );

    // Read video processing step from the given XML reader
    static bool LoadVideoProcessingStep( QXmlStreamReader& xmlReader,
                                         CVSandbox::Automation::XVideoSourceProcessingStep& step );

    // Save video processing graph into the given XML writer
    static void SaveVideoProcessingGraph( QXmlStreamWriter& xmlWriter,
                                          const CVSandbox::Automation::XVideoSourceProcessingGraph& graph );

    // Read video processing graph from the given XML reader
    static bool LoadVideoProcessingGraph( QXmlStreamReader& xmlReader,
                                          CVSandbox::Automation::XVideoSourceProcessingGraph& graph );

    // Save scripting threads into the given XML writer
    static void SaveScriptingThreads( QXmlStreamWriter& xmlWriter,
                                      const std::map<CVSandbox::XGuid, ScriptingThreadDesc>& threads );

    // Read scripting threads configuration from the given XML reader
    static bool LoadScriptingThreads( QXmlStreamReader& xmlReader,
                                      std::map<CVSandbox::XGuid, ScriptingThreadDesc>& threads );
};

#endif // CVS_PROJECT_OBJECT_SERIALIZATION_HELPER_HPP
