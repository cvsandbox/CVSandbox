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
#ifndef CVS_PROJECT_OBJECT_VIEW_FRAME_HPP
#define CVS_PROJECT_OBJECT_VIEW_FRAME_HPP

#include <QFrame>
#include <XGuid.hpp>

class ProjectObjectViewFrame : public QFrame
{
    Q_OBJECT

public:
    explicit ProjectObjectViewFrame( const CVSandbox::XGuid& objectId, QWidget* parent = 0 );

    // Get project object ID shown in the view
    const CVSandbox::XGuid ProjectObjectId( ) const
    {
        return poid;
    }

    // Set preferred size of the project's view content or let it auto size
    virtual void SetPreferredContentSize( bool autoSize, const QSize& size = QSize( ) );

private:
    CVSandbox::XGuid  poid;
};

#endif // CVS_PROJECT_OBJECT_VIEW_FRAME_HPP
