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

#ifndef IE_CORE_RADIXSORT_H
#define IE_CORE_RADIXSORT_H

#include <vector>

#include "boost/static_assert.hpp"

#include "IECore/VectorTypedData.h"

namespace IECore
{

/// A RadixSort implementation derived from Pierre Terdiman's OPCODE library, which has as "free for use in any commercial
/// or non-commercial program" licence. The RadixSort class maintains state so that successive calls to it are able to exploit any coherence
/// in the source data. Sorting is done in ascending order.
class RadixSort
{
	public:

		BOOST_STATIC_ASSERT( sizeof( int ) == 4 );
		BOOST_STATIC_ASSERT( sizeof( unsigned int ) == 4 );
		BOOST_STATIC_ASSERT( sizeof( float ) == 4 );

		RadixSort();
		virtual ~RadixSort();

		/// Sort the given vector of floats, returning a vector of indices representing a mapping from
		/// the given input to the sorted result. For example, the position of input[3] in the sorted result can be
		/// found in indices[3].
		const std::vector<unsigned int> &operator()( const std::vector<float> &input );

		/// Sort the given vector of unsigned ints, returning a vector of indices representing a mapping from
		/// the given input to the sorted result. For example, the position of input[3] in the sorted result can be
		/// found in indices[3].
		const std::vector<unsigned int> &operator()( const std::vector<unsigned int> &input );

		/// Sort the given vector of signed ints, returning a vector of indices representing a mapping from
		/// the given input to the sorted result. For example, the position of input[3] in the sorted result can be
		/// found in indices[3].
		const std::vector<unsigned int> &operator()( const std::vector<int> &input );

	private:

		template<typename T>
		bool createHistograms( const std::vector<T> &input );

		unsigned int m_currentSize;

		unsigned int m_histogram[1024];
		unsigned int *m_link[256];

		UIntVectorDataPtr m_ranks;
		UIntVectorDataPtr m_ranks2;

		template<typename T>
		bool checkPassValidity( const std::vector<T> &input, unsigned int j, unsigned int* &curCount, unsigned char &uniqueVal );

		void resize( unsigned int s );
		void checkResize( unsigned int s );
};

} // namespace IECore

#endif // IE_CORE_RADIXSORT_H
