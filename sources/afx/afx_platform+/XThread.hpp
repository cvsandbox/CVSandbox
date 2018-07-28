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

#pragma once
#ifndef CVS_XTHREAD_HPP
#define CVS_XTHREAD_HPP

#include <stdint.h>
#include <XInterfaces.hpp>

namespace CVSandbox { namespace Threading {

namespace Private
{
    class XThreadImpl;
}

// Thread function's type
typedef void (*XThreadFunction)( void* );

// Thread managing class
class XThread : public Uncopyable
{
public:
    XThread( );
    ~XThread( );

    // Create and start running thread
    bool Create( XThreadFunction callback, void* param );
    // Join the thread - wait till it is done
    void Join( );
    // Wait for the specified amount of time till the thread is done
    bool Join( uint32_t msec );
    // Check if the thread is still running
    bool IsRunning( );

    // Terminate the thread (try to avoid using it ever - too dangerous)
    void Terminate( );

public:
    // Put current thread into sleep state for the specified amount of time
    static void Sleep( uint32_t msec );
    // Give CPU to another waiting thread
    static void YieldCpu( );
    // Get ID of the current thread
    static uint32_t ThreadId( );

private:
    Private::XThreadImpl* pimpl;
};

} } // namespace CVSandbox::Threading

#endif // CVS_XTHREAD_HPP
