//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2017, Image Engine Design Inc. All rights reserved.
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

#include "IECoreScene/Primitive.h"

#include "IECore/ObjectInterpolator.h"

using namespace IECore;
using namespace IECoreScene;

namespace
{

PrimitivePtr interpolatePrimitive( const Primitive *y0, const Primitive *y1, double x )
{
	if(
		y0->variableSize( PrimitiveVariable::Uniform ) != y1->variableSize( PrimitiveVariable::Uniform ) ||
		y0->variableSize( PrimitiveVariable::Varying ) != y1->variableSize( PrimitiveVariable::Varying ) ||
		y0->variableSize( PrimitiveVariable::Vertex ) != y1->variableSize( PrimitiveVariable::Vertex ) ||
		y0->variableSize( PrimitiveVariable::FaceVarying ) != y1->variableSize( PrimitiveVariable::FaceVarying )
	)
	{
		return nullptr;
	}

	PrimitivePtr result = y0->copy();

	// Interpolate blindData
	CompoundDataPtr interpolatedBlindData = boost::static_pointer_cast<CompoundData>(
		linearObjectInterpolation( y0->blindData(), y1->blindData(), x )
	);
	result->blindData()->writable() = interpolatedBlindData->readable();

	// Interpolate primitive variables
	for( const auto &namedPrimitiveVariable : y0->variables )
	{
		PrimitiveVariableMap::const_iterator it = y1->variables.find( namedPrimitiveVariable.first );
		if( it != y1->variables.end() &&
			it->second.data->typeId() == namedPrimitiveVariable.second.data->typeId() &&
			it->second.interpolation == namedPrimitiveVariable.second.interpolation
		)
		{
			ObjectPtr interpolatedData = linearObjectInterpolation( namedPrimitiveVariable.second.data.get(), it->second.data.get(), x );
			if( interpolatedData )
			{
				result->variables[namedPrimitiveVariable.first].data = boost::static_pointer_cast<Data>( interpolatedData );
			}
		}
	}


	return result;
}

IECore::InterpolatorDescription<IECoreScene::Primitive> g_description( interpolatePrimitive );

} // namespace
