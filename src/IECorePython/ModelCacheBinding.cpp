//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECore/ModelCache.h"
#include "IECorePython/RefCountedBinding.h"

using namespace boost::python;
using namespace IECore;

namespace IECorePython
{

static list childNames( const ModelCache &m )
{
	std::vector< IndexedIO::EntryID > n;
	m.childNames( n );
	
	list result;
	for( std::vector<IndexedIO::EntryID>::const_iterator it = n.begin(); it!=n.end(); it++ )
	{
		result.append( (*it).value() );
	}
	
	return result;
}

static ModelCachePtr readableChild( const ModelCache &m, const std::string &c )
{
	return constPointerCast<ModelCache>( m.readableChild( c ) );
}

void bindModelCache()
{

	RefCountedClass<ModelCache, RefCounted>( "ModelCache" )
		.def( init<const std::string &, IndexedIO::OpenMode>() )
		.def( "path", &ModelCache::path, return_value_policy<copy_const_reference>() )
		.def( "name", &ModelCache::name, return_value_policy<copy_const_reference>() )
		.def( "readBound", &ModelCache::readBound )
		.def( "writeBound", &ModelCache::writeBound )
		.def( "readTransform", &ModelCache::readTransform )
		.def( "writeTransform", &ModelCache::writeTransform )
		.def( "readObject", &ModelCache::readObject )
		.def( "writeObject", &ModelCache::writeObject )
		.def( "hasObject", &ModelCache::hasObject )
		.def( "childNames", &childNames )
		.def( "writableChild", &ModelCache::writableChild )
		.def( "readableChild", &readableChild )
	;
	
}

} // namespace IECorePython
