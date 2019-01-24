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
#ifndef CVS_SCRIPTING_THREADS_PAGE_HPP
#define CVS_SCRIPTING_THREADS_PAGE_HPP

#include "WizardPageFrame.hpp"
#include "ScriptingThreadDesc.hpp"

#include <map>

class QTreeWidgetItem;

namespace Ui
{
    class ScriptingThreadsPage;
}

namespace Private
{
    class ScriptingThreadsPageData;
}

class ScriptingThreadsPage : public WizardPageFrame
{
    Q_OBJECT

public:
    explicit ScriptingThreadsPage( QWidget* parent = 0 );
    ~ScriptingThreadsPage( );

    std::map<CVSandbox::XGuid, ScriptingThreadDesc> Threads( ) const;
    void SetThreads( const std::map<CVSandbox::XGuid, ScriptingThreadDesc>& threads );

private slots:
    void on_threadsTree_itemSelectionChanged( );
    void on_addThreadButton_clicked( );
    void on_editThreadButton_clicked( );
    void on_deleteThreadButton_clicked( );
    void on_editThreadPropertiesButton_clicked( );
    void on_threadsTree_itemDoubleClicked( QTreeWidgetItem* item, int column );
    void on_propertiesDialog_HelpRequested( QWidget* sender );

private:
    Ui::ScriptingThreadsPage*          ui;
    Private::ScriptingThreadsPageData* mData;
};

#endif // CVS_SCRIPTING_THREADS_PAGE_HPP
