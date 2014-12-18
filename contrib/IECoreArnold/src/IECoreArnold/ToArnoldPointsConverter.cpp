//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, Image Engine Design Inc. All rights reserved.
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

#include "ai.h"

#include "IECore/PointsPrimitive.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/MessageHandler.h"
#include "IECore/CompoundObject.h"

#include "IECoreArnold/ToArnoldPointsConverter.h"

using namespace IECoreArnold;
using namespace IECore;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( ToArnoldPointsConverter );

ToArnoldPointsConverter::ConverterDescription<ToArnoldPointsConverter> ToArnoldPointsConverter::g_description;

ToArnoldPointsConverter::ToArnoldPointsConverter( IECore::PointsPrimitivePtr toConvert )
	:	ToArnoldShapeConverter( "Converts IECore::PointsPrimitives to arnold points nodes", IECore::PointsPrimitive::staticTypeId() )
{
	srcParameter()->setValue( toConvert );
}

ToArnoldPointsConverter::~ToArnoldPointsConverter()
{
}

AtNode *ToArnoldPointsConverter::doConversion( IECore::ConstObjectPtr from, IECore::ConstCompoundObjectPtr operands ) const
{
	const PointsPrimitive *points = static_cast<const PointsPrimitive *>( from.get() );
	const V3fVectorData *p = points->variableData<V3fVectorData>( "P", PrimitiveVariable::Vertex );
	if( !p )
	{
		throw Exception( "PointsPrimitive does not have \"P\" primitive variable of interpolation type Vertex." );
	}
	
	// make the result points and set the positions
	
	AtNode *result = AiNode( "points" );
		
	convertP( p, result, "points" );
	
	// mode
	
	const StringData *t = points->variableData<StringData>( "type", PrimitiveVariable::Constant );
	if( t )
	{
		if( t->readable() == "particle" || t->readable()=="disk" )
		{
			// default type is disk - no need to do anything
		}
		else if( t->readable() == "sphere" )
		{
			AiNodeSetStr( result, "mode", "sphere" );
		}
		else if( t->readable() == "patch" )
		{
			AiNodeSetStr( result, "mode", "quad" );
		}
		else
		{
			IECore::msg( IECore::Msg::Warning, "ToArnoldPointsConverter::doConversion", boost::format( "Unknown type \"%s\" - reverting to disk mode." ) % t->readable() );
		}
	}

	convertRadius( points, result );

	/// \todo Aspect, rotation
	
	// add arbitrary user parameters
	
	const char *ignore[] = { "P", "width", "radius", 0 };
	convertPrimitiveVariables( points, result, ignore );

	return result;
}
