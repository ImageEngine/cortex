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

#ifndef IECOREMAYA_FROMMAYATRANSFORMCONVERTER_H
#define IECOREMAYA_FROMMAYATRANSFORMCONVERTER_H

#include "maya/MEulerRotation.h"

#include "IECoreMaya/FromMayaDagNodeConverter.h"

#include "IECore/NumericParameter.h"
#include "IECore/TypedParameter.h"

namespace IECoreMaya
{

/// The FromMayaTransformationMatrixConverter extracts transforms from DAG nodes.
class FromMayaTransformConverter : public FromMayaDagNodeConverter
{

	public :

		FromMayaTransformConverter( const MDagPath &dagPath );

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( FromMayaTransformConverter, FromMayaTransformConverterTypeId, FromMayaDagNodeConverter );

		enum Space
		{
			Local = 0,
			World = 1
		};

		IECore::IntParameterPtr spaceParameter();
		IECore::ConstIntParameterPtr spaceParameter() const;

		IECore::BoolParameterPtr eulerFilterParameter();
		IECore::ConstBoolParameterPtr eulerFilterParameter() const;

		IECore::BoolParameterPtr zeroPivotsParameter();
		IECore::ConstBoolParameterPtr zeroPivotsParameter() const;

	protected :

		virtual IECore::ObjectPtr doConversion( const MDagPath &dagPath, IECore::ConstCompoundObjectPtr operands ) const;

	private :

		IECore::IntParameterPtr m_spaceParameter;
		mutable MEulerRotation m_lastRotation;
		mutable bool m_lastRotationValid;
		IECore::BoolParameterPtr m_eulerFilterParameter;
		IECore::BoolParameterPtr m_zeroPivotsParameter;

		static Description<FromMayaTransformConverter> g_description;

};

IE_CORE_DECLAREPTR( FromMayaTransformConverter );

} // namespace IECoreMaya

#endif // IECOREMAYA_FROMMAYATRANSFORMCONVERTER_H
