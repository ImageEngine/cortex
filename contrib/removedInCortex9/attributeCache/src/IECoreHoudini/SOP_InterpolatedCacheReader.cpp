//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2013, Image Engine Design Inc. All rights reserved.
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

#include "boost/format.hpp"

#include "OP/OP_Director.h" 
#include "PRM/PRM_ChoiceList.h"
#include "PRM/PRM_Default.h"
#include "PRM/PRM_Range.h"
#include "PRM/PRM_Template.h"

#include "IECore/CompoundObject.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/TransformationMatrixData.h"
#include "IECore/VectorTypedData.h"

#include "IECoreHoudini/Convert.h"
#include "IECoreHoudini/ToHoudiniAttribConverter.h"
#include "IECoreHoudini/SOP_InterpolatedCacheReader.h"

using namespace IECore;
using namespace IECoreHoudini;

static PRM_Name parameterNames[] = {
	PRM_Name( "cacheSequence", "Cache Sequence" ),
	PRM_Name( "objectFixes", "Object Prefix/Suffix" ),
	PRM_Name( "attributeFixes", "Attribute Prefix/Suffix" ),
	PRM_Name( "transformAttribute", "Transform Attribute" ),
	PRM_Name( "samplesPerFrame", "Samples Per Frame" ),
	PRM_Name( "interpolation", "Interpolation" ),
	PRM_Name( "groupingMode", "Grouping Mode" ),
};

static PRM_Default samplesPerFrameDefault( 1 );
static PRM_Default interpolationDefault( InterpolatedCache::Linear );
static PRM_Default groupingModeDefault( SOP_InterpolatedCacheReader::PointGroup );

static PRM_Range samplesPerFrameRange( PRM_RANGE_RESTRICTED, 1, PRM_RANGE_FREE, 20 );

static PRM_Name groupingModeNames[] = {
	PRM_Name( "0", "PrimitiveGroup" ),
	PRM_Name( "1", "PointGroup" ),
};

static PRM_Name interpolationNames[] = {
	PRM_Name( "0", "None" ),
	PRM_Name( "1", "Linear" ),
	PRM_Name( "2", "Cubic" ),
};

PRM_ChoiceList SOP_InterpolatedCacheReader::interpolationList( PRM_CHOICELIST_SINGLE, &interpolationNames[0] );
PRM_ChoiceList SOP_InterpolatedCacheReader::groupingModeList( PRM_CHOICELIST_SINGLE, &groupingModeNames[0] );

PRM_Template SOP_InterpolatedCacheReader::parameters[] = {
	PRM_Template( PRM_FILE, 1, &parameterNames[0] ),
	PRM_Template( PRM_STRING, 2, &parameterNames[1] ),
	PRM_Template( PRM_STRING, 2, &parameterNames[2] ),
	PRM_Template( PRM_STRING, 1, &parameterNames[3] ),
	PRM_Template( PRM_INT, 1, &parameterNames[4], &samplesPerFrameDefault, 0, &samplesPerFrameRange ),
	PRM_Template( PRM_INT, 1, &parameterNames[5], &interpolationDefault, &interpolationList ),
	PRM_Template( PRM_INT, 1, &parameterNames[6], &groupingModeDefault, &groupingModeList ),
	PRM_Template(),
};

SOP_InterpolatedCacheReader::SOP_InterpolatedCacheReader( OP_Network *net, const char *name, OP_Operator *op )
	: SOP_Node( net, name, op ), m_cache( 0 ), m_interpolation( InterpolatedCache::Linear ), m_samplesPerFrame( 1 ), m_cacheFileName()
{
	flags().setTimeDep( true );
}

SOP_InterpolatedCacheReader::~SOP_InterpolatedCacheReader()
{
}

OP_Node *SOP_InterpolatedCacheReader::create( OP_Network *net, const char *name, OP_Operator *op )
{
	return new SOP_InterpolatedCacheReader( net, name, op );
}

