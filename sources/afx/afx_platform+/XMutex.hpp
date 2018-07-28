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
#ifndef CVS_XMUTEX_HPP
#define CVS_XMUTEX_HPP

#include <XInterfaces.hpp>

namespace CVSandbox { namespace Threading {

namespace Private
{
    class XMutexImpl;
}

// Mutual exclusion synchronization object
class XMutex : public Uncopyable
{
public:
    XMutex( );
    ~XMutex( );

    // Lock the mutex
    void Lock( );
    // Try locking the mutex
    bool TryLock( );
    // Unlock the mutex
    void Unlock( );

    // Check if the instance represents a valid system's synchronization object
    bool IsValid( ) const { return isValid; }

private:
    Private::XMutexImpl* pimpl;
    bool isValid;
};

// Helper scoped lock class, which lock mutex in its constructor and unlock in destructor
class XScopedLock : public Uncopyable
{
public:
    XScopedLock( XMutex* mutex );
    ~XScopedLock( );

private:
    XMutex* mMutex;
};

} } // namespace CVSandbox::Threading

#endif // CVS_XMUTEX_HPP
