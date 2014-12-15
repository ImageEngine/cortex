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

#ifndef IECOREGL_SPHEREPRIMITIVE_H
#define IECOREGL_SPHEREPRIMITIVE_H

#include "IECoreGL/Export.h"
#include "IECoreGL/Primitive.h"

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( Buffer )

class IECOREGL_API SpherePrimitive : public Primitive
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreGL::SpherePrimitive, SpherePrimitiveTypeId, Primitive );

		/// Z range is always 0 <= zMin < zMax <= 1. ie zMin and zMax are measured as a proportion
		/// of radius and not absolutely as in the Renderman spec.
		SpherePrimitive( float radius = 1, float zMin = -1, float zMax = 1, float thetaMax = 360 );
		virtual ~SpherePrimitive();

		virtual Imath::Box3f bound() const;
		virtual void addPrimitiveVariable( const std::string &name, const IECore::PrimitiveVariable &primVar );

		virtual void renderInstances( size_t numInstances = 1 ) const;

	private :

		float m_radius;
		float m_zMin;
		float m_zMax;
		float m_thetaMax;
		Imath::Box3f m_bound;
		IECore::UIntVectorDataPtr m_vertIds;
		mutable IECoreGL::ConstBufferPtr m_vertIdsBuffer;

};

IE_CORE_DECLAREPTR( SpherePrimitive );

} // namespace IECoreGL

#endif // IECOREGL_SPHEREPRIMITIVE_H
