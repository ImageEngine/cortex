//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_MARSCHNERPARAMETER_H
#define IECORE_MARSCHNERPARAMETER_H

#include "IECore/Export.h"
#include "IECore/CompoundParameter.h"
#include "IECore/NumericParameter.h"
#include "IECore/SimpleTypedParameter.h"

#include "IECore/Marschner.h"

namespace IECore
{

/// The MarschnerParameter provides a convenience class to handle the various
/// parameters needed to evaluate the Marschner hair shading model implemented 
/// in the Marschner.h. This parameter is specialised to T = Color3f. It provides
/// the option to present the Absorption parameter of the model as a color.
/// Defaults are derived from the relevant papers.
class IECORE_API MarschnerParameter : public CompoundParameter
{

	public :
		
		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( MarschnerParameter, MarschnerParameterTypeId, IECore::CompoundParameter )

		/// \param absorptionAsColor When true, a color parameter will be presented instead of
		/// absorption. \see MarschnerParameter::createBCSDF()
		MarschnerParameter(
			const std::string &name = "", const std::string &description = "",
			bool absorptionAsColor = true,
			ConstCompoundObjectPtr userData=0
		);
		
		~MarschnerParameter();
		
		/// \return a MarschnerBCSDFC3f initialized with the Parameters current values.
		/// If the parameter was created with the the absorptionAsColor parameter set to true,
		/// the color parameter is converted to an absorption coefficient using the conversion:
		///    absorption = -log(color)/4
		MarschnerBCSDFC3f createBCSDF();
		
	private :
		
		bool m_absorptionAsColor;
		
		IECore::FloatParameterPtr m_refraction;
		IECore::Color3fParameterPtr m_absorption;
		IECore::FloatParameterPtr m_eccentricity;
		IECore::FloatParameterPtr m_shiftR;
		IECore::FloatParameterPtr m_shiftTT;
		IECore::FloatParameterPtr m_shiftTRT;
		IECore::FloatParameterPtr m_widthR;
		IECore::FloatParameterPtr m_widthTT;
		IECore::FloatParameterPtr m_widthTRT;
		IECore::FloatParameterPtr m_glint;
		IECore::FloatParameterPtr m_causticWidth;
		IECore::FloatParameterPtr m_causticFade;
		IECore::FloatParameterPtr m_causticLimit;

};

IE_CORE_DECLAREPTR( MarschnerParameter )

} // namespace IECore

#endif // IECORE_MARSCHNERPARAMETER_H
