/*
    Plug-ins' interface library of Computer Vision Sandbox

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
#ifndef CVS_WRAPPER_DEVICE_HPP
#define CVS_WRAPPER_DEVICE_HPP

#include "iplugintypescpp.hpp"
#include "WrapperBaseForCppPlugin.hpp"

// An internal class to register C++ Device plug-in and wrap it into C structure
class PluginRegister_PluginType_Device : public PluginRegisterAndWrapper
{
private:
    // Extended structure containing pointer to C++ object
    typedef struct _CppDeviceWrapper
    {
        SDevicePlugin  Api;
        IDevicePlugin* PluginObject;
    }
    CppDeviceWrapper;

    // Wrapper for Dispose() method
    static void Wrapper_Dispose( SPluginBase* me )
    {
        reinterpret_cast<CppDeviceWrapper*>( me )->PluginObject->Dispose();
        free( me );
    }

    // Wrapper for GetProperty() method
    static XErrorCode Wrapper_GetProperty( SPluginBase* me, int32_t id, xvariant* value )
    {
        return reinterpret_cast<CppDeviceWrapper*>( me )->PluginObject->GetProperty( id, value );
    }

    // Wrapper for SetProperty() method
    static XErrorCode Wrapper_SetProperty( SPluginBase* me, int32_t id, const xvariant* value )
    {
        return reinterpret_cast<CppDeviceWrapper*>( me )->PluginObject->SetProperty( id, value );
    }

    // Wrapper for GetIndexedProperty() method
    static XErrorCode Wrapper_GetIndexedProperty( SPluginBase* me, int32_t id, uint32_t index, xvariant* value )
    {
        return reinterpret_cast<CppDeviceWrapper*>( me )->PluginObject->GetIndexedProperty( id, index, value );
    }

    // Wrapper for SetIndexedProperty() method
    static XErrorCode Wrapper_SetIndexedProperty( SPluginBase* me, int32_t id, uint32_t index, const xvariant* value )
    {
        return reinterpret_cast<CppDeviceWrapper*>( me )->PluginObject->SetIndexedProperty( id, index, value );
    }

    // Wrapper for CallFunction() method
    static XErrorCode Wrapper_CallFunction( SPluginBase* me, int32_t id, xvariant* returnValue, const xarray* arguments )
    {
        return reinterpret_cast<CppDeviceWrapper*>( me )->PluginObject->CallFunction( id, returnValue, arguments );
    }

    // Wrapper for UpdateDescription() method
    static XErrorCode Wrapper_UpdateDescription( SPluginBase* me, PluginDescriptor* descriptor )
    {
        return reinterpret_cast<CppDeviceWrapper*>( me )->PluginObject->UpdateDescription( descriptor );
    }

    // Wrapper for Connect() method
    static XErrorCode Wrapper_Connect( SDevicePlugin* me )
    {
        return reinterpret_cast<CppDeviceWrapper*>( me )->PluginObject->Connect( );
    }

    // Wrapper for Disconnect() method
    static void Wrapper_Disconnect( SDevicePlugin* me )
    {
        reinterpret_cast<CppDeviceWrapper*>( me )->PluginObject->Disconnect( );
    }

    // Wrapper for IsConnected() method
    static bool Wrapper_IsConnected( SDevicePlugin* me )
    {
        return reinterpret_cast<CppDeviceWrapper*>( me )->PluginObject->IsConnected( );
    }

public:
    PluginRegister_PluginType_Device( xguid id, xguid family,
        PluginType type, xversion version,
        const char* name, const char* shortName, const char* description, const char* help,
        CreatePluginFunc creator,
        const ximage* smallIcon = 0, const ximage* icon = 0,
        int32_t propsCount = 0, PropertyDescriptor** props = 0,
        PluginInitializationHandler init = 0, PluginCleanupHandler cleanup = 0,
        PropertyDescriptorUpdater updater = 0,
        int32_t functionsCount = 0, FunctionDescriptor** funcs = 0 )
        :
        PluginRegisterAndWrapper( id, family, type, version, name, shortName,
                                  description, help, creator,
                                  smallIcon, icon, propsCount, props,
                                  init, cleanup, updater, functionsCount, funcs )
    {
    }

    // Create C structure wrapping C++ object
    static SDevicePlugin* CreateWrapper( IDevicePlugin* po )
    {
        CppDeviceWrapper* wrapper = (CppDeviceWrapper*) malloc( sizeof( CppDeviceWrapper ) );

        // save the C++ object pointer
        wrapper->PluginObject = po;

        // set base plug-in's methods
        wrapper->Api.Base.Dispose            = Wrapper_Dispose;
        wrapper->Api.Base.GetProperty        = Wrapper_GetProperty;
        wrapper->Api.Base.SetProperty        = Wrapper_SetProperty;
        wrapper->Api.Base.GetIndexedProperty = Wrapper_GetIndexedProperty;
        wrapper->Api.Base.SetIndexedProperty = Wrapper_SetIndexedProperty;
        wrapper->Api.Base.CallFunction       = Wrapper_CallFunction;
        wrapper->Api.Base.UpdateDescription  = Wrapper_UpdateDescription;

        // set device plug-in's methods
        wrapper->Api.Connect          = Wrapper_Connect;
        wrapper->Api.Disconnect       = Wrapper_Disconnect;
        wrapper->Api.IsConnected      = Wrapper_IsConnected;

        return reinterpret_cast<SDevicePlugin*>( wrapper );
    }
};

#endif // CVS_WRAPPER_DEVICE_HPP
