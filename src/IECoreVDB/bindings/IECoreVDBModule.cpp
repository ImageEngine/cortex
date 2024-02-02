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

// OpenVDB 10.0 and earlier used `boost::python` for its Python bindings, but
// this was switched to PyBind11 in OpenVDB 10.1. We need to take a different
// approach to binding VDBObject's grid accessors in each case.
#if OPENVDB_LIBRARY_MAJOR_VERSION_NUMBER > 10 || OPENVDB_LIBRARY_MAJOR_VERSION_NUMBER == 10 && OPENVDB_LIBRARY_MINOR_VERSION_NUMBER >= 1
#include "IECorePython/PyBindConverter.h"
#define IECOREVDB_USE_PYBIND
#endif

using namespace boost::python;
using namespace IECoreVDB;

#ifndef IECOREVDB_USE_PYBIND

namespace
{

// all functions in iepyopenvdb namespace are from the pyopenvdb bindings
namespace iepyopenvdb
{

// openvdb/python/pyutil.h
std::string className( boost::python::object obj )
{
	std::string s = boost::python::extract<std::string>( obj.attr( "__class__" ).attr( "__name__" ) );
	return s;
}

// openvdb/python/pyGrid.h
boost::python::object getPyObjectFromGrid( const openvdb::GridBase::Ptr &grid )
{
	if( !grid )
	{
		return boost::python::object();
	}

#define CONVERT_BASE_TO_GRID( GridType, grid ) \
        if (grid->isType<GridType>()) { \
            return boost::python::object(openvdb::gridPtrCast<GridType>(grid)); \
        }

	CONVERT_BASE_TO_GRID( openvdb::FloatGrid, grid );
	CONVERT_BASE_TO_GRID( openvdb::Vec3SGrid, grid );
	CONVERT_BASE_TO_GRID( openvdb::BoolGrid, grid );
	CONVERT_BASE_TO_GRID( openvdb::DoubleGrid, grid );
	CONVERT_BASE_TO_GRID( openvdb::Int32Grid, grid );
	CONVERT_BASE_TO_GRID( openvdb::Int64Grid, grid );
	CONVERT_BASE_TO_GRID( openvdb::Vec3IGrid, grid );
	CONVERT_BASE_TO_GRID( openvdb::Vec3DGrid, grid );

#undef CONVERT_BASE_TO_GRID

	throw openvdb::TypeError( grid->type() + " is not a supported OpenVDB grid type" );
}

// openvdb/python/pyGrid.h
openvdb::GridBase::Ptr getGridFromPyObject( const boost::python::object &gridObj )
{
	if( !gridObj )
	{
		return openvdb::GridBase::Ptr();
	}

#define CONVERT_GRID_TO_BASE( GridPtrType ) \
        { \
            boost::python::extract<GridPtrType> x(gridObj); \
            if (x.check()) return x(); \
        }

	// Extract a grid pointer of one of the supported types
	// from the input object, then cast it to a base pointer.
	CONVERT_GRID_TO_BASE( openvdb::FloatGrid::Ptr );
	CONVERT_GRID_TO_BASE( openvdb::Vec3SGrid::Ptr );
	CONVERT_GRID_TO_BASE( openvdb::BoolGrid::Ptr );
	CONVERT_GRID_TO_BASE( openvdb::DoubleGrid::Ptr );
	CONVERT_GRID_TO_BASE( openvdb::Int32Grid::Ptr );
	CONVERT_GRID_TO_BASE( openvdb::Int64Grid::Ptr );
	CONVERT_GRID_TO_BASE( openvdb::Vec3IGrid::Ptr );
	CONVERT_GRID_TO_BASE( openvdb::Vec3DGrid::Ptr );

#undef CONVERT_GRID_TO_BASE

	throw openvdb::TypeError( className( gridObj ) + " is not a supported OpenVDB grid type" );
}

} // namespace iepyopenvdb

boost::python::object findGrid( VDBObject::Ptr vdbObject, const std::string &gridName )
{
	openvdb::GridBase::Ptr grid = vdbObject->findGrid( gridName );
	if( grid )
	{
		return iepyopenvdb::getPyObjectFromGrid( grid );
	}
	else
	{
		return boost::python::object();
	}
}

void insertGrid( VDBObject::Ptr vdbObject, boost::python::object pyObject )
{
	openvdb::GridBase::Ptr gridPtr = iepyopenvdb::getGridFromPyObject( pyObject );
	vdbObject->insertGrid( gridPtr );
}

} // namespace

#endif // #ifndef IECOREVDB_USE_PYBIND

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

#ifdef IECOREVDB_USE_PYBIND
	IECorePython::PyBindConverter<openvdb::GridBase::Ptr>::registerConverters();
#endif

	IECorePython::RunTimeTypedClass<VDBObject>()
		.def(init<const std::string &>())
		.def(init<>())
		.def("gridNames", &::gridNames)
		.def("metadata", &VDBObject::metadata)
		.def("removeGrid", &VDBObject::removeGrid)
#ifdef IECOREVDB_USE_PYBIND
		.def( "findGrid", (openvdb::GridBase::Ptr (VDBObject::*)( const std::string &name ))&VDBObject::findGrid )
		.def( "insertGrid", &VDBObject::insertGrid )
#else
		.def("findGrid", &::findGrid)
		.def("insertGrid", &::insertGrid)
#endif
		.def("unmodifiedFromFile", &VDBObject::unmodifiedFromFile)
		.def("fileName", &VDBObject::fileName)
		;

}
