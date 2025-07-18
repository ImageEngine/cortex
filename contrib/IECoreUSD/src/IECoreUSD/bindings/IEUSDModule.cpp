//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2017, Image Engine Design. All rights reserved.
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

#include "IECoreUSD/DataAlgo.h"
#include "IECoreUSD/SceneCacheDataAlgo.h"

#include "IECoreScene/SceneInterface.h"

#include "IECore/IndexedIO.h"

#if PXR_VERSION >= 2505
#include "pxr/external/boost/python.hpp"
#endif

#include "boost/python.hpp"

using namespace boost::python;
using namespace IECore;
using namespace IECoreUSD;
using namespace IECoreScene;

namespace
{

inline SceneInterface::Path listToVector( list& l )
{
	SceneInterface::Path result;
	result.reserve( len( l ) );
	for ( int i = 0; i < len( l ); ++i )
	{
		result.emplace_back( extract<IndexedIO::EntryID>( l[i] ) );
	}
	return result;
}

inline list vectorToList( std::vector<IndexedIO::EntryID> &ids )
{
	list result;
	for( SceneInterface::NameList::const_iterator it = ids.begin(); it != ids.end(); it++ )
	{
		result.append( ( *it ).value() );
	}
	return result;
}

static list toInternalPath( list l )
{

	auto vec = listToVector( l );
	auto path = SceneCacheDataAlgo::toInternalPath( vec );
	return vectorToList( path );
}

static list fromInternalPath( list l )
{

	auto vec = listToVector( l );
	auto path = SceneCacheDataAlgo::fromInternalPath( vec );
	return vectorToList( path );
}

#if PXR_VERSION >= 2505

// Registers `boost::python` converters for types
// wrapped using `pxr_boost::python`.
template<typename T>
struct PxrBoostConverter
{

	static void registerConverters()
	{
		boost::python::to_python_converter<T, ToPxrBoost>();
		/// \todo Add conversion from Python when we have
		/// a use for it. See PyBindConverter for an example.
	}

	private :

		struct ToPxrBoost
		{
			static PyObject *convert( const T &t )
			{
				pxr::pxr_boost::python::object o( t );
				Py_INCREF( o.ptr() );
				return o.ptr();
			}
		};

};

#endif // PXR_VERSION >= 2505

} // namespace

BOOST_PYTHON_MODULE( _IECoreUSD )
{
#if PXR_VERSION >= 2505
	PxrBoostConverter<pxr::TfToken>::registerConverters();
	PxrBoostConverter<pxr::VtValue>::registerConverters();
	PxrBoostConverter<pxr::SdfValueTypeName>::registerConverters();
#endif

	{
		object dataAlgoModule( handle<>( borrowed( PyImport_AddModule( "IECoreUSD.DataAlgo" ) ) ) );
		scope().attr( "DataAlgo" ) = dataAlgoModule;
		scope dataAlgoModuleScope( dataAlgoModule );

		def( "role", &DataAlgo::role );
		def( "toUSD", (pxr::VtValue (*)( const IECore::Data *, bool ))&DataAlgo::toUSD, ( arg( "data" ), arg( "arrayRequired" ) = false ) );
		def( "valueTypeName", &DataAlgo::valueTypeName );
	}

	{
		object sceneCacheDataAlgoModule( handle<>( borrowed( PyImport_AddModule( "IECoreUSD.SceneCacheDataAlgo" ) ) ) );
		scope().attr( "SceneCacheDataAlgo" ) = sceneCacheDataAlgoModule;
		scope sceneCacheDataAlgoModuleScope( sceneCacheDataAlgoModule );

		def( "internalRootName", &SceneCacheDataAlgo::internalRootName );
		def( "toInternalName", &SceneCacheDataAlgo::toInternalName );
		def( "fromInternalName", &SceneCacheDataAlgo::fromInternalName );
		def( "toInternalPath", toInternalPath );
		def( "fromInternalPath", fromInternalPath );
	}
}
