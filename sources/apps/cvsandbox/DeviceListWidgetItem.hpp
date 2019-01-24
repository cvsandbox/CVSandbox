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
#ifndef CVS_DEVICE_LIST_WIDGET_ITEM_HPP
#define CVS_DEVICE_LIST_WIDGET_ITEM_HPP

const static char* FPS_ENABLED_ICON  = ":/images/icons/frame_rate_enabled.png";
const static char* FPS_DISABLED_ICON = ":/images/icons/frame_rate_disabled.png";
const static char* FPS_ENABLED_TIP   = "Frame rate of the video source is included into sandbox frame rate";
const static char* FPS_DISABLED_TIP  = "Frame rate of the video source is NOT included into sandbox frame rate";

// Widget used to display items of selected/available device lists
class DeviceListWidgetItem : public QFrame
{
public:
    DeviceListWidgetItem( const QString& path, const QString& name, const QString& toolTip = "", bool showFpsReportState = false, QWidget* parent = 0 ) :
        QFrame( parent ), mImageLabel( nullptr ), mFpsImageLabel( nullptr )
    {
        QHBoxLayout* hLayout = new QHBoxLayout( );
        QVBoxLayout* vLayout = new QVBoxLayout( );

        this->setFrameShape( QFrame::Box );
        this->setFrameShadow( QFrame::Sunken );

        hLayout->setMargin( 3 );
        hLayout->setSpacing( 10 );
        vLayout->setMargin( 0 );
        vLayout->setSpacing( 0 );

        mImageLabel = new QLabel( this );

        QLabel*  pathLabel = new QLabel( path, this );
        QLabel*  nameLabel = new QLabel( name , this );
        QWidget* container = new QWidget( this );

        if ( !toolTip.isEmpty( ) )
        {
            mImageLabel->setToolTip( toolTip );
        }

        // set bold font for the name label
        QFont nameFont = nameLabel->font( );
        nameFont.setBold( true );
        nameLabel->setFont( nameFont );

        container->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Ignored );

        hLayout->addWidget( mImageLabel );
        hLayout->addWidget( container );

        if ( showFpsReportState )
        {
            mFpsImageLabel = new QLabel( this );
            mFpsImageLabel->setPixmap( QPixmap( FPS_ENABLED_ICON ) );
            mFpsImageLabel->setToolTip( FPS_ENABLED_TIP );

            hLayout->addWidget( mFpsImageLabel );
        }

        vLayout->addWidget( pathLabel );
        vLayout->addWidget( nameLabel );

        container->setLayout( vLayout );
        this->setLayout( hLayout );
    }

    void setIcon( const QIcon& icon )
    {
        mImageLabel->setPixmap( icon.pixmap( 16, 16 ) );
    }

    void enableFpsReport( bool enable )
    {
        if ( mFpsImageLabel != nullptr )
        {
            mFpsImageLabel->setPixmap( QPixmap( ( enable ) ? FPS_ENABLED_ICON : FPS_DISABLED_ICON ) );
            mFpsImageLabel->setToolTip( ( enable ) ? FPS_ENABLED_TIP : FPS_DISABLED_TIP );
        }
    }

private:
    QLabel* mImageLabel;
    QLabel* mFpsImageLabel;
};

#endif // CVS_DEVICE_LIST_WIDGET_ITEM_HPP
