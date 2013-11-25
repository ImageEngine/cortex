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

struct MeshPrimitive::MemberData : public IECore::RefCounted
{
	MemberData( IECore::ConstIntVectorDataPtr verts ) : vertIds( verts )
	{
	}

	IECore::ConstIntVectorDataPtr vertIds;
	Imath::Box3f bound;

	/// \todo This could be removed now the ToGLMeshConverter uses FaceVaryingPromotionOp
	/// to convert everything to FaceVarying before being added. The only reason we're even
	/// doing this still is in case client code is creating MeshPrimitives directly rather
	/// than using the converter. We should actually be able to remove the code here, and
	/// instead of accept vertIds in the MeshPrimitive constructor, just accept the number
	/// of triangles instead.
	class ToFaceVaryingConverter
	{
		public:

		typedef IECore::DataPtr ReturnType;

		ToFaceVaryingConverter( IECore::ConstIntVectorDataPtr vertIds ) : m_vertIds( vertIds )
		{
			assert( m_vertIds );
		}

		template<typename T>
		IECore::DataPtr operator()( typename T::Ptr inData )
		{
			assert( inData );

			const typename T::Ptr outData = new T();
			outData->writable().resize( m_vertIds->readable().size() );

			typename T::ValueType::iterator outIt = outData->writable().begin();

			for ( typename T::ValueType::size_type i = 0; i <  m_vertIds->readable().size(); i++ )
			{
				*outIt++ = inData->readable()[ m_vertIds->readable()[ i ] ];
			}

			return outData;
		}

		IECore::ConstIntVectorDataPtr m_vertIds;
	};

};

//////////////////////////////////////////////////////////////////////////
// MeshPrimitive
//////////////////////////////////////////////////////////////////////////

IE_CORE_DEFINERUNTIMETYPED( MeshPrimitive );

MeshPrimitive::MeshPrimitive( IECore::ConstIntVectorDataPtr vertIds )
	:	m_memberData( new MemberData( vertIds->copy() ) )
{
}

MeshPrimitive::~MeshPrimitive()
{
}

IECore::ConstIntVectorDataPtr MeshPrimitive::vertexIds() const
{
	return m_memberData->vertIds;
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
	
	if ( primVar.interpolation==IECore::PrimitiveVariable::Vertex || primVar.interpolation==IECore::PrimitiveVariable::Varying )
	{
		MemberData::ToFaceVaryingConverter primVarConverter( m_memberData->vertIds );
		// convert to facevarying
		IECore::DataPtr newData = IECore::despatchTypedData< MemberData::ToFaceVaryingConverter, IECore::TypeTraits::IsVectorTypedData >( primVar.data, primVarConverter );
		addVertexAttribute( name, newData );
	}
	else if ( primVar.interpolation==IECore::PrimitiveVariable::FaceVarying )
	{
		addVertexAttribute( name, primVar.data );
	}
	else if ( primVar.interpolation==IECore::PrimitiveVariable::Constant )
	{
		addUniformAttribute( name, primVar.data );
	}
}

void MeshPrimitive::renderInstances( size_t numInstances ) const
{
	unsigned vertexCount = m_memberData->vertIds->readable().size();
	glDrawArraysInstancedARB( GL_TRIANGLES, 0, vertexCount, numInstances );
}

Imath::Box3f MeshPrimitive::bound() const
{
	return m_memberData->bound;
}
