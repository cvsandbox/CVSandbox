/*
    Virtual Camera DirectShow filter for Computer Vision Sandbox

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

#include <streams.h>
#include <initguid.h>

#include "cvs_vcam_guids.h"
#include "cvs_vcam.h"

// DirectShow filter name strings
#define FILTER_NAME_STRING     L"CVSandbox Virtual Camera"


// Note: It is better to register no media types than to register a partial 
// media type (subtype == GUID_NULL) because that can slow down intelligent connect 
// for everyone else.

// For a specialized source filter like this, it is best to leave out the 
// AMOVIESETUP_FILTER altogether, so that the filter is not available for 
// intelligent connect. Instead, use the CLSID to create the filter or just 
// use 'new' in your application.

// Filter set-up data

const AMOVIESETUP_MEDIATYPE pinType =
{
    &MEDIATYPE_Video,       // Major type
    &MEDIASUBTYPE_NULL      // Minor type
};

const AMOVIESETUP_PIN outputPinConfig = 
{
    L"Output",      // Obsolete, not used.
    FALSE,          // Is this pin rendered?
    TRUE,           // Is it an output pin?
    FALSE,          // Can the filter create zero instances?
    FALSE,          // Does the filter create multiple instances?
    &CLSID_NULL,    // Obsolete.
    NULL,           // Obsolete.
    1,              // Number of media types.
    &pinType        // Pointer to media types.
};

const AMOVIESETUP_FILTER cvsVCamFilterConfig =
{
    &CLSID_CVSandboxVirtualCamera,  // Filter CLSID
    FILTER_NAME_STRING,             // String name
    MERIT_DO_NOT_USE,               // Filter merit
    1,                              // Number of pins
    &outputPinConfig                // Pin details
};

// List of class IDs and creator functions for the class factory. This
// provides the link between the OLE entry point in the DLL and an object
// being created. The class factory will call the static CreateInstance.
// We provide a set of filters in this one DLL.

CFactoryTemplate g_Templates[1] = 
{
    { 
        FILTER_NAME_STRING,                   // Name
        &CLSID_CVSandboxVirtualCamera,        // CLSID
        CVSVirtualCamera::CreateInstance,     // Method to create an instance of MyComponent
        NULL,                                 // Initialization function
        &cvsVCamFilterConfig                  // Set-up information (for filters)
    },
};

int g_cTemplates = sizeof( g_Templates ) / sizeof( g_Templates[0] );    


////////////////////////////////////////////////////////////////////////
//
// Exported entry points for registration and unregistration 
// (in this case they only call through to default implementations).
//
////////////////////////////////////////////////////////////////////////
#define CreateComObject(clsid, iid, var) CoCreateInstance( clsid, NULL, CLSCTX_INPROC_SERVER, iid, (void **) &var );

STDAPI DllRegisterServer()
{
    HRESULT hr = AMovieDllRegisterServer2( TRUE );

    if ( SUCCEEDED( hr ) )
    {
        IFilterMapper2* filterMapper = nullptr;

        hr = CreateComObject( CLSID_FilterMapper2, IID_IFilterMapper2, filterMapper );

        if ( SUCCEEDED( hr ) )
        {
            IMoniker*   pMoniker = nullptr;
            REGFILTER2  regFilter;

            regFilter.dwVersion = 1;
            regFilter.dwMerit   = MERIT_DO_NOT_USE;
            regFilter.cPins     = 1;
            regFilter.rgPins    = &outputPinConfig;

            hr = filterMapper->RegisterFilter( CLSID_CVSandboxVirtualCamera, FILTER_NAME_STRING, &pMoniker, &CLSID_VideoInputDeviceCategory, NULL, &regFilter );
        }

        if ( filterMapper != nullptr )
        {
            filterMapper->Release( );
        }
    }

    return hr;
}

STDAPI DllUnregisterServer( )
{
    HRESULT hr = AMovieDllRegisterServer2( FALSE );

    if ( SUCCEEDED( hr ) )
    {
        IFilterMapper2* filterMapper = nullptr;

        hr = CreateComObject( CLSID_FilterMapper2, IID_IFilterMapper2, filterMapper );

        if ( SUCCEEDED( hr ) )
        {
            hr = filterMapper->UnregisterFilter( &CLSID_VideoInputDeviceCategory, 0, CLSID_CVSandboxVirtualCamera );
        }

        if ( filterMapper != nullptr )
        {
            filterMapper->Release( );
        }
    }

    return hr;
}

//
// DllEntryPoint
//
extern "C" BOOL WINAPI DllEntryPoint( HINSTANCE, ULONG, LPVOID );

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  dwReason, LPVOID lpReserved )
{
	return DllEntryPoint( static_cast<HINSTANCE>(hModule), dwReason, lpReserved );
}
