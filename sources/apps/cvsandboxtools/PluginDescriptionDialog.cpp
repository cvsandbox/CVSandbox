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

#include "PluginDescriptionDialog.hpp"
#include "ui_PluginDescriptionDialog.h"
#include "GlobalServiceManager.hpp"
#include <QDesktopServices>
#include <QtGlobal>

#include <vector>

#include <XImageProcessingFilterPlugin.hpp>
#include <XImageProcessingFilterPlugin2.hpp>
#include <XVideoProcessingPlugin.hpp>

using namespace std;
using namespace CVSandbox;

namespace Private
{
    class PluginDescriptionDialogData
    {
    public:
        PluginDescriptionDialogData( Ui::PluginDescriptionDialog* ui, QWidget* parent ) :
            Ui( ui ), Parent( parent ), History( ), HistoryIndex( -1 ), IsScriptingMode( false )
        {

        }

        void ShowHelp( const XGuid& id );
        void AddToHistory( const XGuid& id );
        void UpdateNavigationButtonsState( );

    private:
        void ShowModulesList( );
        void ShowModuleDescription( const shared_ptr<const XPluginsModule>& moduleDesc );
        void ShowPluginDescription( const shared_ptr<const XPluginDescriptor>& pluginDesc,
                                    const shared_ptr<const XPluginsModule>& moduleDesc );
        void ShowContent( const QString& bodyHtml );

    public:
        Ui::PluginDescriptionDialog*    Ui;
        QWidget*                        Parent;
        vector<XGuid>                   History;
        int                             HistoryIndex;
        bool                            IsScriptingMode;
    };

    static const QString VariantTypeToString( XVarType type );
    static const QString GetPluginType( const shared_ptr<const XPluginDescriptor>& pluginDesc );
    static const QString GetPluginTypeSpecificInfo( const shared_ptr<const XPluginDescriptor>& pluginDesc, bool scriptingMode );
    static const QString GetPropertyValueInfo( const shared_ptr<const XPropertyDescriptor>& propertyDesc, bool scriptingMode );
    static const QString GetPropertyTypeInfo( const shared_ptr<const XPropertyDescriptor>& propertyDesc );
    static const QString GetStringFromPixelFormats( const vector<XPixelFormat>& formats );
    static const QString GetFunctionDescription( const shared_ptr<const XFunctionDescriptor>& functionDesc );
}

PluginDescriptionDialog::PluginDescriptionDialog( QWidget* parent ) :
    QDialog( parent ),
    ui( new Ui::PluginDescriptionDialog ),
    mData( new Private::PluginDescriptionDialogData( ui, this ) )
{
    ui->setupUi( this );

    // disable "?" button on title bar
    setWindowFlags( windowFlags( ) & ~Qt::WindowContextHelpButtonHint );

    // restore size/position
    GlobalServiceManager::Instance( ).GetUiPersistenceService( )->RestoreWidget( this );
}

PluginDescriptionDialog::~PluginDescriptionDialog( )
{
    delete mData;
    delete ui;
}

// Widget is closed
void PluginDescriptionDialog::closeEvent( QCloseEvent* )
{
    GlobalServiceManager::Instance( ).GetUiPersistenceService( )->SaveWidget( this );
}

// Dialog is accepted
void PluginDescriptionDialog::accept( )
{
    GlobalServiceManager::Instance( ).GetUiPersistenceService( )->SaveWidget( this );
    QDialog::accept( );
}

// Dialog is rejected
void PluginDescriptionDialog::reject( )
{
    GlobalServiceManager::Instance( ).GetUiPersistenceService( )->SaveWidget( this );
    QDialog::reject( );
}

// Show description for the specified plug-in
void PluginDescriptionDialog::ShowPluginDescription( const XGuid& id )
{
    mData->ShowHelp( id );
    mData->AddToHistory( id );
    mData->UpdateNavigationButtonsState( );
}

// Go back in navigation history
void PluginDescriptionDialog::on_goBackButton_clicked( )
{
    if ( mData->HistoryIndex > 0 )
    {
        mData->HistoryIndex--;
        mData->ShowHelp( mData->History[mData->HistoryIndex] );
        mData->UpdateNavigationButtonsState( );
    }
}

