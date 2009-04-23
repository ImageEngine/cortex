//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#include "IECoreMaya/FromMayaShapeConverter.h"
#include "IECoreMaya/FromMayaPlugConverter.h"

#include "IECore/CompoundParameter.h"
#include "IECore/Primitive.h"
#include "IECore/MessageHandler.h"
#include "IECore/DespatchTypedData.h"

#include "maya/MFnDependencyNode.h"
#include "maya/MFnAttribute.h"

using namespace IECoreMaya;

IE_CORE_DEFINERUNTIMETYPED( FromMayaShapeConverter );

FromMayaShapeConverter::FromMayaShapeConverter( const std::string &name, const std::string &description, const MObject &object )
	:	FromMayaObjectConverter( name, description, object )
{
	constructCommon();
}

FromMayaShapeConverter::FromMayaShapeConverter( const std::string &name, const std::string &description, const MDagPath &dagPath )
	:	FromMayaObjectConverter( name, description, dagPath.node() ), m_dagPath( dagPath )
{
	constructCommon();
}
	
void FromMayaShapeConverter::constructCommon()
{

	IECore::IntParameter::PresetsContainer spacePresets;
	spacePresets.push_back( IECore::IntParameter::Preset( "Object", Object ) );
	spacePresets.push_back( IECore::IntParameter::Preset( "World", World ) );
	m_spaceParameter = new IECore::IntParameter(
		"space",
		"The space in which the object is exported.",
		Object,
		Object,
		World,
		spacePresets,
		true
	);

	IECore::StringParameter::PresetsContainer primVarAttrPrefixPresets;
	primVarAttrPrefixPresets.push_back( IECore::StringParameter::Preset( "MTOR", "rman" ) );
	primVarAttrPrefixPresets.push_back( IECore::StringParameter::Preset( "3Delight", "delight" ) );
	primVarAttrPrefixPresets.push_back( IECore::StringParameter::Preset( "None", "" ) );
	m_primVarAttrPrefixParameter = new IECore::StringParameter(
		"primVarAttrPrefix",
		"Any attribute names beginning with this prefix are considered to represent primitive variables and are converted as such."
		"The interpolation type of the variable is guessed, unless the attribute name begins with prefix_?_, in which case the ? is"
		"used to specify type - C for constant, U for uniform, V for Vertex, Y for varying and F for facevarying",
		"delight", // compatibility with 3delight by default
		primVarAttrPrefixPresets
	);
	
	parameters()->addParameter( m_spaceParameter );
	parameters()->addParameter( m_primVarAttrPrefixParameter );

}

IECore::IntParameterPtr FromMayaShapeConverter::spaceParameter()
{
	return m_spaceParameter;
}

IECore::ConstIntParameterPtr FromMayaShapeConverter::spaceParameter() const
{
	return m_spaceParameter;
}

IECore::StringParameterPtr FromMayaShapeConverter::primVarAttrPrefixParameter()
{
	return m_primVarAttrPrefixParameter;
}

IECore::ConstStringParameterPtr FromMayaShapeConverter::primVarAttrPrefixParameter() const
{
	return m_primVarAttrPrefixParameter;
}
		
IECore::ObjectPtr FromMayaShapeConverter::doConversion( const MObject &object, IECore::ConstCompoundObjectPtr operands ) const
{
	IECore::PrimitivePtr p = 0;
	
	const MDagPath *d = dagPath( true );
	if( d )
	{
		p = doPrimitiveConversion( *d, operands );
	}
	else
	{
		p = doPrimitiveConversion( object, operands );
	}
	if( p )
	{
		addPrimVars( object, p );
	}
	return p;
}

