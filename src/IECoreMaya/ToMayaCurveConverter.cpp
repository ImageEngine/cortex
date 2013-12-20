//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

#include "boost/format.hpp"

#include "maya/MFnNurbsCurve.h"
#include "maya/MPointArray.h"
#include "maya/MDoubleArray.h"

#include "IECore/CurvesPrimitive.h"
#include "IECore/PrimitiveVariable.h"
#include "IECore/MessageHandler.h"
#include "IECore/CompoundParameter.h"

#include "IECoreMaya/Convert.h"
#include "IECoreMaya/ToMayaCurveConverter.h"

using namespace IECoreMaya;

ToMayaCurveConverter::Description ToMayaCurveConverter::g_curvesDataDescription( IECore::CurvesPrimitive::staticTypeId(), MFn::kNurbsCurveData );
ToMayaCurveConverter::Description ToMayaCurveConverter::g_curvesDescription( IECore::CurvesPrimitive::staticTypeId(), MFn::kNurbsCurve );

ToMayaCurveConverter::ToMayaCurveConverter( IECore::ConstObjectPtr object )
: ToMayaObjectConverter( "Converts IECore::CurvesPrimitive objects to a Maya object.", object)
{
	m_indexParameter = new IECore::IntParameter( "index", "The index of the curve to be converted.", 0 );
	parameters()->addParameter( m_indexParameter );
}


IECore::IntParameterPtr ToMayaCurveConverter::indexParameter()
{
	return m_indexParameter;
}

IECore::ConstIntParameterPtr ToMayaCurveConverter::indexParameter() const
{
	return m_indexParameter;
}

bool ToMayaCurveConverter::doConversion( IECore::ConstObjectPtr from, MObject &to, IECore::ConstCompoundObjectPtr operands ) const
{
	MStatus s;

	IECore::ConstCurvesPrimitivePtr curves = IECore::runTimeCast<const IECore::CurvesPrimitive>( from );
	
	assert( curves );

	if ( !curves->arePrimitiveVariablesValid() || !curves->numCurves() )
	{
		return false;
	}
	
	int curveIndex = indexParameter()->getNumericValue();
	if( curveIndex < 0 || curveIndex >= (int)curves->numCurves() )
	{
		IECore::msg( IECore::Msg::Warning,"ToMayaCurveConverter::doConversion",  boost::format(  "Invalid curve index \"%d\"") % curveIndex );
		return false;
	}
	
	IECore::ConstV3fVectorDataPtr p = curves->variableData< IECore::V3fVectorData >( "P", IECore::PrimitiveVariable::Vertex );
	if( !p )
	{
		IECore::msg( IECore::Msg::Warning,"ToMayaCurveConverter::doConversion",  "Curve has no \"P\" data" );
		return false;
		
	}
	
	const std::vector<int>& verticesPerCurve = curves->verticesPerCurve()->readable();
	int curveBase = 0;
	for( int i=0; i<curveIndex; ++i )
	{
		curveBase += verticesPerCurve[i];
	}
	
	MPointArray vertexArray;
	
	int numVertices = verticesPerCurve[curveIndex];
	
	const std::vector<Imath::V3f>& pts = p->readable();
	
	// triple up the start points for cubic periodic curves:
	if( curves->periodic() && curves->basis() != IECore::CubicBasisf::linear() )
	{
		vertexArray.append( IECore::convert<MPoint, Imath::V3f>( pts[curveBase] ) );
		vertexArray.append( vertexArray[0] );
	}
	
	for( int i = 0; i < numVertices; ++i )
	{
		vertexArray.append( IECore::convert<MPoint, Imath::V3f>( pts[i + curveBase] ) );
	}
	
	// if the curve is periodic, the first N cvs must be identical to the last N cvs, where N is the degree
	// of the curve:
	if( curves->periodic() )
	{
		if( curves->basis() == IECore::CubicBasisf::linear() )
		{
			// linear: N = 1
			vertexArray.append( vertexArray[0] );
		}
		else
		{
			// cubic: N = 3
			vertexArray.append( vertexArray[0] );
			vertexArray.append( vertexArray[1] );
			vertexArray.append( vertexArray[2] );
		}
	}
	
	MDoubleArray knotSequences;
	if( curves->basis() == IECore::CubicBasisf::linear() )
	{
		for( unsigned i=0; i < vertexArray.length(); ++i )
		{
			knotSequences.append( i );
		}
	}
	else
	{
		// first two cvs must have coincident knots if the curve's open, otherwise
		// they must be spaced out
		knotSequences.append( curves->periodic() ? -1 : 0 );
		for( unsigned i=0; i < vertexArray.length(); ++i )
		{
			knotSequences.append( i );
		}
		// same with the last two:
		knotSequences.append( curves->periodic() ? vertexArray.length() : vertexArray.length() - 1 );
	}
	
	MFnNurbsCurve fnCurve;
	fnCurve.create( vertexArray, knotSequences, curves->basis() == IECore::CubicBasisf::linear() ? 1 : 3, curves->periodic() ? MFnNurbsCurve::kPeriodic : MFnNurbsCurve::kOpen, false, false, to, &s );
	if (!s)
	{
		IECore::msg( IECore::Msg::Warning,"ToMayaCurveConverter::doConversion",  s.errorString().asChar() );
		return false;
	}
	
	return true;
}
