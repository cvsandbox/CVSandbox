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
#ifndef CVS_PROJECT_OBJECT_NAME_PAGE_HPP
#define CVS_PROJECT_OBJECT_NAME_PAGE_HPP

#include <XGuid.hpp>
#include "WizardPageFrame.hpp"

namespace Ui
{
    class ProjectObjectNamePage;
}

namespace Private
{
    class ProjectObjectNamePageData;
}

class ProjectObjectNamePage : public WizardPageFrame
{
    Q_OBJECT

public:
    explicit ProjectObjectNamePage( QWidget *parent = 0 );
    ~ProjectObjectNamePage( );

    // Set properties of the object to edit
    void SetObjectProperties( const CVSandbox::XGuid& parentId,
                              const std::string& name = std::string( ),
                              const std::string& description = std::string( ) );

    // Get name of the object to add/update
    const std::string GetObjectName( ) const;
    // Get description of the object to add/update
    const std::string GetObjectDescription( ) const;

    // Set icon displayed next to object's name entry
    void SetObjectIcon( const QString& iconResource );
    // Set the name of object type
    void SetObjectTypeName( const QString& name );

public:
    // Check if the page is complete, so we can got the next one after applying changes of this one
    bool CanGoNext( ) const;

protected:
    // Handle focus event
    void focusInEvent( QFocusEvent* event );

private slots:
    void on_nameEditBox_textChanged( const QString &arg1 );

private:
    // Check if the specified object's name is valid
    void ValidateObjectName( );

private:
    Ui::ProjectObjectNamePage* ui;
    Private::ProjectObjectNamePageData* mData;
};

#endif // CVS_PROJECT_OBJECT_NAME_PAGE_HPP
