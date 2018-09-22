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

#include <stdlib.h>
#include <string.h>
#include "iplugin.h"
#include "imodule.h"

// Create a copy of the specified plugin descriptor
PluginDescriptor* CopyPluginDescriptor( const PluginDescriptor* src )
{
    PluginDescriptor* copy = 0;

    if ( src != 0 )
    {
        copy = (PluginDescriptor*) XCAlloc( 1, sizeof( PluginDescriptor ) );

        if ( copy != 0 )
        {
            copy->Type     = src->Type;
            copy->Creator  = src->Creator;
            copy->Version  = src->Version;

            copy->Name          = XStringAlloc( src->Name );
            copy->ShortName     = XStringAlloc( src->ShortName );
            copy->Description   = XStringAlloc( src->Description );
            copy->Help          = XStringAlloc( src->Help );

            copy->PropertyUpdater = src->PropertyUpdater;

            XGuidCopy( &src->ID, &copy->ID );
            XGuidCopy( &src->Family, &copy->Family );

            XImageClone( src->SmallIcon, &copy->SmallIcon );
            XImageClone( src->Icon, &copy->Icon );

            // copy properties descriptors
            if ( ( src->PropertiesCount != 0 ) && ( src->Properties != 0 ) )
            {
                copy->Properties = (PropertyDescriptor**) XCAlloc( src->PropertiesCount, sizeof( PropertyDescriptor* ) );

                if ( copy->Properties != 0 )
                {
                    int i;

                    for ( i = 0; i < src->PropertiesCount; i++ )
                    {
                        copy->Properties[i] = CopyPropertyDescriptor( src->Properties[i] );
                    }

                    copy->PropertiesCount = src->PropertiesCount;
                }
            }

            // copy functions' descriptors
            if ( ( src->FunctionsCount != 0 ) && ( src->Functions != 0 ) )
            {
                copy->Functions = (FunctionDescriptor**) XCAlloc( src->FunctionsCount, sizeof( FunctionDescriptor* ) );

                if ( copy->Functions != 0 )
                {
                    int i;

                    for ( i = 0; i < src->FunctionsCount; i++ )
                    {
                        copy->Functions[i] = CopyFunctionDescriptor( src->Functions[i] );
                    }

                    copy->FunctionsCount = src->FunctionsCount;
                }
            }
        }
    }

    return copy;
}

// Create a copy of the specified property descriptor
PropertyDescriptor* CopyPropertyDescriptor( const PropertyDescriptor* src )
{
    PropertyDescriptor* copy = 0;

    if ( src != 0 )
    {
        copy = (PropertyDescriptor*) XCAlloc( 1, sizeof( PropertyDescriptor ) );

        if ( copy != 0 )
        {
            copy->Type  = src->Type;
            copy->Flags = src->Flags;

            XVariantCopy( &src->DefaultValue, &copy->DefaultValue );
            XVariantCopy( &src->MinValue, &copy->MinValue );
            XVariantCopy( &src->MaxValue, &copy->MaxValue );

            copy->Name           = XStringAlloc( src->Name );
            copy->ShortName      = XStringAlloc( src->ShortName );
            copy->Description    = XStringAlloc( src->Description );

            copy->ParentProperty = src->ParentProperty;
            copy->Updater        = src->Updater;

            if ( ( src->ChoicesCount != 0 ) && ( src->Choices != 0 ) )
            {
                copy->Choices = (xvariant*) XCAlloc( src->ChoicesCount, sizeof( xvariant ) );

                if ( copy->Choices != 0 )
                {
                    int i;

                    for ( i = 0; i < src->ChoicesCount; i++ )
                    {
                        XVariantCopy( &src->Choices[i], &copy->Choices[i] );
                    }

                    copy->ChoicesCount = src->ChoicesCount;
                }
            }
        }
    }

    return copy;
}

// Create a copy of the specified function descriptor
FunctionDescriptor* CopyFunctionDescriptor( const FunctionDescriptor* src )
{
    FunctionDescriptor* copy = 0;

    if ( src != 0 )
    {
        copy = (FunctionDescriptor*) XCAlloc( 1, sizeof( FunctionDescriptor ) );

        if ( copy != 0 )
        {
            copy->ReturnType  = src->ReturnType;
            copy->Name        = XStringAlloc( src->Name );
            copy->Description = XStringAlloc( src->Description );

            if ( ( src->ArgumentsCount != 0 ) && ( src->Arguments != 0 ) )
            {
                copy->Arguments = (ArgumentDescriptor**) XCAlloc( src->ArgumentsCount, sizeof( ArgumentDescriptor* ) );

                if ( copy->Arguments != 0 )
                {
                    int i;

                    for ( i = 0; i < src->ArgumentsCount; i++ )
                    {
                        copy->Arguments[i] = CopyArgumentDescriptor( src->Arguments[i] );
                    }

                    copy->ArgumentsCount = src->ArgumentsCount;
                }
            }
        }
    }

    return copy;
}

// Create a copy of the specified argument descriptor
ArgumentDescriptor* CopyArgumentDescriptor( const ArgumentDescriptor* src )
{
    ArgumentDescriptor* copy = 0;

    if ( src != 0 )
    {
        copy = (ArgumentDescriptor*) XCAlloc( 1, sizeof( ArgumentDescriptor ) );

        if ( copy != 0 )
        {
            copy->Name        = XStringAlloc( src->Name );
            copy->Description = XStringAlloc( src->Description );
            copy->Type        = src->Type;
        }
    }

    return copy;
}


