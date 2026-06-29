//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#include "IECoreNuke/ToNukeGeometryConverter.h"
#include "IECoreNuke/Convert.h"
#include "IECoreNuke/LiveScene.h"

#include "IECore/CompoundData.h"
#include "IECore/MessageHandler.h"
#include "IECore/CompoundObject.h"
#include "IECore/CompoundParameter.h"
#include "IECore/SimpleTypedData.h"

#include <cassert>

#include "boost/format.hpp"

using namespace IECoreNuke;
using namespace IECore;
using namespace DD::Image;

namespace
{

void writeAttribute( DD::Image::GeometryList &geoList, int objIndex, const char *name, const IECore::Object *value )
{
	switch( value->typeId() )
	{
		case IECore::FloatDataTypeId :
		{
			auto attr = geoList.writable_attribute( objIndex, GroupType::Group_Object, name, AttribType::FLOAT_ATTRIB );
			attr->flt() = static_cast<const IECore::FloatData *>( value )->readable();
			break;
		}
		case IECore::DoubleDataTypeId :
		{
			// Nuke only supports float attributes, so we narrow double to float.
			auto attr = geoList.writable_attribute( objIndex, GroupType::Group_Object, name, AttribType::FLOAT_ATTRIB );
			attr->flt() = static_cast<float>( static_cast<const IECore::DoubleData *>( value )->readable() );
			break;
		}
		case IECore::IntDataTypeId :
		{
			auto attr = geoList.writable_attribute( objIndex, GroupType::Group_Object, name, AttribType::INT_ATTRIB );
			attr->integer() = static_cast<const IECore::IntData *>( value )->readable();
			break;
		}
		case IECore::BoolDataTypeId :
		{
			auto attr = geoList.writable_attribute( objIndex, GroupType::Group_Object, name, AttribType::INT_ATTRIB );
			attr->integer() = static_cast<const IECore::BoolData *>( value )->readable() ? 1 : 0;
			break;
		}
		case IECore::StringDataTypeId :
		{
			auto attr = geoList.writable_attribute( objIndex, GroupType::Group_Object, name, AttribType::STD_STRING_ATTRIB );
			attr->stdstring() = static_cast<const IECore::StringData *>( value )->readable();
			break;
		}
		case IECore::V2fDataTypeId :
		{
			auto attr = geoList.writable_attribute( objIndex, GroupType::Group_Object, name, AttribType::VECTOR2_ATTRIB );
			const auto &v = static_cast<const IECore::V2fData *>( value )->readable();
			attr->vector2() = DD::Image::Vector2( v.x, v.y );
			break;
		}
		case IECore::V3fDataTypeId :
		{
			auto attr = geoList.writable_attribute( objIndex, GroupType::Group_Object, name, AttribType::VECTOR3_ATTRIB );
			attr->vector3() = IECore::convert<DD::Image::Vector3>( static_cast<const IECore::V3fData *>( value )->readable() );
			break;
		}
		case IECore::Color3fDataTypeId :
		{
			auto attr = geoList.writable_attribute( objIndex, GroupType::Group_Object, name, AttribType::VECTOR3_ATTRIB );
			const auto &c = static_cast<const IECore::Color3fData *>( value )->readable();
			attr->vector3() = DD::Image::Vector3( c[0], c[1], c[2] );
			break;
		}
		case IECore::Color4fDataTypeId :
		{
			auto attr = geoList.writable_attribute( objIndex, GroupType::Group_Object, name, AttribType::VECTOR4_ATTRIB );
			const auto &c = static_cast<const IECore::Color4fData *>( value )->readable();
			attr->vector4() = DD::Image::Vector4( c.r, c.g, c.b, c.a );
			break;
		}
		case IECore::M33fDataTypeId :
		{
			auto attr = geoList.writable_attribute( objIndex, GroupType::Group_Object, name, AttribType::MATRIX3_ATTRIB );
			const auto &m = static_cast<const IECore::M33fData *>( value )->readable();
			DD::Image::Matrix3 result;
			for( int i = 0; i < 3; i++ )
			{
				for( int j = 0; j < 3; j++ )
				{
					result[j][i] = m[j][i];
				}
			}
			attr->matrix3() = result;
			break;
		}
		case IECore::M44fDataTypeId :
		{
			auto attr = geoList.writable_attribute( objIndex, GroupType::Group_Object, name, AttribType::MATRIX4_ATTRIB );
			attr->matrix4() = IECore::convert<DD::Image::Matrix4>( Imath::M44d( static_cast<const IECore::M44fData *>( value )->readable() ) );
			break;
		}
		case IECore::M44dDataTypeId :
		{
			auto attr = geoList.writable_attribute( objIndex, GroupType::Group_Object, name, AttribType::MATRIX4_ATTRIB );
			attr->matrix4() = IECore::convert<DD::Image::Matrix4>( static_cast<const IECore::M44dData *>( value )->readable() );
			break;
		}
		case IECore::M33dDataTypeId :
		{
			auto attr = geoList.writable_attribute( objIndex, GroupType::Group_Object, name, AttribType::MATRIX3_ATTRIB );
			const auto &m = static_cast<const IECore::M33dData *>( value )->readable();
			DD::Image::Matrix3 result;
			for( int i = 0; i < 3; i++ )
			{
				for( int j = 0; j < 3; j++ )
				{
					result[j][i] = m[j][i];
				}
			}
			attr->matrix3() = result;
			break;
		}
		// Nuke has no equivalent for InternedString types, so we silently skip them.
		// We could convert InternedStringVectorData to a delimited string, but
		// round-tripping wouldn't be transparent — LiveScene would need special
		// handling to split the string back into a vector.
		case IECore::InternedStringDataTypeId :
		case IECore::InternedStringVectorDataTypeId :
			break;
		default :
			IECore::msg( IECore::Msg::Warning, "ToNukeGeometryConverter", boost::format( "Unsupported attribute type \"%s\" for \"%s\"" ) % value->typeName() % name );
			break;
	}
}

} // namespace

