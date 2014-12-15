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

#ifndef IECORE_ENVMAPSAMPLER_H
#define IECORE_ENVMAPSAMPLER_H

#include "IECore/Export.h"
#include "IECore/Op.h"
#include "IECore/TypedPrimitiveParameter.h"
#include "IECore/NumericParameter.h"

namespace IECore
{

/// This class uses the MedianCutSampler to calculate a distribution
/// of light positions and orientations based on a lat-long environment
/// map image.
/// \todo: use SphericalToEuclideanTransform that is based on right-hand coordinate system. Currently it is left-hand to match 3delight environment light mapping. But maya and nuke and the spherical harmonics implementation in IECore are right-handed.
/// \ingroup renderingGroup
/// \ingroup imageProcessingGroup
class IECORE_API EnvMapSampler : public Op
{
	public :

		IE_CORE_DECLARERUNTIMETYPED( EnvMapSampler, Op );

		EnvMapSampler();
		virtual ~EnvMapSampler();

		ImagePrimitiveParameter * imageParameter();
		const ImagePrimitiveParameter * imageParameter() const;

		IntParameter * subdivisionDepthParameter();
		const IntParameter * subdivisionDepthParameter() const;

	protected :

		ObjectPtr doOperation( const CompoundObject * operands );

	private :

		ImagePrimitiveParameterPtr m_imageParameter;
		IntParameterPtr m_subdivisionDepthParameter;

};

IE_CORE_DECLAREPTR( EnvMapSampler );

} // namespace IECore

#endif // IECORE_ENVMAPSAMPLER_H