// Go forward in navigation history
void PluginDescriptionDialog::on_goForwardButton_clicked( )
{
    if ( mData->HistoryIndex < static_cast<int>( mData->History.size( ) ) - 1 )
    {
        mData->HistoryIndex++;
        mData->ShowHelp( mData->History[mData->HistoryIndex] );
        mData->UpdateNavigationButtonsState( );
    }
}

// Show/hide scripting related information
void PluginDescriptionDialog::on_scriptingInfoButton_toggled( bool checked )
{
    mData->IsScriptingMode = checked;

    if ( mData->HistoryIndex <  static_cast<int>( mData->History.size( ) ) )
    {
        mData->ShowHelp( mData->History[mData->HistoryIndex] );
    }
}

// Anchor clicked in the help view
void PluginDescriptionDialog::on_textEdit_anchorClicked( const QUrl& url )
{
    if ( url.toString( ).startsWith( "http" ) )
    {
        QDesktopServices::openUrl( url );
    }
    else
    {
        #if QT_VERSION >= 0x050000
            QString strUrl = url.toDisplayString( QUrl::FullyDecoded );
        #else
            QString strUrl = url.toString( );
        #endif

        if ( strUrl.startsWith( '{' ) )
        {
            ShowPluginDescription( XGuid::FromString( strUrl.mid( 1, 35 ).toStdString( ) ) );
        }
    }
}

