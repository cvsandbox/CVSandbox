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
#ifndef CVS_HELP_SERVICE_HPP
#define CVS_HELP_SERVICE_HPP

#include "cvsandboxtools_global.h"
#include <QObject>
#include <XPluginsEngine.hpp>
#include "IHelpService.hpp"

namespace Private
{
    class HelpServiceData;
}

class CVS_SHARED_EXPORT HelpService : public QObject, public IHelpService
{
    Q_OBJECT

public:
    HelpService( );
    ~HelpService( );

    // Show description for the plug-in with the specified ID
    virtual void ShowPluginDescription( QWidget* parent, const CVSandbox::XGuid& );
    // Hide plug-in description window, which was opened for the specified parent
    virtual void HidePluginDescription( QWidget* parent );

    // virtual void StealPluginDescription( QWidget* parent );

private slots:
    void on_helpDialog_finished( int );
    void on_helpDialog_destroyed( QObject* );
    void on_parentDialog_destroyed( );

private:
     // virtual bool eventFilter( QObject* target, QEvent* event );

private:
    Private::HelpServiceData* mData;
};

#endif // CVS_HELP_SERVICE_HPP
