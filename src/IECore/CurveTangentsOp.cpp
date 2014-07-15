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

#include <cassert>
#include <algorithm>

#include "boost/format.hpp"

#include "IECore/DataCastOp.h"
#include "IECore/Convert.h"
#include "IECore/CurveTangentsOp.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/CompoundParameter.h"
#include "IECore/CurvesPrimitiveEvaluator.h"

using namespace IECore;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( CurveTangentsOp );

CurveTangentsOp::CurveTangentsOp() : CurvesPrimitiveOp( "Calculates mesh tangents with respect to texture coordinates." )
{
	m_vTangentPrimVarNameParameter = new StringParameter(
		"vTangentPrimVarName",
		"vTangentPrimVarName description",
		"vTangent"
	);
	parameters()->addParameter( m_vTangentPrimVarNameParameter );
}

CurveTangentsOp::~CurveTangentsOp()
{
}

StringParameter * CurveTangentsOp::vTangentPrimVarNameParameter()
{
	return m_vTangentPrimVarNameParameter.get();
}

const StringParameter * CurveTangentsOp::vTangentPrimVarNameParameter() const
{
	return m_vTangentPrimVarNameParameter.get();
}

struct CurveTangentsOp::CalculateTangents
{
	typedef void ReturnType;

	CalculateTangents( const vector<int> &vertsPerCurve, CurvesPrimitiveEvaluatorPtr evaluator )
		:	m_vertsPerCurve( vertsPerCurve ), m_evaluator( evaluator )
	{

	}

	template<typename T>
	ReturnType operator()( T * data )
	{
		typedef typename T::ValueType VecContainer;

		const VecContainer &points = data->readable();
		
		unsigned numElements = points.size();
		
		typename T::Ptr vD = new T();
		vTangentsData = vD;
		
		VecContainer &vTangents = vD->writable();
		vTangents.resize( numElements );
		
		PrimitiveEvaluator::ResultPtr result = m_evaluator->createResult();
		
		unsigned pIndex = 0;
		for( size_t curveIndex = 0; curveIndex < m_vertsPerCurve.size() ; curveIndex++ )
		{
			float v;
			float vStep = 1.0f / m_vertsPerCurve[curveIndex];
			
			for( int i = 0; i < m_vertsPerCurve[curveIndex]; i++ ) 
			{
				v = min( 1.0f, i * vStep );			
				m_evaluator->pointAtV( curveIndex, v, result.get() );
				vTangents[ pIndex + i ] = result->vTangent().normalized();
			}
			pIndex += m_vertsPerCurve[curveIndex];
		}	
	}
	
	// this is the data filled in by operator() above, ready to be added onto the primitive
	DataPtr vTangentsData;
	
	private :

		const vector<int> &m_vertsPerCurve;
		CurvesPrimitiveEvaluatorPtr m_evaluator;
		
};

struct CurveTangentsOp::HandleErrors
{
	template<typename T, typename F>
	void operator()( const T *d, const F &f )
	{
		string e = boost::str( boost::format( "CurveTangentsOp : P primitive variable has unsupported data type \"%s\"." ) % d->typeName() );
		throw InvalidArgumentException( e );
	}
};

void CurveTangentsOp::modifyTypedPrimitive( CurvesPrimitive *curves, const CompoundObject *operands )
{
	if( !curves->arePrimitiveVariablesValid( ) )
	{
		throw InvalidArgumentException( "CurveTangentsOp : CurvesPrimitive variables are invalid." );
	}
	
	// The CurvesPrimitiveEvaluator currently only supports "P".
	Data * pData = curves->variableData<Data>( "P", PrimitiveVariable::Vertex );
	if( !pData )
	{
		throw InvalidArgumentException( "CurveTangentsOp : CurvesPrimitive has no Vertex \"P\" primitive variable." );
	}

	const IntVectorData *vertsPerCurve = curves->verticesPerCurve();
		
	DataCastOpPtr dco = new DataCastOp();
	dco->targetTypeParameter()->setNumericValue( FloatVectorDataTypeId );

	CurvesPrimitiveEvaluatorPtr evaluator = new CurvesPrimitiveEvaluator( curves );

	CalculateTangents f( vertsPerCurve->readable(), evaluator );

	despatchTypedData<CalculateTangents, TypeTraits::IsVec3VectorTypedData, HandleErrors>( pData, f );

	curves->variables[ vTangentPrimVarNameParameter()->getTypedValue() ] = PrimitiveVariable( PrimitiveVariable::Vertex, f.vTangentsData );

	assert( curves->arePrimitiveVariablesValid() );
}


