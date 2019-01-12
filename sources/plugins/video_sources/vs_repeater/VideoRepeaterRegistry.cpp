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

#include "VideoRepeaterRegistry.hpp"
#include "VideoRepeaterPlugin.hpp"

using namespace std;
using namespace CVSandbox;
using namespace CVSandbox::Threading;

VideoRepeaterRegistry* VideoRepeaterRegistry::mInstance = nullptr;
XMutex                 VideoRepeaterRegistry::mSync;


VideoRepeaterRegistry* VideoRepeaterRegistry::Instance( )
{
    XScopedLock lock( &mSync );

    if ( mInstance == nullptr )
    {
        mInstance = new VideoRepeaterRegistry( );
    }

    return mInstance;
}


void VideoRepeaterRegistry::AddRepeater( string id, VideoRepeaterPlugin* repeater )
{
    XScopedLock lock( &mSync );
    mRepeaters[id] = repeater;
}

void VideoRepeaterRegistry::RemoveRepeater( string id )
{
    XScopedLock lock( &mSync );
    mRepeaters.erase( id );
    if ( mRepeaters.empty( ) )
    {
        mInstance = nullptr;
        delete this;
    }
}

VideoRepeaterPlugin* VideoRepeaterRegistry::GetRepeater( string id ) const
{
    VideoRepeaterPlugin* ret = nullptr;

    XScopedLock lock( &mSync );

    std::map<std::string, VideoRepeaterPlugin*>::const_iterator it = mRepeaters.find( id );
    if ( it != mRepeaters.end( ) )
    {
        ret = it->second;
    }

    return ret;
}
