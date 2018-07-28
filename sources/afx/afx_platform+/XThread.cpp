/*
    Library to wrap some platform specific code of Computer Vision Sandbox

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

#include "XThread.hpp"
#include "internal/XThreadImpl.hpp"

namespace CVSandbox { namespace Threading {

XThread::XThread( ) : pimpl( new Private::XThreadImpl( ) )
{
}

XThread::~XThread( )
{
    delete pimpl;
}

// Create and start running thread
bool XThread::Create( XThreadFunction callback, void* param )
{
    return pimpl->Create( callback, param );
}

// Join the thread - wait till it is done
void XThread::Join( )
{
    pimpl->Join( );
}

// Wait for the specified amount of time till the thread is done
bool XThread::Join( uint32_t msec )
{
    return pimpl->Join( msec );
}

// Check if the thread is still running
bool XThread::IsRunning( )
{
    return pimpl->IsRunning( );
}

// Terminate the thread (try to avoid using it ever - too dangerous)
void XThread::Terminate( )
{
    pimpl->Terminate( );
}

// Put current thread into sleep state for the specified amount of time
void XThread::Sleep( uint32_t msec )
{
    Private::XThreadImpl::Sleep( msec );
}

// Give CPU to another waiting thread
void XThread::YieldCpu( )
{
    Private::XThreadImpl::YieldCpu( );
}

// Get ID of the current thread
uint32_t XThread::ThreadId( )
{
    return Private::XThreadImpl::ThreadId( );
}

} } // namespace CVSandbox::Threading
