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

#ifndef IECORETRUELIGHT_TRUELIGHTCOLORTRANSFORMOP_H
#define IECORETRUELIGHT_TRUELIGHTCOLORTRANSFORMOP_H

#include "IECoreTruelight/TypeIds.h"

#include "IECore/ColorTransformOp.h"
#include "IECore/NumericParameter.h"
#include "IECore/SRGBToLinearDataConversion.h"

namespace IECoreTruelight
{

/// This class applies a color transform using the truelight library.
class TruelightColorTransformOp : public IECore::ColorTransformOp
{

	public :

		TruelightColorTransformOp();
		virtual ~TruelightColorTransformOp();

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreTruelight::TruelightColorTransformOp, TruelightColorTransformOpTypeId, IECore::ColorTransformOp );

		IECore::StringParameterPtr profileParameter();
		IECore::ConstStringParameterPtr profileParameter() const;

		IECore::StringParameterPtr displayParameter();
		IECore::ConstStringParameterPtr displayParameter() const;
		
		IECore::IntParameterPtr inputSpaceParameter();
		IECore::ConstIntParameterPtr inputSpaceParameter() const;
		
		IECore::BoolParameterPtr rawTruelightOutputParameter();
		IECore::ConstBoolParameterPtr rawTruelightOutputParameter() const;

		/// Returns the actual truelight commands used for the transform.
		/// This can be of use when debugging.
		std::string commands() const;

	protected :
	
		virtual void begin( IECore::ConstCompoundObjectPtr operands );
		virtual void transform( Imath::Color3f &color ) const;

	private :
	
		void maybeWarn() const;

		void setInstanceFromParameters() const;

		IECore::StringParameterPtr m_profileParameter;
		IECore::StringParameterPtr m_displayParameter;
		IECore::IntParameterPtr m_inputSpaceParameter;
		IECore::BoolParameterPtr m_rawTruelightOutputParameter;
		IECore::SRGBToLinearDataConversion<float, float> m_srgbToLinearConversion;

		void *m_instance; // truelight instance
		
};

IE_CORE_DECLAREPTR( TruelightColorTransformOp )

} // namespace IECoreTruelight

#endif // IECORETRUELIGHT_TRUELIGHTCOLORTRANSFORMOP_H
