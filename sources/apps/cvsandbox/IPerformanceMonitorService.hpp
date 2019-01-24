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
#ifndef CVS_IPERFORMANCE_MONITOR_SERVICE_HPP
#define CVS_IPERFORMANCE_MONITOR_SERVICE_HPP

#include <vector>
#include <map>
#include <stdint.h>
#include <chrono>

class IPerformanceMonitorService
{
public:
    virtual ~IPerformanceMonitorService( ) { }

    virtual void Start( uint32_t sourceId ) = 0;
    virtual void Start( std::vector<uint32_t> sourceIds, std::map<uint32_t, bool> reportFpsState ) = 0;
    virtual void Stop( ) = 0;

    virtual float TotalCpuLoad( ) const = 0;
    virtual float TotalFrameRate( ) const = 0;

    virtual std::chrono::system_clock::duration UpTime( ) const = 0;
};

#endif // CVS_IPERFORMANCE_MONITOR_SERVICE_HPP
