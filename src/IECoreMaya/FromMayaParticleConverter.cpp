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

#include "boost/format.hpp"

#include "IECoreMaya/FromMayaParticleConverter.h"
#include "IECoreMaya/MArrayIter.h"
#include "IECoreMaya/VectorTraits.h"
#include "IECoreMaya/Convert.h"

#include "IECore/PointsPrimitive.h"
#include "IECore/VectorOps.h"
#include "IECore/Exception.h"
#include "IECore/CompoundParameter.h"
#include "IECore/MessageHandler.h"

#include "maya/MFnParticleSystem.h"

#include <algorithm>

using namespace IECoreMaya;
using namespace Imath;

static const MFn::Type fromTypes[] = { MFn::kParticle, MFn::kInvalid };
static const IECore::TypeId toTypes[] = { IECore::BlindDataHolderTypeId, IECore::RenderableTypeId, IECore::VisibleRenderableTypeId, IECore::PrimitiveTypeId, IECore::PointsPrimitiveTypeId, IECore::InvalidTypeId };

IECoreMaya::FromMayaShapeConverter::Description<FromMayaParticleConverter> FromMayaParticleConverter::m_description( fromTypes, toTypes );

FromMayaParticleConverter::FromMayaParticleConverter( const MObject &object )
	:	FromMayaShapeConverter( staticTypeName(), "Converts Maya particle shapes into IECore::PointsPrimitive objects.", object )
{
	constructCommon();
}

FromMayaParticleConverter::FromMayaParticleConverter( const MDagPath &dagPath )
	:	FromMayaShapeConverter( staticTypeName(), "Converts Maya particle shapes into IECore::PointsPrimitive objects.", dagPath )
{
	constructCommon();
}

void FromMayaParticleConverter::constructCommon()
{

	IECore::StringVectorDataPtr attributeNamesDefaultData = new IECore::StringVectorData();
	IECore::StringVectorData::ValueType &attributeNames = attributeNamesDefaultData->writable();
	
	attributeNames.push_back( "velocity" );

	m_attributeNamesParameter = new IECore::StringVectorParameter(
		"attributeNames",
		"The per-particle attribute names to be added as primitive variables to the PointsPrimitive. "
		"The \"position\" attribute is always added as \"P\" so there is no need to specify it again here",
		attributeNamesDefaultData
		/// \todo Add presets
	);
			
	parameters()->addParameter( m_attributeNamesParameter );	
}

IECore::StringVectorParameterPtr FromMayaParticleConverter::attributeNamesParameter()
{
	return m_attributeNamesParameter;
}

IECore::ConstStringVectorParameterPtr FromMayaParticleConverter::attributeNamesParameter() const
{
	return m_attributeNamesParameter;
}

IECore::PrimitivePtr FromMayaParticleConverter::doPrimitiveConversion( const MObject &object, IECore::ConstCompoundObjectPtr operands ) const
{
	MFnParticleSystem fnParticle( object );
	if( !fnParticle.hasObj( object ) )
	{
		throw IECore::InvalidArgumentException( "FromMayaParticleConverter::doPrimitiveConversion : not a particle shape." );
	}
	return doPrimitiveConversion( fnParticle );
}

IECore::PrimitivePtr FromMayaParticleConverter::doPrimitiveConversion( const MDagPath &dagPath, IECore::ConstCompoundObjectPtr operands ) const
{
	MFnParticleSystem fnParticle( dagPath );
	if( !fnParticle.hasObj( dagPath.node() ) )
	{
		throw IECore::InvalidArgumentException( "FromMayaParticleConverter::doPrimitiveConversion : not a particle shape." );
	}
	return doPrimitiveConversion( fnParticle );
}

IECore::PrimitivePtr FromMayaParticleConverter::doPrimitiveConversion( MFnParticleSystem &fnParticle ) const
{
	MStatus s;
	IECore::PointsPrimitivePtr points = new IECore::PointsPrimitive( fnParticle.count( ) );
	
	/// We always add "position" as "P"
	MVectorArray position;
	fnParticle.position( position );
	
        IECore::V3fVectorDataPtr pos = new IECore::V3fVectorData();
        pos->writable().resize( position.length() );
        std::transform( MArrayIter<MVectorArray>::begin( position ), MArrayIter<MVectorArray>::end( position ), pos->writable().begin(), IECore::VecConvert<MVector, V3f>() );		
	points->variables["P"] = IECore::PrimitiveVariable( IECore::PrimitiveVariable::Vertex, pos );
	
	const IECore::StringVectorParameter::ValueType &attributeNames = attributeNamesParameter()->getTypedValue();
	
	for ( IECore::StringVectorParameter::ValueType::const_iterator it = attributeNames.begin(); it != attributeNames.end(); ++it )
	{
		const MString attrName = it->c_str();
		const std::string &primVarName = *it;
		
		if ( fnParticle.isPerParticleIntAttribute( attrName, &s ) )
		{
			assert( s );
			MIntArray arr;
			unsigned int len = fnParticle.getPerParticleAttribute( attrName, arr, &s );
			assert( len == fnParticle.count() );
			
			IECore::IntVectorDataPtr data = new IECore::IntVectorData();
			data->writable().resize( len );			
			data->writable().assign( MArrayIter<MIntArray>::begin( arr ), MArrayIter<MIntArray>::end( arr ) );
			points->variables[ primVarName ] = IECore::PrimitiveVariable( IECore::PrimitiveVariable::Vertex, data );			
		} 
		else if ( fnParticle.isPerParticleDoubleAttribute( attrName, &s ) )
		{
			assert( s );
			MDoubleArray arr;
			unsigned int len = fnParticle.getPerParticleAttribute( attrName, arr, &s );
			assert( len == fnParticle.count() );
			
			IECore::FloatVectorDataPtr data = new IECore::FloatVectorData();
			data->writable().resize( len );			
			data->writable().assign( MArrayIter<MDoubleArray>::begin( arr ), MArrayIter<MDoubleArray>::end( arr ) );
			points->variables[ primVarName ] = IECore::PrimitiveVariable( IECore::PrimitiveVariable::Vertex, data );			
		} 
		else if ( fnParticle.isPerParticleVectorAttribute( attrName, &s ) )
		{
			assert( s );		
			MVectorArray arr;
			unsigned int len = fnParticle.getPerParticleAttribute( attrName, arr, &s );
			assert( len == fnParticle.count() );
			
			IECore::V3fVectorDataPtr data = new IECore::V3fVectorData();
			data->writable().resize( len );
		        std::transform( MArrayIter<MVectorArray>::begin( arr ), MArrayIter<MVectorArray>::end( arr ), data->writable().begin(), IECore::VecConvert<MVector, V3f>() );		
			points->variables[ primVarName ] = IECore::PrimitiveVariable( IECore::PrimitiveVariable::Vertex, data );			
		}
		else
		{
			IECore::msg( IECore::Msg::Warning, "FromMayaParticleConverter", boost::format( "Ignoring attribute \%s\", which is not a per-particle attribute" ) % primVarName);
		}
		
	}
	
	assert( points->arePrimitiveVariablesValid() );
	return points;
}
