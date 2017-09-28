//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2015, Image Engine Design Inc. All rights reserved.
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

#include "IECore/TypeIds.h"
#include "IECore/PointsPrimitive.h"
#include "IECore/SimpleTypedData.h"
#include "IECoreNuke/Convert.h"
#include "IECoreNuke/PointToNukeGeometryConverter.h"

/// DDIMAGE header /////
#include "DDImage/Particles.h"
#include "DDImage/Vector3.h"

using namespace boost;
using namespace IECore;
using namespace IECoreNuke;
using namespace DD::Image;

PointToNukeGeometryConverter::ToNukeGeometryConverterDescription<PointToNukeGeometryConverter> PointToNukeGeometryConverter::g_description( PointsPrimitiveTypeId );

PointToNukeGeometryConverter::PointToNukeGeometryConverter( IECore::ConstObjectPtr object )
: ToNukeGeometryConverter( "Converts IECore.PointPrimitive objects to geometry in a Nuke GeometryList object.", PointsPrimitiveTypeId, object )
{
}

void PointToNukeGeometryConverter::doConversion( const IECore::Object *from, GeometryList &to, int objIndex, const IECore::CompoundObject *operands ) const
{
	assert( from );

	const PointsPrimitive *points = static_cast<const PointsPrimitive *>( from );
	to.add_primitive(objIndex, new Particles(Point::POINT, unsigned(points->getNumPoints()), 0));

	// get points
	// \todo: add parameters for standard prim vars
	const V3fVectorData *pointPoints = points->variableData< V3fVectorData >( "P", PrimitiveVariable::Vertex );
	if ( pointPoints )
	{
		unsigned numPoints = pointPoints->readable().size();
		PointList* outPoints = to.writable_points( objIndex );
		outPoints->resize( numPoints );
		std::transform( pointPoints->readable().begin(), pointPoints->readable().end(), outPoints->begin(), IECore::convert< DD::Image::Vector3, Imath::V3f > );
	}

	// get normals
	const V3fVectorData *pointNormals = points->variableData< V3fVectorData >( "N", PrimitiveVariable::Vertex );
	if ( pointNormals )
	{
		Attribute* N = to.writable_attribute( objIndex, Group_Points, "N", NORMAL_ATTRIB);
		unsigned p = 0;
		for ( std::vector< Imath::V3f >::const_iterator nIt = pointNormals->readable().begin(); nIt < pointNormals->readable().end(); nIt++, p++)
		{
			N->normal(p) = IECore::convert< Vector3, Imath::V3f >( *nIt );
		}
	}
	// get width
	const FloatData *constWidth = points->variableData< FloatData >( "constantwidth", PrimitiveVariable::Constant );
	const FloatVectorData *width = points->variableData< FloatVectorData >( "width", PrimitiveVariable::Vertex );
	Attribute* size = to.writable_attribute( objIndex, Group_Points, "size", FLOAT_ATTRIB );
	unsigned v = 0;
	if ( constWidth || width )
	{
		if (width)
		{
			std::vector< float >::const_iterator widthIt = width->readable().begin();
			for ( ; widthIt < width->readable().end(); widthIt++, v++)
			{
				size->flt(v) = *widthIt;
			}
		}
		else
		{
			for( ; v< points->getNumPoints(); v++ )
			{
				size->flt(v) = constWidth->readable();
			}
		}
	}
	else
	{
		for( ; v < points->getNumPoints(); v++ )
		{
			size->flt(v) = 0.1;
		}

	}

	// get uvs
	PrimitiveVariableMap::const_iterator uvIt = points->variables.find( "uv" );
	if( uvIt != points->variables.end() && uvIt->second.interpolation == PrimitiveVariable::Vertex && uvIt->second.data->typeId() == V2fVectorDataTypeId )
	{
		Attribute* uv = to.writable_attribute( objIndex, Group_Vertices, "uv", VECTOR4_ATTRIB );
		if( uvIt->second.indices )
		{
			const std::vector<Imath::V2f> &uvs = runTimeCast<V2fVectorData>( uvIt->second.data )->readable();
			const std::vector<int> &indices = uvIt->second.indices->readable();

			for( size_t i = 0; i < indices.size() ; ++i )
			{
				// as of Cortex 10, we take a UDIM centric approach
				// to UVs, which clashes with Nuke, so we must flip
				// the v values during conversion.
				uv->vector4( i ).set( uvs[indices[i]][0], 1.0 - uvs[indices[i]][1], 0.0f, 1.0f );
			}
		}
		else
		{
			const std::vector<Imath::V2f> &uvs = runTimeCast<V2fVectorData>( uvIt->second.data )->readable();

			for( size_t i = 0; i < uvs.size() ; ++i )
			{
				// as of Cortex 10, we take a UDIM centric approach
				// to UVs, which clashes with Nuke, so we must flip
				// the v values during conversion.
				uv->vector4( i ).set( uvs[i][0], 1.0 - uvs[i][1], 0.0f, 1.0f );
			}
		}
	}

	Attribute *Cf = to.writable_attribute( objIndex, Group_Vertices, "Cf", VECTOR4_ATTRIB );
	v = 0;
	// get colours
	const Color3fVectorData *pointColours = points->variableData< Color3fVectorData >( "Cs", PrimitiveVariable::Vertex );
	if ( pointColours )
	{
		for ( std::vector< Imath::Color3f >::const_iterator cIt = pointColours->readable().begin(); cIt < pointColours->readable().end(); cIt++, v++)
		{
			Cf->vector4( v ).set( (*cIt)[0], (*cIt)[1], (*cIt)[2], 1 );
		}
	}
	else
	{
		for ( v=0; v < points->getNumPoints(); v++)
		{
			Cf->vector4( v ).set(1, 1, 1, 1);
		}

	}
}
