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

#include <QApplication>
#include <QImage>
#include <QFileInfo>
#include <QFileDialog>
#include <QVector>
#include <QMap>

#include <XImageImportingPlugin.hpp>
#include <XImageExportingPlugin.hpp>

#include "ImageFileService.hpp"

using namespace std;
using namespace CVSandbox;

// Some private data
namespace Private
{
    class ImageFileServiceData
    {
    public:
        ImageFileServiceData( ) :
            ImporterDescriptons( ), ImporterExtensions( ), ImporterMap( ),
            ExporterDescriptons( ), ExporterExtensions( ), ExporterMap( ),

            mAreImportersCollected( false ), mAreExportersCollected( false )
        {
        }

        // Collect available image file importing plug-ins
        void CollectImporters( const shared_ptr<const XPluginsEngine>& pluginsEngine );
        // Collect available image file exporting plug-ins
        void CollectExporters( const shared_ptr<const XPluginsEngine>& pluginsEngine );
        // Get filter string for file open dialog
        const QString GetFileOpenFilterString( ) const;
        // Get filter string for file save dialog
        const QString GetFileSaveFilterString( ) const;
        // Get importing plug-in for the extension
        const shared_ptr<const XImageImportingPlugin> GetImportingPlugin( const QString& extension );
        // Get exporting plug-in for the extension
        const shared_ptr<const XImageExportingPlugin> GetExportingPlugin( const QString& extension );

    private:
        // Build files' filter string for the provided file types/extensions
        static const QString BuildFilterString( const QVector<QString>& fileDescriptions,
                                                const QVector<QVector<QString> >& fileExtension );

    public:
        QVector<QString>                                        ImporterDescriptons;
        QVector<QVector<QString> >                              ImporterExtensions;
        QMap<QString, shared_ptr<const XImageImportingPlugin> > ImporterMap;

        QVector<QString>                                        ExporterDescriptons;
        QVector<QVector<QString> >                              ExporterExtensions;
        QMap<QString, shared_ptr<const XImageExportingPlugin> > ExporterMap;

    private:
        bool mAreImportersCollected;
        bool mAreExportersCollected;
    };
}


// Create the file service
ImageFileService::ImageFileService( const shared_ptr<const XPluginsEngine>& pluginsEngine ) :
    mData( new Private::ImageFileServiceData( ) ),
    mPluginsEngine( pluginsEngine )
{
}

// Destroy the file service
ImageFileService::~ImageFileService( )
{
    delete mData;
}

// Prompt user to select image file by showing file open dialog
const QString ImageFileService::PromptFileOpen( const QString& title, const QString& folder, const QWidget* parent )
{
    mData->CollectImporters( mPluginsEngine );

    QString fileName = QFileDialog::getOpenFileName( const_cast<QWidget*>( parent ),
                            title, folder, mData->GetFileOpenFilterString( ) );
    QString canonicalFileName = QFileInfo( fileName ).canonicalFilePath( );

    return canonicalFileName;
}

// Prompt user to specify file name to save image to
const QString ImageFileService::PromptFileSave( const QString& title, const QString& folder, const QWidget* parent )
{
    mData->CollectExporters( mPluginsEngine );

    QString fileName = QFileDialog::getSaveFileName( const_cast<QWidget*>( parent ),
                            title, folder, mData->GetFileSaveFilterString( ) );

    return fileName;
}

// Open specified image file
XErrorCode ImageFileService::Open( const QString& fileName, shared_ptr<XImage>& image )
{
    mData->CollectImporters( mPluginsEngine );

    XErrorCode ret = ErrorFailed;
    QFileInfo  fileInfo( fileName );

    shared_ptr<const XImageImportingPlugin> plugin = mData->GetImportingPlugin( fileInfo.suffix( ).toLower( ) );

    if ( !plugin )
    {
        ret = ErrorUnknownImageFileFormat;
    }
    else
    {
        ret = plugin->ImportImage( string( fileName.toUtf8( ).data( ) ), image );
    }

    return ret;
}

// Save image to the specified file
XErrorCode ImageFileService::Save( const QString& fileName, const std::shared_ptr<const CVSandbox::XImage>& image )
{
    mData->CollectExporters( mPluginsEngine );

    XErrorCode ret = ErrorFailed;
    QFileInfo  fileInfo( fileName );

    shared_ptr<const XImageExportingPlugin> plugin = mData->GetExportingPlugin( fileInfo.suffix( ).toLower( ) );

    if ( !plugin )
    {
        ret = ErrorUnknownImageFileFormat;
    }
    else
    {
        ret = plugin->ExportImage( string( fileName.toUtf8( ).data( ) ), image );
    }

    return ret;
}

