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

#ifndef IECOREGL_CACHEDCONVERTER_H
#define IECOREGL_CACHEDCONVERTER_H

#include "boost/function.hpp"

#include "IECore/Object.h"
#include "IECore/MurmurHash.h"

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( CachedConverter )

class CachedConverter : public IECore::RefCounted
{

	public :

		IE_CORE_DECLAREMEMBERPTR( CachedConverter );

		/// Max memory specified in bytes.
		CachedConverter( size_t maxMemory );
		virtual ~CachedConverter();
		
		/// Returns the object converted to an appropriate IECoreGL type, reusing
		/// a previous conversion where possible.
		IECore::ConstRunTimeTypedPtr convert( const IECore::Object *object );

		typedef boost::function<IECore::RunTimeTypedPtr (const IECore::Object *)> ConverterFn;

		/// Uses a custom converter for the given object. The converter is a callable object of the type ConverterFn but it also must implement a hash( object ) method.
		template< typename T >	IECore::ConstRunTimeTypedPtr convert( const IECore::Object *object, T converter );
		
		/// Returns the maximum amount of memory (in bytes) the cache will use.
		size_t getMaxMemory() const;
		/// Sets the maximum amount of memory the cache will use. If this
		/// is less than memoryUsage() then cache removals will result.
		void setMaxMemory( size_t maxMemory );

		/// The CachedConverter removes items from the cache during convert()
		/// whenever it needs to free memory to make way for the new conversion.
		/// However, if the call to convert() is made on a thread for which there's
		/// no valid gl context, it is unable to free the resources immediately.
		/// As a workaround it defers the freeing of all resources until clearUnused()
		/// is called on the main opengl thread. It is the responsibility of the clients
		/// of the CachedConverter to call this from the main thread periodically.
		/// \todo Can we improve this situation?
		void clearUnused();

		/// Returns a static CachedConverter instance to be used by anything
		/// wishing to share its cache with others. It makes sense to use
		/// this wherever possible to conserve memory. This initially
		/// has a memory limit specified in megabytes by the
		/// IECOREGL_CACHEDCONVERTER_MEMORY environment variable.
		static CachedConverterPtr defaultCachedConverter();

	private :

		IECore::ConstRunTimeTypedPtr convert( const IECore::Object *object, ConverterFn converter, const IECore::MurmurHash &converterHash );

		struct MemberData;
		MemberData *m_data;

};

} // namespace IECoreGL

#include "IECoreGL/CachedConverter.inl"

#endif // IECOREGL_CACHEDCONVERTER_H
