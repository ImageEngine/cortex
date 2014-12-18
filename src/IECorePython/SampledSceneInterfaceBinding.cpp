//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

// This include needs to be the very first to prevent problems with warnings
// regarding redefinition of _POSIX_C_SOURCE
#include "boost/python.hpp"

#include "IECore/SampledSceneInterface.h"
#include "IECorePython/RunTimeTypedBinding.h"

#include "IECorePython/SampledSceneInterfaceBinding.h"

using namespace boost::python;
using namespace IECore;

namespace IECorePython
{

static tuple boundSampleInterval( const SampledSceneInterface &m, double time )
{
	size_t floorIndex, ceilIndex;
	double x = m.boundSampleInterval( time, floorIndex, ceilIndex );
	return make_tuple( x, floorIndex, ceilIndex );
}

static tuple transformSampleInterval( const SampledSceneInterface &m, double time )
{
	size_t floorIndex, ceilIndex;
	double x = m.transformSampleInterval( time, floorIndex, ceilIndex );
	return make_tuple( x, floorIndex, ceilIndex );
}

static tuple attributeSampleInterval( const SampledSceneInterface &m, const SceneInterface::Name &name, double time )
{
	size_t floorIndex, ceilIndex;
	double x = m.attributeSampleInterval( name, time, floorIndex, ceilIndex );
	return make_tuple( x, floorIndex, ceilIndex );
}

static tuple objectSampleInterval( const SampledSceneInterface &m, double time )
{
	size_t floorIndex, ceilIndex;
	double x = m.objectSampleInterval( time, floorIndex, ceilIndex );
	return make_tuple( x, floorIndex, ceilIndex );
}

DataPtr readTransformAtSample( SampledSceneInterface &m, size_t sampleIndex )
{
	ConstDataPtr d = m.readTransformAtSample(sampleIndex);
	if ( d )
	{
		return d->copy();
	}
	return 0;
}

ObjectPtr readAttributeAtSample( SampledSceneInterface &m, const SceneInterface::Name &name, size_t sampleIndex )
{
	ConstObjectPtr o = m.readAttributeAtSample(name,sampleIndex);
	if ( o )
	{
		return o->copy();
	}
	return 0;
}

ObjectPtr readObjectAtSample( SampledSceneInterface &m, size_t sampleIndex )
{
	ConstObjectPtr o = m.readObjectAtSample(sampleIndex);
	if ( o )
	{
		return o->copy();
	}
	return 0;
}

void bindSampledSceneInterface()
{
	RunTimeTypedClass<SampledSceneInterface>()
		.def( "numBoundSamples", &SampledSceneInterface::numBoundSamples )
		.def( "numTransformSamples", &SampledSceneInterface::numTransformSamples )
		.def( "numAttributeSamples", &SampledSceneInterface::numAttributeSamples )
		.def( "numObjectSamples", &SampledSceneInterface::numObjectSamples )
		.def( "boundSampleTime", &SampledSceneInterface::boundSampleTime )
		.def( "transformSampleTime", &SampledSceneInterface::transformSampleTime )
		.def( "attributeSampleTime", &SampledSceneInterface::attributeSampleTime )
		.def( "objectSampleTime", &SampledSceneInterface::objectSampleTime )
		.def( "readBoundAtSample", &SampledSceneInterface::readBoundAtSample )
		.def( "readTransformAtSample", &readTransformAtSample )
		.def( "readTransformAsMatrixAtSample", &SampledSceneInterface::readTransformAsMatrixAtSample )
		.def( "readAttributeAtSample", &readAttributeAtSample )
		.def( "readObjectAtSample", &readObjectAtSample )

		.def( "boundSampleInterval", &boundSampleInterval )
		.def( "transformSampleInterval", &transformSampleInterval )
		.def( "attributeSampleInterval", &attributeSampleInterval )
		.def( "objectSampleInterval", &objectSampleInterval )
	;
}

} // namespace IECorePython
