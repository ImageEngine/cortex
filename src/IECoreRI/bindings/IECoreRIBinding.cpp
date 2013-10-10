//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2012, Image Engine Design Inc. All rights reserved.
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

#include <boost/python.hpp>

#include "IECoreRI/IECoreRI.h"

#include "IECoreRI/bindings/RendererBinding.h"
#include "IECoreRI/bindings/SLOReaderBinding.h"

#include "IECoreRI/bindings/PTCParticleReaderBinding.h"
#include "IECoreRI/bindings/PTCParticleWriterBinding.h"
#include "IECoreRI/bindings/RIBWriterBinding.h"
#include "IECoreRI/bindings/SXRendererBinding.h"
#include "IECoreRI/bindings/GXEvaluatorBinding.h"
#include "IECoreRI/bindings/DTEXDeepImageReaderBinding.h"
#include "IECoreRI/bindings/DTEXDeepImageWriterBinding.h"
#include "IECoreRI/bindings/SHWDeepImageReaderBinding.h"
#include "IECoreRI/bindings/SHWDeepImageWriterBinding.h"

using namespace IECoreRI;
using namespace boost::python;

BOOST_PYTHON_MODULE( _IECoreRI )
{
	bindRenderer();
	bindSLOReader();
#ifdef IECORERI_WITH_PTC
	bindPTCParticleReader();
	bindPTCParticleWriter();
#endif // IECORERI_WITH_PTC
	bindRIBWriter();
#ifdef IECORERI_WITH_SX
	bindSXRenderer();	
#endif // IECORERI_WITH_SX
#ifdef IECORERI_WITH_GX
	bindGXEvaluator();	
#endif // IECORERI_WITH_GX
#ifdef IECORERI_WITH_RIXDEEP
	bindDTEXDeepImageReader();
	bindDTEXDeepImageWriter();
#endif // IECORERI_WITH_RIXDEEP
#ifdef IECORERI_WITH_DEEPSHW
	bindSHWDeepImageReader();
	bindSHWDeepImageWriter();
#endif // IECORERI_WITH_DEEPSHW

	def( "withRiProceduralV", &IECoreRI::withRiProceduralV );


}
