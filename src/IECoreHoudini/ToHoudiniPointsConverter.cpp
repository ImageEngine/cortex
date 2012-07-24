//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2012, Image Engine Design Inc. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the name of Image Engine Design nor the names of any
//       other contributors to this software may be used to endorse or
//       promote products derived from this software without specific prior
//       written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////

#include "IECoreHoudini/ToHoudiniPointsConverter.h"

using namespace IECore;
using namespace IECoreHoudini;

IE_CORE_DEFINERUNTIMETYPED( ToHoudiniPointsConverter );

ToHoudiniGeometryConverter::Description<ToHoudiniPointsConverter> ToHoudiniPointsConverter::m_description( PointsPrimitiveTypeId );

ToHoudiniPointsConverter::ToHoudiniPointsConverter( const VisibleRenderable *renderable ) :
	ToHoudiniGeometryConverter( renderable, "Converts an IECore::PointsPrimitive to a Houdini GU_Detail." )
{
}

ToHoudiniPointsConverter::~ToHoudiniPointsConverter()
{
}

bool ToHoudiniPointsConverter::doConversion( const VisibleRenderable *renderable, GU_Detail *geo ) const
{
	const PointsPrimitive *points = static_cast<const PointsPrimitive *>( renderable );
	if ( !points )
	{
		return false;
	}
	
	const IECore::V3fVectorData *positions = points->variableData<V3fVectorData>( "P" );
	if ( !positions )
	{
		// accept "position" so we can convert the results of the PDCParticleReader without having to rename things
		/// \todo: Consider making the ParticleReader create a P if it doesn't exist for Cortex 6.
		positions = points->variableData<V3fVectorData>( "position" );
	}
	
	GA_Range newPoints = appendPoints( geo, positions );
	if ( !newPoints.isValid() || newPoints.empty() )
	{
		return false;
	}
	
	transferAttribs( points, geo, newPoints, GA_Range(), PrimitiveVariable::Vertex, PrimitiveVariable::Vertex );
	
	return true;
}
