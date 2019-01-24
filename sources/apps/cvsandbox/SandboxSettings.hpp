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
#ifndef CVS_SANDBOX_SETTINGS_HPP
#define CVS_SANDBOX_SETTINGS_HPP

class QXmlStreamWriter;
class QXmlStreamReader;
class QString;

class SandboxSettings
{
public:
    enum class ViewButtonsPosition
    {
        VerticalRight    = 0,
        HorizontalBottom = 1
    };

    enum class ViewButtonsAlignment
    {
        TopLeft     = 0,
        Center      = 1,
        BottomRight = 2
    };

public:
    SandboxSettings( );

    // Check if two objects are equal
    bool operator==( const SandboxSettings& rhs ) const;

    // Check if two objects are NOT equal
    bool operator!=( const SandboxSettings& rhs ) const
    {
        return ( !( ( *this ) == rhs ) );
    }

    // Get/Set views rotation flag
    bool IsViewsRotationEnabled( ) const;
    void EnableViewsRotation( bool enable );

    // Get/Set roation start time (seconds) - time before first rotation
    int RotationStartTime( ) const;
    void SetRotationStartTime( int timeSec );

    // Get/Set view rotation time (seconds) - time between rotations
    int ViewsRotationTime( ) const;
    void SetViewsRotationTime( int timeSec );

    // Get/Set view buttons position
    ViewButtonsPosition GetViewButtonsPosition( ) const;
    void SetViewButtonsPosition( ViewButtonsPosition position );

    // Get/Set view buttons alignment
    ViewButtonsAlignment GetViewButtonsAlignment( ) const;
    void SetViewButtonsAlignment( ViewButtonsAlignment alignment );

    // Get/set if frames should be dropped on slow processing
    bool DropFramesOnSlowProcessing( ) const;
    void SetDropFramesOnSlowProcessing( bool drop );

public:
    static const QString XmlTagName( );
    virtual void Save( QXmlStreamWriter& xmlWriter ) const;
    virtual bool Load( QXmlStreamReader& xmlReader );

private:
    bool                    mEnableViewsRotation;
    int                     mRotationStartTimeSec;
    int                     mViewsRotationTimeSec;
    ViewButtonsPosition     mViewButtonsPosition;
    ViewButtonsAlignment    mViewButtonsAlignment;

    bool                    mDropFrameOnSlowProcessing;
};

#endif // CVS_SANDBOX_SETTINGS_HPP
