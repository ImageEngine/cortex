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

#include "IECoreMaya/FromMayaCurveConverter.h"
#include "IECoreMaya/MArrayIter.h"
#include "IECoreMaya/VectorTraits.h"
#include "IECoreMaya/Convert.h"

#include "IECore/CurvesPrimitive.h"
#include "IECore/VectorOps.h"
#include "IECore/Exception.h"
#include "IECore/CompoundParameter.h"

#include "maya/MFnNurbsCurve.h"

#include <algorithm>

using namespace IECoreMaya;
using namespace Imath;

static const MFn::Type fromTypes[] = { MFn::kNurbsCurve, MFn::kNurbsCurveData, MFn::kInvalid };
static const IECore::TypeId toTypes[] = { IECore::BlindDataHolderTypeId, IECore::RenderableTypeId, IECore::VisibleRenderableTypeId, IECore::PrimitiveTypeId, IECore::CurvesPrimitiveTypeId, IECore::InvalidTypeId };

FromMayaObjectConverter::FromMayaObjectConverterDescription<FromMayaCurveConverter> FromMayaCurveConverter::m_description( fromTypes, toTypes );

FromMayaCurveConverter::FromMayaCurveConverter( const MObject &object )
	:	FromMayaShapeConverter( staticTypeName(), "Converts maya curve shapes into IECore::CurvesPrimitive objects.", object )
{
	m_linearParameter = new IECore::BoolParameter(
		"linearBasis",
		"When this parameter is set to true, all curves are converted with a linear basis."
		"When it is false (the default) then cubic curves will be converted with a bSpline"
		"basis. In both cases, curves which are not cubic are converted with a linear basis.",
		false
	);
	parameters()->addParameter( m_linearParameter );
}

IECore::PrimitivePtr FromMayaCurveConverter::doPrimitiveConversion( const MObject &object, IECore::ConstCompoundObjectPtr operands ) const
{
	MFnNurbsCurve fnCurve( object );
	if( !fnCurve.hasObj( object ) )
	{
		throw IECore::InvalidArgumentException( "FromMayaCurveConverter::doPrimitiveConversion : not a nurbs curve." );
	}
		
	// decide on the basis and periodicity
	int mDegree = fnCurve.degree();
	IECore::CubicBasisf basis = IECore::CubicBasisf::linear();
	if( m_linearParameter->getTypedValue()==false && mDegree==3 )
	{
		basis = IECore::CubicBasisf::bSpline();
	}
	bool periodic = false;
	if( fnCurve.form()==MFnNurbsCurve::kPeriodic )
	{
		periodic = true;
	}
	
	// get the points and convert them
	MPointArray mPoints;
	fnCurve.getCVs( mPoints, space() );
	if( periodic )
	{
		// maya duplicates the first points at the end, whereas we just wrap around.
		// remove the duplicates.
		mPoints.setLength( mPoints.length() - mDegree );
	}
	
	bool duplicateEnds = false;
	if( !periodic && mDegree==3 )
	{
		// there's an implicit duplication of the end points that we need to make explicit
		duplicateEnds = true;
	}
	
	IECore::V3fVectorDataPtr pointsData = new IECore::V3fVectorData;
	std::vector<Imath::V3f> &points = pointsData->writable();
	std::vector<Imath::V3f>::iterator transformDst;
	if( duplicateEnds )
	{
		points.resize( mPoints.length() + 4 );
		transformDst = points.begin();
		*transformDst++ = IECore::convert<Imath::V3f>( mPoints[0] );
		*transformDst++ = IECore::convert<Imath::V3f>( mPoints[0] );
	}
	else
	{
		points.resize( mPoints.length() );
		transformDst = points.begin();
	}
	
	std::transform( MArrayIter<MPointArray>::begin( mPoints ), MArrayIter<MPointArray>::end( mPoints ), transformDst, IECore::VecConvert<MPoint, V3f>() );
	
	if( duplicateEnds )
	{
		points[points.size()-1] = IECore::convert<Imath::V3f>( mPoints[mPoints.length()-1] );
		points[points.size()-2] = IECore::convert<Imath::V3f>( mPoints[mPoints.length()-1] );
	}

	// make and return the curve
	IECore::IntVectorDataPtr vertsPerCurve = new IECore::IntVectorData;
	vertsPerCurve->writable().push_back( points.size() );

	return new IECore::CurvesPrimitive( vertsPerCurve, basis, periodic, pointsData );
}
