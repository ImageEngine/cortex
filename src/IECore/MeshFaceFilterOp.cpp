//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, Image Engine Design Inc. All rights reserved.
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

#include "IECore/MeshFaceFilterOp.h"
#include "IECore/CompoundParameter.h"
#include "IECore/VectorTypedData.h"
#include "IECore/VectorDataFilterOp.h"

using namespace IECore;

namespace IECore {

IE_CORE_DEFINERUNTIMETYPED( MeshFaceFilterOp );

MeshFaceFilterOp::MeshFaceFilterOp()
	: MeshPrimitiveOp( "Chops out all but a subset of a mesh's polygons." )
{
	m_filterParameter = new ObjectParameter( 
		"filter", 
		"A bool for every face in the object, indicating weather it's included in the output.", 
		new BoolVectorData(),
		BoolVectorData::staticTypeId()
	);
	
	parameters()->addParameter( m_filterParameter );
}

MeshFaceFilterOp::~MeshFaceFilterOp()
{
}

IECore::ObjectParameter *MeshFaceFilterOp::filterParameter()
{
	return m_filterParameter.get();
}

const IECore::ObjectParameter *MeshFaceFilterOp::filterParameter() const
{
	return m_filterParameter.get();
}

void MeshFaceFilterOp::modifyTypedPrimitive( MeshPrimitive * mesh, const CompoundObject * operands )
{
	ObjectPtr object = m_filterParameter->getValue();
	if( !object )
	{
		throw InvalidArgumentException( "MeshFaceFilterOp : Invalid filter input object." );
	}
	
	BoolVectorDataPtr filterData = runTimeCast<BoolVectorData>( object );
	if( !filterData )
	{
		throw InvalidArgumentException( "MeshFaceFilterOp : The filter input is not a BoolVectorData object." );
	}
	
	const std::vector<bool>& filter = filterData->readable();
	
	if( filter.size() != mesh->numFaces() )
	{
		throw InvalidArgumentException( "MeshFaceFilterOp : The filter must have one entry per mesh face." );
	}
	
	// find all verts/face verts touched by an active face:
	const std::vector<int>& verticesPerFace = mesh->verticesPerFace()->readable();
	const std::vector<int>& vertexIds = mesh->vertexIds()->readable();
	
	size_t numVerts = mesh->variableSize( PrimitiveVariable::Vertex );
	
	BoolVectorDataPtr activeVertsData = new BoolVectorData();
	BoolVectorDataPtr activeFaceVertsData = new BoolVectorData();
	
	std::vector<bool>& activeVerts = activeVertsData->writable();
	std::vector<bool>& activeFaceVerts = activeFaceVertsData->writable();
	
	activeVerts.resize( numVerts, false );
	activeFaceVerts.resize( vertexIds.size(), false );
	
	// why don't we work out the new topology while we're at it?
	IntVectorDataPtr newVerticesPerFaceData = new IntVectorData();
	IntVectorDataPtr newVertexIdsData = new IntVectorData();
	
	std::vector<int>& newVerticesPerFace = newVerticesPerFaceData->writable();
	std::vector<int>& newVertexIds = newVertexIdsData->writable();
	
	size_t vertNum( 0 );
	for( size_t face=0; face < mesh->numFaces(); ++face )
	{
		size_t polyNumVerts = verticesPerFace[face];
		if( filter[ face ] )
		{
			newVerticesPerFace.push_back( polyNumVerts );
			for( size_t j=0; j < polyNumVerts; ++j )
			{
				int vertId = vertexIds[ vertNum + j ];
				activeVerts[vertId] = true;
				activeFaceVerts[ vertNum + j ] = true;
				
				newVertexIds.push_back( vertId );
			}
		}
		vertNum += polyNumVerts;
	}
	
	// Right. So we've gotta chuck away the vertices that aren't touched any more which means resizing arrays and
	// randomly removing their elements. Lets find a mapping between the old arrays and the new ones:
	std::vector<int> vertMapping( activeVerts.size() );
	int idx = 0;
	for( size_t i=0; i < activeVerts.size(); ++i )
	{
		vertMapping[i] = idx;
		idx += int( activeVerts[i] );
	}
	
	// use this to remap the new vertex ids:
	for( size_t i=0; i < newVertexIds.size(); ++i )
	{
		newVertexIds[i] = vertMapping[ newVertexIds[i] ];
	}
	
	// decimate primvars:
	VectorDataFilterOpPtr filterOp = new VectorDataFilterOp();
	filterOp->copyParameter()->setTypedValue( false );
	
	// we keep track of all the primitive variables we've filtered so far, in case some of the
	// actual data buffers are duplicated:
	std::set< IECore::DataPtr > primvarsDone;
	
	for( IECore::PrimitiveVariableMap::iterator it = mesh->variables.begin(); it != mesh->variables.end(); ++it )
	{
		if( primvarsDone.find( it->second.data ) != primvarsDone.end() )
		{
			continue;
		}
		
		switch( it->second.interpolation )
		{
			case PrimitiveVariable::Constant :
				break; // nothing to do

			case PrimitiveVariable::Uniform :
				filterOp->parameters()->parameter<Parameter>( "filter" )->setValue( filterData );
				filterOp->inputParameter()->setValue( it->second.data );
				filterOp->operate();
				break;

			case PrimitiveVariable::Vertex :
			case PrimitiveVariable::Varying:
				filterOp->parameters()->parameter<Parameter>( "filter" )->setValue( activeVertsData );
				filterOp->inputParameter()->setValue( it->second.data );
				filterOp->operate();
				break;

			case PrimitiveVariable::FaceVarying:
				filterOp->parameters()->parameter<Parameter>( "filter" )->setValue( activeFaceVertsData );
				filterOp->inputParameter()->setValue( it->second.data );
				filterOp->operate();
				break;

			default :
				break;

		}
		
		primvarsDone.insert( it->second.data );
		
	}
	
	// set new topology:
	mesh->setTopology( newVerticesPerFaceData, newVertexIdsData, mesh->interpolation() );
}


} // namespace IECore