OP_ERROR SOP_InterpolatedCacheReader::cookMySop( OP_Context &context )
{
	flags().setTimeDep( true );
	
	if ( lockInputs( context ) >= UT_ERROR_ABORT )
	{
		return error();
	}
	
	gdp->stashAll();
	
	float time = context.getTime();
	float frame = context.getFloatFrame();
	
	UT_String paramVal;
	
	evalString( paramVal, "cacheSequence", 0, time );
	std::string cacheFileName = paramVal.toStdString();
	
	evalString( paramVal, "objectFixes", 0, time );
	std::string objectPrefix = paramVal.toStdString();
	evalString( paramVal, "objectFixes", 1, time );
	std::string objectSuffix = paramVal.toStdString();
	
	evalString( paramVal, "attributeFixes", 0, time );
	std::string attributePrefix = paramVal.toStdString();
	evalString( paramVal, "attributeFixes", 1, time );
	std::string attributeSuffix = paramVal.toStdString();
	
	evalString( paramVal, "transformAttribute", 0, time );
	std::string transformAttribute = paramVal.toStdString();
	
	int samplesPerFrame = evalInt( "samplesPerFrame", 0, time );
	InterpolatedCache::Interpolation interpolation = (InterpolatedCache::Interpolation)evalInt( "interpolation", 0, time );
	GroupingMode groupingMode = (GroupingMode)evalInt( "groupingMode", 0, time );
	
	// create the InterpolatedCache
	if ( cacheFileName.compare( m_cacheFileName ) != 0 || samplesPerFrame != m_samplesPerFrame || interpolation != m_interpolation )
	{
		try
		{
			float fps = OPgetDirector()->getChannelManager()->getSamplesPerSec();
			OversamplesCalculator calc( fps, samplesPerFrame );
			m_cache = new InterpolatedCache( cacheFileName, interpolation, calc );
		}
		catch ( IECore::InvalidArgumentException e )
		{
			addWarning( SOP_ATTRIBUTE_INVALID, e.what() );
			unlockInputs();
			return error();
		}
		
		m_cacheFileName = cacheFileName;
		m_samplesPerFrame = samplesPerFrame;
		m_interpolation = interpolation;
	}
	
	if ( !m_cache )
	{
		addWarning( SOP_MESSAGE, "SOP_InterpolatedCacheReader: Cache Sequence not found" );
		unlockInputs();
		return error();
	}
	
	std::vector<InterpolatedCache::ObjectHandle> objects;
	std::vector<InterpolatedCache::AttributeHandle> attrs;
	
	try
	{
		m_cache->objects( frame, objects );
	}
	catch ( IECore::Exception e )
	{
		addWarning( SOP_ATTRIBUTE_INVALID, e.what() );
		unlockInputs();
		return error();
	}
	
	duplicatePointSource( 0, context );
	
	GA_ElementGroupTable *groups = 0;
	if ( groupingMode == PointGroup )
	{
		groups = &gdp->pointGroups();
	}
	else if ( groupingMode == PrimitiveGroup )
	{
		groups = &gdp->primitiveGroups();
	}
	
	for ( GA_GroupTable::iterator<GA_ElementGroup> it=groups->beginTraverse(); !it.atEnd(); ++it )
	{
		GA_ElementGroup *group = it.group();
		if ( group->getInternal() || group->isEmpty() )
		{
			continue;
		}
		
		// match GA_ElementGroup name to InterpolatedCache::ObjectHandle
		std::string searchName = objectPrefix + group->getName().toStdString() + objectSuffix;
		std::vector<InterpolatedCache::ObjectHandle>::iterator oIt = find( objects.begin(), objects.end(), searchName );
		if ( oIt == objects.end() )
		{
			continue;
		}
		
		CompoundObjectPtr attributes = 0;
		
		try
		{
			m_cache->attributes( frame, *oIt, attrs );
			attributes = m_cache->read( frame, *oIt );
		}
		catch ( IECore::Exception e )
		{
			addError( SOP_ATTRIBUTE_INVALID, e.what() );
			unlockInputs();
			return error();
		}
		
		const CompoundObject::ObjectMap &attributeMap = attributes->members();
		
		GA_Range pointRange;
		GA_Range primRange;
		GA_Range vertexRange;
		
		if ( groupingMode == PointGroup )
		{
			pointRange = gdp->getPointRange( (GA_PointGroup*)it.group() );
		}
		else if ( groupingMode == PrimitiveGroup )
		{
			primRange = gdp->getPrimitiveRange( (GA_PrimitiveGroup*)it.group() );
			const GA_PrimitiveList &primitives = gdp->getPrimitiveList();
			
			GA_OffsetList pointOffsets;
			GA_OffsetList vertOffsets;
			for ( GA_Iterator it=primRange.begin(); !it.atEnd(); ++it )
			{
				const GA_Primitive *prim = primitives.get( it.getOffset() );
				GA_Range primPointRange = prim->getPointRange();
				for ( GA_Iterator pIt=primPointRange.begin(); !pIt.atEnd(); ++pIt )
				{
					pointOffsets.append( pIt.getOffset() );
				}
				
				size_t numPrimVerts = prim->getVertexCount();
				for ( size_t v=0; v < numPrimVerts; v++ )
				{
					if ( prim->getTypeId() == GEO_PRIMPOLY )
					{
						vertOffsets.append( prim->getVertexOffset( numPrimVerts - 1 - v ) );
					}
					else
					{
						vertOffsets.append( prim->getVertexOffset( v ) );
					}
				}
			}
			
			pointOffsets.sortAndRemoveDuplicates();
			pointRange = GA_Range( gdp->getPointMap(), pointOffsets );
			vertexRange = GA_Range( gdp->getVertexMap(), vertOffsets );
		}
		
		// transfer the InterpolatedCache::Attributes onto the GA_ElementGroup
		for ( CompoundObject::ObjectMap::const_iterator aIt=attributeMap.begin(); aIt != attributeMap.end(); aIt++ )
		{
			Data *data = IECore::runTimeCast<Data>( aIt->second.get() );
			if ( !data )
			{
				continue;
			}
			
			ToHoudiniAttribConverterPtr converter = ToHoudiniAttribConverter::create( data );
			if ( !converter )
 			{
 				continue;
 			}
			
			// strip the prefix/suffix from the GA_Attribute name
			std::string attrName = aIt->first.value();
			size_t prefixLength = attributePrefix.length();
			if ( prefixLength && ( search( attrName.begin(), attrName.begin()+prefixLength, attributePrefix.begin(), attributePrefix.end() ) == attrName.begin() ) )
			{
				attrName.erase( attrName.begin(), attrName.begin() + prefixLength );
			}
			
			size_t suffixLength = attributeSuffix.length();
			if ( suffixLength && ( search( attrName.end() - suffixLength, attrName.end(), attributeSuffix.begin(), attributeSuffix.end() ) == ( attrName.end() - suffixLength ) ) )
			{
				attrName.erase( attrName.end() - suffixLength, attrName.end() );
			}
			
			if ( attrName == "P" )
			{
				const V3fVectorData *positions = IECore::runTimeCast<const V3fVectorData>( data );
				if ( !positions )
				{
					continue;
				}
				
				size_t index = 0;
				size_t entries = pointRange.getEntries();
				const std::vector<Imath::V3f> &pos = positions->readable();
				
				// Attempting to account for the vertex difference between an IECore::CurvesPrimitive and Houdini curves.
				// As Houdini implicitly triples the endpoints of a curve, a cache generated from a single CurvesPrimitive
				// will have exactly four extra vertices. In this case, we adjust the cache by ignoring the first two and
				// last two V3fs. In all other cases, we report a warning and don't apply the cache to these points.
				if ( pos.size() - 4 == entries )
				{
					index = 2;
				}
				else if ( pos.size() != entries )
				{
					addWarning( SOP_ATTRIBUTE_INVALID, ( boost::format( "Geometry/Cache mismatch: %s contains %d points, while cache expects %d values for P." ) % group->getName().toStdString() % entries % pos.size() ).str().c_str() );
					continue;
				}
				
				/// \todo: try multi-threading this with a GA_SplittableRange
				for ( GA_Iterator it=pointRange.begin(); !it.atEnd(); ++it, ++index )
				{
					gdp->setPos3( it.getOffset(), IECore::convert<UT_Vector3>( pos[index] ) );
				}

			}
			else if ( groupingMode == PrimitiveGroup )
			{
				GA_Range currentRange;
				unsigned size = despatchTypedData<TypedDataSize, TypeTraits::IsVectorTypedData, DespatchTypedDataIgnoreError>( data );
				
				// check for existing attributes
				if ( gdp->findPrimitiveAttribute( attrName.c_str() ).isValid() && size == primRange.getEntries() )
				{
					currentRange = primRange;
				}
				else if ( gdp->findPointAttribute( attrName.c_str() ).isValid() && size == pointRange.getEntries() )
				{
					currentRange = pointRange;
				}
				else if ( gdp->findVertexAttribute( attrName.c_str() ).isValid() && size == vertexRange.getEntries() )
				{
					currentRange = vertexRange;
				}
				// fall back to Cortex standard inferred order
				else if ( size == primRange.getEntries() )
				{
					currentRange = primRange;
				}
				else if ( size == pointRange.getEntries() )
				{
					currentRange = pointRange;
				}
				else if ( size == vertexRange.getEntries() )
				{
					currentRange = vertexRange;
				}
				else
				{
					addWarning( SOP_ATTRIBUTE_INVALID, ( boost::format( "Geometry/Cache mismatch: %s: cache expects %d values for %s." ) % group->getName().toStdString() % size % attrName ).str().c_str() );
					continue;
				}
				
				converter->convert( attrName, gdp, currentRange );
			}
			else
			{
				converter->convert( attrName, gdp, pointRange );
			}
		}
		
		// if transformAttribute is specified, use to to transform the points
		if ( transformAttribute != "" )
		{
			const TransformationMatrixdData *transform = attributes->member<TransformationMatrixdData>( transformAttribute );
			if ( transform )
			{
				UT_Matrix4 matrix( IECore::convert<UT_Matrix4T<double> >( transform->readable().transform() ) );
				gdp->transformGroup( matrix, *group );
			}
			else
			{
				const TransformationMatrixfData *transform = attributes->member<TransformationMatrixfData>( transformAttribute );
				if ( transform )
				{
					UT_Matrix4 matrix = IECore::convert<UT_Matrix4>( transform->readable().transform() );
					gdp->transformGroup( matrix, *group );
				}
			}
		}
	}
	
	unlockInputs();
	return error();
}
