//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_SPHEREPRIMITIVE_H
#define IECORE_SPHEREPRIMITIVE_H

#include "IECore/Primitive.h"
#include "IECore/VectorTypedData.h"

namespace IECore
{

/// SpherePrimitive represents a renderable parametric sphere
class SpherePrimitive : public Primitive
{	  
	
	public:

		IE_CORE_DECLAREOBJECT( SpherePrimitive, Primitive );

		/// Construct a full sphere of radius 1.
		SpherePrimitive();
		
		/// Construct a sphere with specified parameterisation. zMin and zMax different from the Renderman specification
		/// slightly in that they're given as a proportion of that radius, so they should fall within the range [-1, 1 ]
		SpherePrimitive( float radius, float zMin = -1.0f, float zMax = 1.0f, float thetaMax = 360.0f );

		//! @name Sphere geometry access
		/// These functions allow access to get and set the sphere's geometry after construction.
		/////////////////////////////////////////////////////////////////////////////
		//@{
		
		float radius() const;
		float zMin() const;
		float zMax() const;
		
		/// The returned value is specified in degrees
		float thetaMax() const;	
		
		void setRadius( float r );
		void setZMin( float zm );
		void setZMax( float zm );
		
		/// The value of thetaMax shoud be specified in degrees		
		void setThetaMax( float tm );						
		
		//@}
	
		virtual size_t variableSize( PrimitiveVariable::Interpolation interpolation ) const;

		/// Render the sphere
		virtual void render( RendererPtr renderer );

	private:

		static const unsigned int m_ioVersion;
		
		float m_radius;
		float m_zMin;
		float m_zMax;
		float m_thetaMax;
			
};  
	
IE_CORE_DECLAREPTR( SpherePrimitive );

}

#endif // IECORE_SPHEREPRIMITIVE
