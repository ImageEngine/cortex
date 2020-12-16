//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2020, Cinesite VFX Ltd. All rights reserved.
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

#include "IECoreScene/Camera.h"

#include "IECore/DataAlgo.h"
#include "IECore/ObjectInterpolator.h"

using namespace IECore;
using namespace IECoreScene;

namespace
{

CameraPtr interpolateCamera( const Camera *c0, const Camera *c1, double x )
{
	CameraPtr result = c0->copy();

	// Blind data

	CompoundDataPtr interpolatedBlindData = boost::static_pointer_cast<CompoundData>(
		linearObjectInterpolation( c0->blindData(), c1->blindData(), x )
	);
	result->blindData()->writable() = interpolatedBlindData->readable();

	// Parameters

	CompoundDataPtr interpolatedParameters = boost::static_pointer_cast<CompoundData>(
		linearObjectInterpolation( c0->parametersData(), c1->parametersData(), x )
	);
	result->parameters() = interpolatedParameters->readable();

	return result;
}

IECore::InterpolatorDescription<IECoreScene::Camera> g_description( interpolateCamera );

} // namespace
