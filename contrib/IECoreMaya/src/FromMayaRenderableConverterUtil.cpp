//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#include "IECoreMaya/FromMayaRenderableConverterUtil.h"
#include "IECoreMaya/FromMayaPlugConverter.h"

#include "IECore/CompoundParameter.h"
#include "IECore/CompoundData.h"
#include "IECore/MessageHandler.h"

#include "maya/MFn.h"
#include "maya/MFnAttribute.h"
#include "maya/MString.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MFnDagNode.h"
#include "maya/MPlug.h"
#include "maya/MStringArray.h"

#include <string>
#include <algorithm>

using namespace IECoreMaya;
using namespace IECore;
using namespace std;

FromMayaRenderableConverterUtil::FromMayaRenderableConverterUtil( )
	:	Converter( "FromMayaRenderableConverterUtil", "Helps converting renderable objects from Maya scene." )
{
	// blindData vars parameter
	StringParameter::PresetsMap blindDataAttrPrefixPresets;
	blindDataAttrPrefixPresets["ie"] = "ie";
	blindDataAttrPrefixPresets["None"] = "";
	m_blindDataAttrPrefix = new StringParameter(
		"blindDataAttrPrefix",
		"Any attribute names beginning with this prefix will be added to the blindData dictionary on the converted object.",
		"ie", // image engine prefix by default.
		blindDataAttrPrefixPresets
	);

	// parameter that sets the prefix
	m_removeNamespace = new BoolParameter(
		"removeNamespace",
		"Removes namespace when saving names to blindData.",
		true
	);

	parameters()->addParameter( m_removeNamespace );
	parameters()->addParameter( m_blindDataAttrPrefix );

}

void FromMayaRenderableConverterUtil::addBlindDataAttributes( IECore::ConstCompoundObjectPtr operands, const MObject &object, IECore::RenderablePtr renderable )
{
	CompoundDataMap &blindData = renderable->blindData()->writable();
	IECore::CompoundObject::ObjectMap::const_iterator it = operands->members().find("blindDataAttrPrefix");
	if ( it == operands->members().end() )
	{
		return;
	}
	ObjectPtr paramPtr = it->second;
	MString blindPrefix;
	blindPrefix = boost::static_pointer_cast< StringData >(paramPtr)->readable().c_str();

	it = operands->members().find("removeNamespace");
	if ( it == operands->members().end() )
	{
		return;
	}
	paramPtr = it->second;
	bool ignoreNamespace = boost::static_pointer_cast< BoolData >(paramPtr)->readable();

	MFnDependencyNode fnNode( object );
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
