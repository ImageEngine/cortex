//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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
#include "IECore/TriangulateOp.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/MurmurHash.h"

#include "IECoreGL/MeshPrimitive.h"
#include "IECoreGL/GL.h"
#include "IECoreGL/State.h"
#include "IECoreGL/CachedConverter.h"

#include "OpenEXR/ImathMath.h"

using namespace IECoreGL;
using namespace Imath;
using namespace std;

//////////////////////////////////////////////////////////////////////////
// MemberData
//////////////////////////////////////////////////////////////////////////

struct MeshPrimitive::MemberData : public IECore::RefCounted
{
	MemberData( IECore::ConstIntVectorDataPtr verts ) : originalVerticesPerFace(0), originalVertexIds(0), vertIds( verts )
	{
	}

	IECore::ConstIntVectorDataPtr originalVerticesPerFace;
	IECore::ConstIntVectorDataPtr originalVertexIds;

	IECore::ConstIntVectorDataPtr vertIds;
	Imath::Box3f bound;

	/// Class to triangulate face-varying prim vars.
	/// \todo Consider having a specific module for this class. Also consider avoiding code duplication with TriangulateOp.
	class TriangulatedFaceVaryingPrimVar
	{
		public:

		typedef IECore::DataPtr ReturnType;

		/// Constructs the converter
		/// \param faceVaryingLength Gives the expected output length of the face varying prim var after conversion.
		/// \param verticesPerFace Vertices per shape array of the original non-triangulated mesh.
		TriangulatedFaceVaryingPrimVar( size_t faceVaryingLength, const IECore::IntVectorData *verticesPerFace ) :
				m_faceVaryingLength(faceVaryingLength), m_verticesPerFace(verticesPerFace)
		{
			assert( vertIds );
		}

		IECore::MurmurHash hash( const IECore::Object *object ) const
		{
			IECore::MurmurHash h;
			h.append( "TriangulatedFaceVaryingPrimVar");
			if ( m_verticesPerFace )
			{
				m_verticesPerFace->hash(h);
			}
			object->hash(h);
			return h;
		}

		IECore::RunTimeTypedPtr operator()( const IECore::Object *object )
		{	
			IECore::DataPtr data = const_cast< IECore::Data * >( static_cast< const IECore::Data * >(object) );
			return IECore::despatchTypedData< TriangulatedFaceVaryingPrimVar, IECore::TypeTraits::IsVectorTypedData >( data, *this );
		}

		template<typename T>
		IECore::DataPtr operator()( typename T::Ptr inData )
		{
			assert( inData );

			const typename T::Ptr outData = new T();
			outData->writable().reserve( m_faceVaryingLength );
			typename T::ValueType &out = outData->writable();
			const typename T::ValueType &in = inData->readable();

			const std::vector<int> &verticesPerFaceReadable = m_verticesPerFace->readable();

			typename T::ValueType::const_iterator inIt = in.begin();

			for ( std::vector<int>::const_iterator it = verticesPerFaceReadable.begin(); it != verticesPerFaceReadable.end(); ++it )
			{
				int numFaceVerts = *it;

				if ( numFaceVerts > 3 )
				{
					for (int i = 1; i < numFaceVerts - 1; i++)
					{
						out.push_back( *inIt );
						out.push_back( *(inIt+i) );
						out.push_back( *(inIt+i+1) );
					}
					inIt += numFaceVerts;
				}
				else
				{
					out.push_back( *inIt++ );
					out.push_back( *inIt++ );
					out.push_back( *inIt++ );
				}
			}
			return outData;
		}

		private :

		size_t m_faceVaryingLength;
		const IECore::IntVectorData *m_verticesPerFace;
	};


	/// Class to triangulate and to promote vertex/varying prim vars to face varying interpolation.
	/// \todo Consider having a specific module for this class. Also consider avoiding code duplication with TriangulateOp.
	class TriangulatedVertexPrimVar
	{
		public:

		typedef IECore::DataPtr ReturnType;

