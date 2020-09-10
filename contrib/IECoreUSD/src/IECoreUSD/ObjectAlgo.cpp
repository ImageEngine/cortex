//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2020, Cinesite VFX Ltd. All rights reserved.
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

#include "IECoreUSD/ObjectAlgo.h"

//////////////////////////////////////////////////////////////////////////
// Reading
//////////////////////////////////////////////////////////////////////////

namespace
{

struct ReaderFunctions
{
	IECoreUSD::ObjectAlgo::Reader reader;
	IECoreUSD::ObjectAlgo::MightBeTimeVarying mightBeTimeVarying;
};

using ReaderMap = std::map<pxr::TfToken, ReaderFunctions>;
ReaderMap &readers()
{
	static ReaderMap g_readers;
	return g_readers;
}

} // namespace

void IECoreUSD::ObjectAlgo::registerReader( const pxr::TfToken &schemaType, IECoreUSD::ObjectAlgo::Reader reader, IECoreUSD::ObjectAlgo::MightBeTimeVarying mightBeTimeVarying )
{
	readers()[schemaType] = { reader, mightBeTimeVarying };
}

bool IECoreUSD::ObjectAlgo::canReadObject( const pxr::UsdPrim &prim )
{
	const ReaderMap &r = readers();
	return r.find( prim.GetTypeName() ) != r.end();
}

IECore::ObjectPtr IECoreUSD::ObjectAlgo::readObject( const pxr::UsdPrim &prim, pxr::UsdTimeCode time )
{
	const ReaderMap &r = readers();
	auto it = r.find( prim.GetTypeName() );
	if( it == r.end() )
	{
		return nullptr;
	}

	return it->second.reader( prim, time );
}

bool IECoreUSD::ObjectAlgo::objectMightBeTimeVarying( const pxr::UsdPrim &prim )
{
	const ReaderMap &r = readers();
	auto it = r.find( prim.GetTypeName() );
	if( it == r.end() )
	{
		return false;
	}

	return it->second.mightBeTimeVarying( prim );
}

//////////////////////////////////////////////////////////////////////////
// Writing
//////////////////////////////////////////////////////////////////////////

namespace
{

using WriterMap = std::map<IECore::TypeId, IECoreUSD::ObjectAlgo::Writer>;

WriterMap &writers()
{
	static WriterMap g_writers;
	return g_writers;
}

} // namespace

void IECoreUSD::ObjectAlgo::registerWriter( const IECore::TypeId typeId, IECoreUSD::ObjectAlgo::Writer writer )
{
	writers()[typeId] = writer;
}

bool IECoreUSD::ObjectAlgo::writeObject( const IECore::Object *object, const pxr::UsdStagePtr &stage, const pxr::SdfPath &path, pxr::UsdTimeCode time )
{
	const auto &w = writers();
	auto it = w.find( object->typeId() );
	if( it == w.end() )
	{
		return false;
	}
	return it->second( object, stage, path, time );
}
