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

#ifndef IECORE_WARPOP_H
#define IECORE_WARPOP_H

#include "IECore/ChannelOp.h"
#include "IECore/NumericParameter.h"

namespace IECore
{

/// A base class for warp operations on ImagePrimitive objects.
// This Op modifies an image by remapping pixels values to other locations. 
// The mapping is determined by the derived classes. The base class is responsible for applying 
// filtering on the colors based on the floating point positions returned by warp method.
class WarpOp : public ChannelOp
{
	public:

		enum FilterType { None = 0, Bilinear, TypeCount };

		WarpOp( const std::string &name, const std::string &description );
		virtual ~WarpOp();

		IntParameterPtr filterParameter();
		ConstIntParameterPtr filterParameter() const;

		IE_CORE_DECLARERUNTIMETYPED( WarpOp, ChannelOp );
	
	protected :
	
		/// Implemented to call modifyChannels(). Derived classes should implement begin,warp and end rather than this function.
		virtual void modifyChannels( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, ChannelVector &channels );

		/// Called once per operation. This is an opportunity to perform any preprocessing
		/// necessary before many calls to warp() are made.
		virtual void begin( ConstCompoundObjectPtr operands );
		/// Called once per element (pixel for ImagePrimitives).
		/// Must be implemented by subclasses to determine where the color will come from the given pixel location.
		virtual Imath::V2f warp( const Imath::V2f &p ) const = 0;
		/// Called once per operation, after all calls to transform() have been made. This is
		/// an opportunity to perform any cleanup necessary.
		virtual void end();
	
	private :
	
		IntParameterPtr m_filterParameter;

		struct Warp;
		friend class Warp;
};

IE_CORE_DECLAREPTR( WarpOp );


} // namespace IECore

#endif // IECORE_WARPOP_H

