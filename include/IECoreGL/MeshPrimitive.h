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

#ifndef IECOREGL_MESHPRIMITIVE_H
#define IECOREGL_MESHPRIMITIVE_H

#include "IECoreGL/Primitive.h"

#include "IECore/VectorTypedData.h"

namespace IECoreGL
{

/// \todo Fast drawing, uvs etc. Consider using NVIDIA tristrip library? something else? GLU?
class MeshPrimitive : public Primitive
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreGL::MeshPrimitive, MeshPrimitiveTypeId, Primitive );

		/// Copies of all data are taken.
		/// \deprecated. This constructor was being used to allow the MeshPrimitive to support
		/// Vertex and Varying primitive variables in addPrimitiveVariable(), but it lacks the
		/// information necessary to support Uniform primitive variables. In the future this
		/// constructor will be removed - for forwards compatibility, use a ToGLMeshConverter
		/// to create MeshPrimitives.
		/// \todo Replace this with a simple MeshPrimitive( numTriangles ) constructor, remove all
		/// the conversions from addPrimitiveVariable, and just rely on the work the ToGLMeshConverter
		/// already does.
		MeshPrimitive( IECore::ConstIntVectorDataPtr vertIds );
		virtual ~MeshPrimitive();

		IECore::ConstIntVectorDataPtr vertexIds() const;

		virtual Imath::Box3f bound() const;

		virtual void addPrimitiveVariable( const std::string &name, const IECore::PrimitiveVariable &primVar );

		virtual void renderInstances( size_t numInstances = 1 ) const;

	private :

		IE_CORE_FORWARDDECLARE( MemberData );
		MemberDataPtr m_memberData;

		/// So Font can use the render( state, style ) method.
		friend class Font;

};

IE_CORE_DECLAREPTR( MeshPrimitive );

} // namespace IECoreGL

#endif // IECOREGL_MESHPRIMITIVE_H
