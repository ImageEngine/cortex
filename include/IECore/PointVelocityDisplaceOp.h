//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
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

#ifndef IE_CORE_POINTVELOCITYDISPLACEOP_H
#define IE_CORE_POINTVELOCITYDISPLACEOP_H

#include "IECore/Export.h"
#include "IECore/ModifyOp.h"
#include "IECore/NumericParameter.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( ObjectParameter )

/// The PointVelocityDisplaceOp displaces points by their velocity (v).
/// the input Primitive should have two V3fVectorData primitive variables
/// specified by the positionVar and velocityVar parameters (default to 'P'
/// and 'v' respectively). These variables must have the same number of
/// entries.
///
/// A uniform velocity scale can be applied using the sampleLength parameter.
/// In addition this scale can be modulated on a per-point basis be specifying
/// an addition variable via the sampleLengthVar parameter (defaults to an empty
/// string).
///
/// Pnew = P + ( v * sampleLength )
/// \ingroup geometryProcessingGroup
class IECORE_API PointVelocityDisplaceOp : public ModifyOp
{
	public :
		IE_CORE_DECLARERUNTIMETYPED( PointVelocityDisplaceOp, ModifyOp );

		PointVelocityDisplaceOp();
		virtual ~PointVelocityDisplaceOp();

		StringParameter * positionVarParameter();
		const StringParameter * positionVarParameter() const;
		StringParameter * velocityVarParameter();
		const StringParameter * velocityVarParameter() const;
		FloatParameter * sampleLengthParameter();
		const FloatParameter * sampleLengthParameter() const;
		StringParameter * sampleLengthVarParameter();
		const StringParameter * sampleLengthVarParameter() const;

	protected :
		virtual void modify( Object *object, const CompoundObject * operands );

	private :
		StringParameterPtr m_positionVarParameter;
		StringParameterPtr m_velocityVarParameter;
		FloatParameterPtr m_sampleLengthParameter;
		StringParameterPtr m_sampleLengthVarParameter;
};

IE_CORE_DECLAREPTR( PointVelocityDisplaceOp );

} // namespace IECore

#endif // IE_CORE_POINTVELOCITYDISPLACEOP_H
