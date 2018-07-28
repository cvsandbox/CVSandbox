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
#ifndef CVS_XMANUAL_RESET_EVENT_HPP
#define CVS_XMANUAL_RESET_EVENT_HPP

#include <stdint.h>
#include <XInterfaces.hpp>

namespace CVSandbox { namespace Threading {

namespace Private
{
    class XManualResetEventImpl;
}

// Manual reset synchronization event
class XManualResetEvent : public Uncopyable
{
public:
    XManualResetEvent( );
    ~XManualResetEvent( );

    // Set event to not signalled state
    void Reset( );
    // Set event to signalled state
    void Signal( );
    // Wait till the event gets into signalled state
    void Wait( );
    // Wait the specified amount of time (milliseconds) till the event gets signalled
    bool Wait( uint32_t msec );
    // Check current state of the event
    bool IsSignaled( );

    // Check if the instance represents a valid system's synchronization object
    bool IsValid( ) const { return isValid; }

private:
    Private::XManualResetEventImpl* pimpl;
    bool isValid;
};

} } // namespace CVSandbox::Threading

#endif // CVS_XMANUAL_RESET_EVENT_HPP
