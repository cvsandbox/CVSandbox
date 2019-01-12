/*
    Video repeater plug-ins of Computer Vision Sandbox

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
#ifndef CVS_VIDEO_REPEATER_REGISTRY_HPP
#define CVS_VIDEO_REPEATER_REGISTRY_HPP

#include <string>
#include <map>
#include <XInterfaces.hpp>
#include <XMutex.hpp>

class VideoRepeaterPlugin;

class VideoRepeaterRegistry : private CVSandbox::Uncopyable
{
private:
    VideoRepeaterRegistry( ) { }

public:
    static VideoRepeaterRegistry* Instance( );

    void AddRepeater( std::string id, VideoRepeaterPlugin* repeater );
    void RemoveRepeater( std::string id );
    VideoRepeaterPlugin* GetRepeater( std::string id ) const;

private:
    static VideoRepeaterRegistry*       mInstance;
    static CVSandbox::Threading::XMutex mSync;
    std::map<std::string, VideoRepeaterPlugin*> mRepeaters;
};

#endif // CVS_VIDEO_REPEATER_REGISTRY_HPP
