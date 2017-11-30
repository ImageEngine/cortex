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

#include "boost/python.hpp"
#include "boost/python/suite/indexing/container_utils.hpp"

// This must come before the Cortex includes, because on OSX headers included
// by TBB define macros which conflict with the inline functions in ai_types.h.
#include "ai.h"

#include "IECorePython/RefCountedBinding.h"
#include "IECorePython/ScopedGILRelease.h"

#include "IECoreArnold/InstancingConverter.h"
#include "IECoreArnold/bindings/NodeAlgoBinding.h"
#include "IECoreArnold/bindings/InstancingConverterBinding.h"

using namespace std;
using namespace boost::python;
using namespace IECore;
using namespace IECoreScene;
using namespace IECoreArnold;
using namespace IECoreArnoldBindings;

namespace
{

object convertWrapper1( InstancingConverter &c, object pythonSamples, float motionStart, float motionEnd, const std::string &nodeName )
{
	vector<const Primitive *> samples;
	boost::python::container_utils::extend_container( samples, pythonSamples );

	AtNode *node;
	{
		IECorePython::ScopedGILRelease gilRelease;
		node = c.convert( samples, motionStart, motionEnd, nodeName, nullptr );
	}
	return atNodeToPythonObject( node );
}

object convertWrapper2( InstancingConverter &c, object pythonSamples, float motionStart, float motionEnd, const IECore::MurmurHash &h, const std::string &nodeName )
{
	vector<const Primitive *> samples;
	boost::python::container_utils::extend_container( samples, pythonSamples );

	AtNode *node;
	{
		IECorePython::ScopedGILRelease gilRelease;
		node = c.convert( samples, motionStart, motionEnd, h, nodeName, nullptr );
	}
	return atNodeToPythonObject( node );
}

object convertWrapper3( InstancingConverter &c, IECoreScene::Primitive *primitive, const std::string &nodeName )
{
	AtNode *node;
	{
		IECorePython::ScopedGILRelease gilRelease;
		node = c.convert( primitive, nodeName, nullptr );
	}
	return atNodeToPythonObject( node );
}

object convertWrapper4( InstancingConverter &c, IECoreScene::Primitive *primitive, const IECore::MurmurHash &h, const std::string &nodeName )
{
	AtNode *node;
	{
		IECorePython::ScopedGILRelease gilRelease;
		node = c.convert( primitive, h, nodeName, nullptr );
	}
	return atNodeToPythonObject( node );
}

} // namespace

void IECoreArnold::bindInstancingConverter()
{
	IECorePython::RefCountedClass<InstancingConverter, IECore::RefCounted>( "InstancingConverter" )
		.def( init<>() )
		.def( "convert", &convertWrapper1 )
		.def( "convert", &convertWrapper2 )
		.def( "convert", &convertWrapper3 )
		.def( "convert", &convertWrapper4 )
	;
}
