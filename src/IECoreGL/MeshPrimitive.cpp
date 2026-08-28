//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

#include "IECoreGL/MeshPrimitive.h"

#include "IECoreGL/GL.h"
#include "IECoreGL/State.h"
#include "IECoreGL/CachedConverter.h"

#include "IECore/DespatchTypedData.h"

#include "Imath/ImathMath.h"

#include <cassert>

using namespace IECoreGL;
using namespace Imath;
using namespace std;


//////////////////////////////////////////////////////////////////////////
// MemberData
//////////////////////////////////////////////////////////////////////////

class MeshPrimitive::MemberData : public IECore::RefCounted
{

	public :

		MemberData( IECore::ConstIntVectorDataPtr meshIndices ) : meshIndices( meshIndices )
		{
		}

		IECore::ConstIntVectorDataPtr meshIndices;
		ConstBufferPtr meshIndicesGL;
		Imath::Box3f bound;

};

//////////////////////////////////////////////////////////////////////////
// MeshPrimitive
//////////////////////////////////////////////////////////////////////////

IE_CORE_DEFINERUNTIMETYPED( MeshPrimitive );

MeshPrimitive::MeshPrimitive( IECore::ConstIntVectorDataPtr meshIndices )
	:	m_memberData( new MemberData( meshIndices ) )
{
}

MeshPrimitive::~MeshPrimitive()
{
}

void MeshPrimitive::addPrimitiveVariable( const std::string &name, const IECoreScene::PrimitiveVariable &primVar )
{
	if( name == "P" )
	{
		// update the bounding box.
		m_memberData->bound.makeEmpty();
		IECore::ConstV3fVectorDataPtr points = IECore::runTimeCast< IECore::V3fVectorData >( primVar.data );
		if( points )
		{
			const std::vector<Imath::V3f> &p = points->readable();
			for( unsigned int i=0; i<p.size(); i++ )
			{
				m_memberData->bound.extendBy( p[i] );
			}
		}
	}

	if ( primVar.interpolation==IECoreScene::PrimitiveVariable::FaceVarying )
	{
		addVertexAttribute( name, primVar.expandedData() );
	}
	else if ( primVar.interpolation==IECoreScene::PrimitiveVariable::Constant )
	{
		addUniformAttribute( name, primVar.expandedData() );
	}
	else if ( primVar.interpolation==IECoreScene::PrimitiveVariable::Vertex || primVar.interpolation==IECoreScene::PrimitiveVariable::Varying )
	{
		throw( "IECoreGL::MeshPrimitive : Invalid interpolation for \"" + name + "\". Must be FaceVarying or Constant." );
	}
}

void MeshPrimitive::renderInstances( size_t numInstances ) const
{
	if( !m_memberData->meshIndicesGL )
	{
		// \todo : The use of defaultCachedConverter here is very inefficient - the mesh indices have already
		// been expanded from verticesPerFace to an explicit index list before being passed in ... any time
		// we find the index list in the cache, the time we previously spent recomputing the expanded list
		// was wasted.
		// The same problem exists for primvars - we first fully expand the data out to FaceVarying in
		// ToGLMeshConverter, and then we look up if it's in the cache and that work has already been done.
		// The solution would be passing in more context to this convert call about how it's supposed to be
		// converted, so we could just pass in the verticesPerFace, but indicate that they need to be converted
		// to expanded indices. I had considered just adding a "Purpose" enum to this call, John suggested
		// it should take some sort of function pointer, so this cache could be used for anything. Either
		// way, it now seems complicated enough that we're not going to tackle it now.
		m_memberData->meshIndicesGL = IECore::runTimeCast<const Buffer>( CachedConverter::defaultCachedConverter()->convert( m_memberData->meshIndices.get() ) );
	}

	Buffer::ScopedBinding binding( *m_memberData->meshIndicesGL, GL_ELEMENT_ARRAY_BUFFER );
	glDrawElementsInstancedARB( GL_TRIANGLES, m_memberData->meshIndices->readable().size(), GL_UNSIGNED_INT, 0, numInstances );
}

Imath::Box3f MeshPrimitive::bound() const
{
	return m_memberData->bound;
}
