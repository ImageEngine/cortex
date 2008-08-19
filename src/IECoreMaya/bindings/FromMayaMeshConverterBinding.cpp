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

#include "IECoreMaya/FromMayaMeshConverter.h"
#include "IECoreMaya/bindings/FromMayaMeshConverterBinding.h"

#include "IECore/bindings/IntrusivePtrPatch.h"
#include "IECore/bindings/RunTimeTypedBinding.h"

using namespace IECoreMaya;
using namespace boost::python;

void IECoreMaya::bindFromMayaMeshConverter()
{
	typedef class_<FromMayaMeshConverter, FromMayaMeshConverterPtr, boost::noncopyable, bases<FromMayaShapeConverter> > FromMayaMeshConverterPyClass;

	FromMayaMeshConverterPyClass( "FromMayaMeshConverter", no_init )
		.IE_COREPYTHON_DEFRUNTIMETYPEDSTATICMETHODS( FromMayaMeshConverter )
		/// \todo All of these should be replaced by bindings with the trailing underscore removed.
		/// They're only bound this way because the names currently conflict with the names of parameters.
		/// Once we've done the todo in CompoundParameterBinding (get rid of the __getattr__ for child access)
		/// we can convert these to the proper form and they won't mask parameters any more (parameters will be
		/// accessed using [] syntax instead).
		.def( "points_", &FromMayaMeshConverter::points )
		.def( "normals_", &FromMayaMeshConverter::normals )
		.def( "s_", &FromMayaMeshConverter::s )
		.def( "t_", &FromMayaMeshConverter::t )
	;
	
	INTRUSIVE_PTR_PATCH( FromMayaMeshConverter, FromMayaMeshConverterPyClass );
	implicitly_convertible<FromMayaMeshConverterPtr, FromMayaShapeConverterPtr>();
}
