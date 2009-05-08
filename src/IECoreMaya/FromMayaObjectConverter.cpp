//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#include "IECoreMaya/FromMayaObjectConverter.h"
#include "IECoreMaya/FromMayaPlugConverter.h"

#include "IECore/CompoundParameter.h"
#include "IECore/BlindDataHolder.h"
#include "IECore/MessageHandler.h"

#include "maya/MFnDependencyNode.h"
#include "maya/MStringArray.h"
#include "maya/MFnAttribute.h"

using namespace IECoreMaya;
using namespace IECore;

IE_CORE_DEFINERUNTIMETYPED( FromMayaObjectConverter );

FromMayaObjectConverter::FromMayaObjectConverter( const std::string &name, const std::string &description, const MObject &object )
	:	FromMayaConverter( name, description ), m_objectHandle( object )
{

	StringParameter::PresetsContainer blindDataAttrPrefixPresets;
	blindDataAttrPrefixPresets.push_back( StringParameter::Preset( "ie", "ie" ) );
	blindDataAttrPrefixPresets.push_back( StringParameter::Preset( "None", "" ) );
	m_blindDataAttrPrefixParameter = new StringParameter(
		"blindDataAttrPrefix",
		"Any attribute names beginning with this prefix will be added to the blindData dictionary on the converted object. "
		"Note that this parameter is only valid if the object being converted is a node, and the converted object is capable "
		"of holding the blind data.",
		"ie", // image engine prefix by default. maybe change this if cortex gets adopted elsewhere.
		blindDataAttrPrefixPresets
	);

	m_blindDataRemoveNamespaceParameter = new BoolParameter(
		"blindDataRemoveNamespace",
		"In addition to converting attributes to blind data on the converted object, the node name is "
		"saved as an additional piece of blind data. If this parameter is set, then the maya namespace "
		"will be removed from the name before saving.",
		true
	);

	parameters()->addParameter( m_blindDataAttrPrefixParameter );
	parameters()->addParameter( m_blindDataRemoveNamespaceParameter );

}

/////////////////////////////////////////////////////////////////////////////////
// Object to convert
/////////////////////////////////////////////////////////////////////////////////


const MObject &FromMayaObjectConverter::object() const
{
	if( objectIsAlive() )
	{
		return m_objectHandle.objectRef();
	}
	return MObject::kNullObj;
}

bool FromMayaObjectConverter::objectIsAlive() const
{
	return m_objectHandle.isAlive();
}

/////////////////////////////////////////////////////////////////////////////////
// Conversion
/////////////////////////////////////////////////////////////////////////////////

IECore::ObjectPtr FromMayaObjectConverter::doConversion( IECore::ConstCompoundObjectPtr operands ) const
{
	if( !objectIsAlive() )
	{
		return 0;
	}
	IECore::ObjectPtr converted = doConversion( object(), operands );
	addBlindData( object(), converted );
	return converted;
}

void FromMayaObjectConverter::addBlindData( const MObject &object, IECore::ObjectPtr convertedObject ) const
{
	IECore::BlindDataHolderPtr blindDataHolder = IECore::runTimeCast<BlindDataHolder>( convertedObject );
	if( !blindDataHolder )
	{
		return;
	}

	MFnDependencyNode fnNode( object );
	if( !fnNode.hasObj( object ) )
	{
		return;
	}

	CompoundDataMap &blindData = blindDataHolder->blindData()->writable();

	MString blindPrefix = m_blindDataAttrPrefixParameter->getTypedValue().c_str();
	bool ignoreNamespace = m_blindDataRemoveNamespaceParameter->getTypedValue();

	unsigned int n = fnNode.attributeCount();
	std::string objectName;
	objectName = fnNode.name().asChar();

	// eliminate namespace from name...
	if ( ignoreNamespace )
	{
		MStringArray parts;
		if ( fnNode.name().split( ':', parts ) )
		{
			objectName = parts[ parts.length() - 1 ].asChar();
		}
	}
	blindData[ "name" ] = new StringData( objectName );

	if (blindPrefix == "")
	{
		// empty string matches no attributes.
		return;
	}

	for( unsigned int i=0; i<n; i++ )
	{
		MObject attr = fnNode.attribute( i );
		MFnAttribute fnAttr( attr );
		MString attrName = fnAttr.name();
		if( attrName.length() > blindPrefix.length() && attrName.substring( 0, blindPrefix.length()-1 )==blindPrefix )
		{
			MPlug plug = fnNode.findPlug( attr );
			if( !plug.parent().isNull() )
			{
				continue; // we don't want to pick up the children of compound numeric attributes
			}
			MString plugName = plug.name();

			// find a converter for the plug
			FromMayaConverterPtr converter = FromMayaPlugConverter::create( plug );

			// run the conversion and check we've got data as a result
			DataPtr data = 0;
			if( converter )
			{
				 data = runTimeCast<Data>( converter->convert() );
			}
			if( !data )
			{
				msg( Msg::Warning, "FromMayaRenderableConverterUtil::addBlindDataAttributes", boost::format( "Attribute \"%s\" could not be converted to Data." ) % plugName.asChar() );
				continue;
			}
			blindData[ attrName.asChar() ] = data;
		}
	}


}

/////////////////////////////////////////////////////////////////////////////////
// Parameters
/////////////////////////////////////////////////////////////////////////////////

IECore::StringParameterPtr FromMayaObjectConverter::blindDataAttrPrefixParameter()
{
	return m_blindDataAttrPrefixParameter;
}

IECore::ConstStringParameterPtr FromMayaObjectConverter::blindDataAttrPrefixParameter() const
{
	return m_blindDataAttrPrefixParameter;
}

IECore::BoolParameterPtr FromMayaObjectConverter::blindDataRemoveNamespaceParameter()
{
	return m_blindDataRemoveNamespaceParameter;
}
IECore::ConstBoolParameterPtr FromMayaObjectConverter::blindDataRemoveNamespaceParameter() const
{
	return m_blindDataRemoveNamespaceParameter;
}

/////////////////////////////////////////////////////////////////////////////////
// Factory
/////////////////////////////////////////////////////////////////////////////////

FromMayaObjectConverterPtr FromMayaObjectConverter::create( const MObject &object, IECore::TypeId resultType )
{
	const TypesToFnsMap *m = typesToFns();

	TypesToFnsMap::const_iterator it = m->find( Types( object.apiType(), resultType ) );
	if( it!=m->end() )
	{
		return it->second( object );
	}

	return 0;
}

void FromMayaObjectConverter::registerConverter( const MFn::Type fromType, IECore::TypeId resultType, CreatorFn creator )
{
	TypesToFnsMap *m = typesToFns();
	m->insert( TypesToFnsMap::value_type( Types( fromType, resultType ), creator ) );
	m->insert( TypesToFnsMap::value_type( Types( fromType, IECore::InvalidTypeId ), creator ) ); // for the create function which doesn't care about resultType
}

FromMayaObjectConverter::TypesToFnsMap *FromMayaObjectConverter::typesToFns()
{
	static TypesToFnsMap *m = new TypesToFnsMap;
	return m;
}

/////////////////////////////////////////////////////////////////////////////////
// Implementation of nested Types class
/////////////////////////////////////////////////////////////////////////////////

FromMayaObjectConverter::Types::Types( MFn::Type from, IECore::TypeId result )
	:	fromType( from ), resultType( result )
{
}

bool FromMayaObjectConverter::Types::operator < ( const Types &other ) const
{
	if( fromType != other.fromType )
	{
		return fromType < other.fromType;
	}
	else
	{
		return resultType < other.resultType;
	}
}
