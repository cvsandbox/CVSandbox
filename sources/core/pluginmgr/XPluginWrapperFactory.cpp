/*
    Plug-ins' management library of Computer Vision Sandbox

    Copyright (C) 2011-2018, cvsandbox
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

#include "XPluginWrapperFactory.hpp"

#include "XImageProcessingFilterPlugin.hpp"
#include "XImageProcessingFilterPlugin2.hpp"
#include "XImageImportingPlugin.hpp"
#include "XImageExportingPlugin.hpp"
#include "XVideoSourcePlugin.hpp"
#include "XImageGenerationPlugin.hpp"
#include "XDevicePlugin.hpp"
#include "XVideoProcessingPlugin.hpp"
#include "XImageProcessingPlugin.hpp"

using namespace std;

shared_ptr<XPlugin> XPluginWrapperFactory::CreateWrapper( void* pluginObject, PluginType type, bool ownIt )
{
    shared_ptr<XPlugin> pluginInstance;

    if ( pluginObject != nullptr )
    {
        switch ( type )
        {
        case PluginType_ImageProcessingFilter:
            pluginInstance = XImageProcessingFilterPlugin::Create( pluginObject, ownIt );
            break;

        case PluginType_ImageProcessingFilter2:
            pluginInstance = XImageProcessingFilterPlugin2::Create( pluginObject, ownIt );
            break;

        case PluginType_ImageImporter:
            pluginInstance = XImageImportingPlugin::Create( pluginObject, ownIt );
            break;

        case PluginType_ImageExporter:
            pluginInstance = XImageExportingPlugin::Create( pluginObject, ownIt );
            break;

        case PluginType_VideoSource:
            pluginInstance = XVideoSourcePlugin::Create( pluginObject, ownIt );
            break;

        case PluginType_ImageGenerator:
            pluginInstance = XImageGenerationPlugin::Create( pluginObject, ownIt );
            break;

        case PluginType_Device:
            pluginInstance = XDevicePlugin::Create( pluginObject, ownIt );
            break;

        case PluginType_VideoProcessing:
            pluginInstance = XVideoProcessingPlugin::Create( pluginObject, ownIt );
            break;

        case PluginType_ImageProcessing:
            pluginInstance = XImageProcessingPlugin::Create( pluginObject, ownIt );
            break;

        default:
            pluginInstance = XPlugin::Create( pluginObject, type, ownIt );
            break;
        }
    }

    return pluginInstance;
}
