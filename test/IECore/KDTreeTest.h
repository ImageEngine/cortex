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
#include "Imath/ImathVec.h"
#include "Imath/ImathRandom.h"
IECORE_POP_DEFAULT_VISIBILITY

IECORE_PUSH_DEFAULT_VISIBILITY
#include "boost/test/unit_test.hpp"
IECORE_POP_DEFAULT_VISIBILITY

#include <algorithm>
#include <iostream>

namespace IECore
{

void addKDTreeTest(boost::unit_test::test_suite* test);

template<typename T>
class KDTreeTest
{
	public:
		KDTreeTest(unsigned int numPoints);
		virtual ~KDTreeTest();

		void testNearestNeighbour();
		void testNearestNeighbours();
		void testNearestNNeighbours();

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

// I'm trying to use this test to document the current requirements we have for the types we support.
// It's not sufficient to specialize VectorTraits, we also require that the vector type behave mostly
// like an Imath vector. This requirement just arose over time because we only actually needed support
// for Imath vectors, and Maya's MPoint/MVector, both of which support some standard interface for
// vectors, like operator[] and operator-.
struct TestVecType
{
	float v[3] = { 0, 0, 0 };

	float &operator[]( int i ){ return v[i]; };
	const float &operator[]( int i ) const { return v[i]; };

	TestVecType operator-( const TestVecType &a ) { return { v[0] - a.v[0], v[1] - a.v[1], v[2] - a.v[2] }; };
};

template<>
struct VectorTraits<TestVecType>
{
	typedef float BaseType;
	static unsigned int dimensions() { return 3; };
	static double get( const TestVecType &v, unsigned int i ) { return v.v[i]; };
	static void set( TestVecType &v, unsigned int i, float x ) { v.v[i] = x; };
};

template<unsigned int N>
struct KDTreeTestSuite : public boost::unit_test::test_suite
{

	KDTreeTestSuite() : boost::unit_test::test_suite("KDTreeTestSuite" + std::to_string( N ) )
	{
		addTest<Imath::V3f>( "V3f" );
		addTest<Imath::V3d>( "V3d" );
		addTest<Imath::V2f>( "V2f" );
		addTest<Imath::V2d>( "V2d" );
		addTest<TestVecType>( "TestVecType" );
	}

	template<typename T>
	void addTest( const std::string &nameSuffix )
	{
		static boost::shared_ptr<KDTreeTest<T> > instance(new KDTreeTest<T>(N));

		auto test = BOOST_CLASS_TEST_CASE( &KDTreeTest<T>::testNearestNeighbour, instance );
		test->p_name.set( test->p_name.get() + nameSuffix );
		add( test );

		test = BOOST_CLASS_TEST_CASE( &KDTreeTest<T>::testNearestNeighbours, instance );
		test->p_name.set( test->p_name.get() + nameSuffix );
		add( test );

		test = BOOST_CLASS_TEST_CASE( &KDTreeTest<T>::testNearestNNeighbours, instance );
		test->p_name.set( test->p_name.get() + nameSuffix );
		add( test );
	}
};

}

#include "KDTreeTest.inl"

#endif // IE_CORE_KDTREETEST_H
