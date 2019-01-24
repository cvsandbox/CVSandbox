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
#ifndef CVS_CONFIGURE_PLUGIN_PAGE_HPP
#define CVS_CONFIGURE_PLUGIN_PAGE_HPP

#include <memory>
#include <XPluginsEngine.hpp>
#include "WizardPageFrame.hpp"

namespace Ui
{
    class ConfigurePluginPage;
}

namespace Private
{
    class ConfigurePluginPagePrivate;
}

class ConfigurePluginPage : public WizardPageFrame
{
    Q_OBJECT

public:
    explicit ConfigurePluginPage( bool canBeSkipped = false, QWidget* parent = 0 );
    ~ConfigurePluginPage( );

    // Set title of the group box
    void SetTitle(  const QString& title );

    // Set plug-in to configure properties of
    void SetPluginToConfigure( const std::shared_ptr<const XPluginDescriptor>& pluginDesc,
                               const std::shared_ptr<XPlugin>& plugin );

public:
    // Check if the page is complete, so we can got the next one after applying changes of this one
    virtual bool CanGoNext( ) const;
    // Check if the page provides any help
    virtual bool ProvidesHelp( ) const;
    // Show help dialog
    virtual void ShowHelp( );

protected:
    // Handle focus event
    void focusInEvent( QFocusEvent* event );

private:
    Ui::ConfigurePluginPage*  ui;
    Private::ConfigurePluginPagePrivate* mData;
};

#endif // CVS_CONFIGURE_PLUGIN_PAGE_HPP