void FromMayaShapeConverter::addPrimVars( const MObject &object, IECore::PrimitivePtr primitive ) const
{
	MFnDependencyNode fnNode( object );
	if( !fnNode.hasObj( object ) )
	{
		return;
	}
	
	MString prefix = m_primVarAttrPrefixParameter->getTypedValue().c_str();
	unsigned int n = fnNode.attributeCount();
	for( unsigned int i=0; i<n; i++ )
	{
		MObject attr = fnNode.attribute( i );
		MFnAttribute fnAttr( attr );
		MString attrName = fnAttr.name();
		if( attrName.substring( 0, prefix.length()-1 )==prefix && attrName.length() > prefix.length() )
		{
			MPlug plug = fnNode.findPlug( attr );
			if( !plug.parent().isNull() )
			{
				continue; // we don't want to pick up the children of compound numeric attributes
			}
			MString plugName = plug.name();
			
			// find a converter for the plug, asking for conversion to float types by preference			
			FromMayaConverterPtr converter = FromMayaPlugConverter::create( plug, IECore::FloatDataTypeId );
			if( !converter )
			{
				converter = FromMayaPlugConverter::create( plug, IECore::V3fDataTypeId );
			}
			if( !converter )
			{
				converter = FromMayaPlugConverter::create( plug, IECore::V3fVectorDataTypeId );
			}
			if( !converter )
			{
				converter = FromMayaPlugConverter::create( plug, IECore::FloatVectorDataTypeId );
			}
			if( !converter )
			{
				converter = FromMayaPlugConverter::create( plug );
			}
			
			// run the conversion and check we've got data as a result
			IECore::DataPtr data = 0;
			if( converter )
			{
				 data = IECore::runTimeCast<IECore::Data>( converter->convert() );
			}
			if( !data )
			{
				IECore::msg( IECore::Msg::Warning, "FromMayaShapeConverter::addPrimVars", boost::format( "Attribute \"%s\" could not be converted to Data." ) % plugName.asChar() );
				continue;
			}
			
			// convert V3fData to Color3fData if attribute has usedAsColor() set.
			if( IECore::V3fDataPtr vData = IECore::runTimeCast<IECore::V3fData>( data ) )
			{
				if( fnAttr.isUsedAsColor() )
				{
					Imath::V3f v = vData->readable();
					data = new IECore::Color3fData( Imath::Color3f( v.x, v.y, v.z ) ); 
				}
			}

			// see if interpolation has been specified, and find primitive variable name
			std::string primVarName = attrName.asChar() + prefix.length();
			IECore::PrimitiveVariable::Interpolation interpolation = IECore::PrimitiveVariable::Invalid;
			if( attrName.length()>prefix.length()+3 )
			{
				const char *c = attrName.asChar();
				if( c[prefix.length()]=='_' && c[prefix.length()+2]=='_' )
				{
					char t = c[prefix.length()+1];
					primVarName = attrName.asChar() + prefix.length() + 3;
					switch( t )
					{
						case 'C' :
							interpolation = IECore::PrimitiveVariable::Constant;
							break;
						case 'U' :
							interpolation = IECore::PrimitiveVariable::Uniform;
							break;
						case 'V' :
							interpolation = IECore::PrimitiveVariable::Vertex;
							break;
						case 'Y' :
							interpolation = IECore::PrimitiveVariable::Varying;
							break;
						case 'F' :
							interpolation = IECore::PrimitiveVariable::FaceVarying;
							break;
						default :
							IECore::msg( IECore::Msg::Warning, "FromMayaShapeConverter::addPrimVars", boost::format( "Attribute \"%s\" has unknown interpolation - guessing interpolation." ) % plugName.asChar() );
							break;
					}
				}
			}

			// guess interpolation if not specified
			if( interpolation==IECore::PrimitiveVariable::Invalid )
			{
				interpolation = primitive->inferInterpolation( data );
			}
			
			if( interpolation==IECore::PrimitiveVariable::Invalid )
			{
				IECore::msg( IECore::Msg::Warning, "FromMayaShapeConverter::addPrimVars", boost::format( "Attribute \"%s\" has unsuitable size." ) % plugName.asChar() );
				continue;
			}

			// finally add the primvar
			primitive->variables[primVarName] = IECore::PrimitiveVariable( interpolation, data );

		}
	}


}

MSpace::Space FromMayaShapeConverter::space() const
{
	Space s = (Space)m_spaceParameter->getNumericValue();
	switch( s )
	{
		case Object :
			return MSpace::kObject;
		case World :
			return MSpace::kWorld;
	}
	assert( 0 ); // should never get here
	return MSpace::kObject;
}

const MDagPath *FromMayaShapeConverter::dagPath( bool emitSpaceWarnings ) const
{
	if( m_dagPath.isValid() )
	{
		return &m_dagPath;
	}
	
	if( emitSpaceWarnings && !object().hasFn( MFn::kData ) && space()==MSpace::kWorld )
	{
		IECore::msg( IECore::Msg::Warning, "FromMayaShapeConverter", "World space requested but no dag path provided." );
	}
	
	return 0;
}

FromMayaShapeConverterPtr FromMayaShapeConverter::create( const MDagPath &dagPath, IECore::TypeId resultType )
{
	const ShapeTypesToFnsMap *m = shapeTypesToFns();
	ShapeTypesToFnsMap::const_iterator it = m->find( ShapeTypes( dagPath.apiType(), resultType ) );
	if( it!=m->end() )
	{
		return it->second( dagPath );
	}
	return 0;
}
	
void FromMayaShapeConverter::registerShapeConverter( const MFn::Type fromType, IECore::TypeId resultType, ShapeCreatorFn creator )
{
	ShapeTypesToFnsMap *m = shapeTypesToFns();
	m->insert( ShapeTypesToFnsMap::value_type( ShapeTypes( fromType, resultType ), creator ) );
	m->insert( ShapeTypesToFnsMap::value_type( ShapeTypes( fromType, IECore::InvalidTypeId ), creator ) ); // for the create function which doesn't care about resultType
}

FromMayaShapeConverter::ShapeTypesToFnsMap *FromMayaShapeConverter::shapeTypesToFns()
{
	static ShapeTypesToFnsMap *m = new ShapeTypesToFnsMap;
	return m;
}

