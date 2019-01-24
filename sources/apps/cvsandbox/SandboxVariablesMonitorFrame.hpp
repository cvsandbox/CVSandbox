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
#ifndef CVS_SANDBOX_VARIABLES_MONITOR_FRAME_HPP
#define CVS_SANDBOX_VARIABLES_MONITOR_FRAME_HPP

#include <QFrame>
#include <IAutomationVariablesListener.hpp>

namespace Ui
{
    class SandboxVariablesMonitorFrame;
}

namespace Private
{
    class SandboxVariablesMonitorFrameData;
}

class SandboxVariablesMonitorFrame : public QFrame,
                                     public CVSandbox::Automation::IAutomationVariablesListener
{
    Q_OBJECT

public:
    explicit SandboxVariablesMonitorFrame( QWidget* parent = 0 );
    ~SandboxVariablesMonitorFrame( );

    void OnVariableSet( const std::string& name, const CVSandbox::XVariant& value ) override;
    void OnClearAllVariables( ) override;

signals:
    void SignalVariableSet( const QString& name );
    void SignalClearAllVariables( );

public slots:
    void ClearAllVariable( );

private slots:
    void VariableSet( const QString& name );

private:
    Ui::SandboxVariablesMonitorFrame* ui;
    Private::SandboxVariablesMonitorFrameData* mData;
};

#endif // CVS_SANDBOX_VARIABLES_MONITOR_FRAME_HPP
