//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012-2016, Image Engine Design Inc. All rights reserved.
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

#include "IECoreArnold/NodeAlgo.h"
#include "IECoreArnold/ShapeAlgo.h"
#include "IECoreArnold/PointsAlgo.h"

using namespace std;
using namespace IECore;
using namespace IECoreArnold;

//////////////////////////////////////////////////////////////////////////
// Internal utilities
//////////////////////////////////////////////////////////////////////////

namespace
{

NodeAlgo::ConverterDescription<PointsPrimitive> g_description( PointsAlgo::convert, PointsAlgo::convert );

AtNode *convertCommon( const IECore::PointsPrimitive *points )
{

	AtNode *result = AiNode( "points" );

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

	// arbitrary user parameters

	const char *ignore[] = { "P", "width", "radius", nullptr };
	ShapeAlgo::convertPrimitiveVariables( points, result, ignore );

	return result;

}

} // namespace

//////////////////////////////////////////////////////////////////////////
// Implementation of public API
//////////////////////////////////////////////////////////////////////////

namespace IECoreArnold
{

namespace PointsAlgo
{

AtNode *convert( const IECore::PointsPrimitive *points )
{
	AtNode *result = convertCommon( points );

	ShapeAlgo::convertP( points, result, "points" );
	ShapeAlgo::convertRadius( points, result );

	/// \todo Aspect, rotation

	return result;
}

AtNode *convert( const std::vector<const IECore::PointsPrimitive *> &samples, float motionStart, float motionEnd )
{
	AtNode *result = convertCommon( samples.front() );

	std::vector<const IECore::Primitive *> primitiveSamples( samples.begin(), samples.end() );
	ShapeAlgo::convertP( primitiveSamples, result, "points" );
	ShapeAlgo::convertRadius( primitiveSamples, result );

	
	AiNodeSetFlt( result, "motion_start", motionStart );
	AiNodeSetFlt( result, "motion_end", motionEnd );

	/// \todo Aspect, rotation

	return result;
}

} // namespace PointsAlgo

} // namespace IECoreArnold

