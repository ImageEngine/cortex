//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2015, Image Engine Design Inc. All rights reserved.
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

#include "GEO/GEO_Curve.h"

#include "boost/python.hpp"

#include "IECore/DespatchTypedData.h"

#include "IECoreHoudini/FromHoudiniCurvesConverter.h"
#include "IECoreHoudini/TypeTraits.h"

using namespace IECore;
using namespace IECoreHoudini;

IE_CORE_DEFINERUNTIMETYPED( FromHoudiniCurvesConverter );

FromHoudiniGeometryConverter::Description<FromHoudiniCurvesConverter> FromHoudiniCurvesConverter::m_description( CurvesPrimitiveTypeId );

FromHoudiniCurvesConverter::FromHoudiniCurvesConverter( const GU_DetailHandle &handle ) :
	FromHoudiniGeometryConverter( handle, "Converts a Houdini GU_Detail to an IECore::CurvesPrimitive." )
{
}

FromHoudiniCurvesConverter::FromHoudiniCurvesConverter( const SOP_Node *sop ) :
	FromHoudiniGeometryConverter( sop, "Converts a Houdini GU_Detail to an IECore::CurvesPrimitive." )
{
}

FromHoudiniCurvesConverter::~FromHoudiniCurvesConverter()
{
}

FromHoudiniGeometryConverter::Convertability FromHoudiniCurvesConverter::canConvert( const GU_Detail *geo )
{
	const GA_PrimitiveList &primitives = geo->getPrimitiveList();
	
	unsigned numPrims = geo->getNumPrimitives();
	GA_Iterator firstPrim = geo->getPrimitiveRange().begin();
	if ( !numPrims || !compatiblePrimitive( primitives.get( firstPrim.getOffset() )->getTypeId() ) )
	{
		return Inapplicable;
	}
	
	const GEO_Curve *firstCurve = (const GEO_Curve*)primitives.get( firstPrim.getOffset() );
	bool periodic = firstCurve->isClosed();
	unsigned order = firstCurve->getOrder();
	
	for ( GA_Iterator it=firstPrim; !it.atEnd(); ++it )
	{
		const GA_Primitive *prim = primitives.get( it.getOffset() );
		if ( !compatiblePrimitive( prim->getTypeId() ) )
		{
			return Inapplicable;
		}
		
		const GEO_Curve *curve = (const GEO_Curve*)prim;
		if ( curve->getOrder() != order )
		{
			return Inapplicable;
		}
		
		if ( curve->isClosed() != periodic )
		{
			return Inapplicable;
		}
	}
	
	// is there a single named shape?
	GA_ROAttributeRef attrRef = geo->findPrimitiveAttribute( "name" );
	if ( attrRef.isValid() && attrRef.isString() )
	{
		const GA_Attribute *nameAttr = attrRef.getAttribute();
		const GA_AIFSharedStringTuple *tuple = nameAttr->getAIFSharedStringTuple();
		GA_StringTableStatistics stats;
		tuple->getStatistics( nameAttr, stats );
		if ( stats.getEntries() < 2 )
		{
			return Ideal;
		}
	}
	
	return Suitable;
}

