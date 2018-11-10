/*
    Automation server library of Computer Vision Sandbox

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
#ifndef CVS_XVIDEO_SOURCE_PROCESSING_GRAPH_HPP
#define CVS_XVIDEO_SOURCE_PROCESSING_GRAPH_HPP

#include <stdint.h>
#include <vector>
#include "XVideoSourceProcessingStep.hpp"

namespace CVSandbox { namespace Automation
{

/* TODO: The class is a subject for heavy re-factoring in the future versions, when video processing
   sequence will become more like a graph, but not just a linear sequence of steps.
 */

class XVideoSourceProcessingGraph
{
public:
    XVideoSourceProcessingGraph( );

    // Check if two processing graphs are equal
    bool operator==( const XVideoSourceProcessingGraph& rhs ) const;

    // Check if two processing graphs are NOT equal
    bool operator!=( const XVideoSourceProcessingGraph& rhs ) const
    {
        return ( !( ( *this ) == rhs ) );
    }

    // Get steps count
    int32_t StepsCount( ) const;

    // Add step to the end of the graph
    int32_t AddStep( const XVideoSourceProcessingStep& step );
    // Insert step at the specified index
    bool InsertStep( int32_t stepIndex, const XVideoSourceProcessingStep& step );
    // Remove the specified step from the graph
    bool RemoveStep( int32_t stepIndex );
    // Get/Set processing step with the specified index
    const XVideoSourceProcessingStep GetStep( int32_t stepIndex ) const;
    void SetStep( int32_t stepIndex, const XVideoSourceProcessingStep& processingStep );

    // Enumeration API
    typedef std::vector<XVideoSourceProcessingStep>::const_iterator ConstIterator;
    typedef std::vector<XVideoSourceProcessingStep>::iterator Iterator;
    ConstIterator begin( ) const { return mProcessingSteps.begin( ); }
    ConstIterator end( ) const { return mProcessingSteps.end( ); }
    Iterator      begin( ) { return mProcessingSteps.begin( ); }
    Iterator      end( ) { return mProcessingSteps.end( ); }

private:
    std::vector<XVideoSourceProcessingStep> mProcessingSteps;
};

} } // namespace CVSandbox::Automation

#endif // CVS_XVIDEO_SOURCE_PROCESSING_GRAPH_HPP
