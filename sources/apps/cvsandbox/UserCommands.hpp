/*
    Computer Vision Sandbox

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
#ifndef CVS_USER_COMMANDS_HPP
#define CVS_USER_COMMANDS_HPP

#include <UserCommand.hpp>

// Handler of add folder command
class AddFolderCommand : public UserCommand
{
public slots:
    virtual void Exec( );
};

// Handler of add camera command
class AddCameraCommand : public UserCommand
{
public slots:
    virtual void Exec( );
};

// Handler of add sandbox command
class AddSandboxCommand : public UserCommand
{
public slots:
    virtual void Exec( );
};

// Handler of edit item's properties command
class EditPropertiesCommand : public UserCommand
{
public slots:
    virtual void Exec( );
};

// Handler of delete PO object command
class DeleteProjectObjectCommand : public UserCommand
{
public slots:
    virtual void Exec( );
};

// Handler of open project object command
class OpenProjectObjectCommand : public UserCommand
{
public slots:
    virtual void Exec( );
};

// Handler of open sandbox wizard command
class OpenSandboxWizardCommand : public UserCommand
{
public slots:
    virtual void Exec( );
};

#endif // CVS_USER_COMMANDS_HPP
