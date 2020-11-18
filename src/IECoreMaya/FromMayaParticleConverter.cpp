//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECore/VectorOps.h"
#include "IECore/Exception.h"
#include "IECore/CompoundParameter.h"
#include "IECore/MessageHandler.h"
#include "IECoreScene/PointsPrimitive.h"

#include "IECore/AngleConversion.h"

#include "maya/MFnParticleSystem.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MPlug.h"
#if MAYA_API_VERSION >= 201800
#include "maya/MDGContextGuard.h"
#endif

#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/classification.hpp"

#include <algorithm>

using namespace IECoreMaya;
using namespace Imath;

IE_CORE_DEFINERUNTIMETYPED( FromMayaParticleConverter );

IECoreMaya::FromMayaShapeConverter::Description<FromMayaParticleConverter> FromMayaParticleConverter::m_kNParticleDescription( MFn::kNParticle, IECoreScene::PointsPrimitive::staticTypeId(), true );
IECoreMaya::FromMayaShapeConverter::Description<FromMayaParticleConverter> FromMayaParticleConverter::m_kParticleDescription( MFn::kParticle, IECoreScene::PointsPrimitive::staticTypeId(), true );

FromMayaParticleConverter::FromMayaParticleConverter( const MObject &object )
	:	FromMayaShapeConverter( "Converts Maya particle shapes into IECoreScene::PointsPrimitive objects.", object )
{
	constructCommon();
}

FromMayaParticleConverter::FromMayaParticleConverter( const MDagPath &dagPath )
	:	FromMayaShapeConverter( "Converts Maya particle shapes into IECoreScene::PointsPrimitive objects.", dagPath )
{
	constructCommon();
}

