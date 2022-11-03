//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2022, Image Engine Design Inc. All rights reserved.
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

#include "IECoreNuke/LiveSceneKnob.h"

#include "IECorePython/ScopedGILLock.h"

using namespace IECoreNuke;
using namespace DD::Image;
using namespace boost::python;

LiveSceneKnob::LiveSceneKnob( DD::Image::Knob_Closure* f, IECoreNuke::LiveSceneHolder* op, const char *name, const char *label )
	:	DD::Image::Knob( f, name, label ), m_value( nullptr ), m_op(op)
{

	set_flag( NO_ANIMATION );

	// set up the object that will provide the python binding
	IECorePython::ScopedGILLock gilLock;
	Detail::PythonLiveSceneKnobPtr pythonKnob = new Detail::PythonLiveSceneKnob;
	pythonKnob->sceneKnob = this;
	object pythonKnobLiveScene( pythonKnob );
	Py_INCREF( pythonKnobLiveScene.ptr() );
	setPyObject( pythonKnobLiveScene.ptr() );
}

LiveSceneKnob::~LiveSceneKnob()
{
	// tidy up the object for the python binding
	IECorePython::ScopedGILLock gilLock;
	object pythonKnobLiveScene( handle<>( borrowed( (PyObject *)pyObject() ) ) );
	Detail::PythonLiveSceneKnobPtr pythonKnob = extract<Detail::PythonLiveSceneKnobPtr>( pythonKnobLiveScene );
	pythonKnob->sceneKnob = nullptr;
	Py_DECREF( pythonKnobLiveScene.ptr() );
}

IECoreNuke::LiveScenePtr LiveSceneKnob::getValue()
{
	if( auto geoOp = dynamic_cast<DD::Image::GeoOp*>( m_op ) )
	{
		geoOp->validate(true);
		m_value.reset();
		m_value = new IECoreNuke::LiveScene( m_op );
	}
	return m_value;
}

LiveSceneKnob *LiveSceneKnob::sceneKnob( DD::Image::Knob_Callback f, IECoreNuke::LiveSceneHolder* op, const char *name, const char *label )
{
	return CustomKnob2( LiveSceneKnob, f, op, name, label );
}

const char *LiveSceneKnob::Class() const
{
	return "LiveSceneKnob";
}
