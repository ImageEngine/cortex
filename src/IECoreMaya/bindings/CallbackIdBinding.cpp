//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#include "IECoreMaya/bindings/CallbackIdBinding.h"
#include "IECoreMaya/CallbackId.h"

#include "IECore/bindings/PointerFromSWIG.h"

using namespace boost::python;

namespace IECoreMaya
{

class CallbackIdWrapper : public CallbackId
{

	public :
	
		CallbackIdWrapper( PyObject *id )
			// extract the MCallbackId from the SWIG object we're being passed. This isn't
			// typesafe in any way - calling with anything other than the correct type
			// is likely to explode. i haven't yet found a good way of checking the type.
			:	CallbackId( *(MCallbackId *)(((IECore::Detail::PySwigObject *)id)->ptr) )
		{
			// if we allow a bound MCallbackId to die in python then we get this error printing out :
			//
			// 		"swig/python detected a memory leak of type 'MCallbackId *', no destructor found".
			//
			// this appears to be due to a bug in maya's bindings for MCallbackId. we increment a reference here so the
			// object will never die and the message will never appear. this is far from ideal, but the
			// test in CallbackIdTest.py verifies that this doesn't cause the callback to leak (this could
			// be a big deal as it could be a member function on a large object). according to the message
			// the MCallbackId is going to leak anyway, so we're not making matters any worse.
			/// \todo Remove this when a future maya version fixes the bug.
			Py_INCREF( id );
		}
	
};

void bindCallbackId()
{

	class_<CallbackIdWrapper, boost::noncopyable>( "CallbackId", init<PyObject *>() )
	;

}

} // namespace IECoreMaya
