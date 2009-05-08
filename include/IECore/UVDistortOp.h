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

#ifndef IECORE_UVDISTORT_H
#define IECORE_UVDISTORT_H

#include "IECore/WarpOp.h"
#include "ObjectParameter.h"

namespace IECore
{

/// Distorts an ImagePrimitive by using a UV map as reference.
// This Op expects an UV map with the red and green values being normalized coordinates (x,y) from the undistorted image.
// The normalized coordinates (0,0)x(1,1) in the uv map will map to the display window of the input image.
// The resulting image will have the same data window as the reference UV map.
class UVDistortOp : public WarpOp
{
	public:

		UVDistortOp();
		virtual ~UVDistortOp();

		ObjectParameterPtr uvMapParameter();
		ConstObjectParameterPtr uvMapParameter() const;

		IE_CORE_DECLARERUNTIMETYPED( UVDistortOp, WarpOp );

	protected :

		virtual void begin( ConstCompoundObjectPtr operands );
		virtual Imath::Box2i warpedDataWindow( const Imath::Box2i &dataWindow ) const;
		virtual Imath::V2f warp( const Imath::V2f &p ) const;
		virtual void end();

	private :

		ObjectParameterPtr m_uvMapParameter;
		Imath::V2f m_imageSize;
		Imath::V2i m_imageOrigin;
		Imath::V2i m_uvOrigin;
		Imath::V2i m_uvSize;
		ConstDataPtr m_u;
		ConstDataPtr m_v;

		struct Lookup;
};

IE_CORE_DECLAREPTR( UVDistortOp );


} // namespace IECore

#endif // IECORE_UVDISTORT_H

