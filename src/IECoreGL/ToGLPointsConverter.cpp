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

#include "IECore/Exception.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/MessageHandler.h"
#include "IECoreScene/PointsPrimitive.h"

#include "IECoreGL/ToGLPointsConverter.h"
#include "IECoreGL/PointsPrimitive.h"

using namespace IECoreGL;

IE_CORE_DEFINERUNTIMETYPED( ToGLPointsConverter );

ToGLConverter::ConverterDescription<ToGLPointsConverter> ToGLPointsConverter::g_description;

ToGLPointsConverter::ToGLPointsConverter( IECoreScene::ConstPointsPrimitivePtr toConvert )
	:	ToGLConverter( "Converts IECoreScene::PointsPrimitive objects to IECoreGL::PointsPrimitive objects.", IECore::PointsPrimitiveTypeId )
{
	srcParameter()->setValue( boost::const_pointer_cast<IECoreScene::PointsPrimitive>( toConvert ) );
}

ToGLPointsConverter::~ToGLPointsConverter()
{
}

IECore::RunTimeTypedPtr ToGLPointsConverter::doConversion( IECore::ConstObjectPtr src, IECore::ConstCompoundObjectPtr operands ) const
{
	IECoreScene::PointsPrimitive::ConstPtr pointsPrim = boost::static_pointer_cast<const IECoreScene::PointsPrimitive>( src ); // safe because the parameter validated it for us

	IECore::V3fVectorData::ConstPtr points = pointsPrim->variableData<IECore::V3fVectorData>( "P", IECoreScene::PrimitiveVariable::Vertex );
	if( !points )
	{
		throw IECore::Exception( "Must specify primitive variable \"P\", of type V3fVectorData and interpolation type Vertex." );
	}

	// get type
	PointsPrimitive::Type type = PointsPrimitive::Disk;
	IECore::ConstStringDataPtr t = pointsPrim->variableData<IECore::StringData>( "type", IECoreScene::PrimitiveVariable::Constant );
	if ( !t )
	{
		t = pointsPrim->variableData<IECore::StringData>( "type", IECoreScene::PrimitiveVariable::Uniform );
	}
	if( t )
	{
		if( t->readable()=="particle" || t->readable()=="disk" || t->readable()=="blobby" )
		{
			type = PointsPrimitive::Disk;
		}
		else if( t->readable()=="sphere" )
		{
			type = PointsPrimitive::Sphere;
		}
		else if( t->readable()=="patch" )
		{
			type = PointsPrimitive::Quad;
		}
		else if( t->readable()=="gl:point" )
		{
			type = PointsPrimitive::Point;
		}
		else
		{
			IECore::msg( IECore::Msg::Warning, "ToGLPointsConverter::doConversion", boost::format( "Unknown type \"%s\" - reverting to particle type." ) % t->readable() );
		}
	}

	PointsPrimitive::Ptr result = new PointsPrimitive( type );

	for ( IECoreScene::PrimitiveVariableMap::const_iterator pIt = pointsPrim->variables.begin(); pIt != pointsPrim->variables.end(); ++pIt )
	{
		if( pIt->first == "type" )
		{
			continue;
		}

		if ( pIt->second.data )
		{
			result->addPrimitiveVariable( pIt->first, pIt->second );
		}
		else
		{
			IECore::msg( IECore::Msg::Warning, "ToGLPointsConverter", boost::format( "No data given for primvar \"%s\"" ) % pIt->first );
		}
	}

	return result;
}
