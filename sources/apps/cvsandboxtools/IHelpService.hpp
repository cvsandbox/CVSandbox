/*
    Common tools for Computer Vision Sandbox

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
#ifndef CVS_IHELP_SERVICE_HPP
#define CVS_IHELP_SERVICE_HPP

#include <XGuid.hpp>
#include <QWidget>

class IHelpService
{
public:
    virtual ~IHelpService( ) { }

    virtual void ShowPluginDescription( QWidget* parent, const CVSandbox::XGuid& ) = 0;

    virtual void HidePluginDescription( QWidget* parent ) = 0;

    // virtual void StealPluginDescription( QWidget* parent ) = 0;
};

#endif // CVS_IHELP_SERVICE_HPP