ObjectPtr FromHoudiniCurvesConverter::doDetailConversion( const GU_Detail *geo, const CompoundObject *operands ) const
{
	const GA_PrimitiveList &primitives = geo->getPrimitiveList();
	
	CurvesPrimitivePtr result = new CurvesPrimitive();
	
	GA_Iterator firstPrim = geo->getPrimitiveRange().begin();
	if ( !geo->getNumPrimitives() || !compatiblePrimitive( primitives.get( firstPrim.getOffset() )->getTypeId() ) )
	{
		throw std::runtime_error( "FromHoudiniCurvesConverter: Geometry contains no curves or non-curve primitives" );
	}
	
	// set periodic based on the first curve
	const GEO_Curve *firstCurve = (const GEO_Curve*)primitives.get( firstPrim.getOffset() );
	bool periodic = firstCurve->isClosed();
	
	// set basis based on the first curve
	bool duplicateEnds = false;
	CubicBasisf basis = CubicBasisf::linear();
	unsigned order = firstCurve->getOrder();
	if ( order == 4 )
	{
		basis = CubicBasisf::bSpline();
		
		if ( !periodic )
		{
			// there's an implicit duplication of the end points that we need to make explicit
			duplicateEnds = true;
		}
	}
	
	std::vector<int> origVertsPerCurve;
	std::vector<int> finalVertsPerCurve;
	for ( GA_Iterator it=firstPrim; !it.atEnd(); ++it )
	{
		const GA_Primitive *prim = primitives.get( it.getOffset() );
		if ( !compatiblePrimitive( prim->getTypeId() ) )
		{
			throw std::runtime_error( "FromHoudiniCurvesConverter: Geometry contains non-curve primitives" );
		}
		
		const GEO_Curve *curve = (const GEO_Curve*)prim;
		if ( curve->getOrder() != order )
		{
			throw std::runtime_error( "FromHoudiniCurvesConverter: Geometry contains multiple curves with differing order. Set all curves to order 2 (linear) or 4 (cubic bSpline)" );
		}
		
		if ( curve->isClosed() != periodic )
		{
			throw std::runtime_error( "FromHoudiniCurvesConverter: Geometry contains both open and closed curves" );
		}
		
		int numPrimVerts = prim->getVertexCount();
		
		origVertsPerCurve.push_back( numPrimVerts );
		
		if ( duplicateEnds && numPrimVerts )
		{
			numPrimVerts += 4;
		}
		
		finalVertsPerCurve.push_back( numPrimVerts );
	}
	
	if ( !origVertsPerCurve.size() )
	{
		throw std::runtime_error( "FromHoudiniCurvesConverter: Geometry does not contain curve vertices" );
	}
	
	result->setTopology( new IntVectorData( origVertsPerCurve ), basis, periodic );
	
	transferAttribs( geo, result.get(), operands, PrimitiveVariable::Vertex );
	
	if ( !duplicateEnds )
	{
		return result;
	}
	
	// adjust for duplicated end points
	DuplicateEnds func( result->verticesPerCurve()->readable() );
	for ( PrimitiveVariableMap::const_iterator it=result->variables.begin() ; it != result->variables.end(); it++ )
	{
		// only duplicate point and vertex attrib end points
		if ( it->second.interpolation == IECore::PrimitiveVariable::Vertex )
		{
			despatchTypedData<DuplicateEnds, TypeTraits::IsVectorAttribTypedData, DespatchTypedDataIgnoreError>( it->second.data.get(), func );
		}
	}
	
	result->setTopology( new IntVectorData( finalVertsPerCurve ), basis, periodic );
	
	return result;
}

FromHoudiniCurvesConverter::DuplicateEnds::DuplicateEnds( const std::vector<int> &vertsPerCurve ) : m_vertsPerCurve( vertsPerCurve )
{
}

template <typename T>
FromHoudiniCurvesConverter::DuplicateEnds::ReturnType FromHoudiniCurvesConverter::DuplicateEnds::operator()( T *data ) const
{
	assert( data );

	typedef typename T::ValueType::value_type ValueType;

	std::vector<ValueType> newValues;
	const std::vector<ValueType> &origValues = data->readable();
	newValues.reserve( origValues.size() + m_vertsPerCurve.size()*4 );
	
	size_t index = 0;
	for ( size_t i=0; i < m_vertsPerCurve.size(); i++ )
	{
		for ( size_t j=0; j < (size_t)m_vertsPerCurve[i]; j++, index++ )
		{
			newValues.push_back( origValues[index] );
			
			if ( j == 0 || j == (size_t)m_vertsPerCurve[i]-1 )
			{
				newValues.push_back( origValues[index] );
				newValues.push_back( origValues[index] );
			}
		}
	}
	
	data->writable().swap( newValues );
}