namespace Private
{

// Show help for the specified ID
void PluginDescriptionDialogData::ShowHelp( const XGuid& id )
{
    if ( id.IsEmpty( ) )
    {
        ShowModulesList( );
    }
    else
    {
        shared_ptr<const XPluginsModule>    moduleDesc;
        shared_ptr<const XPluginDescriptor> pluginDesc;

        if ( ( moduleDesc = GlobalServiceManager::Instance( ).GetPluginsEngine( )->GetModule( id ) ) )
        {
            ShowModuleDescription( moduleDesc );
        }
        else if ( ( pluginDesc = GlobalServiceManager::Instance( ).GetPluginsEngine( )->GetPlugin( id, moduleDesc ) ) )
        {
            ShowPluginDescription( pluginDesc, moduleDesc );
        }
        else
        {
            Parent->setWindowTitle( QString::null );
            Ui->textEdit->setHtml( QString::null );
        }
    }
}

// Custom comparer to sort modules by name
static bool ModulesListByName( const shared_ptr<const XPluginsModule>& module1, const shared_ptr<const XPluginsModule>& module2 )
{
    return module1->Name( ) < module2->Name( );
}

// Show list of modules available in the system
void PluginDescriptionDialogData::ShowModulesList( )
{
    Parent->setWindowTitle( "Modules list" );

    shared_ptr<const XModulesCollection>     modulesCollection = GlobalServiceManager::Instance( ).GetPluginsEngine( )->GetModules( );
    vector<shared_ptr<const XPluginsModule>> modules;

    // get all modules into a vector and sort it
    modules.resize( modulesCollection->Count( ) );
    copy( modulesCollection->begin( ), modulesCollection->end( ), modules.begin( ) );
    sort( modules.begin( ), modules.end( ), ModulesListByName );

    // prepare the content
    QString modulesStr = "";
    int     i = 0;

    for ( vector<shared_ptr<const XPluginsModule>>::const_iterator it = modules.begin( ); it != modules.end( ); ++it, ++i  )
    {
        shared_ptr<const XPluginsModule> module = *it;

        modulesStr += QString( "<tr class='%0'><td width='250'><a href='{%1}'>%2</a></td><td>%3</td></tr>" ).
                            arg( ( ( i & 1 ) == 0 ) ? "tro" : "tre" ).
                            arg( QString::fromStdString( module->ID( ).ToString( ) ) ).
                            arg( QString::fromUtf8( module->Name( ).c_str( ) ) ).
                            arg( QString::fromUtf8( module->Description( ).c_str( ) ) );
    }

    ShowContent( QString( "<p><b>List of modules loaded in the system:</b><br>"
                          "<table border='0' cellpadding='5' width='100%'>%0</table></p><p> </p>" ).arg( modulesStr ) );
}

// Custom comparer to sort plug-ins by name
static bool PluginsListByName( const shared_ptr<const XPluginDescriptor>& plugin1, const shared_ptr<const XPluginDescriptor>& plugin2 )
{
    return plugin1->Name( ) < plugin2->Name( );
}
// Custom comparer to sort families by name
static bool FamiliesListByName( const shared_ptr<const XFamily>& fam1, const shared_ptr<const XFamily>& fam2 )
{
    return fam1->Name( ) < fam2->Name( );
}

// Show description for the specified module
void PluginDescriptionDialogData::ShowModuleDescription( const shared_ptr<const XPluginsModule>& moduleDesc )
{
    Parent->setWindowTitle( QString( "Module description: %0" ).arg( QString::fromUtf8( moduleDesc->Name( ).c_str( ) ) ) );

    shared_ptr<const XFamiliesCollection>       familiesCollection = GlobalServiceManager::Instance( ).
                                                                     GetPluginsEngine( )->GetFamilies( );
    shared_ptr<const XPluginsCollection>        pluginsCollection  = moduleDesc->GetPlugins( );
    vector<shared_ptr<const XPluginDescriptor>> plugins;
    vector<shared_ptr<const XFamily>>           families;
    map<XGuid, list<shared_ptr<const XPluginDescriptor>>> familyMap;

    // get all plug-ins into a vector and sort it
    plugins.resize( pluginsCollection->Count( ) );
    copy( pluginsCollection->begin( ), pluginsCollection->end( ), plugins.begin( ) );
    sort( plugins.begin( ), plugins.end( ), PluginsListByName );

    // get plug-ins by families
    for ( vector<shared_ptr<const XPluginDescriptor>>::const_iterator it = plugins.begin( ); it != plugins.end( ); ++it )
    {
        shared_ptr<const XPluginDescriptor> plugin = *it;
        XGuid familyId = plugin->FamilyID( );
        auto  familyIt = familyMap.find( familyId );

        if ( familyIt == familyMap.end( ) )
        {
            shared_ptr<const XFamily> family = familiesCollection->GetFamily( familyId );
            list<shared_ptr<const XPluginDescriptor>> familyList;

            families.push_back( family );
            familyList.push_back( plugin );

            familyMap.insert( pair<XGuid, list<shared_ptr<const XPluginDescriptor>>>( familyId, familyList ) );
        }
        else
        {
            familyIt->second.push_back( plugin );
        }
    }
    sort( families.begin( ), families.end( ), FamiliesListByName );

    // prepare the content
    QString pluginsStr = "";
    int     i = 0;

    for ( vector<shared_ptr<const XFamily>>::const_iterator fit = families.begin( ); fit != families.end( ); ++fit )
    {
        shared_ptr<const XFamily> family = *fit;
        list<shared_ptr<const XPluginDescriptor>> plugins = familyMap[family->ID( )];

        pluginsStr += QString( "<tr class='%0'><td colspan='2'><b>%1</b></td></tr>" ).
                arg( ( ( i & 1 ) == 0 ) ? "tro" : "tre" ).
                arg( QString::fromUtf8( family->Name( ).c_str( ) ) );
        i++;

        for ( list<shared_ptr<const XPluginDescriptor>>::const_iterator it = plugins.begin( ); it != plugins.end( ); ++it, ++i  )
        {
            shared_ptr<const XPluginDescriptor> plugin = *it;

            pluginsStr += QString( "<tr class='%0'><td width='250'><a href='{%1}'>%2</a></td><td>%3</td></tr>" ).
                                arg( ( ( i & 1 ) == 0 ) ? "tro" : "tre" ).
                                arg( QString::fromStdString( plugin->ID( ).ToString( ) ) ).
                                arg( QString::fromUtf8( plugin->Name( ).c_str( ) ) ).
                                arg( QString::fromUtf8( plugin->Description( ).c_str( ) ) );
        }
    }

    QString name = QString( "<b>Module name</b>: %0" ).arg( QString::fromUtf8( moduleDesc->Name( ).c_str( ) ) );
    QString scriptingName;

    if ( IsScriptingMode )
    {
        scriptingName = QString( " (<span>%0</span>)" ).arg( moduleDesc->ShortName( ).c_str( ) );
    }

    ShowContent( QString( "<p>%0%1</p>"
                          "<p>%2</p>"
                          "<p><b>List of plug-ins provided by the module:</b><br>"
                          "<table border='0' cellpadding='5' width='100%'>%3</table></p>"
                          "<br><p><b>Version</b>: %4</p>"
                          "<p><b>Provider</b>: <a href='%5'>%6</a></p>" ).
                          arg( name ).arg( scriptingName ).
                          arg( QString::fromUtf8( moduleDesc->Description( ).c_str( ) ) ).
                          arg( pluginsStr ).
                          arg( moduleDesc->Version( ).ToString( ).c_str( ) ).
                          arg( moduleDesc->Website( ).c_str( ) ).
                          arg( QString::fromUtf8( moduleDesc->Vendor( ).c_str( ) ) ) );
}

// Show description for the specified plug-in
void PluginDescriptionDialogData::ShowPluginDescription( const shared_ptr<const XPluginDescriptor>& pluginDesc,
                                                         const shared_ptr<const XPluginsModule>& moduleDesc )
{
    Parent->setWindowTitle( QString( "Plug-in description: %0" ).arg( QString::fromUtf8( pluginDesc->Name( ).c_str( ) ) ) );

    QString name = QString( "<b>Plug-in name</b>: %0" ).arg( pluginDesc->Name( ).c_str( ) );

    QString scriptingName;
    QString properties;
    QString runTimeProperties;
    QString functions;
    QString scriptingNote;
    QString moduleStr;
    int32_t setupPropertyCount = 0, runTimePropertyCount = 0;

    if ( moduleDesc )
    {
        moduleStr = QString( "<p><b>Module</b>: <a href='{%0}'>%1</a></p>" ).
                            arg( moduleDesc->ID( ).ToString( ).c_str( ) ).
                            arg( QString::fromUtf8( moduleDesc->Name( ).c_str( ) ) );
    }

    if ( IsScriptingMode )
    {
        scriptingName = QString( " (<span>%0</span>)" ).arg( pluginDesc->ShortName( ).c_str( ) );

        scriptingNote = QString(
                        "<br><p><span><b>Note:</b></span> The information provided in <span>blue</span> "
                        "may be useful while using the plug-in from scripting environment. That is plug-in's, "
                        "properties' and functions' (if available) scripting names and types.</p>" );
    }

    if ( pluginDesc->PropertiesCount( ) != 0 )
    {
        properties = "<p><b>Properties:</b><table border='0' cellpadding='5' width='100%'>";
        runTimeProperties = "<p><b>Run time properties:</b><table border='0' cellpadding='5' width='100%'>";

        for ( int32_t i = 0, n = pluginDesc->PropertiesCount( ); i < n; i++ )
        {
            auto    propertyDesc = pluginDesc->GetPropertyDescriptor( i );
            QString propertyScriptingInfo;
            QString propertyTypeInfo;
            int     counter;

            if ( propertyDesc->IsHidden( ) )
            {
                continue;
            }

            if ( propertyDesc->IsDeviceRuntimeConfiguration( ) )
            {
                counter = runTimePropertyCount++;
            }
            else
            {
                counter = setupPropertyCount++;
            }

            if ( IsScriptingMode )
            {
                propertyScriptingInfo = QString( "<br><span>%0</span>" ).arg( propertyDesc->ShortName( ).c_str( ) );
                propertyTypeInfo      = GetPropertyTypeInfo( propertyDesc );
            }

            QString propertyString = QString( "<tr class='%0'><td width='150'><b>%1</b>%2</td><td>%3%4%5</td></tr>" ).
                    arg( ( ( counter & 1 ) == 0 ) ? "tro" : "tre" ).
                    arg( QString::fromUtf8( propertyDesc->Name( ).c_str( ) ) ).
                    arg( propertyScriptingInfo ).
                    arg( QString::fromUtf8( propertyDesc->Description( ).c_str( ) ) ).
                    arg( GetPropertyValueInfo( propertyDesc, IsScriptingMode ) ).
                    arg( propertyTypeInfo );

            if ( propertyDesc->IsDeviceRuntimeConfiguration( ) )
            {
                runTimeProperties += propertyString;
            }
            else
            {
                properties += propertyString;
            }
        }

        properties += "</table></p>";
        runTimeProperties += "</table></p>";
    }

    if ( pluginDesc->FunctionsCount( ) != 0 )
    {
        if ( !IsScriptingMode )
        {
            functions = "<p><b>Note:</b> The plug-in provides functions to call from scripts.</p>";
        }
        else
        {
            functions = "<p><b>Functions to call from scripts:</b><table border='0' cellpadding='5' width='100%'>";

            for ( int32_t i = 0, n = pluginDesc->FunctionsCount( ); i < n; i++ )
            {
                auto functionDesc = pluginDesc->GetFunctionDescriptor( i );

                QString functionString = QString( "<tr class='%0'><td width='150'><b><span>%1</span></b></td><td>%2</td></tr>" ).
                        arg( ( ( i & 1 ) == 0 ) ? "tro" : "tre" ).
                        arg( functionDesc->Name( ).c_str( ) ).
                        arg( GetFunctionDescription( functionDesc ) );

                functions += functionString;
            }

            functions += "</table></p>";
        }
    }

    // set final HTML as plug-in's description
    ShowContent( QString(
       "<p>%0%1</p><p>%2</p>%3<p>%4</p><p>%5</p>%6%7%8%9%10<p> </p>" ).
                           arg( name ).arg( scriptingName ).
                           arg( GetPluginType( pluginDesc ) ).
                           arg( moduleStr ).
                           arg( QString::fromUtf8( pluginDesc->Description( ).c_str( ) ) ).
                           arg( QString::fromUtf8( pluginDesc->Help( ).c_str( ) ) ).
                           arg( GetPluginTypeSpecificInfo( pluginDesc, IsScriptingMode ) ).
                           arg( ( setupPropertyCount != 0 ) ? properties : QString::null ).
                           arg( ( runTimePropertyCount != 0 ) ? runTimeProperties : QString::null ).
                           arg( functions ).
                           arg( scriptingNote) );
}

// Show content of a help topic
void PluginDescriptionDialogData::ShowContent( const QString& bodyHtml )
{
    Ui->textEdit->setHtml( QString(
       "<html><head><style>span { color: #0000C0; } .tro { background-color: #FFFFEE } .tre { background-color: #F8F8F8 }</style></head>"
       "<body bgcolor='#FFFFDD'>%0</body></html>" ).arg( bodyHtml ) );
}

// Add plug-in descriptor into the history
void PluginDescriptionDialogData::AddToHistory( const XGuid& id )
{
    // remove any descriptor past the current index
    History.resize( HistoryIndex + 1 );

    // remove same descriptor from the history
    for ( auto it = History.begin( ); it != History.end( ); ++it )
    {
        if ( *it == id )
        {
            History.erase( it );
            HistoryIndex--;
            break;
        }
    }

    // add new ID and point to it
    History.push_back( id );
    HistoryIndex++;
}

// Enable/diable navigation buttons according to current browsing history
void PluginDescriptionDialogData::UpdateNavigationButtonsState( )
{
    Ui->goBackButton->setEnabled( HistoryIndex > 0 );
    Ui->goForwardButton->setEnabled( HistoryIndex < static_cast<int>( History.size( ) ) - 1 );
}

// Get string name of the XVarType
const QString VariantTypeToString( XVarType type )
{
    static const char* strIneger  = "Integer";
    static const char* strFloat   = "Real Number";
    static const char* strTypes[] = { "Boolean", strIneger, strIneger, strIneger, strIneger, strIneger, strIneger,
                                      strFloat, strFloat, "String", "Color", "Range", "Range (real numbers)", "Point", "Point (real numbers)", "Size", "Image" };

    XVarType elementType = type & XVT_Any;
    QString  strType     = ( ( elementType >= XVT_Bool ) && ( elementType <= XVT_Image ) ) ?
                           QString( strTypes[elementType - XVT_Bool] ) : QString( "Unknown" );

    if ( type & XVT_Array )
    {
        if ( ( type & XVT_Any ) == XVT_Any )
        {
            strType = "Array with elements of arbitrary type.";
        }
        else
        {
            strType = QString( "Array. Elements' type: %0" ).arg( strType );
        }
    }
    else if ( type & XVT_Array2d )
    {
        if ( ( type & XVT_Any ) == XVT_Any )
        {
            strType = "2D array with elements of arbitrary type.";
        }
        else
        {
            strType = QString( "2D array. Elements' type: %0" ).arg( strType );
        }
    }
    else if ( type & XVT_ArrayJagged )
    {
        if ( ( type & XVT_Any ) == XVT_Any )
        {
            strType = "Jagged array with elements of arbitrary type.";
        }
        else
        {
            strType = QString( "Jagged array. Elements' type: %0" ).arg( strType );
        }
    }

    return strType;
}

// Get plug-in type string
const QString GetPluginType( const shared_ptr<const XPluginDescriptor>& pluginDesc )
{
    QString typeName;

    switch ( pluginDesc->Type( ) )
    {
    case PluginType_ImageProcessingFilter:
        typeName = "Image Processing Filter";
        break;

    case PluginType_ImageProcessingFilter2:
        typeName = "Two Source Image Processing Filter";
        break;

    case PluginType_ImageProcessing:
        typeName = "Image Processing";
        break;

    case PluginType_VideoSource:
        typeName = "Video Source";
        break;

    case PluginType_VideoProcessing:
        typeName = "Video Processing";
        break;

    case PluginType_ScriptingEngine:
        typeName = "Scripting Engine";
        break;

    case PluginType_ImageImporter:
        typeName = "Image Importer";
        break;

    case PluginType_ImageExporter:
        typeName = "Image Exporter";
        break;

    case PluginType_Device:
        typeName = "Device plug-in";
        break;

    case PluginType_ScriptingApi:
        typeName = "Scripting APIs";
        break;

    default:
        typeName = "Unknown";
        break;
    }

    return QString( "<b>Plug-in type:</b> %1" ).arg( typeName );
}

// Get type specific information about the plug-in
const QString GetPluginTypeSpecificInfo( const shared_ptr<const XPluginDescriptor>& pluginDesc, bool scriptingMode )
{
    shared_ptr<XPlugin> plugin   = pluginDesc->CreateInstance( );
    QString             typeInfo;

    switch ( pluginDesc->Type( ) )
    {
    case PluginType_ImageProcessingFilter:
    case PluginType_ImageProcessingFilter2:
        {
            auto ipfPlugin             = static_pointer_cast<XImageProcessingFilterPlugin>( plugin );
            auto supportedPixelFormats = ipfPlugin->GetSupportedPixelFormats( );

            if ( ipfPlugin->CanProcessInPlace( ) )
            {
                typeInfo = GetStringFromPixelFormats( supportedPixelFormats );
            }
            else
            {
                int counter = 1;

                typeInfo = "<p><b>Pixel format translation:</b><table border='0' cellpadding='5' width='100%'><tr bgcolor=#FFFFEE><td><b>Input</b></td><td><b>Output</b></td></tr>";

                for ( auto& pixelFormat : supportedPixelFormats )
                {
                    typeInfo += QString( "<tr bgcolor=#%0><td>%1</td><td>%2</td></tr>" ).
                            arg( ( ( counter & 1 ) == 0) ? "FFFFEE" : "F8F8F8" ).
                            arg( XImage::PixelFormatName( pixelFormat ).c_str( ) ).
                            arg( XImage::PixelFormatName( ipfPlugin->GetOutputPixelFormat( pixelFormat ) ).c_str( ) );

                    counter++;
                }

                typeInfo += "</table></p>";
            }

            if ( pluginDesc->Type( ) == PluginType_ImageProcessingFilter2 )
            {
                auto ipfPlugin2      = static_pointer_cast<XImageProcessingFilterPlugin2>( plugin );
                bool samePixelFormat = true;

                typeInfo += QString( "<p><b>Second image format</b>: " );

                for ( auto& pixelFormat : supportedPixelFormats )
                {
                    samePixelFormat &= ( pixelFormat == ipfPlugin2->GetSecondImageSupportedFormat( pixelFormat ) );
                }

                if ( samePixelFormat )
                {
                    typeInfo += "same as first image";
                }
                else
                {
                    int counter = 1;

                    typeInfo += "<br><table border='0' cellpadding='5' width='100%'><tr bgcolor=#FFFFEE><td><b>First</b></td><td><b>Second</b></td></tr>";

                    for ( auto& pixelFormat : supportedPixelFormats )
                    {
                        typeInfo += QString( "<tr bgcolor=#%0><td>%1</td><td>%2</td></tr>" ).
                                arg( ( ( counter & 1 ) == 0) ? "FFFFEE" : "F8F8F8" ).
                                arg( XImage::PixelFormatName( pixelFormat ).c_str( ) ).
                                arg( XImage::PixelFormatName( ipfPlugin2->GetSecondImageSupportedFormat( pixelFormat ) ).c_str( ) );

                        counter++;
                    }

                    typeInfo += "</table>";
                }

                typeInfo += "</p>";
            }

            if ( scriptingMode )
            {
                typeInfo += QString( "<p>In-place image processing available: <span>%0</span></p>" ).
                            arg( ( ipfPlugin->CanProcessInPlace( ) ) ? "Yes" : "No" );
            }
        }
        break;

    case PluginType_VideoProcessing:
        {
            auto vpPlugin = static_pointer_cast<XVideoProcessingPlugin>( plugin );

            typeInfo = GetStringFromPixelFormats( vpPlugin->GetSupportedPixelFormats( ) );
        }
        break;

    }

    return typeInfo;
}

// Get information about possible property values (min, max, default, selection values)
const QString GetPropertyValueInfo( const shared_ptr<const XPropertyDescriptor>& propertyDesc, bool scriptingMode )
{
    QString ret = "";

    if ( !propertyDesc->IsRuntimeConfiguration( ) )
    {
        XVarType propertyType = propertyDesc->Type( );
        XVariant defaultValue = propertyDesc->GetDefaultValue( );

        if ( !propertyDesc->IsSelection( ) )
        {
            XVariant minValue = propertyDesc->GetMinValue( );
            XVariant maxValue = propertyDesc->GetMaxValue( );

            if ( ( ( propertyType >= XVT_Bool ) && ( propertyType <= XVT_R8 ) ) ||
                 ( ( propertyType >= XVT_ARGB ) && ( propertyType <= XVT_Point ) ) )
            {
                if ( minValue.Type( ) == propertyType )
                {
                    ret += QString( "Minimum: %0." ).arg(
                                ( ( propertyType == XVT_R8 ) || ( propertyType == XVT_R4 ) ) ?
                                    QString::number( minValue.ToDouble( ), 'f', 2 ) :
                                    QString( minValue.ToString( ).c_str( ) ) );
                }
                if ( maxValue.Type( ) == propertyType )
                {
                    ret += QString( " Maximum: %0." ).arg(
                                ( ( propertyType == XVT_R8 ) || ( propertyType == XVT_R4 ) ) ?
                                    QString::number( maxValue.ToDouble( ), 'f', 2 ) :
                                    QString( maxValue.ToString( ).c_str( ) ) );
                }
                if ( defaultValue.Type( ) == propertyType )
                {
                    ret += QString( " Default: %0." ).arg(
                                ( ( propertyType == XVT_R8 ) || ( propertyType == XVT_R4 ) ) ?
                                    QString::number( defaultValue.ToDouble( ), 'f', 2 ) :
                                    QString( defaultValue.ToString( ).c_str( ) ) );
                }
            }

            if ( propertyDesc->IsReadOnly( ) )
            {
                ret += " Read only.";
            }

            if ( ( ret.length( ) > 0 ) && ( ret.at( 0 ) == ' ' ) )
            {
                ret.remove( 0, 1 );
            }
        }
        else
        {
            int  choicesCount     = propertyDesc->ChoicesCount( );
            bool selectionByIndex = propertyDesc->IsSelectionByIndex( );

            ret = QString( "Available choices: " );

            for ( int i = 0; i < choicesCount; i++ )
            {
                QString displayValue = QString::fromUtf8( propertyDesc->GetPossibleChoice( i ).ToString( ).c_str( ) );
                int     newLineIndex = displayValue.indexOf( "\n" );

                if ( newLineIndex != -1 )
                {
                    displayValue.truncate( newLineIndex );
                }

                if ( i != 0 )
                {
                    ret += ", ";
                }
                ret += displayValue;

                if ( ( scriptingMode ) && ( selectionByIndex ) )
                {
                    ret += QString( " (<span>%0</span>)" ).arg( i );
                }
            }

            ret += ".";

            if ( defaultValue.Type( ) == propertyType )
            {
                string  propertyValue;

                if ( selectionByIndex )
                {
                    propertyValue = propertyDesc->GetPossibleChoice( defaultValue.ToUInt( ) ).ToString( );
                }
                else
                {
                    propertyValue = defaultValue.ToString( );
                }

                QString displayValue = QString::fromUtf8( propertyValue.c_str( ) );
                int     newLineIndex = displayValue.indexOf( "\n" );

                if ( newLineIndex != -1 )
                {
                    displayValue.truncate( newLineIndex );
                }

                ret += QString( "<br><br>Default: %0" ).arg( displayValue );

                if ( ( scriptingMode ) && ( selectionByIndex ) )
                {
                    ret += QString( " (<span>%0</span>)" ).arg( defaultValue.ToUInt( ) );
                }
            }
        }

        if ( !ret.isEmpty( ) )
        {
            ret.prepend( "<br><br>" );
        }
    }

    return ret;
}

// Get information about property type
const QString GetPropertyTypeInfo( const shared_ptr<const XPropertyDescriptor>& propertyDesc )
{
    XVarType type = propertyDesc->Type( );
    QString  elementsRange;

    if ( type & ( XVT_Array | XVT_Array2d | XVT_ArrayJagged ) )
    {
        XVarType elementType = type & XVT_Any;

        XVariant minValue = propertyDesc->GetMinValue( );
        XVariant maxValue = propertyDesc->GetMaxValue( );

        if ( ( minValue.Type( ) == elementType ) && ( maxValue.Type( ) == elementType ) )
        {
            elementsRange = QString( ". Elements' range: [%0, %1]." ).
                        arg( ( ( elementType == XVT_R8 ) || ( elementType == XVT_R4 ) ) ?
                            QString::number( minValue.ToDouble( ), 'f', 2 ) :
                            QString( minValue.ToString( ).c_str( ) ) ).
                        arg(( ( elementType == XVT_R8 ) || ( elementType == XVT_R4 ) ) ?
                            QString::number( maxValue.ToDouble( ), 'f', 2 ) :
                            QString( maxValue.ToString( ).c_str( ) ) );
        }
    }

    return QString( "<br><br><span>Type: %0%1</span>" ).arg( VariantTypeToString( type ) ).arg( elementsRange );
}

// Get vector of pixel formats as string
const QString GetStringFromPixelFormats( const vector<XPixelFormat>& formats )
{
    QString ret = "<p><b>Supported pixel formats:</b> ";
    bool    first = true;

    for ( auto& pixelFormat : formats )
    {
        if ( !first )
        {
            ret += ", ";
        }

        ret += XImage::PixelFormatName( pixelFormat ).c_str( );

        first = false;
    }

    ret += "</p>";

    return ret;
}

// Get description of a function
const QString GetFunctionDescription( const shared_ptr<const XFunctionDescriptor>& functionDesc )
{
    QString  ret   = QString( "%0" ).arg( QString::fromUtf8( functionDesc->Description( ).c_str( ) ) );
    XVarType fType = functionDesc->ReturnType( );

    if ( functionDesc->ArgumentsCount( ) != 0 )
    {
        ret += "<p>Arguments:<br>";

        for ( int32_t i = 0, n = functionDesc->ArgumentsCount( ); i < n; i++ )
        {
            string   name, desc;
            XVarType type;

            functionDesc->GetArgumentInfo( i, name, desc, &type );

            ret += QString( "<b><span>%0</span></b> (%2): %1.<br>" ).arg( QString::fromUtf8( name.c_str( ) ) ).
                                                                     arg( QString::fromUtf8( desc.c_str( ) ) ).
                                                                     arg( VariantTypeToString( type) );
        }

        ret += "</p>";

    }

    if ( ( fType != XVT_Empty ) && ( fType != XVT_Null ) )
    {
        ret += QString( "<p><b>Returns:</b> %0" ).arg( VariantTypeToString( fType ) );
    }

    return ret;
}

} // namespace Private