void FromMayaParticleConverter::constructCommon()
{

	IECore::StringVectorDataPtr attributeNamesDefaultData = new IECore::StringVectorData();
	IECore::StringVectorData::ValueType &attributeNames = attributeNamesDefaultData->writable();

	// add all Maya default per-particle attributes
	attributeNames.push_back( "position=P" );
	attributeNames.push_back( "velocity" );
	attributeNames.push_back( "age" );
	attributeNames.push_back( "mass" );
	attributeNames.push_back( "acceleration" );

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

IECoreScene::PrimitivePtr FromMayaParticleConverter::doPrimitiveConversion( const MObject &object, IECore::ConstCompoundObjectPtr operands ) const
{
	MFnParticleSystem fnParticle( object );
	if( !fnParticle.hasObj( object ) )
	{
		throw IECore::InvalidArgumentException( boost::str( boost::format( "FromMayaParticleConverter::doPrimitiveConversion : '%s' is not a particle shape.") % MFnDependencyNode( object ).name().asChar() ) );
	}
	return doPrimitiveConversion( fnParticle );
}

IECoreScene::PrimitivePtr FromMayaParticleConverter::doPrimitiveConversion( const MDagPath &dagPath, IECore::ConstCompoundObjectPtr operands ) const
{
	MFnParticleSystem fnParticle( dagPath );
	if( !fnParticle.hasObj( dagPath.node() ) )
	{
		throw IECore::InvalidArgumentException( boost::str( boost::format( "FromMayaParticleConverter::doPrimitiveConversion : '%s' is not a particle shape.") % dagPath.partialPathName().asChar() ) );
	}
	return doPrimitiveConversion( fnParticle );
}

IECoreScene::PrimitivePtr FromMayaParticleConverter::doPrimitiveConversion( MFnParticleSystem &fnParticle ) const
{
	MStatus s;
	IECoreScene::PointsPrimitivePtr points = new IECoreScene::PointsPrimitive( fnParticle.count( ) );

	const IECore::StringVectorParameter::ValueType &attributeNames = attributeNamesParameter()->getTypedValue();

	IECore::StringVectorParameter::ValueType allAttributeNames = attributeNames;

	MPlug particleAttributePlugs = fnParticle.findPlug( "ieParticleAttributes", false, &s );

	if ( s )
	{
		MString particleAttributes;

#if MAYA_API_VERSION >= 201800
		{
			MDGContextGuard ctxGuard( MDGContext::fsNormal );
			particleAttributes = particleAttributePlugs.asString( &s );
		}
#else
		particleAttributes = particleAttributePlugs.asString(MDGContext::fsNormal, &s);
#endif

		if ( s )
		{
			std::string strParticleAttributes( particleAttributes.asChar() );
			std::vector<std::string> additionalAttributeNames;
			boost::split( additionalAttributeNames, strParticleAttributes, boost::is_any_of( ",: " ) );

			allAttributeNames.insert( allAttributeNames.end(),
				additionalAttributeNames.begin(),
				additionalAttributeNames.end()
			);
		}
		else
		{
			IECore::msg(
				IECore::Msg::Warning,
				"FromMayaParticleConverter::doPrimitiveConversion",
				boost::format( "Attribute 'ieParticleAttributes' must be string" )
			);
		}
	}

	bool usesConstantColor = false;
	if ( fnParticle.hasAttribute( "colorInput" ) )
	{
		usesConstantColor = true ? fnParticle.findPlug( "colorInput" ).asInt() == 0: false;
	}

	for ( IECore::StringVectorParameter::ValueType::const_iterator it = allAttributeNames.begin(); it != allAttributeNames.end(); ++it )
	{
		MString attrName = it->c_str();
		std::string primVarName = it->c_str();

		// check if primvar name should be remapped
		std::vector<std::string> primVarMapping;
		boost::split( primVarMapping, primVarName, boost::is_any_of( "=" ) );
		if ( primVarMapping.size() == 2 )
		{
			attrName = primVarMapping[ 0 ].c_str();
			primVarName = primVarMapping[ 1 ].c_str();
		}

		if ( fnParticle.isPerParticleIntAttribute( attrName, &s ) )
		{
			assert( s );
			MIntArray arr;
			unsigned int len = fnParticle.getPerParticleAttribute( attrName, arr, &s );
			assert( len == fnParticle.count() );

			IECore::IntVectorDataPtr data = new IECore::IntVectorData();
			data->writable().resize( len );
			data->writable().assign( MArrayIter<MIntArray>::begin( arr ), MArrayIter<MIntArray>::end( arr ) );
			points->variables[ primVarName ] = IECoreScene::PrimitiveVariable( IECoreScene::PrimitiveVariable::Vertex, data );
		}
		else if ( fnParticle.isPerParticleDoubleAttribute( attrName, &s ) )
		{
			assert( s );
			MDoubleArray arr;
			unsigned int len = fnParticle.getPerParticleAttribute( attrName, arr, &s );
			assert( len == fnParticle.count() );

			IECore::FloatVectorDataPtr data = new IECore::FloatVectorData();
			data->writable().resize( len );
			if ( attrName == MString("radiusPP") && primVarName == "width" )
			{
				// convert radius to width (as used in Cortex / Gaffer)
				std::vector<float> &dataVector = data->writable();
				for ( unsigned int i = 0; i < len; i++ )
				{
					dataVector[i] = arr[i] * 2.0f;
				}
			}
			else
			{
				data->writable().assign( MArrayIter<MDoubleArray>::begin( arr ), MArrayIter<MDoubleArray>::end( arr ) );
			}

			points->variables[ primVarName ] = IECoreScene::PrimitiveVariable( IECoreScene::PrimitiveVariable::Vertex, data );
		}
		else if ( attrName == MString( "rgbPP" ) && ( !fnParticle.isPerParticleVectorAttribute( attrName, &s ) || usesConstantColor ) )
		{
			// if there is no RGB per-particle attribute or constant color is used, write a constant primvar instead
			if ( fnParticle.hasAttribute( "colorRed" ) && fnParticle.hasAttribute( "colorGreen" ) && fnParticle.hasAttribute( "colorBlue" ) )
			{
				float r = (float) fnParticle.findPlug( "colorRed" ).asDouble();
				float g = (float) fnParticle.findPlug( "colorGreen" ).asDouble();
				float b = (float) fnParticle.findPlug( "colorBlue" ).asDouble();

				IECore::Color3fDataPtr data = new IECore::Color3fData( { r, g, b } );
				points->variables[ primVarName ] = IECoreScene::PrimitiveVariable( IECoreScene::PrimitiveVariable::Constant, data );
			}
			else
			{
				IECore::msg( IECore::Msg::Warning, "FromMayaParticleConverter", boost::format( "Node \"%s\", does not have per-particle or per-object color attributes." ) % fnParticle.name().asChar() );
			}
		}
		else if ( fnParticle.isPerParticleVectorAttribute( attrName, &s ) )
		{
			assert( s );
			MVectorArray arr;
			unsigned int len = fnParticle.getPerParticleAttribute( attrName, arr, &s );
			assert( len == fnParticle.count() );

			if ( attrName == MString( "rotationPP" ) && primVarName == "orientation" )
			{
				// get euler data and convert to quaternion
				IECore::V3fVectorDataPtr eulerData = new IECore::V3fVectorData();
				eulerData->writable().resize( len );
				std::transform( MArrayIter<MVectorArray>::begin( arr ), MArrayIter<MVectorArray>::end( arr ), eulerData->writable().begin(), IECore::VecConvert<MVector, V3f>() );

				auto &readableEulerData = eulerData->readable();

				IECore::QuatfVectorDataPtr quatData = new IECore::QuatfVectorData();
				auto &writableQuatData = quatData->writable();

				writableQuatData.reserve( readableEulerData.size() );

				for( const auto rotation : readableEulerData )
				{
					float x = IECore::degreesToRadians( rotation.x );
					float y = IECore::degreesToRadians( rotation.y );
					float z = IECore::degreesToRadians( rotation.z );

					Imath::Eulerf euler( x, y, z, Imath::Eulerf::Default );
					writableQuatData.push_back( euler.toQuat() );
				}

				points->variables[ primVarName ] = IECoreScene::PrimitiveVariable( IECoreScene::PrimitiveVariable::Vertex, quatData );
			}
			else
			{
				IECore::V3fVectorDataPtr data = new IECore::V3fVectorData();
				if ( primVarName == "P" )
				{
					data->setInterpretation( IECore::GeometricData::Point );
				}
				else if ( attrName == MString( "rgbPP" ) )
				{
					data->setInterpretation( IECore::GeometricData::Color );
				}
				else
				{
					data->setInterpretation( IECore::GeometricData::Vector );
				}

				data->writable().resize( len );
				std::transform( MArrayIter<MVectorArray>::begin( arr ), MArrayIter<MVectorArray>::end( arr ), data->writable().begin(), IECore::VecConvert<MVector, V3f>() );
				points->variables[ primVarName ] = IECoreScene::PrimitiveVariable( IECoreScene::PrimitiveVariable::Vertex, data );
			}


		}
		else if ( attrName.length() > 1 )
		{
			IECore::msg( IECore::Msg::Warning, "FromMayaParticleConverter", boost::format( "Ignoring attribute \"%s\", which is not a valid per-particle attribute" ) % attrName.asChar() );
		}

	}

	assert( points->arePrimitiveVariablesValid() );
	return points;
}
