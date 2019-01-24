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
#ifndef CVS_PERFORMANCE_MONITOR_SERVICE_HPP
#define CVS_PERFORMANCE_MONITOR_SERVICE_HPP

#include <QObject>
#include "IPerformanceMonitorService.hpp"

namespace Private
{
    class PerformanceMonitorServiceData;
}

class PerformanceMonitorService : public QObject, public IPerformanceMonitorService
{
    Q_OBJECT

public:
    PerformanceMonitorService( );
    ~PerformanceMonitorService( );

    virtual void Start( uint32_t sourceId );
    virtual void Start( std::vector<uint32_t> sourceIds, std::map<uint32_t, bool> reportFpsState );
    virtual void Stop( );

    virtual float TotalCpuLoad( ) const;
    virtual float TotalFrameRate( ) const;

    virtual std::chrono::system_clock::duration UpTime( ) const;

private:
    void Start( );

signals:
    void Updated( );

private slots:
    void on_UpdateTimer_timeout( );

private:
    Private::PerformanceMonitorServiceData* mData;
};

#endif // CVS_PERFORMANCE_MONITOR_SERVICE_HPP