		/// Constructs the converter
		/// \param faceVaryingLength Gives the expected output length of the face varying prim var after conversion.
		/// \param vertexIds Vertex ids of the original non-triangulated mesh
		/// \param verticesPerFace Vertices per shape array of the original non-triangulated mesh or NULL if it was already triangulated.
		TriangulatedVertexPrimVar( size_t faceVaryingLength, const IECore::IntVectorData *vertexIds, const IECore::IntVectorData *verticesPerFace ) :
				m_faceVaryingLength(faceVaryingLength), m_vertexIds(vertexIds), m_verticesPerFace(verticesPerFace)
		{
			assert( vertIds );
		}

		IECore::MurmurHash hash( const IECore::Object *object ) const
		{
			IECore::MurmurHash h;
			h.append( "TriangulatedVertexPrimVar");
			h.append( m_faceVaryingLength );
			m_vertexIds->hash(h);
			if ( m_verticesPerFace )
			{
				m_verticesPerFace->hash(h);
			}
			object->hash(h);
			return h;
		}

		IECore::RunTimeTypedPtr operator()( const IECore::Object *object )
		{	
			IECore::DataPtr data = const_cast< IECore::Data * >( static_cast< const IECore::Data * >(object) );
			return IECore::despatchTypedData< TriangulatedVertexPrimVar, IECore::TypeTraits::IsVectorTypedData >( data, *this );
		}

		template<typename T>
		IECore::DataPtr operator()( typename T::Ptr inData )
		{
			assert( inData );

			const typename T::Ptr outData = new T();
			outData->writable().reserve( m_faceVaryingLength );
			typename T::ValueType &out = outData->writable();
			const typename T::ValueType &in = inData->readable();

			const std::vector<int> &vertexIdsReadable = m_vertexIds->readable();

			if ( m_verticesPerFace )
			{
				const std::vector<int> &verticesPerFaceReadable = m_verticesPerFace->readable();
				std::vector<int>::const_iterator vit = vertexIdsReadable.begin();

				for ( std::vector<int>::const_iterator it = verticesPerFaceReadable.begin(); it != verticesPerFaceReadable.end(); ++it )
				{
					int numFaceVerts = *it;

					if ( numFaceVerts > 3 )
					{
						const int v0 = *vit++;

						for (int i = 1; i < numFaceVerts - 1; i++)
						{
							// convert vertex prim vars to face varying
							out.push_back( in[v0] );
							out.push_back( in[*vit++] );
							out.push_back( in[*vit] );
						}
						vit++;
					}
					else
					{
						// convert vertex prim vars to face varying
						out.push_back( in[*vit++] );
						out.push_back( in[*vit++] );
						out.push_back( in[*vit++] );
					}
				}
			}
			else 	/// triangulated case
			{
				for ( std::vector<int>::const_iterator vit = vertexIdsReadable.begin(); vit != vertexIdsReadable.end(); vit++ )
				{
					out.push_back( in[*vit] );
				}
			}
			return outData;
		}

		private :

		size_t m_faceVaryingLength;
		const IECore::IntVectorData *m_vertexIds;
		const IECore::IntVectorData *m_verticesPerFace;
	};

	/// Class to triangulate and to promote uniform prim vars to face varying interpolation.
	/// \todo Consider having a specific module for this class. Also consider avoiding code duplication with TriangulateOp.
	class TriangulatedUniformPrimVar
	{
		public:

		typedef IECore::DataPtr ReturnType;

		/// Constructs the converter
		/// \param faceVaryingLength Gives the expected output length of the face varying prim var after conversion.
		/// \param vertexIds Vertex ids of the original non-triangulated mesh
		/// \param verticesPerFace Vertices per shape array of the original non-triangulated mesh or NULL if it was already triangulated.
		TriangulatedUniformPrimVar( size_t faceVaryingLength, const IECore::IntVectorData *verticesPerFace ) :
				m_faceVaryingLength(faceVaryingLength),  m_verticesPerFace(verticesPerFace)
		{
			assert( vertIds );
		}

		IECore::MurmurHash hash( const IECore::Object *object ) const
		{
			IECore::MurmurHash h;
			h.append( "TriangulatedUniformPrimVar");
			if ( m_verticesPerFace )
			{
				m_verticesPerFace->hash(h);
			}
			object->hash(h);
			return h;
		}

		IECore::RunTimeTypedPtr operator()( const IECore::Object *object )
		{	
			IECore::DataPtr data = const_cast< IECore::Data * >( static_cast< const IECore::Data * >(object) );
			return IECore::despatchTypedData< TriangulatedUniformPrimVar, IECore::TypeTraits::IsVectorTypedData >( data, *this );
		}

