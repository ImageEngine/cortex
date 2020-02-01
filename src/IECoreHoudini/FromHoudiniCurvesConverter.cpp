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

#include "IECoreHoudini/FromHoudiniCurvesConverter.h"
#include "IECoreHoudini/TypeTraits.h"

#include "IECore/DespatchTypedData.h"

#include "GA/GA_Names.h"
#include "GEO/GEO_Curve.h"

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreHoudini;

IE_CORE_DEFINERUNTIMETYPED( FromHoudiniCurvesConverter );

FromHoudiniGeometryConverter::Description<FromHoudiniCurvesConverter> FromHoudiniCurvesConverter::m_description( CurvesPrimitive::staticTypeId() );

FromHoudiniCurvesConverter::FromHoudiniCurvesConverter( const GU_DetailHandle &handle ) :
	FromHoudiniGeometryConverter( handle, "Converts a Houdini GU_Detail to an IECoreScene::CurvesPrimitive." )
{
}

FromHoudiniCurvesConverter::FromHoudiniCurvesConverter( const SOP_Node *sop ) :
	FromHoudiniGeometryConverter( sop, "Converts a Houdini GU_Detail to an IECoreScene::CurvesPrimitive." )
{
}

FromHoudiniCurvesConverter::~FromHoudiniCurvesConverter()
{
}

FromHoudiniGeometryConverter::Convertability FromHoudiniCurvesConverter::canConvert( const GU_Detail *geo )
{
	const GA_PrimitiveList &primitives = geo->getPrimitiveList();

	unsigned numPrims = geo->getNumPrimitives();

	if ( !numPrims )
	{
		return Inapplicable;
	}

	GA_Iterator firstPrim = geo->getPrimitiveRange().begin();
	GA_PrimitiveTypeId firstPrimitiveType =  primitives.get( firstPrim.getOffset() )->getTypeId() ;

	if ( !compatiblePrimitive( firstPrimitiveType ) )
	{
		return Inapplicable;
	}

	if ( firstPrimitiveType != GEO_PRIMPOLY )
	{
		const GEO_Curve *firstCurve = (const GEO_Curve*)primitives.get( firstPrim.getOffset() );
		bool periodic = firstCurve->isClosed();
		unsigned order = firstCurve->getOrder();

		GA_Offset start, end;
		for( GA_Iterator it( geo->getPrimitiveRange() ); it.blockAdvance( start, end ); )
		{
			for( GA_Offset offset = start; offset < end; ++offset )
			{
				const GA_Primitive *prim = primitives.get( offset );
				if( !compatiblePrimitive( prim->getTypeId() ) )
				{
					return Inapplicable;
				}

				const GEO_Curve *curve = (const GEO_Curve *) prim;
				if( curve->getOrder() != order )
				{
					return Inapplicable;
				}

				if( curve->isClosed() != periodic )
				{
					return Inapplicable;
				}
			}
		}
	}
	else
	{
		if ( hasOnlyOpenPolygons( geo) )
		{
			return Ideal;
		}
		else
		{
			return Inapplicable;
		}
	}

	// is there a single named shape?
	GA_ROHandleS nameAttrib( geo, GA_ATTRIB_PRIMITIVE, GA_Names::name );
	if( nameAttrib.isValid() )
	{
		GA_StringTableStatistics stats;
		const GA_Attribute *nameAttr = nameAttrib.getAttribute();
		const GA_AIFSharedStringTuple *tuple = nameAttr->getAIFSharedStringTuple();
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
	int totalVerts = 0;

	GA_Offset start, end;
	for( GA_Iterator it( geo->getPrimitiveRange() ); it.blockAdvance( start, end ); )
	{
		for( GA_Offset offset = start; offset < end; ++offset )
		{
			const GA_Primitive *prim = primitives.get( offset );
			if( !compatiblePrimitive( prim->getTypeId() ) )
			{
				throw std::runtime_error( "FromHoudiniCurvesConverter: Geometry contains non-curve primitives" );
			}

			const GEO_Curve *curve = (const GEO_Curve *) prim;
			if( curve->getOrder() != order )
			{
				throw std::runtime_error( "FromHoudiniCurvesConverter: Geometry contains multiple curves with differing order. Set all curves to order 2 (linear) or 4 (cubic bSpline)" );
			}

			if( curve->isClosed() != periodic )
			{
				throw std::runtime_error( "FromHoudiniCurvesConverter: Geometry contains both open and closed curves" );
			}

			size_t numPrimVerts = prim->getVertexCount();
			origVertsPerCurve.push_back( numPrimVerts );
			totalVerts += numPrimVerts;

			if( duplicateEnds && numPrimVerts )
			{
				numPrimVerts += 4;
			}
			finalVertsPerCurve.push_back( numPrimVerts );
		}
	}

	if ( geo->getPointRange().getEntries() > totalVerts )
	{
		throw std::runtime_error( "FromHoudiniCurvesConverter: Geometry contains more points than curve vertices" );
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
		if ( it->second.interpolation == IECoreScene::PrimitiveVariable::Vertex )
		{
			IECore::Data *data = it->second.indices ? it->second.indices.get() : it->second.data.get();
			despatchTypedData<DuplicateEnds, TypeTraits::IsVectorAttribTypedData, DespatchTypedDataIgnoreError>( data, func );
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
