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

#ifndef IECORE_MARSCHNERLOOKUPTABLEOP_H
#define IECORE_MARSCHNERLOOKUPTABLEOP_H

#include "IECore/TypeIds.h"

#include "IECore/Export.h"
#include "IECore/Op.h"
#include "IECore/MarschnerParameter.h"
#include "IECore/NumericParameter.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( ObjectParameter );

/// This Op allows lookup tables to be generated to cover a range of light
/// and eye angles incident to the Marschner et al. reflectance model. These
/// lookups may be used for GPU acceleration, and other such things.
/// Based on the ideas presented in GPUGems Chapter 23.
///
///   http://http.developer.nvidia.com/GPUGems2/gpugems2_chapter23.html
///
/// The resulting image contains the following channels:
///
///   with:
///   
///     s = sin( light.y ) from -1.0 to 1.0
///     t = sin( eye.y ) from -1.0 to 1.0
/// 
///     : MR            = MR(eye,light) / 30.0
///     : MTT           = MTT(eye,light) / 30.0
///     : MTRT          = MTRT(eye,light) / 30.0
///     : cosDiffTheta  = ( cos( (light.y-eye.y)/2.0 ) + 1.0 ) / 2.0
///
///   with:
///     
///     s = cosDiffTheta -1.0 to 1.0
///     t = cos( light.x - eye.x ) from -1.0 to 1.0 
///
///     : NR            = NR( eye, light ).x
///     : NTT.[rgb]     = NR( eye, light )
///     : NTRT.[rgb]    = NR( eye, light )
///
/// NOTE: This this is keyed with the -1.0 values of s/t at 0. In an OpenGL
/// implementation you may need to use 1-t as your lookup, rather than t.
/// \ingroup renderingGroup
class IECORE_API MarschnerLookupTableOp : public Op
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( MarschnerLookupTableOp, MarschnerLookupTableOpTypeId, Op );

		MarschnerLookupTableOp();
		/// \param absorptionAsColor Switches the behaviour of the MarschnerParameter
		/// \see MarschnerParameter
		MarschnerLookupTableOp( const bool absorptionAsColor );
		
		virtual ~MarschnerLookupTableOp();

		/// The parameters for Marschner model to be evaluated.
		MarschnerParameter *modelParameter();
		const MarschnerParameter *modelParameter() const;

		/// The resolution of the resulting lookup images.
		IntParameter *resolutionParameter();
		const IntParameter *resolutionParameter() const;

	protected :

		virtual ObjectPtr doOperation( const CompoundObject * operands );

	private :
	
		void createParameters( const bool asColor );

		MarschnerParameterPtr m_modelParameter;
		IntParameterPtr m_resolutionParameter;

};

IE_CORE_DECLAREPTR( MarschnerLookupTableOp );

} // namespace IECore

#endif // IECORE_MARSCHNERLOOKUPTABLEOP_H
