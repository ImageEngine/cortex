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

#ifndef IE_CORE_SWEEPANDPRUNE_INL
#define IE_CORE_SWEEPANDPRUNE_INL

#include <cassert>
#include <set>
#include <algorithm>
#include <limits.h>

#include "boost/static_assert.hpp"

#include "IECore/BoxTraits.h"
#include "IECore/VectorOps.h"

namespace IECore
{

template<typename BoundIterator, template<typename> class CB>
SweepAndPrune<BoundIterator, CB>::SweepAndPrune( )
{
}

template<typename BoundIterator, template<typename> class CB>
SweepAndPrune<BoundIterator, CB>::~SweepAndPrune( )
{
}

template<typename BoundIterator, template<typename> class CB>
bool SweepAndPrune<BoundIterator, CB>::axisIntersects( const Bound &b0, const Bound &b1, char axis )
{
	if ( vecGet( BoxTraits<Bound>::max(b0), axis ) < vecGet( BoxTraits<Bound>::min(b1), axis )
	     ||  vecGet( BoxTraits<Bound>::min(b0), axis ) > vecGet( BoxTraits<Bound>::max(b1), axis ) )
	{
		assert( !b0.intersects(b1) );
		return false;
	}

	return true;
}

template<typename BoundIterator, template<typename> class CB>
void SweepAndPrune<BoundIterator, CB>::intersectingBounds( BoundIterator first, BoundIterator last, typename SweepAndPrune<BoundIterator, CB>::Callback &cb, AxisOrder axisOrder )
{
	unsigned long numBounds = std::distance( first, last );

	/// Can't radix sort more than this!
	assert( numBounds <= std::numeric_limits< uint32_t >::max() );

	if (! numBounds )
	{
		return;
	}

	char axes[3] = { 0, 1, 2 };

	switch (axisOrder)
	{
		case XYZ :
			axes[0] = 0; axes[1] = 1; axes[2] = 2; break;
		case XZY :
			axes[0] = 0; axes[1] = 2; axes[2] = 1; break;
		case YXZ :
			axes[0] = 1; axes[1] = 0; axes[2] = 2; break;
		case YZX :
			axes[0] = 1; axes[1] = 2; axes[2] = 0; break;
		case ZXY :
			axes[0] = 2; axes[1] = 0; axes[2] = 1; break;
		case ZYX :
			axes[0] = 2; axes[1] = 1; axes[2] = 0; break;
		default:
			assert( false );
	}

	assert( axes[0] + axes[1] + axes[2] == 3 );

	typedef std::pair< bool, BoundIterator> IntervalId;

	std::vector<IntervalId> intervalIds;
	std::vector<float> boundExtents;
	boundExtents.reserve( numBounds * 2 );
	intervalIds.reserve( numBounds * 2 );

	for ( BoundIterator it = first; it != last; ++it )
	{
		boundExtents.push_back( vecGet( BoxTraits<Bound>::min(*it), axes[0] ) );
		intervalIds.push_back( IntervalId( true, it  ) );

		boundExtents.push_back( vecGet( BoxTraits<Bound>::max(*it), axes[0] ) );
		intervalIds.push_back( IntervalId( false, it ) );
	}

	assert( boundExtents.size() == intervalIds.size() );

	const std::vector<unsigned int> &sortedIndices = m_radixSort( boundExtents );

	typedef std::vector<BoundIterator> ActiveSet;
	ActiveSet activeSet;

	for ( std::vector<unsigned int>::const_iterator st = sortedIndices.begin(); st != sortedIndices.end(); ++st )
	{
		const IntervalId &id = intervalIds[ *st ];

		if ( id.first )
		{
			const Bound& bound0 = *id.second;

			for ( typename ActiveSet::const_iterator it = activeSet.begin(); it != activeSet.end(); it++ )
			{
				assert( id.second != *it );

				if ( axisIntersects( bound0, **it, axes[1] ) )
				{
					if ( axisIntersects( bound0, **it, axes[2] ) )
					{
						assert( bound0.intersects( **it ) );

						cb( id.second, *it );
					}
					else
					{
						assert( !bound0.intersects( **it ) );
					}
				}
				else
				{
					assert( !bound0.intersects( **it ) );
				}
			}

			activeSet.push_back( id.second );
		}
		else
		{
			assert( activeSet.size() );
			typename ActiveSet::iterator it = std::find ( activeSet.begin(), activeSet.end(), id.second );
			assert( it != activeSet.end() );
			activeSet.erase( it );
		}
	}
}

} // namespace IECore


#endif // IE_CORE_SWEEPANDPRUNE_INL
