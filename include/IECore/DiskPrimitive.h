//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012-2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_DISKPRIMITIVE_H
#define IECORE_DISKPRIMITIVE_H

#include "IECore/Export.h"
#include "IECore/Primitive.h"

namespace IECore
{

/// The DiskPrimitive represents a renderable parametric disk
/// \ingroup geometryGroup
class IECORE_API DiskPrimitive : public Primitive
{

	public:

		IE_CORE_DECLAREOBJECT( DiskPrimitive, Primitive );

		DiskPrimitive( float radius = 1.0f, float z = 0.0f, float thetaMax = 360.0f );

		//! @name Geometry access
		/// These functions allow access to get and set the disk's geometry after construction.
		/////////////////////////////////////////////////////////////////////////////
		//@{
		float getRadius() const;
		void setRadius( float radius );
		float getZ() const;
		void setZ( float z );
		float getThetaMax() const;
		void setThetaMax( float degrees );
		//@}
		
		virtual size_t variableSize( PrimitiveVariable::Interpolation interpolation ) const;

		virtual Imath::Box3f bound() const;
		virtual void render( Renderer *renderer ) const;
		
		virtual void topologyHash( MurmurHash &h ) const;

	private:

		static const unsigned int m_ioVersion;

		float m_radius;
		float m_z;
		float m_thetaMax;

};

IE_CORE_DECLAREPTR( DiskPrimitive );

}

#endif // IECORE_DISKPRIMITIVE_H
