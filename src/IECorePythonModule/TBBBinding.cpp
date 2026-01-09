//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2018, Image Engine Design Inc. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//      * Redistributions of source code must retain the above
//        copyright notice, this list of conditions and the following
//        disclaimer.
//
//      * Redistributions in binary form must reproduce the above
//        copyright notice, this list of conditions and the following
//        disclaimer in the documentation and/or other materials provided with
//        the distribution.
//
//      * Neither the name of Image Engine Design Inc nor the names of
//        any other contributors to this software may be used to endorse or
//        promote products derived from this software without specific prior
//        written permission.
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

#include "TBBBinding.h"

#define TBB_PREVIEW_GLOBAL_CONTROL 1
#include "tbb/global_control.h"

#include <thread>

using namespace boost::python;

namespace
{

class GlobalControlWrapper : public boost::noncopyable
{

	public :

		GlobalControlWrapper( tbb::global_control::parameter parameter, size_t value )
			:	m_parameter( parameter ), m_value( value )
		{
		}

		void enter()
		{
			m_globalControl.reset( new tbb::global_control( m_parameter, m_value ) );
		}

		bool exit( boost::python::object excType, boost::python::object excValue, boost::python::object excTraceBack )
		{
			m_globalControl.reset();
			return false; // don't suppress exceptions
		}

	private :

		const tbb::global_control::parameter m_parameter;
		const size_t m_value;
		std::unique_ptr<tbb::global_control> m_globalControl;

};

} // namespace

void IECorePythonModule::bindTBB()
{

	class_<GlobalControlWrapper, boost::noncopyable> globalControl( "tbb_global_control", no_init );
	{
		scope globalControlScope = globalControl;
		enum_<tbb::global_control::parameter>( "parameter" )
			.value( "max_allowed_parallelism", tbb::global_control::max_allowed_parallelism )
			.value( "thread_stack_size", tbb::global_control::thread_stack_size )
		;
	}
	globalControl
		.def( init<tbb::global_control::parameter, size_t>() )
		.def( "__enter__", &GlobalControlWrapper::enter, return_self<>() )
		.def( "__exit__", &GlobalControlWrapper::exit )
		.def( "active_value", (size_t (*)(tbb::global_control::parameter))&tbb::global_control::active_value )
		.staticmethod( "active_value" )
	;

	def( "hardwareConcurrency", &std::thread::hardware_concurrency );

}

