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
#ifndef CVS_SELECTPLUGINPAGE_HPP
#define CVS_SELECTPLUGINPAGE_HPP

#include <XGuid.hpp>
#include <iplugin.h>
#include "WizardPageFrame.hpp"

namespace Ui
{
    class SelectPluginPage;
}

namespace Private
{
    class SelectPluginPageData;
}

class SelectPluginPage : public WizardPageFrame
{
    Q_OBJECT

public:
    explicit SelectPluginPage( PluginType typesMask = PluginType_All, QWidget* parent = 0 );
    ~SelectPluginPage( );

    // Set title of the group box
    void SetTitle(  const QString& title );

    // Get ID of the currently selected plug-in
    const CVSandbox::XGuid GetSelectedPluginID( ) const;

public:
    virtual bool CanGoNext( ) const;

protected:
    // Handle focus event
    void focusInEvent( QFocusEvent* event );

private slots:
    void on_SelectedPluginChanged( );
    void on_GetPluginHelpAction_triggered( );

private:
    Ui::SelectPluginPage*          ui;
    Private::SelectPluginPageData* mData;
};

#endif // CVS_SELECTPLUGINPAGE_HPP