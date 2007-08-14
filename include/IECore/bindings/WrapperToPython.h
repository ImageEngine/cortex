//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_WRAPPERTOPYTHON_H
#define IECORE_WRAPPERTOPYTHON_H

#include <boost/python.hpp>
#include <boost/static_assert.hpp>
#include <cassert>

#include "IECore/bindings/Wrapper.h"
#include "IECore/WrapperGarbageCollectorBase.h"

namespace IECore
{

/// This class is used to register a to_python_converter which
/// ensures that wrapped objects go back into python as the exact
/// PyObject they originated from.
/// \todo This could be merged with the intrusive ptr patch code,
/// but only if we wrap every class.
template<typename TPtr>
struct WrapperToPython
{	
	typedef typename TPtr::element_type T;
	 
	WrapperToPython() 
	{
		using namespace boost::python;
		to_python_converter<TPtr, WrapperToPython<TPtr> >();
	}
	
	virtual ~WrapperToPython()
	{
	}
	
	static PyObject* convert( TPtr const &x )
	{		
		if (!x)
		{
			return Py_None;
		}
		
		PyObject* converted = WrapperGarbageCollectorBase::pyObject( x.get() );	
		if( !converted )
		{
			using namespace boost::python::objects;
			
			converted = class_value_wrapper< 
				TPtr, make_ptr_instance< 
					T, 
					pointer_holder<TPtr, T> 
					>
				>::convert(x);			
		}
		
		assert(converted);

		// Have observed some nasty crashes if we don't INCREF here!		
		Py_INCREF( converted );
		return converted;
	}

};

}

#endif // IECORE_WRAPPERTOPYTHON_H
