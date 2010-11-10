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

#include "boost/python.hpp"
#include "boost/python/suite/indexing/container_utils.hpp"

#include "IECorePython/ScopedGILRelease.h"

#include "IECoreRI/GXEvaluator.h"
#include "IECoreRI/bindings/GXEvaluatorBinding.h"

using namespace boost::python;

namespace IECoreRI
{

static IECore::CompoundDataPtr evaluate1( GXEvaluator &e, const IECore::IntVectorData *faceIndices, const IECore::FloatVectorData *u, const IECore::FloatVectorData *v, const object &primVarNames )
{
	std::vector<std::string> pvn;
	container_utils::extend_container( pvn, primVarNames );
	
	IECorePython::ScopedGILRelease gilRelease;
		
	return e.evaluate( faceIndices, u, v, pvn );
}

static IECore::CompoundDataPtr evaluate2( GXEvaluator &e, const IECore::FloatVectorData *s, const IECore::FloatVectorData *t, const object &primVarNames )
{
	std::vector<std::string> pvn;
	container_utils::extend_container( pvn, primVarNames );
	
	IECorePython::ScopedGILRelease gilRelease;
		
	return e.evaluate( s, t, pvn );
}

void bindGXEvaluator()
{
	class_<GXEvaluator, boost::noncopyable>( "GXEvaluator", no_init )
		.def( init<const IECore::Primitive *>() )
		.def( "numFaces", &GXEvaluator::numFaces )
		.def( "evaluate", &evaluate1 )
		.def( "evaluate", &evaluate2 )
	;
}

} // namespace IECoreRI
