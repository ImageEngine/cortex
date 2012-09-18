//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2012, Image Engine Design Inc. All rights reserved.
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

#include "GU/GU_PrimNURBCurve.h"

#include "IECore/DespatchTypedData.h"

#include "IECoreHoudini/ToHoudiniAttribConverter.h"
#include "IECoreHoudini/ToHoudiniCurvesConverter.h"
#include "IECoreHoudini/TypeTraits.h"

using namespace IECore;
using namespace IECoreHoudini;

IE_CORE_DEFINERUNTIMETYPED( ToHoudiniCurvesConverter );

ToHoudiniGeometryConverter::Description<ToHoudiniCurvesConverter> ToHoudiniCurvesConverter::m_description( CurvesPrimitiveTypeId );

ToHoudiniCurvesConverter::ToHoudiniCurvesConverter( const VisibleRenderable *renderable ) :
	ToHoudiniGeometryConverter( renderable, "Converts an IECore::CurvesPrimitive to a Houdini GU_Detail." )
{
}

ToHoudiniCurvesConverter::~ToHoudiniCurvesConverter()
{
}

bool ToHoudiniCurvesConverter::doConversion( const VisibleRenderable *renderable, GU_Detail *geo ) const
{
	const CurvesPrimitive *curves = static_cast<const CurvesPrimitive *>( renderable );
	if ( !curves )
	{
		return false;
	}
	
	GA_Range newPoints;
	bool periodic = curves->periodic();
	bool duplicatedEnds = !periodic && ( curves->basis() == CubicBasisf::bSpline() );
	
	// adjust for duplicated end points
	std::map<std::string, DataPtr> modifiedData;
	if ( duplicatedEnds )
	{
		RemoveDuplicateEnds func( curves->verticesPerCurve()->readable() );
		for ( PrimitiveVariableMap::const_iterator it=curves->variables.begin() ; it != curves->variables.end(); it++ )
		{
			// only remove duplicates from vertex variables
			if ( it->second.interpolation == IECore::PrimitiveVariable::Vertex )
			{
				modifiedData[it->first] = despatchTypedData<RemoveDuplicateEnds, TypeTraits::IsVectorAttribTypedData, DespatchTypedDataIgnoreError>( it->second.data, func );
			}
		}
		
		std::map<std::string, DataPtr>::iterator pIt = modifiedData.find( "P" );
		if ( pIt == modifiedData.end() )
		{
			return false;
		}
		
		newPoints = appendPoints( geo, runTimeCast<const V3fVectorData>( pIt->second ) );
	}
	else
	{
		newPoints = appendPoints( geo, curves->variableData<V3fVectorData>( "P" ) );
	}
	
	if ( !newPoints.isValid() || newPoints.empty() )
	{
		return false;
	}
	
	GA_OffsetList pointOffsets;
	pointOffsets.reserve( newPoints.getEntries() );
	for ( GA_Iterator it=newPoints.begin(); !it.atEnd(); ++it )
	{
		pointOffsets.append( it.getOffset() );
	}
	
	const std::vector<int> &verticesPerCurve = curves->verticesPerCurve()->readable();
	int order = ( curves->basis() == CubicBasisf::bSpline() ) ? 4 : 2;
	bool interpEnds = !(periodic && ( curves->basis() == CubicBasisf::bSpline() ));
	
	GA_OffsetList offsets;
	offsets.reserve( verticesPerCurve.size() );
	
	size_t vertCount = 0;
	size_t numPrims = geo->getNumPrimitives();
	for ( size_t c=0; c < verticesPerCurve.size(); c++ )
	{
		size_t numVerts = duplicatedEnds ? verticesPerCurve[c] - 4 : verticesPerCurve[c];
		GU_PrimNURBCurve *curve = GU_PrimNURBCurve::build( geo, numVerts, order, periodic, interpEnds, false );
		if ( !curve )
		{
			return false;
		}
		
		offsets.append( geo->primitiveOffset( numPrims + c ) );
		
		for ( size_t v=0; v < numVerts; v++ )
		{
			curve->setVertexPoint( v, pointOffsets.get( vertCount + v ) );
		}
		
		vertCount += numVerts;
	}
	
	GA_Range newPrims( geo->getPrimitiveMap(), offsets );
	transferAttribs( curves, geo, newPoints, newPrims, PrimitiveVariable::Vertex );
	
	// add the modified vertex variables
	for ( std::map<std::string, DataPtr>::const_iterator it=modifiedData.begin() ; it != modifiedData.end(); it++ )
	{
		// P should already have been added as points
		if ( it->first == "P" )
		{
			continue;
		}
		
		// add point attribs
		ToHoudiniAttribConverterPtr converter = ToHoudiniAttribConverter::create( it->second );
 		if ( !converter )
 		{
 			continue;
 		}

 		converter->convert( it->first, geo, newPoints );
	}
	
	return true;
}

ToHoudiniCurvesConverter::RemoveDuplicateEnds::RemoveDuplicateEnds( const std::vector<int> &vertsPerCurve ) : m_vertsPerCurve( vertsPerCurve )
{
}

template <typename T>
ToHoudiniCurvesConverter::RemoveDuplicateEnds::ReturnType ToHoudiniCurvesConverter::RemoveDuplicateEnds::operator()( typename T::ConstPtr data ) const
{
	assert( data );

	typedef typename T::ValueType::value_type ValueType;

	const std::vector<ValueType> &origValues = data->readable();

	typename T::Ptr result = new T();
	std::vector<ValueType> &newValues = result->writable();
	
	size_t index = 0;
	for ( size_t i=0; i < m_vertsPerCurve.size(); i++ )
	{
		for ( size_t j=0; j < m_vertsPerCurve[i]; j++, index++ )
		{
			if ( j > 1 && j < m_vertsPerCurve[i]-2 )
			{
				newValues.push_back( origValues[index] );
			}
		}
	}
	
	return result;
}
