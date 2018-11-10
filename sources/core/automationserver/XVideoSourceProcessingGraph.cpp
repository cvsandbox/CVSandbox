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

#include "XVideoSourceProcessingGraph.hpp"

using namespace std;
using namespace CVSandbox;

namespace CVSandbox { namespace Automation
{

XVideoSourceProcessingGraph::XVideoSourceProcessingGraph( ) :
    mProcessingSteps( )
{

}

// Check if two processing graphs are equal
bool XVideoSourceProcessingGraph::operator==( const XVideoSourceProcessingGraph& rhs ) const
{
    return ( mProcessingSteps == rhs.mProcessingSteps );
}

// Get steps count
int32_t XVideoSourceProcessingGraph::StepsCount( ) const
{
    return static_cast<int32_t>( mProcessingSteps.size( ) );
}

// Add step to the end of the graph
int32_t XVideoSourceProcessingGraph::AddStep( const XVideoSourceProcessingStep& step )
{
    mProcessingSteps.push_back( step );

    return static_cast<int32_t>( mProcessingSteps.size( ) - 1 );
}

// Insert step at the specified index
bool XVideoSourceProcessingGraph::InsertStep( int32_t stepIndex, const XVideoSourceProcessingStep& step )
{
    bool ret = false;

    if ( ( stepIndex >= 0 ) && ( stepIndex <= static_cast<int32_t>( mProcessingSteps.size( ) ) ) )
    {
        mProcessingSteps.insert( mProcessingSteps.begin( ) + stepIndex, step );
        ret = true;
    }

    return ret;
}

// Remove the specified step from the graph
bool XVideoSourceProcessingGraph::RemoveStep( int32_t stepIndex )
{
    bool ret = false;

    if ( ( stepIndex >= 0 ) && ( stepIndex < static_cast<int32_t>( mProcessingSteps.size( ) ) ) )
    {
        mProcessingSteps.erase( mProcessingSteps.begin( ) + stepIndex );
        ret = true;
    }

    return ret;
}

// Get processing step with the specified index
const XVideoSourceProcessingStep XVideoSourceProcessingGraph::GetStep( int32_t stepIndex ) const
{
    XVideoSourceProcessingStep step;

    if ( ( stepIndex >= 0 ) && ( stepIndex < static_cast<int32_t>( mProcessingSteps.size( ) ) ) )
    {
        step = mProcessingSteps[stepIndex];
    }

    return step;
}

// Set processing step with the specified index
void XVideoSourceProcessingGraph::SetStep( int32_t stepIndex, const XVideoSourceProcessingStep& processingStep )
{
    if ( ( stepIndex >= 0 ) && ( stepIndex < static_cast<int32_t>( mProcessingSteps.size( ) ) ) )
    {
        mProcessingSteps[stepIndex] = processingStep;
    }
}

} } // namespace CVSandbox::Automation