IE_CORE_DEFINERUNTIMETYPED( ToNukeGeometryConverter );

ToNukeGeometryConverter::ToNukeGeometryConverter( const std::string &description, IECore::TypeId fromType, ConstObjectPtr object )
	:	ToNukeConverter( description, fromType )
{
	srcParameter()->setValue( boost::const_pointer_cast<Object>( object ) );

	m_objIndexParameter = new IntParameter( "objIndex", "Index for the first object inserted on the GeometryList. Use -1 to simply add on the next index available", -1 );
	parameters()->addParameter( m_objIndexParameter );

	m_pathParameter = new StringParameter( "path", "The object path in the hierarchy.", new StringData() );
	parameters()->addParameter( m_pathParameter );

	m_attributesParameter = new CompoundObjectParameter( "attributes", "Scene attributes to write as Group_Object attributes.", new CompoundObject() );
	parameters()->addParameter( m_attributesParameter );

}

void ToNukeGeometryConverter::convert( GeometryList &geoList ) const
{
	int objIndex = m_objIndexParameter->getNumericValue();
	if ( objIndex == -1 )
	{
		objIndex = (int)geoList.objects();
	}
	geoList.add_object(objIndex);
	
	// add path attribute
	auto nameAttribute = geoList.writable_attribute( objIndex, GroupType::Group_Object, IECoreNuke::LiveScene::nameAttribute.data(), AttribType::STD_STRING_ATTRIB);
	nameAttribute->stdstring() = m_pathParameter->getTypedValue();

	// add scene attributes as Group_Object attributes
	ConstCompoundObjectPtr attributes = m_attributesParameter->getTypedValue<CompoundObject>();
	for( const auto &attr : attributes->members() )
	{
		writeAttribute( geoList, objIndex, attr.first.c_str(), attr.second.get() );
	}

	ConstCompoundObjectPtr operands = parameters()->getTypedValidatedValue<CompoundObject>();
	doConversion( srcParameter()->getValidatedValue(), geoList, objIndex, operands.get() );
}

/////////////////////////////////////////////////////////////////////////////////
// Factory
/////////////////////////////////////////////////////////////////////////////////

ToNukeGeometryConverterPtr ToNukeGeometryConverter::create( ConstObjectPtr object )
{
	const TypesToFnsMap *m = typesToFns();
	TypesToFnsMap::const_iterator it = m->find( Types( object->typeId() ) );
	if( it!=m->end() )
	{
		return it->second( object );
	}
	return 0;
}

void ToNukeGeometryConverter::registerConverter( IECore::TypeId fromType, CreatorFn creator )
{
	TypesToFnsMap *m = typesToFns();
	m->insert( TypesToFnsMap::value_type( Types( fromType ), creator ) );
}

ToNukeGeometryConverter::TypesToFnsMap *ToNukeGeometryConverter::typesToFns()
{
	static TypesToFnsMap *m = new TypesToFnsMap;
	return m;
}

/////////////////////////////////////////////////////////////////////////////////
// Implementation of nested Types class
/////////////////////////////////////////////////////////////////////////////////

ToNukeGeometryConverter::Types::Types( IECore::TypeId from )
	:	fromType( from )
{
}

bool ToNukeGeometryConverter::Types::operator < ( const Types &other ) const
{
	return fromType < other.fromType;
}
