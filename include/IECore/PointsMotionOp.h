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

#ifndef IE_CORE_POINTSMOTIONOP_H
#define IE_CORE_POINTSMOTIONOP_H

#if BOOST_VERSION >= 103600
#include "boost/unordered_map.hpp"
#else
#include <map>
#endif

#include "IECore/Export.h"
#include "IECore/Op.h"
#include "IECore/SimpleTypedParameter.h"
#include "IECore/TypedObjectParameter.h"
#include "IECore/VectorTypedParameter.h"


namespace IECore
{

IE_CORE_FORWARDDECLARE( ObjectParameter )

/// The PointsMotionOp creates a MotionPrimitive object from a list of PointsPrimitive objects.
/// If a point does not exist on any given snapshot then it's non-masked primvars are copied from the closest available snapshot.
/// Masked primvars are set to zero.
/// \ingroup geometryProcessingGroup
/// \ingroup renderingGroup
class IECORE_API PointsMotionOp : public Op
{
	public :

		IE_CORE_DECLARERUNTIMETYPED( PointsMotionOp, Op );

		PointsMotionOp();
		~PointsMotionOp() override;

		FloatVectorParameter * snapshotTimesParameter();
		const FloatVectorParameter * snapshotTimesParameter() const;

		ObjectVectorParameter * pointsPrimitiveVectorParameter();
		const ObjectVectorParameter * pointsPrimitiveVectorParameter() const;

		StringParameter * idPrimVarNameParameter();
		const StringParameter * idPrimVarNameParameter() const;

		StringVectorParameter * maskedPrimVarsParameter();
		const StringVectorParameter * maskedPrimVarsParameter() const;

	protected :

		ObjectPtr doOperation( const CompoundObject * operands ) override;

	private :

		struct IdInfo;
#if BOOST_VERSION >= 103600
		typedef boost::unordered_map< unsigned, IdInfo > IdMap;
#else
		typedef std::map< unsigned, IdInfo > IdMap;
#endif
		struct PrimVarBuilder;

		FloatVectorParameterPtr m_snapshotTimesParameter;
		ObjectVectorParameterPtr m_pointsPrimitiveVectorParameter;
		StringParameterPtr m_idPrimVarNameParameter;
		StringVectorParameterPtr m_maskedPrimVarsParameter;

};

IE_CORE_DECLAREPTR( PointsMotionOp );

} // namespace IECore

#endif // IE_CORE_POINTSMOTIONOP_H
