/*
    Virtual video source plug-ins of Computer Vision Sandbox

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
#ifndef CVS_FIRE_EFFECT_VIRTUAL_VIDEO_SOURCE_PLUGIN_HPP
#define CVS_FIRE_EFFECT_VIRTUAL_VIDEO_SOURCE_PLUGIN_HPP

#include "VirtualVideoSourcePlugin.hpp"

class FireEffectVirtualVideoSourcePlugin : public VirtualVideoSourcePlugin
{
public:
    FireEffectVirtualVideoSourcePlugin( );

    // IPluginBase interface
    virtual XErrorCode GetProperty( int32_t id, xvariant* value ) const;
    virtual XErrorCode SetProperty( int32_t id, const xvariant* value );

protected:
    // Method to be called from a background thread - should provide video frames,
    // till WaitTillNeedToStop() is true
    virtual void VideoSourceWorker( );

private:
    static const PropertyDescriptor** propertiesDescription;

    uint16_t mFrameWidth;
    uint16_t mFrameHeight;
    uint16_t mFrameRate;
};

#endif // CVS_FIRE_EFFECT_VIRTUAL_VIDEO_SOURCE_PLUGIN_HPP
