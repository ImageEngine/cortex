//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_SIMPLESUBSURFACE_H
#define IECORE_SIMPLESUBSURFACE_H

#include "IECore/RefCounted.h"
#include "IECore/KDTree.h"
#include "IECore/VectorTypedData.h"
#include "IECore/Spline.h"

namespace IECore
{

/// A simple subsurface scattering implementation using a hierarchical pointcloud. Rather
/// than use the standard dipole approximation it allows the falloff to be controlled
/// explicitly using a spline.
class SimpleSubsurface : public IECore::RefCounted
{

	public :
	
		IE_CORE_DECLAREMEMBERPTR( SimpleSubsurface );
	
		SimpleSubsurface( ConstV3fVectorDataPtr p, ConstColor3fVectorDataPtr c, ConstFloatVectorDataPtr a, const SplinefColor3f &falloff );
		virtual ~SimpleSubsurface();
		
		Imath::Color3f lookup( const Imath::V3f &p, float radius ) const;
		
	private :

		typedef V3fTree Tree;

		struct PrivateData;
		boost::shared_ptr<PrivateData> m_privateData;
		
		void buildWalk( Tree::NodeIndex nodeIndex );

		inline void accum( const Imath::V3f &p, float radius, const Imath::V3f &sampleP, const Imath::Color3f &sampleC, Imath::Color3f &result ) const;
		void lookupWalk( Tree::NodeIndex nodeIndex, const Imath::V3f &p, float radius, Imath::Color3f &result ) const;
		
};

IE_CORE_DECLAREPTR( SimpleSubsurface );

} // namespace IECore

#endif // IECORE_SIMPLESUBSURFACE_H