		template<typename T>
		IECore::DataPtr operator()( typename T::Ptr inData )
		{
			assert( inData );

			const typename T::Ptr outData = new T();
			outData->writable().reserve( m_faceVaryingLength );
			typename T::ValueType &out = outData->writable();
			const typename T::ValueType &in = inData->readable();

			if ( m_verticesPerFace )
			{
				const std::vector<int> &verticesPerFaceReadable = m_verticesPerFace->readable();
				typename T::ValueType::const_iterator inIt = in.begin();

				for ( std::vector<int>::const_iterator it = verticesPerFaceReadable.begin(); it != verticesPerFaceReadable.end(); ++it )
				{
					int numFaceVerts = *it;

					if ( numFaceVerts > 3 )
					{
						for (int i = 1; i < numFaceVerts - 1; i++)
						{
							out.push_back( *inIt );
							out.push_back( *inIt );
							out.push_back( *inIt );
						}
					}
					else
					{
						out.push_back( *inIt );
						out.push_back( *inIt );
						out.push_back( *inIt );
					}
					inIt++;
				}
			}
			else 	/// triangulated case
			{
				for ( typename T::ValueType::const_iterator inIt = in.begin(); inIt != in.end(); inIt++ )
				{
					out.push_back( *inIt );
					out.push_back( *inIt );
					out.push_back( *inIt );
				}
			}
			return outData;
		}

		private :

		size_t m_faceVaryingLength;
		const IECore::IntVectorData *m_verticesPerFace;
	};

	class TriangulatedVertices
	{
		public:

		TriangulatedVertices( const IECore::IntVectorData *verticesPerFace ) :
				m_verticesPerFace(verticesPerFace)
		{
		}

		IECore::MurmurHash hash( const IECore::Object *object ) const
		{
			IECore::MurmurHash h;
			h.append( "TriangulatedVertices");
			m_verticesPerFace->hash(h);
			object->hash(h);
			return h;
		}

		IECore::RunTimeTypedPtr operator()( const IECore::Object *object )
		{	
			const vector<int> &verticesPerFace =  m_verticesPerFace->readable();
			const vector<int> &vertexIds = static_cast< const IECore::IntVectorData * >(object)->readable();

			/// triangulate the mesh topology
			IECore::IntVectorDataPtr vertIds = new IECore::IntVectorData();
			std::vector<int> &newVertexIdsWritable = vertIds->writable();
			newVertexIdsWritable.reserve( vertexIds.size() + vertexIds.size()/2 );	/// assume we have quads

			int faceVertexIdStart = 0;
			for ( std::vector<int>::const_iterator it = verticesPerFace.begin(); it != verticesPerFace.end(); ++it )
			{
				int numFaceVerts = *it;

				if ( numFaceVerts > 3 )
				{
					/// For the time being, just do a simple triangle fan.
					const int i0 = faceVertexIdStart + 0;
					const int v0 = vertexIds[ i0 ];
	
					int i1 = faceVertexIdStart + 1;
					int i2 = faceVertexIdStart + 2;
					int v1 = vertexIds[ i1 ];
					int v2 = vertexIds[ i2 ];
	
					for (int i = 1; i < numFaceVerts - 1; i++)
					{
						i1 = faceVertexIdStart + ( (i + 0) % numFaceVerts );
						i2 = faceVertexIdStart + ( (i + 1) % numFaceVerts );
						v1 = vertexIds[ i1 ];
						v2 = vertexIds[ i2 ];
	
						/// Triangulate the vertices
						newVertexIdsWritable.push_back( v0 );
						newVertexIdsWritable.push_back( v1 );
						newVertexIdsWritable.push_back( v2 );
					}
				}
				else
				{
					/// Copy across the vertexId data
					newVertexIdsWritable.push_back( vertexIds[ faceVertexIdStart + 0 ] );
					newVertexIdsWritable.push_back( vertexIds[ faceVertexIdStart + 1 ] );
					newVertexIdsWritable.push_back( vertexIds[ faceVertexIdStart + 2 ] );
				}
				faceVertexIdStart += numFaceVerts;
			}
			return vertIds;
		}

		const IECore::IntVectorData *m_verticesPerFace;

	};

};

