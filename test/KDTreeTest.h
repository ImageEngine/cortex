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

#include <iostream>
#include <algorithm>

#include <boost/test/unit_test.hpp>

#include <OpenEXR/ImathVec.h>
#include <OpenEXR/ImathRandom.h>

#include <IECore/KDTree.h>

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
		typedef KDTree<typename PointVector::const_iterator> Tree;
		typedef std::vector< typename Tree::Iterator > NeighbourVector;
	
		std::vector<T> m_points;
		Tree *m_tree;
		Imath::Rand32 m_randGen;
		unsigned int m_numPoints;
	
		typename  Tree::Iterator randomPoint();
		
};

template<unsigned int N>
struct KDTreeTestSuite : public boost::unit_test::test_suite
{
	
	KDTreeTestSuite() : boost::unit_test::test_suite("KDTreeTestSuite")
	{
		addTest<Imath::V3f>();
		addTest<Imath::V3d>();
		addTest<Imath::V2f>();
		addTest<Imath::V2d>();
	}
	
	template<typename T>
	void addTest()		
	{	
		static boost::shared_ptr<KDTreeTest<T> > instance(new KDTreeTest<T>(N));
	
		add( BOOST_CLASS_TEST_CASE( &KDTreeTest<T>::testNearestNeighour, instance ) );
		add( BOOST_CLASS_TEST_CASE( &KDTreeTest<T>::testNearestNeighours, instance ) );
		add( BOOST_CLASS_TEST_CASE( &KDTreeTest<T>::testNearestNNeighours, instance ) );
	}
};

}

#include "KDTreeTest.inl"

#endif // IE_CORE_KDTREETEST_H
