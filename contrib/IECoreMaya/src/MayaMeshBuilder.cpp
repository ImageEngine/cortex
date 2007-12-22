//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#include <cassert>

#include "maya/MPointArray.h"
#include "maya/MVectorArray.h"
#include "maya/MIntArray.h"
#include "maya/MFloatPointArray.h"
#include "maya/MFloatVectorArray.h"
#include "maya/MFnMesh.h"
#include "maya/MFnMeshData.h"

#include "IECoreMaya/StatusException.h"

#include "IECoreMaya/MayaMeshBuilder.h"

using namespace IECoreMaya;

namespace IECoreMaya
{

template<>
struct MayaMeshBuilder<double>::Data
{
	MPointArray m_P;
	MVectorArray m_N;		
	MIntArray m_verticesPerFace;
	MIntArray m_vertexIds;
};

template<>
struct MayaMeshBuilder<float>::Data
{
	MFloatPointArray m_P;
	MVectorArray m_N;		
	MIntArray m_verticesPerFace;
	MIntArray m_vertexIds;
};

}

template<typename T>
MayaMeshBuilder<T>::MayaMeshBuilder( MObject parentOrOwner ) : m_parentOrOwner( parentOrOwner )
{
	m_data = new Data();		
	
}

template<typename T>
MayaMeshBuilder<T>::~MayaMeshBuilder()
{
	delete m_data;			
}


template<>				
void MayaMeshBuilder<float>::addVertex( const Imath::V3f &p, const Imath::V3f &n )
{
	m_data->m_P.append( MFloatPoint( p.x, p.y, p.z ) );
	m_data->m_N.append( MFloatVector( n.x, n.y, n.z ) );	
}

template<>				
void MayaMeshBuilder<double>::addVertex( const Imath::V3d &p, const Imath::V3d &n )
{
	m_data->m_P.append( MPoint( p.x, p.y, p.z ) );
	m_data->m_N.append( MVector( n.x, n.y, n.z ) );	
}

template<typename T>
void MayaMeshBuilder<T>::addTriangle( int v0, int v1, int v2 )
{
	m_data->m_verticesPerFace.append( 3 );

	m_data->m_vertexIds.append( v0 );
	m_data->m_vertexIds.append( v1 );
	m_data->m_vertexIds.append( v2 );
}
		
template<typename T>		
MObject MayaMeshBuilder<T>::mesh() const
{
	MStatus s;
	MFnMesh fnMesh;

	MObject result = fnMesh.create(
		m_data->m_P.length(),	
		m_data->m_verticesPerFace.length(), 
		m_data->m_P,
		m_data->m_verticesPerFace, 
		m_data->m_vertexIds,
		m_parentOrOwner,
		&s
	);
	
	if (!s)
	{
		throw StatusException( s );
	}
	assert( result != MObject::kNullObj );
	
	MIntArray vertexList;
	for (unsigned i = 0; i < m_data->m_N.length(); ++i)
	{
		vertexList.append( i );
	}
	
	fnMesh.setVertexNormals(
		m_data->m_N,
		vertexList
	);

	return m_parentOrOwner;
}				
