//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREIMAGE_MEDIANCUTSAMPLER_H
#define IECOREIMAGE_MEDIANCUTSAMPLER_H

#include "IECore/Op.h"
#include "IECore/TypedPrimitiveParameter.h"
#include "IECore/NumericParameter.h"
#include "IECore/SimpleTypedParameter.h"

#include "IECoreImage/Export.h"
#include "IECoreImage/TypeIds.h"

namespace IECoreImage
{

/// An implementation of the median cut algorithm described in
/// A Median Cut Algorithm for Light Probe Sampling by Paul Debevec.
/// This class just worries about the point sampling and centroid
/// calculations, and leaves it to a caller to interpret them as
/// light directions and positions. This allows the use of the class
/// as a simple 2d point distribution algorithm in addition to
/// a light probe sampler.
/// \ingroup imageProcessingGroup
class IECOREIMAGE_API MedianCutSampler : public IECore::Op
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( MedianCutSampler, MedianCutSamplerTypeId, IECore::Op );

		MedianCutSampler();
		virtual ~MedianCutSampler();

		IECore::ImagePrimitiveParameter *imageParameter();
		const IECore::ImagePrimitiveParameter *imageParameter() const;

		IECore::StringParameter *channelNameParameter();
		const IECore::StringParameter *channelNameParameter() const;

		IECore::IntParameter *subdivisionDepthParameter();
		const IECore::IntParameter *subdivisionDepthParameter() const;

		enum Projection
		{
			Invalid,
			Rectilinear,
			LatLong
		};

		IECore::IntParameter *projectionParameter();
		const IECore::IntParameter *projectionParameter() const;

	protected :

		IECore::ObjectPtr doOperation( const IECore::CompoundObject *operands );

	private :

		IECore::ImagePrimitiveParameterPtr m_imageParameter;
		IECore::StringParameterPtr m_channelNameParameter;
		IECore::IntParameterPtr m_subdivisionDepthParameter;
		IECore::IntParameterPtr m_projectionParameter;

};

IE_CORE_DECLAREPTR( MedianCutSampler );

} // namespace IECoreImage

#endif // IECOREIMAGE_MEDIANCUTSAMPLER_H
