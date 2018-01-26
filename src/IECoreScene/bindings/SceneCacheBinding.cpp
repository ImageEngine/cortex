//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

// This include needs to be the very first to prevent problems with warnings
// regarding redefinition of _POSIX_C_SOURCE
#include "boost/python.hpp"

#include "SceneCacheBinding.h"

#include "IECoreScene/SceneCache.h"
#include "IECoreScene/SharedSceneInterfaces.h"

#include "IECorePython/RunTimeTypedBinding.h"

#include "tbb/tbb.h"

using namespace tbb;
using namespace boost::python;
using namespace IECore;
using namespace IECorePython;
using namespace IECoreScene;

//////////////////////////////////////////////////////////////////////////
// Wrappers
//////////////////////////////////////////////////////////////////////////

namespace
{

SceneCachePtr constructor( const std::string &fileName, IndexedIO::OpenMode mode )
{
	return new SceneCache( fileName, mode );
}

SceneCachePtr constructor2( IECore::IndexedIOPtr indexedIO )
{
	return new SceneCache( indexedIO );
}

} // namespace

//////////////////////////////////////////////////////////////////////////
// Tests
//////////////////////////////////////////////////////////////////////////

namespace
{

struct TestSceneCache
{
	public :

		TestSceneCache( const char *attribute ) : m_errors( 0 ), m_attribute( attribute )
		{
		}

		TestSceneCache( TestSceneCache &that, tbb::split ) : m_errors( 0 ), m_attribute( that.m_attribute )
		{
		}

		void operator()( const blocked_range<size_t> &r ) const
		{
			for ( size_t i = r.begin(); i != r.end(); ++i )
			{
				for ( size_t j = 0; j < 1000; j++ )
				{
					if ( ( i + j ) % 7 == 0 )
					{
						SharedSceneInterfaces::clear();
					}

					ConstSceneInterfacePtr scene = SharedSceneInterfaces::get("test/IECore/data/sccFiles/attributeAtRoot.scc");

					try
					{
						scene->readAttribute( m_attribute, 0 );
					}
					catch( Exception &e )
					{
						m_errors++;
					}
				}
			}
		}

		void join( const TestSceneCache &that )
		{
			m_errors += that.m_errors;
		}

		size_t errors() const
		{
			return m_errors;
		}

	private :
		mutable size_t m_errors;
		SceneInterface::Name m_attribute;
};

void testSceneCacheParallelAttributeRead()
{
	task_scheduler_init scheduler( 100 );

	TestSceneCache task( "w" );

	parallel_reduce( blocked_range<size_t>( 0, 100 ), task );
	if( task.errors() )
	{
		throw Exception( "Error detected" );
	}
}

void testSceneCacheParallelFakeAttributeRead()
{
	task_scheduler_init scheduler( 100 );

	TestSceneCache task( "fake" );

	parallel_reduce( blocked_range<size_t>( 0, 100 ), task );
	if( task.errors() != 100000 )
	{
		throw Exception( "Unexpected number of errors" );
	}
}

} // namespace

//////////////////////////////////////////////////////////////////////////
// Binding
//////////////////////////////////////////////////////////////////////////

namespace IECoreSceneModule
{

void bindSceneCache()
{
	RunTimeTypedClass<SceneCache>()
		.def( "__init__", make_constructor( &constructor ), "Opens a scene file for read or write." )
		.def( "__init__", make_constructor( &constructor2 ), "Opens a scene from a previously opened file handle." )
	;

	def( "testSceneCacheParallelAttributeRead", &testSceneCacheParallelAttributeRead );
	def( "testSceneCacheParallelFakeAttributeRead", &testSceneCacheParallelFakeAttributeRead );

}

} // namespace IECoreSceneModule