// Create a copy of the specified module descriptor
ModuleDescriptor* CopyModuleDescriptor( const ModuleDescriptor* src )
{
    ModuleDescriptor* copy = 0;

    if ( src != 0 )
    {
        copy = (ModuleDescriptor*) XCAlloc( 1, sizeof( ModuleDescriptor ) );

        if ( copy != 0 )
        {
            copy->Version  = src->Version;
            copy->PluginsCount = src->PluginsCount;

            copy->Name        = XStringAlloc( src->Name );
            copy->ShortName   = XStringAlloc( src->ShortName );
            copy->Description = XStringAlloc( src->Description );
            copy->Vendor      = XStringAlloc( src->Vendor );
            copy->Copyright   = XStringAlloc( src->Copyright );
            copy->Website     = XStringAlloc( src->Website );

            XImageClone( src->SmallIcon, &copy->SmallIcon );
            XImageClone( src->Icon, &copy->Icon );
            XGuidCopy( &src->ID, &copy->ID );
        }
    }

    return copy;
}

// Free memory taken by the specified plugin descriptor
void FreePluginDescriptor( PluginDescriptor** pDescriptor )
{
    if ( ( pDescriptor != 0 ) && ( *pDescriptor != 0 ) )
    {
        PluginDescriptor* descriptor = *pDescriptor;

        XStringFree( &descriptor->Name );
        XStringFree( &descriptor->ShortName );
        XStringFree( &descriptor->Description );
        XStringFree( &descriptor->Help );
        XImageFree( &descriptor->SmallIcon );
        XImageFree( &descriptor->Icon );

        if ( descriptor->Properties != 0 )
        {
            int i;

            for ( i = 0; i < descriptor->PropertiesCount; i++ )
            {
                FreePropertyDescriptor( &descriptor->Properties[i] );
            }
            XFree( (void**) &descriptor->Properties );
        }

        if ( descriptor->Functions != 0 )
        {
            int i;

            for ( i = 0; i < descriptor->FunctionsCount; i++ )
            {
                FreeFunctionDescriptor( &descriptor->Functions[i] );
            }
            XFree( (void**) &descriptor->Functions );
        }

        XFree( (void**) pDescriptor );
    }
}

// Free memory taken by the specified property descriptor
void FreePropertyDescriptor( PropertyDescriptor** pDescriptor )
{
    if ( ( pDescriptor != 0 ) && ( *pDescriptor != 0 ) )
    {
        PropertyDescriptor* descriptor = *pDescriptor;

        XStringFree( &descriptor->Name );
        XStringFree( &descriptor->ShortName );
        XStringFree( &descriptor->Description );

        XVariantClear( &descriptor->DefaultValue );
        XVariantClear( &descriptor->MinValue );
        XVariantClear( &descriptor->MaxValue );

        FreePropertyChoices( descriptor );

        XFree( (void**) pDescriptor );
    }
}

// Free memory taken by the specified function descriptor
void FreeFunctionDescriptor( FunctionDescriptor** pDescriptor )
{
    if ( ( pDescriptor != 0 ) && ( *pDescriptor != 0 ) )
    {
        FunctionDescriptor* descriptor = *pDescriptor;

        XStringFree( &descriptor->Name );
        XStringFree( &descriptor->Description );

        if ( descriptor->Arguments != 0 )
        {
            int i;

            for ( i = 0; i < descriptor->ArgumentsCount; i++ )
            {
                FreeArgumentDescriptor( &descriptor->Arguments[i] );
            }
            XFree( (void**) &descriptor->Arguments );
        }

        XFree( (void**) pDescriptor );
    }
}

// Free memory taken by the specified argument descriptor
void FreeArgumentDescriptor( ArgumentDescriptor** pDescriptor )
{
    if ( ( pDescriptor != 0 ) && ( *pDescriptor != 0 ) )
    {
        ArgumentDescriptor* descriptor = *pDescriptor;

        XStringFree( &descriptor->Name );
        XStringFree( &descriptor->Description );

        XFree( (void**) pDescriptor );
    }
}

// Free memory taken by choices only
void FreePropertyChoices( PropertyDescriptor* pDescriptor )
{
    if ( ( pDescriptor->ChoicesCount != 0 ) && ( pDescriptor->Choices != 0 ) )
    {
        int i;

        for ( i = 0; i < pDescriptor->ChoicesCount; i++ )
        {
            XVariantClear( &pDescriptor->Choices[i] );
        }

        XFree( (void**) &pDescriptor->Choices );
    }
}


// Free memory taken by the specified module descriptor
void FreeModuleDescriptor( ModuleDescriptor** pDescriptor )
{
    if ( ( pDescriptor != 0 ) && ( *pDescriptor != 0 ) )
    {
        ModuleDescriptor* descriptor = *pDescriptor;

        XStringFree( &descriptor->ShortName );
        XStringFree( &descriptor->Name );
        XStringFree( &descriptor->Description );
        XStringFree( &descriptor->Vendor );
        XStringFree( &descriptor->Copyright );
        XStringFree( &descriptor->Website );
        XImageFree( &descriptor->SmallIcon );
        XImageFree( &descriptor->Icon );

        XFree( (void**) pDescriptor );
    }
}