namespace Private
{

// Collect available image file importing plug-ins
void ImageFileServiceData::CollectImporters( const shared_ptr<const XPluginsEngine>& pluginsEngine )
{
    if ( !mAreImportersCollected )
    {
        shared_ptr<const XPluginsCollection> plugins = pluginsEngine->GetPluginsOfType( PluginType_ImageImporter );

        // check all available image importing plug-ins
        for ( XPluginsCollection::ConstIterator pluginDesc = plugins->begin( );
              pluginDesc != plugins->end( ); pluginDesc++ )
        {
            shared_ptr<XImageImportingPlugin> plugin = static_pointer_cast<XImageImportingPlugin>( (*pluginDesc)->CreateInstance( ) );

            QVector<QString> supportedExtensions;
            vector<string>   exts = plugin->GetSupportedExtensions( );

            // check all supported file extensions of the plug-in
            for ( vector<string>::const_iterator ext = exts.begin( ); ext != exts.end( ); ext++ )
            {
                QString qext = QString::fromUtf8( ext->c_str( ) ).toLower( );

                if ( !ImporterMap.contains( qext ) )
                {
                    // collect the plug-in only if we don't have another one which supports same file extension
                    ImporterMap.insert( qext, plugin );
                    supportedExtensions.append( qext );
                }
            }

            if ( supportedExtensions.count( ) != 0 )
            {
                ImporterExtensions.append( supportedExtensions );
                ImporterDescriptons.append( QString::fromUtf8( plugin->GetFileTypeDescription( ).c_str( ) ) );
            }
        }

        mAreImportersCollected = true;
    }
}

// Collect available image file exporting plug-ins
void ImageFileServiceData::CollectExporters( const shared_ptr<const XPluginsEngine>& pluginsEngine )
{
    if ( !mAreExportersCollected )
    {
        shared_ptr<const XPluginsCollection> plugins = pluginsEngine->GetPluginsOfType( PluginType_ImageExporter );

        // check all available image exporting plug-ins
        for ( XPluginsCollection::ConstIterator pluginDesc = plugins->begin( );
              pluginDesc != plugins->end( ); pluginDesc++ )
        {
            shared_ptr<XImageExportingPlugin> plugin = static_pointer_cast<XImageExportingPlugin>( (*pluginDesc)->CreateInstance( ) );

            QVector<QString> supportedExtensions;
            vector<string>   exts = plugin->GetSupportedExtensions( );

            // check all supported file extensions of the plug-in
            for ( vector<string>::const_iterator ext = exts.begin( ); ext != exts.end( ); ext++ )
            {
                QString qext = QString::fromUtf8( ext->c_str( ) ).toLower( );

                if ( !ExporterMap.contains( qext ) )
                {
                    // collect the plug-in only if we don't have another one which supports same file extension
                    ExporterMap.insert( qext, plugin );
                    supportedExtensions.append( qext );
                }
            }

            if ( supportedExtensions.count( ) != 0 )
            {
                ExporterExtensions.append( supportedExtensions );
                ExporterDescriptons.append( QString::fromUtf8( plugin->GetFileTypeDescription( ).c_str( ) ) );
            }
        }

        mAreImportersCollected = true;
    }
}

// Get filter string for file open dialog
const QString ImageFileServiceData::GetFileOpenFilterString( ) const
{
    return BuildFilterString( ImporterDescriptons, ImporterExtensions );
}

// Get filter string for file save dialog
const QString ImageFileServiceData::GetFileSaveFilterString( ) const
{
    return BuildFilterString( ExporterDescriptons, ExporterExtensions );
}

// Build files' filter string for the provided file types/extensions
const QString ImageFileServiceData::BuildFilterString( const QVector<QString>& fileDescriptions,
                                                       const QVector<QVector<QString> >& fileExtension )
{
    QString completeFilterString = "All image files (";
    QString filterString;

    for ( int i = 0, n = fileDescriptions.count( ); i < n; i++ )
    {
        filterString.append( ";;" );
        filterString.append( fileDescriptions[i] );
        filterString.append( " (" );

        QVector<QString> pluginExts = fileExtension[i];
        QString mergedExts;

        for ( int j = 0, m = pluginExts.count( ); j < m; j++ )
        {
            if ( j != 0 )
            {
                mergedExts.append( " " );
            }
            mergedExts.append( "*." );
            mergedExts.append( pluginExts[j] );
        }

        filterString.append( mergedExts );
        filterString.append( ")" );

        if ( i != 0 )
        {
            completeFilterString.append( " " );
        }
        completeFilterString.append( mergedExts );
    }

    completeFilterString.append( ")" );
    completeFilterString.append( filterString );

    return completeFilterString;
}


// Get importing plug-in for the extension
const shared_ptr<const XImageImportingPlugin> ImageFileServiceData::GetImportingPlugin( const QString& extension )
{
    return ( ImporterMap.contains( extension ) ) ? ImporterMap[extension] : shared_ptr<const XImageImportingPlugin>( );
}

// Get exporting plug-in for the extension
const shared_ptr<const XImageExportingPlugin> ImageFileServiceData::GetExportingPlugin( const QString& extension )
{
    return ( ExporterMap.contains( extension ) ) ? ExporterMap[extension] : shared_ptr<const XImageExportingPlugin>( );
}

}
