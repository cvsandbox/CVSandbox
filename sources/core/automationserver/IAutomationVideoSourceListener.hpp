/*
    Automation server library of Computer Vision Sandbox

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
#ifndef CVS_IAUTOMATION_VIDEO_SOURCE_LISTENER_HPP
#define CVS_IAUTOMATION_VIDEO_SOURCE_LISTENER_HPP

namespace CVSandbox { namespace Automation
{

class IAutomationVideoSourceListener
{
public:
    virtual ~IAutomationVideoSourceListener( ) { }

    virtual void OnNewVideoFrame( uint32_t videoSourceId, const std::shared_ptr<const XImage>& image ) = 0;

    virtual void OnErrorMessage( uint32_t videoSourceId, const std::string& errorMessage ) = 0;
};

} } // namespace CVSandbox::Automation

#endif // CVS_IAUTOMATION_VIDEO_SOURCE_LISTENER_HPP
