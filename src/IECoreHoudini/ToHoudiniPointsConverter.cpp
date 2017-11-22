//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2013, Image Engine Design Inc. All rights reserved.
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

#include "GU/GU_PrimPart.h"

#include "IECoreHoudini/ToHoudiniPointsConverter.h"

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreHoudini;

IE_CORE_DEFINERUNTIMETYPED( ToHoudiniPointsConverter );

ToHoudiniGeometryConverter::Description<ToHoudiniPointsConverter> ToHoudiniPointsConverter::m_description( PointsPrimitiveTypeId );

ToHoudiniPointsConverter::ToHoudiniPointsConverter( const Object *object ) :
	ToHoudiniGeometryConverter( object, "Converts an IECoreScene::PointsPrimitive to a Houdini GU_Detail." )
{
}

ToHoudiniPointsConverter::~ToHoudiniPointsConverter()
{
}

bool ToHoudiniPointsConverter::doConversion( const Object *object, GU_Detail *geo ) const
{
	const PointsPrimitive *points = static_cast<const PointsPrimitive *>( object );
	if ( !points )
	{
		return false;
	}

	size_t numPrims = geo->getNumPrimitives();
	GU_PrimParticle *system = GU_PrimParticle::build( geo, points->getNumPoints(), true );
	GA_Range newPoints = system->getPointRange();
	if ( !newPoints.isValid() || newPoints.empty() )
	{
		return false;
	}

	GA_OffsetList offsets;
	offsets.append( geo->primitiveOffset( numPrims ) );
	GA_Range newPrims( geo->getPrimitiveMap(), offsets );

	transferAttribs( geo, newPoints, newPrims );

	return true;
}

void ToHoudiniPointsConverter::transferAttribs( GU_Detail *geo, const GA_Range &points, const GA_Range &prims ) const
{
	const Primitive *primitive = IECore::runTimeCast<const Primitive>( srcParameter()->getValidatedValue() );
	if ( primitive )
	{
		transferAttribValues( primitive, geo, points, prims, PrimitiveVariable::Vertex );
	}

	setName( geo, prims );
}
