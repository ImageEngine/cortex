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

#include <cassert>

#include "IECore/DespatchTypedData.h"

#include "IECoreGL/MeshPrimitive.h"
#include "IECoreGL/GL.h"
#include "IECoreGL/State.h"

#include "OpenEXR/ImathMath.h"

using namespace IECoreGL;
using namespace Imath;
using namespace std;


//////////////////////////////////////////////////////////////////////////
// MemberData
//////////////////////////////////////////////////////////////////////////

class MeshPrimitive::MemberData : public IECore::RefCounted
{
	
	public :
	
		MemberData( unsigned numTriangles ) : numTriangles( numTriangles )
		{
		}

		unsigned numTriangles;
		Imath::Box3f bound;

};

//////////////////////////////////////////////////////////////////////////
// MeshPrimitive
//////////////////////////////////////////////////////////////////////////

IE_CORE_DEFINERUNTIMETYPED( MeshPrimitive );

MeshPrimitive::MeshPrimitive( unsigned numTriangles )
	:	m_memberData( new MemberData( numTriangles ) )
{
}

MeshPrimitive::~MeshPrimitive()
{
}

void MeshPrimitive::addPrimitiveVariable( const std::string &name, const IECore::PrimitiveVariable &primVar )
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

	if ( primVar.interpolation==IECore::PrimitiveVariable::FaceVarying )
	{
		addVertexAttribute( name, primVar.data );
	}
	else if ( primVar.interpolation==IECore::PrimitiveVariable::Constant )
	{
		addUniformAttribute( name, primVar.data );
	}
	else if ( primVar.interpolation==IECore::PrimitiveVariable::Vertex || primVar.interpolation==IECore::PrimitiveVariable::Varying )
	{
		throw( "IECoreGL::MeshPrimitive : Invalid interpolation for \"" + name + "\". Must be FaceVarying or Constant." );
	}
}

void MeshPrimitive::renderInstances( size_t numInstances ) const
{
	glDrawArraysInstancedARB( GL_TRIANGLES, 0, m_memberData->numTriangles * 3, numInstances );
}

Imath::Box3f MeshPrimitive::bound() const
{
	return m_memberData->bound;
}
