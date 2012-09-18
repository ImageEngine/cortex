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

#ifndef IECORENUKE_WARPNODE_H
#define IECORENUKE_WARPNODE_H

#include "DDImage/Iop.h"

#include "OpenEXR/ImathVec.h"

namespace IECoreNuke
{

/// A base class to simplify the implementation of Iops which apply an arbitrary
/// distortion to an image.
class Warp : public DD::Image::Iop
{

	public :

		Warp( Node *node );
		virtual ~Warp();

	protected :

		/// Must be implemented by subclasses to return the pixel position from the
		/// input that should be used to fill the outputPixel. This function will
		/// be called from multiple threads.
		virtual Imath::V2f warp( const Imath::V2f &outputPixel ) const = 0;

		virtual void knobs( DD::Image::Knob_Callback f );
		virtual void _validate( bool forReal );
		virtual void _request( int x, int y, int r, int t, const DD::Image::ChannelSet &channels, int count );
		virtual void engine( int y, int x, int r, const DD::Image::ChannelSet &channels, DD::Image::Row &out );

	private :

		DD::Image::ChannelSet m_channels;

};

} // namespace IECoreNuke

#endif // IECORENUKE_WARPNODE_H
