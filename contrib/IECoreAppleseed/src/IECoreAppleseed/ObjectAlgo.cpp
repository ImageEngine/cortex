//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2016, Esteban Tovagliari. All rights reserved.
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

#include "IECoreAppleseed/ObjectAlgo.h"

#include "IECoreAppleseed/MeshAlgo.h"
#include "IECoreAppleseed/MotionAlgo.h"

#include "IECore/Exception.h"
#include "IECoreScene/MeshPrimitive.h"

using namespace IECore;

namespace IECoreAppleseed
{

namespace ObjectAlgo
{

bool isPrimitiveSupported( const Object *primitive )
{
	return runTimeCast<const IECoreScene::MeshPrimitive>( primitive );
}

renderer::Object *convert( const Object *primitive )
{
	if( !isPrimitiveSupported( primitive ) )
	{
		throw Exception( "AppleseedRenderer: Unsupported primitive" );
	}

	return MeshAlgo::convert( primitive );
}

renderer::Object *convert( const std::vector<const Object *> &samples, const std::vector<float> &times, float shutterOpenTime, float shutterCloseTime )
{
	if( !isPrimitiveSupported( samples[0] ) )
	{
		throw Exception( "Unsupported primitive" );
	}

	const Object *firstSample = samples.front();
	const TypeId firstSampleTypeId = firstSample->typeId();
	for( std::vector<const Object *>::const_iterator it = samples.begin()+1, eIt = samples.end(); it != eIt; ++it )
	{
		if( (*it)->typeId() != firstSampleTypeId )
		{
			throw Exception( "Inconsistent object types." );
		}
	}

	if( !MotionAlgo::checkTimeSamples( times, shutterOpenTime, shutterCloseTime ) )
	{
		std::vector<ObjectPtr> resampled;
		MotionAlgo::resamplePrimitiveKeys( samples, times, shutterOpenTime, shutterCloseTime, resampled );
		return MeshAlgo::convert( resampled );
	}
	else
	{
		return MeshAlgo::convert( samples );
	}
}

} // namespace ObjectAlgo

} // namespace IECoreAppleseed
