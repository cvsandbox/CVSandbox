/*
    Virtual Camera DirectShow filter for Computer Vision Sandbox

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
#ifndef CVS_VCAM_GUIDS_H
#define CVS_VCAM_GUIDS_H

#if defined(_M_X64) || defined(__amd64__)
    // {0C051B8D-41B9-4E7C-A328-52991E8D3017}
    DEFINE_GUID( CLSID_CVSandboxVirtualCamera,
        0xc051b8d, 0x41b9, 0x4e7c, 0xa3, 0x28, 0x52, 0x99, 0x1e, 0x8d, 0x30, 0x17 );
#else
    // {B9666AFB-EF08-43A4-8117-7E07EE076663}
    DEFINE_GUID( CLSID_CVSandboxVirtualCamera,
        0xB9666AFB, 0xEF08, 0x43A4, 0x81, 0x17, 0x7E, 0x07, 0xEE, 0x07, 0x66, 0x63 );
#endif

#endif // CVS_VCAM_GUIDS_H
