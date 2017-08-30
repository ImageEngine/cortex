//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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
#include "IECore/MeshPrimitive.h"
#include "IECoreNuke/Convert.h"
#include "IECoreNuke/MeshToNukeGeometryConverter.h"

#include "DDImage/Polygon.h"

using namespace boost;
using namespace IECore;
using namespace IECoreNuke;
using namespace DD::Image;

MeshToNukeGeometryConverter::ToNukeGeometryConverterDescription<MeshToNukeGeometryConverter> MeshToNukeGeometryConverter::g_description( MeshPrimitiveTypeId );

MeshToNukeGeometryConverter::MeshToNukeGeometryConverter( IECore::ConstObjectPtr object )
: ToNukeGeometryConverter( "Converts IECore.MeshPrimitive objects to geometry in a Nuke GeometryList object.", MeshPrimitiveTypeId, object )
{
}

void MeshToNukeGeometryConverter::doConversion( const IECore::Object *from, GeometryList &to, int objIndex, const IECore::CompoundObject *operands ) const
{
	assert( from );
	const MeshPrimitive *mesh = static_cast<const MeshPrimitive *>( from );

	const std::vector<int> &vertPerFace = mesh->verticesPerFace()->readable();
	const std::vector<int> &vertIds = mesh->vertexIds()->readable();
	std::vector<int>::const_iterator ids = vertIds.begin();

	// create polygons
	for ( std::vector<int>::const_iterator vpf = vertPerFace.begin(); vpf != vertPerFace.end(); vpf++ )
	{
		Polygon *p = new Polygon( *vpf, true );
		for ( int v = 0; v < *vpf; v++, ids++ )
		{
			p->vertex(v) = *ids;
		}
		to.add_primitive( objIndex, p );
	}

	// get points
	// \todo: add parameters for standard prim vars
	const V3fVectorData *meshPoints = mesh->variableData< V3fVectorData >( "P", PrimitiveVariable::Vertex );
	if ( meshPoints )
	{
		unsigned numPoints = meshPoints->readable().size();
		PointList* points = to.writable_points( objIndex );
		points->resize( numPoints );
		std::transform( meshPoints->readable().begin(), meshPoints->readable().end(), points->begin(), IECore::convert< DD::Image::Vector3, Imath::V3f > );
	}

	// get normals
	const V3fVectorData *meshNormals = mesh->variableData< V3fVectorData >( "N", PrimitiveVariable::Vertex );
	if ( meshNormals )
	{
		Attribute* N = to.writable_attribute( objIndex, Group_Points, "N", NORMAL_ATTRIB);
		unsigned p = 0;
		for ( std::vector< Imath::V3f >::const_iterator nIt = meshNormals->readable().begin(); nIt < meshNormals->readable().end(); nIt++, p++)
		{
			N->normal(p) = IECore::convert< Vector3, Imath::V3f >( *nIt );
		}
	}

	// get uvs
	const FloatVectorData *meshS = mesh->variableData< FloatVectorData >( "s", PrimitiveVariable::FaceVarying );
	const FloatVectorData *meshT = mesh->variableData< FloatVectorData >( "t", PrimitiveVariable::FaceVarying );
	if ( meshS && meshT )
	{
		Attribute* uv = to.writable_attribute( objIndex, Group_Vertices, "uv", VECTOR4_ATTRIB );
		unsigned v = 0;
		std::vector< float >::const_iterator sIt = meshS->readable().begin();
		std::vector< float >::const_iterator tIt = meshT->readable().begin();
		for ( ; sIt < meshS->readable().end(); sIt++, tIt++, v++)
		{
			// as of Cortex 10, we take a UDIM centric approach
			// to UVs, which clashes with Nuke, so we must flip
			// the v values during conversion.
			uv->vector4(v).set( *sIt, 1.0 - *tIt, 0.0f, 1.0f);
		}
	}

	// get colours
	const Color3fVectorData *meshColours = mesh->variableData< Color3fVectorData >( "Cs", PrimitiveVariable::FaceVarying );
	if ( meshColours )
	{
		Attribute *Cf = to.writable_attribute( objIndex, Group_Vertices, "Cf", VECTOR4_ATTRIB );	
		unsigned v = 0;
		for ( std::vector< Imath::Color3f >::const_iterator cIt = meshColours->readable().begin(); cIt < meshColours->readable().end(); cIt++, v++)
		{
			Cf->vector4( v ).set( (*cIt)[0], (*cIt)[1], (*cIt)[2], 1 );
		}
	}

	// \todo Implement custom prim vars...
}
