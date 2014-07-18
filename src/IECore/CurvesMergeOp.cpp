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

#include "IECore/CurvesMergeOp.h"
#include "IECore/CompoundParameter.h"
#include "IECore/NullObject.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/TypeTraits.h"

#include <algorithm>

using namespace IECore;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( CurvesMergeOp );

CurvesMergeOp::CurvesMergeOp()
	:	CurvesPrimitiveOp( "Merges one set of curves with another." )
{
	m_curvesParameter = new CurvesPrimitiveParameter(
		"curves",
		"The curves to be merged with the input.",
		new CurvesPrimitive
	);

	parameters()->addParameter( m_curvesParameter );
}

CurvesMergeOp::~CurvesMergeOp()
{
}

CurvesPrimitiveParameter * CurvesMergeOp::curvesParameter()
{
	return m_curvesParameter.get();
}

const CurvesPrimitiveParameter * CurvesMergeOp::curvesParameter() const
{
	return m_curvesParameter.get();
}

struct CurvesMergeOp::AppendPrimVars
{
	typedef void ReturnType;

	AppendPrimVars( const CurvesPrimitive * curves2, const std::string &name )
		:	m_curves2( curves2 ), m_name( name )
	{
	}

	template<typename T>
	ReturnType operator()( T * data )
	{

		PrimitiveVariableMap::const_iterator it = m_curves2->variables.find( m_name );
		if( it!=m_curves2->variables.end() )
		{
			const T *data2 = runTimeCast<const T>( it->second.data.get() );
			if( data2 )
			{
				data->writable().insert( data->writable().end(), data2->readable().begin(), data2->readable().end() );
			}
		}
	}

	private :

		const CurvesPrimitive * m_curves2;
		std::string m_name;

};

void CurvesMergeOp::modifyTypedPrimitive( CurvesPrimitive * curves, const CompoundObject * operands )
{
	const CurvesPrimitive *curves2 = static_cast<const CurvesPrimitive *>( m_curvesParameter->getValue() );

	const vector<int> &verticesPerCurve1 = curves->verticesPerCurve()->readable();
	const vector<int> &verticesPerCurve2 = curves2->verticesPerCurve()->readable();
	
	IntVectorDataPtr verticesPerCurveData = new IntVectorData;
	vector<int> &verticesPerCurve = verticesPerCurveData->writable();
	verticesPerCurve.resize( verticesPerCurve1.size() + verticesPerCurve2.size() );
	
	vector<int>::iterator it = copy( verticesPerCurve1.begin(), verticesPerCurve1.end(), verticesPerCurve.begin() );
	copy( verticesPerCurve2.begin(), verticesPerCurve2.end(), it );

	curves->setTopology( verticesPerCurveData, curves->basis(), curves->periodic() );

	PrimitiveVariableMap::iterator pvIt;
	for( pvIt=curves->variables.begin(); pvIt!=curves->variables.end(); pvIt++ )
	{
		if( pvIt->second.interpolation!=PrimitiveVariable::Constant )
		{
			AppendPrimVars f( curves2, pvIt->first );
			despatchTypedData<AppendPrimVars, TypeTraits::IsVectorTypedData, DespatchTypedDataIgnoreError>( pvIt->second.data.get(), f );
		}
	}
}
