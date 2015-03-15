//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_IMAGECONVOLVEOP_H
#define IECORE_IMAGECONVOLVEOP_H

#include "IECore/Export.h"
#include "IECore/ChannelOp.h"
#include "IECore/NumericParameter.h"
#include "IECore/SphericalHarmonicsProjector.h"

namespace IECore
{

// Convolves image channels using Spherical Harmonics. 
// Based on "An efficient representation for irradiance environment maps." by Ramamoorthi, Ravi and Hanrahan, Pat - 2001.
// Using 3 bands you get diffuse irradiance environment maps.
// Using more bands you get sharper maps ( for specular convolution ).
/// \ingroup imageProcessingGroup
/// \ingroup shGroup
class IECORE_API ImageConvolveOp : public ChannelOp
{
	public:

		ImageConvolveOp();
		virtual ~ImageConvolveOp();

		IE_CORE_DECLARERUNTIMETYPED( ImageConvolveOp, ChannelOp );

	protected :

		struct ParallelTask;

		virtual void modifyChannels( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, ChannelVector &channels );

		void processChannels( SHProjectorf &projector, const SHf &kernel, int imgWidth, int imgHeight, unsigned bands,  std::vector<float> &channel0, std::vector<float> &channel1, std::vector<float> &channel2 ) const;

		IntParameterPtr m_bands;
		IntParameterPtr m_samples;
};

IE_CORE_DECLAREPTR( ImageConvolveOp );


} // namespace IECore

#endif // IECORE_IMAGECONVOLVEOP_H

