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

#include "SandboxSettings.hpp"
#include <xtypes.h>

#include <QString>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

static const QString STR_SETTINGS          = QString::fromUtf8( "Settings" );
static const QString STR_VIEW_SETTINGS     = QString::fromUtf8( "ViewSettings" );
static const QString STR_ROTATION          = QString::fromUtf8( "Rotation" );
static const QString STR_ROTATION_START    = QString::fromUtf8( "RotationStart" );
static const QString STR_ROTATION_TIME     = QString::fromUtf8( "RotationTime" );
static const QString STR_BUTTONS_POSITION  = QString::fromUtf8( "ButtonsPosition" );
static const QString STR_BUTTONS_ALIGNMENT = QString::fromUtf8( "ButtonsAlignment" );
static const QString STR_DROP_FRMAES       = QString::fromUtf8( "DropFrames" );


SandboxSettings::SandboxSettings( ) :
    mEnableViewsRotation( false ),
    mRotationStartTimeSec( 60 ),
    mViewsRotationTimeSec( 15 ),
    mViewButtonsPosition( SandboxSettings::ViewButtonsPosition::VerticalRight ),
    mViewButtonsAlignment( SandboxSettings::ViewButtonsAlignment::Center ),
    mDropFrameOnSlowProcessing( false )
{

}

// Check if two objects are equal
bool SandboxSettings::operator==( const SandboxSettings& rhs ) const
{
    return ( ( mEnableViewsRotation  == rhs.mEnableViewsRotation ) &&
             ( mRotationStartTimeSec == rhs.mRotationStartTimeSec ) &&
             ( mViewsRotationTimeSec == rhs.mViewsRotationTimeSec ) &&
             ( mViewButtonsPosition  == rhs.mViewButtonsPosition ) &&
             ( mViewButtonsAlignment == rhs.mViewButtonsAlignment ) &&
             ( mDropFrameOnSlowProcessing == rhs.mDropFrameOnSlowProcessing ));
}

// Get/Set views rotation flag
bool SandboxSettings::IsViewsRotationEnabled( ) const
{
    return mEnableViewsRotation;
}
void SandboxSettings::EnableViewsRotation( bool enable )
{
    mEnableViewsRotation = enable;
}

// Get/Set rotation start time (seconds) - time before first rotation
int SandboxSettings::RotationStartTime( ) const
{
    return mRotationStartTimeSec;
}
void SandboxSettings::SetRotationStartTime( int timeSec )
{
    mRotationStartTimeSec = XINRANGE( timeSec, 60, 600 );;
}

// Get/Set view rotation time (seconds)
int SandboxSettings::ViewsRotationTime( ) const
{
    return mViewsRotationTimeSec;
}
void SandboxSettings::SetViewsRotationTime( int timeSec )
{
    mViewsRotationTimeSec = XINRANGE( timeSec, 10, 300 );
}

// Get/Set view buttons position
SandboxSettings::ViewButtonsPosition SandboxSettings::GetViewButtonsPosition( ) const
{
    return mViewButtonsPosition;
}
void SandboxSettings::SetViewButtonsPosition( SandboxSettings::ViewButtonsPosition position )
{
    mViewButtonsPosition = position;
}

// Get/Set view buttons alignment
SandboxSettings::ViewButtonsAlignment SandboxSettings::GetViewButtonsAlignment( ) const
{
    return mViewButtonsAlignment;
}
void SandboxSettings::SetViewButtonsAlignment( SandboxSettings::ViewButtonsAlignment alignment )
{
    mViewButtonsAlignment = alignment;
}

// Get/set if frames should be dropped on slow processing
bool SandboxSettings::DropFramesOnSlowProcessing( ) const
{
    return mDropFrameOnSlowProcessing;
}
void SandboxSettings::SetDropFramesOnSlowProcessing( bool drop )
{
    mDropFrameOnSlowProcessing = drop;
}


// Returns enclosing XML tag name used for saving the setting
const QString SandboxSettings::XmlTagName( )
{
    return STR_SETTINGS;
}

// Save setting into the specified XML writer
void SandboxSettings::Save( QXmlStreamWriter& xmlWriter ) const
{
    xmlWriter.writeStartElement( STR_SETTINGS );

    xmlWriter.writeStartElement( STR_VIEW_SETTINGS );
    xmlWriter.writeAttribute( STR_ROTATION, QString::number( ( mEnableViewsRotation ) ? 1 : 0 ) );
    xmlWriter.writeAttribute( STR_ROTATION_START, QString::number( mRotationStartTimeSec ) );
    xmlWriter.writeAttribute( STR_ROTATION_TIME, QString::number( mViewsRotationTimeSec ) );
    xmlWriter.writeAttribute( STR_BUTTONS_POSITION, QString::number( static_cast<int>( mViewButtonsPosition ) ) );
    xmlWriter.writeAttribute( STR_BUTTONS_ALIGNMENT, QString::number( static_cast<int>( mViewButtonsAlignment ) ) );
    xmlWriter.writeAttribute( STR_DROP_FRMAES, QString::number( ( mDropFrameOnSlowProcessing ) ? 1 : 0 ) );
    xmlWriter.writeEndElement( );

    xmlWriter.writeEndElement( );
}

// Load setting from the specified XML readed
bool SandboxSettings::Load( QXmlStreamReader& xmlReader )
{
    bool ret = false;

    if ( xmlReader.name( ) == STR_SETTINGS )
    {
        xmlReader.readNextStartElement( );

        if ( xmlReader.name( ) == STR_VIEW_SETTINGS )
        {
            QXmlStreamAttributes xmlAttrs = xmlReader.attributes( );
            QStringRef           strRef;

            // rotation
            strRef = xmlAttrs.value( STR_ROTATION );
            if ( !strRef.isEmpty( ) )
            {
                mEnableViewsRotation = ( strRef.toString( ).toInt( ) == 1 );
            }

            // rotation start
            strRef = xmlAttrs.value( STR_ROTATION_START );
            if ( !strRef.isEmpty( ) )
            {
                mRotationStartTimeSec = XINRANGE( strRef.toString( ).toInt( ), 60, 600 );
            }

            // rotation time
            strRef = xmlAttrs.value( STR_ROTATION_TIME );
            if ( !strRef.isEmpty( ) )
            {
                mViewsRotationTimeSec = XINRANGE( strRef.toString( ).toInt( ), 10, 300 );
            }

            // buttons position
            strRef = xmlAttrs.value( STR_BUTTONS_POSITION );
            if ( !strRef.isEmpty( ) )
            {
                mViewButtonsPosition = static_cast<ViewButtonsPosition>( strRef.toString( ).toInt( ) );
            }

            // buttons alignment
            strRef = xmlAttrs.value( STR_BUTTONS_ALIGNMENT );
            if ( !strRef.isEmpty( ) )
            {
                mViewButtonsAlignment = static_cast<ViewButtonsAlignment>( strRef.toString( ).toInt( ) );
            }

            // drop frames
            strRef = xmlAttrs.value( STR_DROP_FRMAES );
            if ( !strRef.isEmpty( ) )
            {
                mDropFrameOnSlowProcessing = ( strRef.toString( ).toInt( ) == 1 );
            }

            xmlReader.skipCurrentElement( );

            ret = true;
        }
        xmlReader.skipCurrentElement( );
    }

    return ret;
}
