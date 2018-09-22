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
#ifndef CVS_WRAPPER_IMAGE_PROCESSING_FILTER_2_HPP
#define CVS_WRAPPER_IMAGE_PROCESSING_FILTER_2_HPP

#include "iplugintypescpp.hpp"
#include "WrapperBaseForCppPlugin.hpp"

// An internal class to register C++ Image Processing Filter plug-in and wrap it into C structure
class PluginRegister_PluginType_ImageProcessingFilter2 : public PluginRegisterAndWrapper
{
private:
    // Extended structure containing pointer to C++ object
    typedef struct _CppImageProcessingFilter2Wrapper
    {
        SImageProcessingFilterPlugin2  Api;
        IImageProcessingFilterPlugin2* PluginObject;
    }
    CppImageProcessingFilter2Wrapper;

    // Wrapper for Dispose() method
    static void Wrapper_Dispose( SPluginBase* me )
    {
        reinterpret_cast<CppImageProcessingFilter2Wrapper*>( me )->PluginObject->Dispose();
        free( me );
    }

    // Wrapper for GetProperty() method
    static XErrorCode Wrapper_GetProperty( SPluginBase* me, int32_t id, xvariant* value )
    {
        return reinterpret_cast<CppImageProcessingFilter2Wrapper*>( me )->PluginObject->GetProperty( id, value );
    }

    // Wrapper for SetProperty() method
    static XErrorCode Wrapper_SetProperty( SPluginBase* me, int32_t id, const xvariant* value )
    {
        return reinterpret_cast<CppImageProcessingFilter2Wrapper*>( me )->PluginObject->SetProperty( id, value );
    }

    // Wrapper for GetIndexedProperty() method
    static XErrorCode Wrapper_GetIndexedProperty( SPluginBase* me, int32_t id, uint32_t index, xvariant* value )
    {
        return reinterpret_cast<CppImageProcessingFilter2Wrapper*>( me )->PluginObject->GetIndexedProperty( id, index, value );
    }

    // Wrapper for SetIndexedProperty() method
    static XErrorCode Wrapper_SetIndexedProperty( SPluginBase* me, int32_t id, uint32_t index, const xvariant* value )
    {
        return reinterpret_cast<CppImageProcessingFilter2Wrapper*>( me )->PluginObject->SetIndexedProperty( id, index, value );
    }

    // Wrapper for CallFunction() method
    static XErrorCode Wrapper_CallFunction( SPluginBase* me, int32_t id, xvariant* returnValue, const xarray* arguments )
    {
        return reinterpret_cast<CppImageProcessingFilter2Wrapper*>( me )->PluginObject->CallFunction( id, returnValue, arguments );
    }

    // Wrapper for UpdateDescription() method
    static XErrorCode Wrapper_UpdateDescription( SPluginBase* me, PluginDescriptor* descriptor )
    {
        return reinterpret_cast<CppImageProcessingFilter2Wrapper*>( me )->PluginObject->UpdateDescription( descriptor );
    }

    // Wrapper for CanProcessInPlace() method
    static xbool Wrapper_CanProcessInPlace( SImageProcessingFilterPlugin2* me )
    {
        return reinterpret_cast<CppImageProcessingFilter2Wrapper*>( me )->PluginObject->CanProcessInPlace( );
    }

    // Wrapper for GetPixelFormatTranslations() method
    static XErrorCode Wrapper_GetPixelFormatTranslations( SImageProcessingFilterPlugin2* me,
                                            XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
    {
        return reinterpret_cast<CppImageProcessingFilter2Wrapper*>( me )->PluginObject->GetPixelFormatTranslations( inputFormats, outputFormats, count );
    }

    // Wrapper for GetSecondImageDescription() method
    static xstring Wrapper_GetSecondImageDescription( SImageProcessingFilterPlugin2* me )
    {
        return reinterpret_cast<CppImageProcessingFilter2Wrapper*>( me )->PluginObject->GetSecondImageDescription( );
    }

    // Get supported size of the second image
    static XSupportedImageSize Wrapper_GetSecondImageSupportedSize( SImageProcessingFilterPlugin2* me )
    {
        return reinterpret_cast<CppImageProcessingFilter2Wrapper*>( me )->PluginObject->GetSecondImageSupportedSize( );
    }

    // Get supported pixel format of the second image for the given format of the source image
    static XPixelFormat Wrapper_GetSecondImageSupportedFormat( SImageProcessingFilterPlugin2* me,
                                                               XPixelFormat inputPixelFormat )
    {
        return reinterpret_cast<CppImageProcessingFilter2Wrapper*>( me )->PluginObject->GetSecondImageSupportedFormat( inputPixelFormat );
    }

    // Wrapper for ProcessImage() method
    static XErrorCode Wrapper_ProcessImage( SImageProcessingFilterPlugin2* me, const ximage* src, const ximage* src2, ximage** dst )
    {
        return reinterpret_cast<CppImageProcessingFilter2Wrapper*>( me )->PluginObject->ProcessImage( src, src2, dst );
    }

    // Wrapper for ProcessImageInPlace() method
    static XErrorCode Wrapper_ProcessImageInPlace( SImageProcessingFilterPlugin2* me, ximage* src, const ximage* src2 )
    {
        return reinterpret_cast<CppImageProcessingFilter2Wrapper*>( me )->PluginObject->ProcessImageInPlace( src, src2 );
    }

public:
    PluginRegister_PluginType_ImageProcessingFilter2( xguid id, xguid family,
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
    static SImageProcessingFilterPlugin* CreateWrapper( IImageProcessingFilterPlugin2* po )
    {
        CppImageProcessingFilter2Wrapper* wrapper = (CppImageProcessingFilter2Wrapper*) malloc( sizeof( CppImageProcessingFilter2Wrapper ) );

        // save the C++ object pointer
        wrapper->PluginObject = po;

        // set base plug-in's methods
        wrapper->Api.Base.Dispose                  = Wrapper_Dispose;
        wrapper->Api.Base.GetProperty              = Wrapper_GetProperty;
        wrapper->Api.Base.SetProperty              = Wrapper_SetProperty;
        wrapper->Api.Base.GetIndexedProperty       = Wrapper_GetIndexedProperty;
        wrapper->Api.Base.SetIndexedProperty       = Wrapper_SetIndexedProperty;
        wrapper->Api.Base.CallFunction             = Wrapper_CallFunction;
        wrapper->Api.Base.UpdateDescription        = Wrapper_UpdateDescription;

        // set image processing plug-in's methods
        wrapper->Api.CanProcessInPlace             = Wrapper_CanProcessInPlace;
        wrapper->Api.GetPixelFormatTranslations    = Wrapper_GetPixelFormatTranslations;
        wrapper->Api.GetSecondImageDescription     = Wrapper_GetSecondImageDescription;
        wrapper->Api.GetSecondImageSupportedSize   = Wrapper_GetSecondImageSupportedSize;
        wrapper->Api.GetSecondImageSupportedFormat = Wrapper_GetSecondImageSupportedFormat;
        wrapper->Api.ProcessImage                  = Wrapper_ProcessImage;
        wrapper->Api.ProcessImageInPlace           = Wrapper_ProcessImageInPlace;

        return reinterpret_cast<SImageProcessingFilterPlugin*>( wrapper );
    }
};

#endif // CVS_WRAPPER_IMAGE_PROCESSING_FILTER_2_HPP
