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
#ifndef CVS_CAMERA_PROJECT_OBJECT_HPP
#define CVS_CAMERA_PROJECT_OBJECT_HPP

#include <string>
#include <memory>
#include <map>
#include <XVariant.hpp>
#include "ProjectObject.hpp"

class ProjectObjectFactory;

class CameraProjectObject : public ProjectObject
{
    friend class ProjectObjectFactory;

private:
    CameraProjectObject(
            const CVSandbox::XGuid& id = CVSandbox::XGuid( ),
            const std::string& name = std::string( ), const std::string& description = std::string( ),
            const CVSandbox::XGuid& parentId = CVSandbox::XGuid( ) );

public:
    static std::shared_ptr<CameraProjectObject> CreateNew(
            const std::string& name, const std::string& description = std::string( ),
            const CVSandbox::XGuid& parentId = CVSandbox::XGuid( ) );

    // Get ID of the video source plug-in
    const CVSandbox::XGuid PluginId( ) const;

    // Get/Set video source plug-in properties
    const std::map<std::string, CVSandbox::XVariant> PluginProperties( ) const;
    void SetPluginProperties( const std::map<std::string, CVSandbox::XVariant>& pluginProperties );
    void MergePluginProperties( const std::map<std::string, CVSandbox::XVariant>& pluginProperties );

    // Set both plug-in ID and its properties
    void SetPluginConfiguration( const CVSandbox::XGuid& pluginId,
                                 const std::map<std::string, CVSandbox::XVariant>& pluginProperties );

public:
    virtual void Save( QXmlStreamWriter& xmlWriter ) const;
    virtual bool Load( QXmlStreamReader& xmlReader );

private:
    CVSandbox::XGuid                           mPluginId;
    std::map<std::string, CVSandbox::XVariant> mPluginProperties;
};

#endif // CVS_CAMERA_PROJECT_OBJECT_HPP
