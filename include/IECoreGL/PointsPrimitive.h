//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2012, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREGL_POINTSPRIMITIVE_H
#define IECOREGL_POINTSPRIMITIVE_H

#include "IECoreGL/Export.h"
#include "IECoreGL/Primitive.h"

#include "IECore/VectorTypedData.h"

namespace IECoreGL
{

class IECOREGL_API PointsPrimitive : public Primitive
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreGL::PointsPrimitive, PointsPrimitiveTypeId, Primitive );

		/// \todo We should consider removing the Point enum value and always using the PointsPrimitiveUseGLPoints
		/// attribute instead.
		enum Type {
			Point,	// GL_POINTS
			Disk,	// camera facing disks
			Quad,	// camera facing quads
			Sphere,	// spheres
		};

		PointsPrimitive( Type type );
		~PointsPrimitive() override;

		Imath::Box3f bound() const override;

		void addPrimitiveVariable( const std::string &name, const IECore::PrimitiveVariable &primVar ) override;
		const Shader::Setup *shaderSetup( const Shader *shader, State *state ) const override;
		void render( const State *currentState, IECore::TypeId style ) const override;
		void renderInstances( size_t numInstances = 1 ) const override;

		//! @name StateComponents
		/// The following StateComponent classes have an effect only on
		/// PointsPrimitive objects.
		//////////////////////////////////////////////////////////////////////////////
		//@{
		///
		/// Specifies an override for rendering PointsPrimitives with gl points.
		typedef TypedStateComponent<GLPointsUsage, PointsPrimitiveUseGLPointsTypeId> UseGLPoints;
		IE_CORE_DECLAREPTR( UseGLPoints );
		/// Specifies an attribute for defining the glPointSize of PointsPrimitives rendered as gl points.
		typedef TypedStateComponent<float, PointsPrimitiveGLPointWidthTypeId> GLPointWidth;
		IE_CORE_DECLAREPTR( GLPointWidth );
		//@}

	private :

		template<typename T>
		static const T *dataAndStride( const IECore::Data *data, const T *defaultValue, unsigned int &stride );
		void updateBounds() const;

		Type effectiveType( const State *state ) const;

		static std::string &instancingVertexSource();

		void depthSort() const;

		IE_CORE_FORWARDDECLARE( MemberData );

		MemberDataPtr m_memberData;

};

IE_CORE_DECLAREPTR( PointsPrimitive );

} // namespace IECoreGL

#endif // IECOREGL_POINTSPRIMITIVE_H
