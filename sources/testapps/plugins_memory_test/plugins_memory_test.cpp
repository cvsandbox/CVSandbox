/*
    Automation server's test application

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

#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <stdio.h>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include <XError.hpp>
#include <XPluginsEngine.hpp>
#include <XImageProcessingFilterPlugin.hpp>
#include <XImageProcessingFilterPlugin2.hpp>
#include <XImageGenerationPlugin.hpp>
#include <XImageImportingPlugin.hpp>

using namespace std;
using namespace CVSandbox;

typedef map<XPixelFormat, shared_ptr<XImage>> ImageMap;
typedef map<string, vector<string>> ImageFilesMap;

// Forward declaration of test function
static void TestImageProcessingFilterPlugins( const shared_ptr<const XPluginsCollection>& plugins,
                                              const ImageMap& testImages );
static void TestImageProcessingFilterPlugins2( const shared_ptr<const XPluginsCollection>& plugins,
                                               const ImageMap& testImages1,
                                               const ImageMap& testImages2 );
static void TestImageGenerationPlugins( const shared_ptr<const XPluginsCollection>& plugins );
static void TestImageImportingPlugins( const shared_ptr<const XPluginsCollection>& plugins, const ImageFilesMap& testFiles );
static void TestImageImportingPlugins_Negative( const shared_ptr<const XPluginsCollection>& plugins, const ImageFilesMap& testFiles );

int main( int argc, char* argv[] )
{
    //_CrtSetBreakAlloc( 4276 );

    {
        ImageMap      testImages1;
        ImageMap      testImages2;
        ImageFilesMap testImageFiles;
        ImageFilesMap testBadImageFiles;

        printf( "Testing plug-ins for memory usage (and re-usage) ... \n" );

        // prepare test images of different pixel format
        testImages1.insert( pair<XPixelFormat, shared_ptr<XImage>>( XPixelFormatGrayscale8,  XImage::Allocate( 320, 240, XPixelFormatGrayscale8 ) ) );
        testImages1.insert( pair<XPixelFormat, shared_ptr<XImage>>( XPixelFormatRGB24,       XImage::Allocate( 320, 240, XPixelFormatRGB24 ) ) );
        testImages1.insert( pair<XPixelFormat, shared_ptr<XImage>>( XPixelFormatRGBA32,      XImage::Allocate( 320, 240, XPixelFormatRGBA32 ) ) );
        testImages1.insert( pair<XPixelFormat, shared_ptr<XImage>>( XPixelFormatGrayscale16, XImage::Allocate( 320, 240, XPixelFormatGrayscale16 ) ) );
        testImages1.insert( pair<XPixelFormat, shared_ptr<XImage>>( XPixelFormatRGB48,       XImage::Allocate( 320, 240, XPixelFormatRGB48 ) ) );
        testImages1.insert( pair<XPixelFormat, shared_ptr<XImage>>( XPixelFormatRGBA64,      XImage::Allocate( 320, 240, XPixelFormatRGBA64 ) ) );
        testImages1.insert( pair<XPixelFormat, shared_ptr<XImage>>( XPixelFormatBinary1,     XImage::Allocate( 320, 240, XPixelFormatBinary1 ) ) );
        testImages1.insert( pair<XPixelFormat, shared_ptr<XImage>>( XPixelFormatIndexed1,    XImage::Allocate( 320, 240, XPixelFormatIndexed1 ) ) );
        testImages1.insert( pair<XPixelFormat, shared_ptr<XImage>>( XPixelFormatIndexed2,    XImage::Allocate( 320, 240, XPixelFormatIndexed2 ) ) );
        testImages1.insert( pair<XPixelFormat, shared_ptr<XImage>>( XPixelFormatIndexed4,    XImage::Allocate( 320, 240, XPixelFormatIndexed4 ) ) );
        testImages1.insert( pair<XPixelFormat, shared_ptr<XImage>>( XPixelFormatIndexed8,    XImage::Allocate( 320, 240, XPixelFormatIndexed8 ) ) );

        for_each( testImages1.begin( ), testImages1.end( ), [&testImages2] ( pair<XPixelFormat, shared_ptr<XImage>> testPair )
        {
            testImages2.insert( pair<XPixelFormat, shared_ptr<XImage>>( testPair.first, testPair.second->Clone( ) ) );
        } );

        // prepare test image files of different formats
        testImageFiles["jpg"].push_back( "gray_8bpp.jpg" );
        testImageFiles["jpg"].push_back( "rgb_24bpp.jpg" );

        testImageFiles["png"].push_back( "binary.png" );
        testImageFiles["png"].push_back( "gray_8bpp.png" );
        testImageFiles["png"].push_back( "rgb_24bpp.png" );
        testImageFiles["png"].push_back( "rgba_32bpp.png" );
        testImageFiles["png"].push_back( "indexed_1bpp.png" );
        testImageFiles["png"].push_back( "indexed_2bpp.png" );
        testImageFiles["png"].push_back( "indexed_2bpp_with_transp.png" );
        testImageFiles["png"].push_back( "indexed_4bpp.png" );
        testImageFiles["png"].push_back( "indexed_8bpp.png" );

        testBadImageFiles["jpg"] = testImageFiles["png"];
        testBadImageFiles["png"] = testImageFiles["jpg"];

        // load plug-ins
        shared_ptr<XPluginsEngine> engine = XPluginsEngine::Create( );
        engine->CollectModules( "./cvsplugins/" );

        // get all image processing filer plug-ins
        printf( "> Testing image processing filter plug-ins \n" );
        TestImageProcessingFilterPlugins( engine->GetPluginsOfType( PluginType_ImageProcessingFilter ), testImages1 );

        // get all image processing filer plug-ins which require 2 images
        printf( "> Testing two source image processing filter plug-ins \n" );
        TestImageProcessingFilterPlugins2( engine->GetPluginsOfType( PluginType_ImageProcessingFilter2 ), testImages1, testImages2 );

        // get all image generation plug-ins
        printf( "> Testing image generation plug-ins \n" );
        TestImageGenerationPlugins( engine->GetPluginsOfType( PluginType_ImageGenerator ) );

        // get all image importing plug-ins
        printf( "> Testing image importing plug-ins \n" );
        TestImageImportingPlugins( engine->GetPluginsOfType( PluginType_ImageImporter ), testImageFiles );
        printf( "> Testing image importing plug-ins (with wrong files) \n" );
        TestImageImportingPlugins_Negative( engine->GetPluginsOfType( PluginType_ImageImporter ), testBadImageFiles );
    }

    printf( "========================== \r\n" );
    printf( "Test Done \r\n" );
    printf( "========================== \r\n" );

    getchar( );

#ifdef _MSC_VER
    _CrtDumpMemoryLeaks( );
#endif

    return 0;
}

void TestImageProcessingFilterPlugins( const shared_ptr<const XPluginsCollection>& plugins,
                                       const ImageMap& testImages )
{
    printf( "Plug-ins count: %d \n", static_cast<int>( plugins->Count( ) ) );

    for_each( plugins->begin( ), plugins->end( ), [&testImages] ( const shared_ptr<const XPluginDescriptor>& pluginDesc )
    {
        printf( "." );

        const string pluginName = pluginDesc->Name( );
        shared_ptr<XImageProcessingFilterPlugin> plugin = static_pointer_cast<XImageProcessingFilterPlugin>( pluginDesc->CreateInstance( ) );

        if ( !plugin )
        {
            printf( "\nFailed creating plug-in's instance: %s \n", pluginName.c_str( ) );
        }
        else
        {
            for_each( testImages.begin( ), testImages.end( ), [&plugin, &pluginName] ( pair<XPixelFormat, shared_ptr<XImage>> testPair )
            {
                shared_ptr<XImage> testImage = testPair.second;
                XErrorCode         status;

                if ( plugin->IsPixelFormatSupported( testImage->Format( ) ) )
                {
                    // do in-place filtering first
                    if ( plugin->CanProcessInPlace( ) )
                    {
                        status = plugin->ProcessImage( testImage );

                        if ( status != SuccessCode )
                        {
                            printf( "\n [%s] plug-in failed to process (in-place) image format [%s] : %d (%s) \n",
                                pluginName.c_str( ), XImage::PixelFormatName( testImage->Format( ) ).c_str( ),
                                status, XError::Description( status ).c_str( ) );
                        }
                    }

                    // do processing to another image
                    shared_ptr<XImage> destImage;

                    status = plugin->ProcessImage( testImage, destImage );

                    if ( status != SuccessCode )
                    {
                        printf( "\n [%s] plug-in failed to process image format [%s] : %d (%s) \n",
                            pluginName.c_str( ), XImage::PixelFormatName( testImage->Format( ) ).c_str( ),
                            status, XError::Description( status ).c_str( ) );
                    }
                    else
                    {
                        const void* imagePtr   = static_cast<void*>( destImage->ImageData( ) );
                        const void* palettePtr = static_cast<void*>( destImage->ImageData( )->palette );

                        // do processing again with the same target
                        status = plugin->ProcessImage( testImage, destImage );

                        if ( status != SuccessCode )
                        {
                            printf( "\n [%s] plug-in failed to process image format [%s] : %d (%s) (second call) \n",
                                pluginName.c_str( ), XImage::PixelFormatName( testImage->Format( ) ).c_str( ),
                                status, XError::Description( status ).c_str( ) );
                        }
                        else
                        {
                            bool isSameImage   = ( imagePtr   == static_cast<void*>( destImage->ImageData( ) ) );
                            bool isSamePalette = ( palettePtr == static_cast<void*>( destImage->ImageData( )->palette ) );

                            if ( ( !isSameImage ) || ( !isSamePalette ) )
                            {
                                printf( "\n [%s] plug-in did not reuse target memory when processing format [%s], image data: %s, palette data: %s \n",
                                    pluginName.c_str( ), XImage::PixelFormatName( testImage->Format( ) ).c_str( ),
                                    ( isSameImage ) ? "yes" : "no",
                                    ( isSamePalette ) ? "yes" : "no" );
                            }
                        }
                    }
                }
                else
                {
                    // if pixel format is not supported, we'll still try it to check for error

                    // do in-place filtering first
                    if ( plugin->CanProcessInPlace( ) )
                    {
                        status = plugin->ProcessImage( testImage );

                        if ( status == SuccessCode )
                        {
                            printf( "\n [%s] plug-in succeeded to process (in-place) image format [%s] while it should not \n",
                                pluginName.c_str( ), XImage::PixelFormatName( testImage->Format( ) ).c_str( ) );
                        }
                    }

                    // do processing to another image
                    shared_ptr<XImage> destImage;

                    status = plugin->ProcessImage( testImage, destImage );

                    if ( status == SuccessCode )
                    {
                        printf( "\n [%s] plug-in succeeded to process image format [%s] while it should not \n",
                            pluginName.c_str( ), XImage::PixelFormatName( testImage->Format( ) ).c_str( ) );
                    }
                }
            } );
        }
    } );

    printf( "\n< Done \n" );
}

void TestImageProcessingFilterPlugins2( const shared_ptr<const XPluginsCollection>& plugins,
                                        const ImageMap& testImages1,
                                        const ImageMap& testImages2 )
{
    printf( "Plug-ins count: %d \n", static_cast<int>( plugins->Count( ) ) );

    for_each( plugins->begin( ), plugins->end( ), [&testImages1, &testImages2] ( const shared_ptr<const XPluginDescriptor>& pluginDesc )
    {
        printf( "." );

        const string pluginName = pluginDesc->Name( );
        shared_ptr<XImageProcessingFilterPlugin2> plugin = static_pointer_cast<XImageProcessingFilterPlugin2>( pluginDesc->CreateInstance( ) );

        if ( !plugin )
        {
            printf( "\nFailed creating plug-in's instance: %s \n", pluginName.c_str( ) );
        }
        else
        {
            for_each( testImages1.begin( ), testImages1.end( ), [&plugin, &pluginName, &testImages2] ( pair<XPixelFormat, shared_ptr<XImage>> testPair )
            {
                shared_ptr<XImage>  testImage1 = testPair.second;
                XErrorCode          status;

                if ( plugin->IsPixelFormatSupported( testImage1->Format( ) ) )
                {
                    XPixelFormat             testImage2Format = plugin->GetSecondImageSupportedFormat( testImage1->Format( ) );
                    ImageMap::const_iterator image2It = testImages2.find( testImage2Format );

                    if ( image2It == testImages2.end( ) )
                    {
                        printf( "\nDid not find second image of [%s] format for [%s] plug-in. First image format is [%s] \n",
                            XImage::PixelFormatName( testImage2Format ).c_str( ), pluginName.c_str( ), XImage::PixelFormatName( testImage1->Format( ) ).c_str( ) );
                    }
                    else
                    {
                        shared_ptr<XImage> testImage2 = image2It->second;

                        // do in-place filtering first
                        if ( plugin->CanProcessInPlace( ) )
                        {
                            status = plugin->ProcessImage( testImage1, testImage2 );

                            if ( status != SuccessCode )
                            {
                                printf( "\n [%s] plug-in failed to process (in-place) image format [%s] : %d (%s) \n",
                                    pluginName.c_str( ), XImage::PixelFormatName( testImage1->Format( ) ).c_str( ),
                                    status, XError::Description( status ).c_str( ) );
                            }
                        }

                        // do processing to another image
                        shared_ptr<XImage> destImage;

                        status = plugin->ProcessImage( testImage1, testImage2, destImage );

                        if ( status != SuccessCode )
                        {
                            printf( "\n [%s] plug-in failed to process image format [%s] : %d (%s) \n",
                                pluginName.c_str( ), XImage::PixelFormatName( testImage1->Format( ) ).c_str( ),
                                status, XError::Description( status ).c_str( ) );
                        }
                        else
                        {
                            const void* imagePtr   = static_cast<void*>( destImage->ImageData( ) );
                            const void* palettePtr = static_cast<void*>( destImage->ImageData( )->palette );

                            // do processing again with the same target
                            status = plugin->ProcessImage( testImage1, testImage2, destImage );

                            if ( status != SuccessCode )
                            {
                                printf( "\n [%s] plug-in failed to process image format [%s] : %d (%s) (second call) \n",
                                    pluginName.c_str( ), XImage::PixelFormatName( testImage1->Format( ) ).c_str( ),
                                    status, XError::Description( status ).c_str( ) );
                            }
                            else
                            {
                                bool isSameImage   = ( imagePtr   == static_cast<void*>( destImage->ImageData( ) ) );
                                bool isSamePalette = ( palettePtr == static_cast<void*>( destImage->ImageData( )->palette ) );

                                if ( ( !isSameImage ) || ( !isSamePalette ) )
                                {
                                    printf( "\n [%s] plug-in did not reuse target memory when processing format [%s], image data: %s, palette data: %s \n",
                                        pluginName.c_str( ), XImage::PixelFormatName( testImage1->Format( ) ).c_str( ),
                                        ( isSameImage ) ? "yes" : "no",
                                        ( isSamePalette ) ? "yes" : "no" );
                                }
                            }
                        }
                    }
                }
            } );
        }
    } );

    printf( "\n< Done \n" );
}

void TestImageGenerationPlugins( const shared_ptr<const XPluginsCollection>& plugins )
{
    printf( "Plug-ins count: %d \n", static_cast<int>( plugins->Count( ) ) );

    for_each( plugins->begin( ), plugins->end( ), [] ( const shared_ptr<const XPluginDescriptor>& pluginDesc )
    {
        printf( "." );

        const string pluginName = pluginDesc->Name( );
        shared_ptr<XImageGenerationPlugin> plugin = static_pointer_cast<XImageGenerationPlugin>( pluginDesc->CreateInstance( ) );

        if ( !plugin )
        {
            printf( "\nFailed creating plug-in's instance: %s \n", pluginName.c_str( ) );
        }
        else
        {
            shared_ptr<XImage> generatedImage;
            XErrorCode         status;

            status = plugin->GenerateImage( generatedImage );

            if ( status != SuccessCode )
            {
                printf( "\n [%s] plug-in failed to generate an image : %d (%s) \n",
                    pluginName.c_str( ), status, XError::Description( status ).c_str( ) );
            }
            else
            {
                const void* imagePtr   = static_cast<void*>( generatedImage->ImageData( ) );
                const void* palettePtr = static_cast<void*>( generatedImage->ImageData( )->palette );

                // do image generation again with the same target
                status = plugin->GenerateImage( generatedImage );

                if ( status != SuccessCode )
                {
                    printf( "\n [%s] plug-in failed to generate an image : %d (%s) (second call) \n",
                        pluginName.c_str( ), status, XError::Description( status ).c_str( ) );
                }
                else
                {
                    bool isSameImage   = ( imagePtr == static_cast<void*>( generatedImage->ImageData( ) ) );
                    bool isSamePalette = ( palettePtr == static_cast<void*>( generatedImage->ImageData( )->palette ) );

                    if ( ( !isSameImage ) || ( !isSamePalette ) )
                    {
                        printf( "\n [%s] plug-in did not reuse target memory when generating new image, image data: %s, palette data: %s \n",
                            pluginName.c_str( ),
                            ( isSameImage ) ? "yes" : "no",
                            ( isSamePalette ) ? "yes" : "no" );
                    }
                }
            }
        }
    } );

    printf( "\n< Done \n" );
}

void TestImageImportingPlugins( const shared_ptr<const XPluginsCollection>& plugins, const ImageFilesMap& testFilesMap )
{
    printf( "Plug-ins count: %d \n", static_cast<int>( plugins->Count( ) ) );

    for_each( plugins->begin( ), plugins->end( ), [&testFilesMap] ( const shared_ptr<const XPluginDescriptor>& pluginDesc )
    {
        printf( "." );

        const string pluginName = pluginDesc->Name( );
        shared_ptr<XImageImportingPlugin> plugin = static_pointer_cast<XImageImportingPlugin>( pluginDesc->CreateInstance( ) );

        if ( !plugin )
        {
            printf( "\nFailed creating plug-in's instance: %s \n", pluginName.c_str( ) );
        }
        else
        {
            vector<string> supportedExtensions = plugin->GetSupportedExtensions( );
            bool           foundSomeTestFiles  = false;

            for_each( supportedExtensions.begin( ), supportedExtensions.end( ), [&testFilesMap, &plugin, &pluginName, &foundSomeTestFiles] ( const string& ext )
            {
                ImageFilesMap::const_iterator testFilesMapIt = testFilesMap.find( ext );

                if ( testFilesMapIt != testFilesMap.end( ) )
                {
                    vector<string> testFiles = testFilesMapIt->second;

                    foundSomeTestFiles = true;

                    for_each( testFiles.begin( ), testFiles.end( ), [&plugin, &pluginName] ( const string& fileName )
                    {
                        string              pathName      = "test_images\\" + fileName;
                        shared_ptr<XImage>  importedImage;
                        XErrorCode          status;

                        status = plugin->ImportImage( pathName, importedImage );

                        if ( status != SuccessCode )
                        {
                            printf( "\n [%s] plug-in failed to import the [%s] image : %d (%s) \n",
                                pluginName.c_str( ), fileName.c_str( ), status, XError::Description( status ).c_str( ) );
                        }
                        else
                        {
                            const void* imagePtr   = static_cast<void*>( importedImage->ImageData( ) );
                            const void* palettePtr = static_cast<void*>( importedImage->ImageData( )->palette );

                            // do image importing again with the same target
                            status = plugin->ImportImage( pathName, importedImage );

                            if ( status != SuccessCode )
                            {
                                printf( "\n [%s] plug-in failed to import the [%s] image : %d (%s) (second call) \n",
                                    pluginName.c_str( ), fileName.c_str( ), status, XError::Description( status ).c_str( ) );
                            }
                            else
                            {
                                bool isSameImage   = ( imagePtr == static_cast<void*>( importedImage->ImageData( ) ) );
                                bool isSamePalette = ( palettePtr == static_cast<void*>( importedImage->ImageData( )->palette ) );

                                if ( ( !isSameImage ) || ( !isSamePalette ) )
                                {
                                    printf( "\n [%s] plug-in did not reuse target memory when importing [%s] image (format: %s), image data: %s, palette data: %s \n",
                                        pluginName.c_str( ), fileName.c_str( ),
                                        XImage::PixelFormatName( importedImage->Format( ) ).c_str( ),
                                        ( isSameImage ) ? "yes" : "no",
                                        ( isSamePalette ) ? "yes" : "no" );
                                }
                            }
                        }
                    } );
                }
            } );

            if ( !foundSomeTestFiles )
            {
                printf( "\n Did not find any test files for [%s] plug-in \n", pluginName.c_str( ) );
            }
        }
    } );

    printf( "\n< Done \n" );
}

void TestImageImportingPlugins_Negative( const shared_ptr<const XPluginsCollection>& plugins, const ImageFilesMap& testFilesMap )
{
    printf( "Plug-ins count: %d \n", static_cast<int>( plugins->Count( ) ) );

    for_each( plugins->begin( ), plugins->end( ), [&testFilesMap] ( const shared_ptr<const XPluginDescriptor>& pluginDesc )
    {
        printf( "." );

        const string pluginName = pluginDesc->Name( );
        shared_ptr<XImageImportingPlugin> plugin = static_pointer_cast<XImageImportingPlugin>( pluginDesc->CreateInstance( ) );

        if ( !plugin )
        {
            printf( "\nFailed creating plug-in's instance: %s \n", pluginName.c_str( ) );
        }
        else
        {
            vector<string> supportedExtensions = plugin->GetSupportedExtensions( );
            bool           foundSomeTestFiles = false;

            for_each( supportedExtensions.begin( ), supportedExtensions.end( ), [&testFilesMap, &plugin, &pluginName, &foundSomeTestFiles] ( const string& ext )
            {
                ImageFilesMap::const_iterator testFilesMapIt = testFilesMap.find( ext );

                if ( testFilesMapIt != testFilesMap.end( ) )
                {
                    vector<string> testFiles = testFilesMapIt->second;

                    foundSomeTestFiles = true;

                    for_each( testFiles.begin( ), testFiles.end( ), [&plugin, &pluginName] ( const string& fileName )
                    {
                        string              pathName = "test_images\\" + fileName;
                        shared_ptr<XImage>  importedImage;
                        XErrorCode          status;

                        status = plugin->ImportImage( pathName, importedImage );

                        if ( status == SuccessCode )
                        {
                            printf( "\n [%s] plug-in succeeded to import the [%s] image while it should not \n",
                                pluginName.c_str( ), fileName.c_str( ) );
                        }
                        else
                        {
                            // now allocte some image and pretend we want to import into that
                            importedImage = XImage::Allocate( 320, 240, XPixelFormatIndexed8 );

                            if ( !importedImage )
                            {
                                printf( "\n Failed allocating dummy image \n" );
                            }
                            else
                            {
                                XPalleteAllocate( 256, &( importedImage->ImageData( )->palette ) );

                                // do image importing again into some target
                                status = plugin->ImportImage( pathName, importedImage );

                                if ( status == SuccessCode )
                                {
                                    printf( "\n [%s] plug-in succeeded to import the [%s] image while it should not (second call) \n",
                                        pluginName.c_str( ), fileName.c_str( ) );
                                }
                            }
                        }
                    } );
                }
            } );

            if ( !foundSomeTestFiles )
            {
                printf( "\n Did not find any test files for [%s] plug-in \n", pluginName.c_str( ) );
            }
        }
    } );

    printf( "\n< Done \n" );
}
