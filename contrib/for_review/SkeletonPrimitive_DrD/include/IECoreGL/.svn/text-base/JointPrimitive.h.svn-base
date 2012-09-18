//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
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

#ifndef IECOREGL_JOINTRIMITIVE_H
#define IECOREGL_JOINTPRIMITIVE_H

#include "OpenEXR/ImathVec.h"
#include "IECoreGL/Primitive.h"

namespace IECoreGL
{

class JointPrimitive : public Primitive
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreGL::JointPrimitive, JointPrimitiveTypeId, Primitive );


		JointPrimitive( float radius = 1.0, float length = 1.0 );
		virtual ~JointPrimitive();

		void setRadius( float radius );
		float getRadius() const;

		void setLength( float length );
		float getLength() const;

		virtual Imath::Box3f bound() const;
		virtual void addPrimitiveVariable( const std::string &name, const IECore::PrimitiveVariable &primVar );

	protected :

		virtual void render( const State *state, IECore::TypeId style ) const;

	private :

		Imath::V3f p0 , p1, p2, p3, p4, p5;


		friend class SkeletonPrimitive;

};

IE_CORE_DECLAREPTR( JointPrimitive );

} // namespace IECoreGL

#endif // IECOREGL_JOINTPRIMITIVE_H
