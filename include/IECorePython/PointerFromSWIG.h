//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2010, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREPYTHON_POINTERFROMSWIG_H
#define IECOREPYTHON_POINTERFROMSWIG_H

#include "boost/python.hpp"

#include "IECorePython/Export.h"

namespace IECorePython
{

/// Instantiating one of these registers a from python converter
/// which will extract a pointer to T from a SWIG wrapped object. This allows
/// functions accepting T * to be bound as usual using boost::python, but to
/// operate on objects which were bound with SWIG. The current implementation
/// is inherently dangerous in that it'll happily return a T * from any
/// SWIG object regardless of whether or not that object is holding the right
/// type - for this reason it's best to use it in controlled circumstances.
/// See the IECoreMaya bindings for examples - they use this class to allow
/// interoperation with types held by the Maya Python API.
template<typename T>
class IECOREPYTHON_API PointerFromSWIG
{
	public :

		PointerFromSWIG();

};

} // namespace IECorePython

#include "IECorePython/PointerFromSWIG.inl"

#endif // IECOREPYTHON_POINTERFROMSWIG_H
