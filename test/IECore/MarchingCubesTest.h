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

#ifndef IE_CORE_MARCHINGCUBESTEST_H
#define IE_CORE_MARCHINGCUBESTEST_H

#include <math.h>
#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <IECore/IECore.h>
#include "IECore/VectorTypedData.h"
#include "IECore/VectorTraits.h"
#include "IECore/MarchingCubes.h"
#include "IECore/MeshPrimitive.h"
#include "IECore/Writer.h"
#include "IECore/PerlinNoise.h"

using namespace Imath;

namespace IECore
{

void addMarchingCubesTest(boost::unit_test::test_suite* test);

class SphereIsoSurfaceFn
{
	public:
		typedef Imath::V3f Point;
		typedef VectorTraits<Point> PointTraits;
		typedef VectorTraits<Point>::BaseType PointBaseType;
		typedef float Value;
		typedef VectorTraits<Value> ValueTraits;
		typedef VectorTraits<Value>::BaseType ValueBaseType;
		
		typedef SphereIsoSurfaceFn* Ptr;
		SphereIsoSurfaceFn( Value r ) : m_radius(r) {}
	
		inline Value operator()( const Point &p )
		{			
			return p.length() - m_radius;
		}
		
	protected:
	
		Value m_radius;
	
};

struct MarchingCubesTest
{	
	void testSphere()
	{
		typedef MarchingCubes<SphereIsoSurfaceFn, MeshPrimitiveBuilder<float> > Cubes;
		typedef boost::intrusive_ptr<Cubes> CubesPtr;
	
		int x = 100;
		int y = 100;
		int z = 100;
		
		float radius = 0.5;
		
		SphereIsoSurfaceFn::Ptr fn = new SphereIsoSurfaceFn( radius );	
		MeshPrimitiveBuilder<float>::Ptr builder = new MeshPrimitiveBuilder<float>();
					
		CubesPtr c = new Cubes( fn, builder );
	
		c->march( Box3f( V3f(-5,-5,-5),V3f(5,5,5) ), V3i(x,y,z));
		
		MeshPrimitivePtr result = builder->mesh();
			
		BOOST_CHECK( result );
		
		V3fVectorDataPtr p = runTimeCast<V3fVectorData>(result->variables["P"].data);
		
		BOOST_CHECK( p );
		
		BOOST_CHECK( (int)p->readable().size() > 400 && (int)p->readable().size() < 450 );
		
		for (int i = 0; i < (int)p->readable().size(); i++)
		{
			BOOST_CHECK_CLOSE( p->readable()[i].length(), radius, 0.5 );
		}
		
		delete fn;
	}
	
	void testPerlinNoise()
	{
		typedef MarchingCubes<PerlinNoiseV3ff, MeshPrimitiveBuilder<float> > Cubes;
		typedef boost::intrusive_ptr<Cubes> CubesPtr;
	
		int x = 20;
		int y = 20;
		int z = 20;
		
		PerlinNoiseV3ff::Ptr fn = new PerlinNoiseV3ff();	
		MeshPrimitiveBuilder<float>::Ptr builder = new MeshPrimitiveBuilder<float>();
					
		CubesPtr c = new Cubes( fn, builder );
	
		c->march( Box3f( V3f(-5,-5,-5),V3f(5,5,5) ), V3i(x,y,z));
		
		MeshPrimitivePtr result = builder->mesh();
			
		BOOST_CHECK( result );
		
		V3fVectorDataPtr p = runTimeCast<V3fVectorData>(result->variables["P"].data);
		
		BOOST_CHECK( p );
		BOOST_CHECK_EQUAL( (int)p->readable().size(), 8609 );
		
		delete fn;
	}
};

struct MarchingCubesTestSuite : public boost::unit_test::test_suite
{
	
	MarchingCubesTestSuite() : boost::unit_test::test_suite("MarchingCubesTestSuite")
	{
		static boost::shared_ptr<MarchingCubesTest> instance(new MarchingCubesTest());
		
		add( BOOST_CLASS_TEST_CASE( &MarchingCubesTest::testSphere, instance ) );
		add( BOOST_CLASS_TEST_CASE( &MarchingCubesTest::testPerlinNoise, instance ) );							
	}
	
};
}


#endif

