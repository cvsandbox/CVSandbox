/*
    DirectShow video source library of Computer Vision Sandbox

    Copyright (C) 2011-2018, cvsandbox
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
#ifndef CVS_XHTTP_COMMANDS_THREAD_HPP
#define CVS_XHTTP_COMMANDS_THREAD_HPP

#include <memory>
#include <string>
#include <XInterfaces.hpp>

namespace CVSandbox { namespace Video { namespace MJpeg
{

namespace Private
{
    class XHttpCommandsThreadData;
};

class IHttpCommandResultListener
{
public:
    virtual ~IHttpCommandResultListener( ) { }

    // Successful completion of HTTP command
    virtual void OnCompletion( uint32_t requestId, const uint8_t* data, uint32_t dataLength ) = 0;

    // HTTP command error notification
    virtual void OnError( uint32_t requestId, const std::string& errorMessage ) = 0;
};

// TODO: should be extracted out of this library

class XHttpCommandsThread : private Uncopyable
{
private:
    XHttpCommandsThread( );

public:
    ~XHttpCommandsThread( );

    static std::shared_ptr<XHttpCommandsThread> Create( );

    // Start the thread waiting for commands to run
    XErrorCode Start( );
    // Signal thread to stop
    void SignalToStop( );
    // Wait till thread stops
    void WaitForStop( );
    // Check if control thread is still running
    bool IsRunning( );

    // Set commands' result listener
    void SetListener( IHttpCommandResultListener* listener );

    // Set base address (starts with "http://" and followed by ip:port or DNS name; no final '/')
    bool SetBaseAddress( const std::string& baseAddress );
    // Set new authentication credentials (user name and password)
    bool SetAuthenticationCredentials( const std::string& userName, const std::string& password );
    // Set user agent
    bool SetUserAgent( const std::string& userAgent );
    // Set if basic authentcation must be forced
    bool SetForceBasicAuthorization( bool setForceBasic );

    // Add GET request to the queue
    uint32_t EnqueueGetRequest( const std::string& url, bool clearPreviousRequests = false );
    // Add POST request to the queue
    uint32_t EnqueuePostRequest( const std::string& url, const std::string& body, bool clearPreviousRequest = false );
    // Clear all queued requests
    void ClearRequestsQueue( );
    // Clear queued requests for the specified URL
    void ClearRequests( const std::string& url );

private:
    Private::XHttpCommandsThreadData* mData;
};

} } } // namespace CVSandbox::Video::MJpeg

#endif // CVS_XHTTP_COMMANDS_THREAD_HPP
