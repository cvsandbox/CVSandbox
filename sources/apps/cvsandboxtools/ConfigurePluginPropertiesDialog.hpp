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
#ifndef CVS_CONFIGURE_PLUGIN_PROPERTIES_DIALOG_HPP
#define CVS_CONFIGURE_PLUGIN_PROPERTIES_DIALOG_HPP

#include <QDialog>
#include <XPluginsEngine.hpp>

#include "cvsandboxtools_global.h"
#include "PluginPropertyEditorFrame.hpp"

namespace Ui
{
    class ConfigurePluginPropertiesDialog;
}

class CVS_SHARED_EXPORT ConfigurePluginPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigurePluginPropertiesDialog( const std::shared_ptr<const XPluginDescriptor>& pluginDesc,
                                              const std::shared_ptr<XPlugin>& plugin,
                                              bool showDeviceRuntimeProperties = false,
                                              const QWidget* parent = nullptr );
    ~ConfigurePluginPropertiesDialog( );

    // Show/hide help button
    void EnableHelpButton( bool enable );
    // Show/hide restore defaults button
    void EnableDefaultsButton( bool enable );

signals:
    void ConfigurationUpdated( );
    void HelpRequested( QWidget* sender );

protected:
    void closeEvent( QCloseEvent* event );

public slots:
    virtual void accept( );
    virtual void reject( );

private slots:
    void on_helpButton_clicked( );
    void on_defaultsButton_clicked( );
    void on_okButton_clicked( );
    void on_cancelButton_clicked( );
    void on_propertyEditor_ConfigurationUpdated( );

private:
    Ui::ConfigurePluginPropertiesDialog* ui;
    PluginPropertyEditorFrame            mPropertyEditor;
};

#endif // CVS_CONFIGURE_PLUGIN_PROPERTIES_DIALOG_HPP
