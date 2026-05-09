//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2017, Image Engine Design. All rights reserved.
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
//      * Neither the name of Image Engine Design nor the names of
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

#include "IECorePython/RefCountedBinding.h"
#include "IECorePython/RunTimeTypedBinding.h"

#include "IECoreVDB/VDBObject.h"

// OpenVDB 10.1 to 11.x used PyBind11, and OpenVDB 12 onwards uses Nanobind.
// We need to take a different approach to binding VDBObject's grid accessors
// in each case.
#if OPENVDB_LIBRARY_MAJOR_VERSION_NUMBER >= 12
#include "IECorePython/NanobindConverter.h"
#define IECOREVDB_USE_NANOBIND
#elif OPENVDB_LIBRARY_MAJOR_VERSION_NUMBER == 11 || OPENVDB_LIBRARY_MAJOR_VERSION_NUMBER == 10 && OPENVDB_LIBRARY_MINOR_VERSION_NUMBER >= 1
#include "IECorePython/PyBindConverter.h"
#define IECOREVDB_USE_PYBIND
#endif

using namespace boost::python;
using namespace IECoreVDB;

namespace
{

boost::python::list gridNames( VDBObject::Ptr vdbObject )
{
	boost::python::list result;
	std::vector<std::string> names = vdbObject->gridNames();
	for( const auto &name : names )
	{
		result.append( name );
	}
	return result;
}

} // namespace

BOOST_PYTHON_MODULE( _IECoreVDB )
{

#if defined( IECOREVDB_USE_NANOBIND )
	IECorePython::NanobindConverter<openvdb::GridBase::Ptr>::registerConverters();
#elif defined( IECOREVDB_USE_PYBIND )
	IECorePython::PyBindConverter<openvdb::GridBase::Ptr>::registerConverters();
#endif

	IECorePython::RunTimeTypedClass<VDBObject>()
		.def(init<const std::string &>())
		.def(init<>())
		.def("gridNames", &::gridNames)
		.def("metadata", &VDBObject::metadata)
		.def("removeGrid", &VDBObject::removeGrid)
		.def( "findGrid", (openvdb::GridBase::Ptr (VDBObject::*)( const std::string &name ))&VDBObject::findGrid )
		.def( "insertGrid", &VDBObject::insertGrid )
		.def("unmodifiedFromFile", &VDBObject::unmodifiedFromFile)
		.def("fileName", &VDBObject::fileName)
		;

}
