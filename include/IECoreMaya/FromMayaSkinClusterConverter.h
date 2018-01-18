//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREMAYA_FROMMAYASKINCLUSTERCONVERTER_H
#define IECOREMAYA_FROMMAYASKINCLUSTERCONVERTER_H

#include "IECoreMaya/FromMayaObjectConverter.h"

#include "IECore/CompoundParameter.h"
#include "IECoreMaya/FromMayaDagNodeConverter.h"
#include "IECore/NumericParameter.h"
#include "IECore/TypedParameter.h"


namespace IECoreMaya
{

IE_CORE_FORWARDDECLARE( FromMayaSkinClusterConverter );

/// The FromMayaSkinClusterConverter converts the smooth bind data on a maya skinCluster node
/// into IECore::SmoothSkinningData.
/// \ingroup conversionGroup
class IECOREMAYA_API FromMayaSkinClusterConverter : public FromMayaObjectConverter
{

	public :

		FromMayaSkinClusterConverter( const MObject &object );

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( FromMayaSkinClusterConverter, FromMayaSkinClusterConverterTypeId, FromMayaObjectConverter );

		enum InfluenceName
		{
			Partial = 0,
			Full = 1
		};

		IECore::IntParameterPtr influenceNameParameter();
		IECore::ConstIntParameterPtr influenceNameParameter() const;


	protected :

		virtual IECore::ObjectPtr doConversion( const MObject &object, IECore::ConstCompoundObjectPtr operands ) const;

	private :

		IECore::IntParameterPtr m_influenceNameParameter;

		static FromMayaObjectConverterDescription<FromMayaSkinClusterConverter> m_description;

};

IE_CORE_DECLAREPTR( FromMayaSkinClusterConverter );

} // namespace IECoreMaya

#endif // IECOREMAYA_FROMMAYASKINCLUSTERCONVERTER_H
