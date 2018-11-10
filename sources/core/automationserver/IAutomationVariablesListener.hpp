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
#ifndef CVS_IAUTOMATION_VARIABLE_LISTENER_HPP
#define CVS_IAUTOMATION_VARIABLE_LISTENER_HPP

#include <string>
#include <XVariant.hpp>

namespace CVSandbox { namespace Automation
{

class IAutomationVariablesListener
{
public:
    virtual ~IAutomationVariablesListener( ) { }

    // Notifies a variable has changed
    virtual void OnVariableSet( const std::string& name, const CVSandbox::XVariant& value ) = 0;

    // Notifies all variables have been cleared/removed
    virtual void OnClearAllVariables( ) = 0;
};

} } // namespace CVSandbox::Automation

#endif // CVS_IAUTOMATION_VARIABLE_LISTENER_HPP
