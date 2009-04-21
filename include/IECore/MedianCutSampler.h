//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_MEDIANCUTSAMPLER_H
#define IECORE_MEDIANCUTSAMPLER_H

#include "IECore/Op.h"
#include "IECore/TypedObjectParameter.h"
#include "IECore/NumericParameter.h"
#include "IECore/SimpleTypedParameter.h"

namespace IECore
{

/// An implementation of the median cut algorithm described in
/// A Median Cut Algorithm for Light Probe Sampling by Paul Debevec.
/// This class just worries about the point sampling and centroid
/// calculations, and leaves it to a caller to interpret them as
/// light directions and positions. This allows the use of the class
/// as a simple 2d point distribution algorithm in addition to
/// a light probe sampler.
class MedianCutSampler : public Op
{
	public :
		
		IE_CORE_DECLARERUNTIMETYPED( MedianCutSampler, Op );
		
		MedianCutSampler();
		virtual ~MedianCutSampler();
		
		ImagePrimitiveParameterPtr imageParameter();
		ConstImagePrimitiveParameterPtr imageParameter() const;
		
		StringParameterPtr channelNameParameter();
		ConstStringParameterPtr channelNameParameter() const;
		
		IntParameterPtr subdivisionDepthParameter();
		ConstIntParameterPtr subdivisionDepthParameter() const;

		enum Projection
		{
			Invalid,
			Rectilinear,
			LatLong
		};

		IntParameterPtr projectionParameter();
		ConstIntParameterPtr projectionParameter() const;

	protected :

		ObjectPtr doOperation( ConstCompoundObjectPtr operands );
		
	private :
		
		ImagePrimitiveParameterPtr m_imageParameter;
		StringParameterPtr m_channelNameParameter;
		IntParameterPtr m_subdivisionDepthParameter;
		IntParameterPtr m_projectionParameter;
		
};

IE_CORE_DECLAREPTR( MedianCutSampler );

} // namespace IECore

#endif // IECORE_MEDIANCUTSAMPLER_H