//////////////////////////////////////////////////////////////////////////
// MeshPrimitive
//////////////////////////////////////////////////////////////////////////

IE_CORE_DEFINERUNTIMETYPED( MeshPrimitive );

MeshPrimitive::MeshPrimitive( IECore::ConstIntVectorDataPtr verticesPerFace, IECore::ConstIntVectorDataPtr vertexIds ) : m_memberData( new MemberData(0) )
{
	if ( !verticesPerFace || !vertexIds )
	{
		throw IECore::Exception( "NULL pointers passed to MeshPrimitive constructor!" );
	}

	/// keep a copy of the mesh topology for future calls to addPrimitiveVariable
	m_memberData->originalVerticesPerFace = verticesPerFace->copy();
	m_memberData->originalVertexIds = vertexIds->copy();

	/// triangulate the mesh topology
	CachedConverterPtr cachedConverter = CachedConverter::defaultCachedConverter();
	MemberData::TriangulatedVertices triangulate( verticesPerFace );
	m_memberData->vertIds = IECore::staticPointerCast< const IECore::IntVectorData >( cachedConverter->convert( vertexIds, triangulate ) );
}

MeshPrimitive::MeshPrimitive( IECore::ConstIntVectorDataPtr vertIds )
	:	m_memberData( new MemberData( vertIds->copy() ) )
{
		m_memberData->originalVerticesPerFace = 0;
		m_memberData->originalVertexIds = 0;
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
	if ( primVar.interpolation==IECore::PrimitiveVariable::Vertex || primVar.interpolation==IECore::PrimitiveVariable::Varying )
	{
		if ( name == "P" )
		{
			// update the bounding box.
			m_memberData->bound.makeEmpty();
			IECore::ConstV3fVectorDataPtr points = IECore::runTimeCast< IECore::V3fVectorData >( primVar.data );
			if ( points )
			{
				const std::vector<Imath::V3f> &p = points->readable();
				for( unsigned int i=0; i<p.size(); i++ )
				{
					m_memberData->bound.extendBy( p[i] );
				}
			}
		}
	}

	CachedConverterPtr cachedConverter = CachedConverter::defaultCachedConverter();

	if ( primVar.interpolation==IECore::PrimitiveVariable::Vertex || 
		 primVar.interpolation==IECore::PrimitiveVariable::Varying )
	{
		// triangulate and convert to facevarying
		MemberData::TriangulatedVertexPrimVar primVarConverter(m_memberData->vertIds->readable().size(), 
			( m_memberData->originalVerticesPerFace ? m_memberData->originalVertexIds : m_memberData->vertIds ), m_memberData->originalVerticesPerFace );

		IECore::ConstDataPtr newData = IECore::staticPointerCast< const IECore::Data >( cachedConverter->convert( primVar.data, primVarConverter ) );

		addVertexAttribute( name, newData );
	}
	else if ( primVar.interpolation==IECore::PrimitiveVariable::FaceVarying )
	{
		// triangulate facevarying
		MemberData::TriangulatedFaceVaryingPrimVar primVarConverter(m_memberData->vertIds->readable().size(), m_memberData->originalVerticesPerFace );

		IECore::ConstDataPtr newData = IECore::staticPointerCast< const IECore::Data >( cachedConverter->convert( primVar.data, primVarConverter ) );

		addVertexAttribute( name, newData );
	}
	else if ( primVar.interpolation==IECore::PrimitiveVariable::Uniform )
	{
		// triangulate and convert to facevarying
		MemberData::TriangulatedUniformPrimVar primVarConverter(m_memberData->vertIds->readable().size(), m_memberData->originalVerticesPerFace );

		IECore::ConstDataPtr newData = IECore::staticPointerCast< const IECore::Data >( cachedConverter->convert( primVar.data, primVarConverter ) );

		addVertexAttribute( name, newData );
	}
	else if ( primVar.interpolation==IECore::PrimitiveVariable::Constant )
	{
		addUniformAttribute( name, primVar.data );
	}
}

void MeshPrimitive::renderInstances( size_t numInstances ) const
{
	unsigned vertexCount = m_memberData->vertIds->readable().size();
	glDrawArraysInstanced( GL_TRIANGLES, 0, vertexCount, numInstances );
}

Imath::Box3f MeshPrimitive::bound() const
{
	return m_memberData->bound;
}
