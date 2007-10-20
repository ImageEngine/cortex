//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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
//	     other contributors to this software may be used to endorse or
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

using namespace IECore;
using namespace Imath;
using namespace std;

namespace IECore
{
	
template<typename T>
typename KDTreeTest<T>::Tree::Iterator KDTreeTest<T>::randomPoint()
{
	unsigned int testPtIdx = (unsigned int)(m_numPoints * m_randGen.nextf()) % m_numPoints;
	
	return m_points.begin() + testPtIdx;
}
	
template<typename T>
KDTreeTest<T>::KDTreeTest(unsigned int numPoints) : m_numPoints(numPoints)
{
	m_points = vector<T>( numPoints );

	for( unsigned int i=0; i<numPoints; i++ )
	{
		T p;
		for ( unsigned int j = 0; j < VectorTraits< T >::dimensions(); j++)
			p[j] = m_randGen.nextf();
		m_points[i] = p;
	}
		
	m_tree = new Tree( m_points.begin(), m_points.end(), 16 );
}

template<typename T>
KDTreeTest<T>::~KDTreeTest()
{
	delete m_tree;
}

template<typename T>
void KDTreeTest<T>::testNearestNeighour()
{
	for( typename Tree::Iterator it=m_points.begin(); it!=m_points.end(); it++ )
	{
		// The nearest neighbour to me should be myself!
		BOOST_CHECK( m_tree->nearestNeighbour( *it )==it );
	}
}

template<typename T>
void KDTreeTest<T>::testNearestNeighours()
{
	Rand32 r;
	NeighbourVector nearNeighbours;
	for( typename Tree::Iterator it=m_points.begin(); it!=m_points.end(); it++ )
	{		
		typename T::BaseType radius = 0.05;
		unsigned int numNeighbours = m_tree->nearestNeighbours( *it, radius, nearNeighbours );
		
		BOOST_CHECK(numNeighbours <= m_numPoints);
		
		typename NeighbourVector::const_iterator nit = nearNeighbours.begin();
		for (; nit != nearNeighbours.end(); ++nit)
		{	
			// Each point should be within the given radius
			BOOST_CHECK( vecDistance2(**nit, *it) <= radius * radius );

			// A randomly chosen point which wasn't in the list of near neighbours should be further away than the
			// given radius
			
			typename Tree::Iterator testPoint = randomPoint();
			
			if (std::find( nearNeighbours.begin(), nearNeighbours.end(), testPoint) == nearNeighbours.end())
			{
				BOOST_CHECK( vecDistance2(*it, *testPoint) >= radius * radius );
			}
			
		}
	}
}

template<typename T>
void KDTreeTest<T>::testNearestNNeighours()
{		
	unsigned int neighboursRequested = 4;
	NeighbourVector nearNeighbours;	
	for( typename Tree::Iterator it=m_points.begin(); it!=m_points.end(); it++ )
	{				
		unsigned int numNeighbours = m_tree->nearestNNeighbours( *it, neighboursRequested,  nearNeighbours );
		BOOST_CHECK(numNeighbours <= neighboursRequested);
		
		// One of our nearest-N neighbours should be the actual nearest!
		BOOST_CHECK( std::find( nearNeighbours.begin(), nearNeighbours.end(), m_tree->nearestNeighbour( *it ))
				!= nearNeighbours.end() );
		
		// Make sure points are returned in order furthest->closest
		typename NeighbourVector::const_iterator nit;
		for (nit = nearNeighbours.begin(); nit != nearNeighbours.end(); ++nit)
		{	
			if (nit != nearNeighbours.begin())
			{
				BOOST_CHECK( vecDistance2(**nit, *it) <= 
					vecDistance2(**(nit-1), *it)
					);
			}
		}
		
		typename Tree::Iterator furthest = nearNeighbours[0];	
		
		for (nit = nearNeighbours.begin(); nit != nearNeighbours.end(); ++nit)
		{	
			// A randomly chosen point which wasn't in the list of near neighbours should be further away than the
			// further point
		
			typename Tree::Iterator randomPt = randomPoint();
			
			if (std::find( nearNeighbours.begin(), nearNeighbours.end(), randomPt) == nearNeighbours.end())
			{
				typename T::BaseType distanceToRandomPt = vecDistance2(*randomPt, *it);
				typename T::BaseType distanceToFurthestNeighbour = vecDistance2(*furthest, *it);
				
				BOOST_CHECK( distanceToRandomPt >= distanceToFurthestNeighbour);				
			}
			
		}
		
	}
	
	// Check for equivalence with nearestNeighbour when only 1 neighbour is requested.
	for( typename Tree::Iterator it=m_points.begin(); it!=m_points.end(); it++ )
	{
		NeighbourVector nearNeighbours;	
		unsigned int numNeighbours = m_tree->nearestNNeighbours( *it, 1, nearNeighbours );
		BOOST_CHECK(numNeighbours == 1);
		
		typename Tree::Iterator p1 = m_tree->nearestNeighbour(*it);
		typename Tree::Iterator p2 = nearNeighbours[0];

		BOOST_CHECK( p1 == p2 );
	}

}

}
