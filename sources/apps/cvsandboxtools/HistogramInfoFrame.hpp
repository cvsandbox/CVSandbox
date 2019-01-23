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

#pragma once
#ifndef CVS_HISTOGRAM_INFO_FRAME_HPP
#define CVS_HISTOGRAM_INFO_FRAME_HPP

#include <QFrame>
#include <XImage.hpp>
#include "cvsandboxtools_global.h"

namespace Ui
{
    class HistogramInfoFrame;
}

namespace Private
{
    class HistogramInfoFrameData;
}

class CVS_SHARED_EXPORT HistogramInfoFrame : public QFrame
{
    Q_OBJECT

public:
    explicit HistogramInfoFrame( QWidget *parent = 0 );
    ~HistogramInfoFrame( );

    void ShowHistogramForImage( const std::shared_ptr<const CVSandbox::XImage>& image );
    void ClearHistogram( );
    void SetHistogramChannel( XRGBComponent channel );

private slots:
    void on_channelCombo_currentIndexChanged( int index );
    void on_logCheckBox_clicked( bool checked );
    void on_histogram_PositionChanged( int pos );
    void on_histogram_SelectionChanged( int start, int end );

private:
    void ClearHistogramInfoLabels( );
    void ClearSelectionInfoLabels( );

private:
    Ui::HistogramInfoFrame* ui;
    Private::HistogramInfoFrameData* mData;
};

#endif // CVS_HISTOGRAM_INFO_FRAME_HPP
