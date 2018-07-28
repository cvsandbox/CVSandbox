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
#ifndef CVS_XMUTEX_IMPL_HPP
#define CVS_XMUTEX_IMPL_HPP

namespace CVSandbox { namespace Threading { namespace Private {

class XMutexImplData;

// Platform specific implementation of mutual exclusion synchronization object
class XMutexImpl
{
public:
    XMutexImpl( );
    ~XMutexImpl( );

    // Create system's synchronization object
    bool Create( );
    // Destroy system's synchronization object
    void Destroy( );

    // Lock the mutex
    void Lock( );
    // Try locking the mutex
    bool TryLock( );
    // Unlock the mutex
    void Unlock( );

private:
    XMutexImplData* mData;
};

} } } // namespace CVSandbox::Threading::Private

#endif // CVS_XMUTEX_IMPL_HPP
