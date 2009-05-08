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

#ifndef IE_COREMAYA_MDATAHANDLEACCESSOR_H
#define IE_COREMAYA_MDATAHANDLEACCESSOR_H

#include "boost/static_assert.hpp"

#include "maya/MDataHandle.h"

namespace IECoreMaya
{

/// A templated class to allow straight-forward access to data stored in MDataHandle objects.
/// The class is templated on the "raw" type of the data returned, e.g. "float", but in additional
/// the ReturnType typedef can be used to find out the exact type with any qualifiers on it. So, using
/// the same example, MDataHandleAccessor<float>::ReturnType is "float&". This allows simplicity
/// of use, but also permits compatibility with boost::type_traits::is_reference, for more flexible
/// template code.
template< typename T >
struct MDataHandleAccessor
{
	BOOST_STATIC_ASSERT( sizeof( T ) == 0 );

	typedef void ReturnType;

	static ReturnType get( const MDataHandle &h )
	{
	}
};

} // namespace IECoreMaya

#include "IECoreMaya/MDataHandleAccessor.inl"

#endif // IE_COREMAYA_MDATAHANDLEACCESSOR_H
