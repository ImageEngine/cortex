//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_KDTREETEST_H
#define IE_CORE_KDTREETEST_H

#include "IECore/Export.h"
#include "IECore/KDTree.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "OpenEXR/ImathVec.h"
IECORE_POP_DEFAULT_VISIBILITY

#include "OpenEXR/ImathRandom.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "boost/test/unit_test.hpp"
IECORE_POP_DEFAULT_VISIBILITY

#include <algorithm>
#include <iostream>
#include <boost/mpl/list.hpp>

namespace IECore
{

void addKDTreeTest(boost::unit_test::test_suite* test);

template<typename T>
class KDTreeTest
{
	public:
		KDTreeTest(unsigned int numPoints);
		virtual ~KDTreeTest();

		void testNearestNeighour();
		void testNearestNeighours();
		void testNearestNNeighours();

	private:

		/// Some typedefs for convenience
		typedef std::vector<T> PointVector;
		typedef typename std::vector<T>::const_iterator PointIterator;
		typedef KDTree<typename PointVector::const_iterator> Tree;
		typedef std::vector< typename Tree::Iterator > IteratorVector;
		typedef std::vector< typename Tree::Neighbour > NeighbourVector;

		std::vector<T> m_points;
		Tree *m_tree;
		Imath::Rand32 m_randGen;
		unsigned int m_numPoints;

		typename  Tree::Iterator randomPoint();

};

BOOST_TEST_CASE_TEMPLATE_FUNCTION(test_10, T)
{
	static boost::shared_ptr<KDTreeTest<T> > instance(new KDTreeTest<T>(10));
	instance->testNearestNeighour();
	instance->testNearestNeighours();
	instance->testNearestNNeighours();
}

BOOST_TEST_CASE_TEMPLATE_FUNCTION(test_150, T)
{
	static boost::shared_ptr<KDTreeTest<T> > instance(new KDTreeTest<T>(150));
	instance->testNearestNeighour();
	instance->testNearestNeighours();
	instance->testNearestNNeighours();
}

struct KDTreeTestSuite : public boost::unit_test::test_suite
{

	KDTreeTestSuite() : boost::unit_test::test_suite("KDTreeTestSuite")
	{
		typedef boost::mpl::list<Imath::V3f,Imath::V3d,Imath::V2f,Imath::V2d> test_types;
		add( BOOST_TEST_CASE_TEMPLATE( test_10, test_types ) );
		add( BOOST_TEST_CASE_TEMPLATE( test_150, test_types ) );

	}
};

}

#include "KDTreeTest.inl"

#endif // IE_CORE_KDTREETEST_H
