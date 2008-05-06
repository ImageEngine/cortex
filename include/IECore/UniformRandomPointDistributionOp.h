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

#ifndef IE_CORE_UUNIFORMRANDOMPOINTDISTRIBUTIONOP_H
#define IE_CORE_UUNIFORMRANDOMPOINTDISTRIBUTIONOP_H

#include "IECore/Op.h"
#include "IECore/NumericParameter.h"
#include "IECore/TypedParameter.h"
#include "IECore/TypedObjectParameter.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( ObjectParameter )

/// The UniformRandomPointDistributionOp distributes points over a mesh using a random distribution. Evenness is
/// approximated by weighting the amount of expected particles per mesh face to be proportional to that face's area.
class UniformRandomPointDistributionOp : public Op
{
	public :
		
		IE_CORE_DECLARERUNTIMETYPED( UniformRandomPointDistributionOp, Op );
		
		UniformRandomPointDistributionOp();
		virtual ~UniformRandomPointDistributionOp();
		
		MeshPrimitiveParameterPtr meshParameter();
		ConstMeshPrimitiveParameterPtr meshParameter() const;
		
		IntParameterPtr numPointsParameter();
		ConstIntParameterPtr numPointsParameter() const;	
		
		IntParameterPtr seedParameter();
		ConstIntParameterPtr seedParameter() const;
		
		BoolParameterPtr addSTParameter();
		ConstBoolParameterPtr addSTParameter() const;				
			
		
	protected :
	
		struct DistributeFn;

		virtual ObjectPtr doOperation( ConstCompoundObjectPtr operands );
	
	private :
		
		MeshPrimitiveParameterPtr m_meshParameter;
		IntParameterPtr m_numPointsParameter;
		IntParameterPtr m_seedParameter;
		BoolParameterPtr m_addSTParameter;

};

IE_CORE_DECLAREPTR( UniformRandomPointDistributionOp );

} // namespace IECore

#endif // IE_CORE_UUNIFORMRANDOMPOINTDISTRIBUTIONOP_H
