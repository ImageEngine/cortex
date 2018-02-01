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

#ifndef IECORESCENE_POINTSMOTIONOP_H
#define IECORESCENE_POINTSMOTIONOP_H

#include "IECoreScene/Export.h"
#include "IECoreScene/TypeIds.h"

#include "IECore/Op.h"
#include "IECore/SimpleTypedParameter.h"
#include "IECore/TypedObjectParameter.h"
#include "IECore/VectorTypedParameter.h"

#if BOOST_VERSION >= 103600
#include "boost/unordered_map.hpp"
#else
#include <map>
#endif

namespace IECoreScene
{

/// The PointsMotionOp creates a MotionPrimitive object from a list of PointsPrimitive objects.
/// If a point does not exist on any given snapshot then it's non-masked primvars are copied from the closest available snapshot.
/// Masked primvars are set to zero.
/// \ingroup geometryProcessingGroup
/// \ingroup renderingGroup
class IECORESCENE_API PointsMotionOp : public IECore::Op
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( PointsMotionOp, PointsMotionOpTypeId, IECore::Op );

		PointsMotionOp();
		~PointsMotionOp() override;

		IECore::FloatVectorParameter * snapshotTimesParameter();
		const IECore::FloatVectorParameter * snapshotTimesParameter() const;

		IECore::ObjectVectorParameter * pointsPrimitiveVectorParameter();
		const IECore::ObjectVectorParameter * pointsPrimitiveVectorParameter() const;

		IECore::StringParameter * idPrimVarNameParameter();
		const IECore::StringParameter * idPrimVarNameParameter() const;

		IECore::StringVectorParameter * maskedPrimVarsParameter();
		const IECore::StringVectorParameter * maskedPrimVarsParameter() const;

	protected :

		IECore::ObjectPtr doOperation( const IECore::CompoundObject * operands ) override;

	private :

		struct IdInfo;
#if BOOST_VERSION >= 103600
		typedef boost::unordered_map< unsigned, IdInfo > IdMap;
#else
		typedef std::map< unsigned, IdInfo > IdMap;
#endif
		struct PrimVarBuilder;

		IECore::FloatVectorParameterPtr m_snapshotTimesParameter;
		IECore::ObjectVectorParameterPtr m_pointsPrimitiveVectorParameter;
		IECore::StringParameterPtr m_idPrimVarNameParameter;
		IECore::StringVectorParameterPtr m_maskedPrimVarsParameter;

};

IE_CORE_DECLAREPTR( PointsMotionOp );

} // namespace IECoreScene

#endif // IECORESCENE_POINTSMOTIONOP_H
