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

#ifndef IE_CORE_MAPPEDRANDOMPOINTDISTRIBUTIONOP_H
#define IE_CORE_MAPPEDRANDOMPOINTDISTRIBUTIONOP_H

#include "IECore/UniformRandomPointDistributionOp.h"
#include "IECore/SimpleTypedParameter.h"
#include "IECore/ImagePrimitiveEvaluator.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( ImagePrimitive );

/// The MappedRandomPointDistributionOp distributes points over a mesh using a random distribution. Evenness is
/// approximated by weighting the amount of expected particles per mesh face to be proportional to that face's area.
class MappedRandomPointDistributionOp : public UniformRandomPointDistributionOp
{
	public :

		IE_CORE_DECLARERUNTIMETYPED( MappedRandomPointDistributionOp, UniformRandomPointDistributionOp );

		MappedRandomPointDistributionOp();
		virtual ~MappedRandomPointDistributionOp();

		ImagePrimitiveParameterPtr imageParameter();
		ConstImagePrimitiveParameterPtr imageParameter() const;

		StringParameterPtr channelNameParameter();
		ConstStringParameterPtr channelNameParameter() const;

	protected :

		/// Derived classes can override this method and return a number in the range [0,1] defining the
		/// required density at the given point.
		virtual float density( ConstMeshPrimitivePtr mesh, const Imath::V3f &point, const Imath::V2f &uv ) const;

	private :

		virtual ObjectPtr doOperation( ConstCompoundObjectPtr operands );

		ImagePrimitiveParameterPtr m_imageParameter;
		StringParameterPtr m_channelNameParameter;

		ImagePrimitiveEvaluatorPtr m_imageEvaluator;
		PrimitiveVariableMap::iterator m_channelIterator;
		PrimitiveEvaluator::ResultPtr m_result;

};

IE_CORE_DECLAREPTR( MappedRandomPointDistributionOp );

} // namespace IECore

#endif // IE_CORE_MAPPEDRANDOMPOINTDISTRIBUTIONOP_H
