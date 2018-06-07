//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#include "ParameterThreadingTest.h"

#include "IECore/CompoundObject.h"
#include "IECore/CompoundParameter.h"
#include "IECore/DataCastOp.h"
#include "IECore/NumericParameter.h"
#include "IECore/SimpleTypedParameter.h"
#include "IECore/TypedObjectParameter.h"
#include "IECore/VectorTypedParameter.h"

#include "OpenEXR/ImathRandom.h"

#include "tbb/tbb.h"

#include <iostream>
#include <set>

using namespace boost;
using namespace boost::unit_test;
using namespace tbb;

namespace IECore
{

struct ParameterThreadingTest
{

	struct ReadCompoundChildren
	{
		public :

			ReadCompoundChildren( CompoundParameter &p )
				:	m_compoundParameter( p )
			{
			}

			void operator()( const blocked_range<size_t> &r ) const
			{
				const CompoundParameter::ParameterMap &children = m_compoundParameter.parameters();
				std::vector<std::string> path;
				for( size_t i=r.begin(); i!=r.end(); ++i )
				{
					for( CompoundParameter::ParameterMap::const_iterator it=children.begin(); it!=children.end(); it++ )
					{
						m_compoundParameter.parameter<Parameter>( it->first );
						path.clear();
						m_compoundParameter.parameterPath( it->second.get(), path );
						m_compoundParameter.getParameterValue( it->first );
						m_compoundParameter.getValidatedParameterValue( it->first );
					}
				}
			}

		private :

			CompoundParameter &m_compoundParameter;

	};

	void testReadingCompoundChildren()
	{
		CompoundParameterPtr c = new CompoundParameter( "c", "" );
		c->addParameter( new FloatParameter( "a", "" ) );
		c->addParameter( new IntParameter( "b", "" ) );
		c->addParameter( new V3fParameter( "c", "" ) );
		c->addParameter( new V3iParameter( "d", "" ) );
		c->addParameter( new V3dParameter( "e", "" ) );
		c->addParameter( new Box3fParameter( "f", "" ) );
		c->addParameter( new Box3fParameter( "g", "" ) );

		tbb::task_group_context taskGroupContext( tbb::task_group_context::isolated );
		parallel_for( blocked_range<size_t>( 0, 1000000 ), ReadCompoundChildren( *c ), taskGroupContext );
	}

	struct ReadParameters
	{
		public :

			ReadParameters( std::vector<Parameter *> &parameters )
				:	m_parameters( parameters )
			{
			}

			void operator()( const blocked_range<size_t> &r ) const
			{
				for( size_t i=r.begin(); i!=r.end(); ++i )
				{
					m_parameters[i]->validate();
					m_parameters[i]->defaultValue();
					m_parameters[i]->getValidatedValue();
					m_parameters[i]->getCurrentPresetName();
					switch( m_parameters[i]->typeId() )
					{
						case IntParameterTypeId :
							static_cast<IntParameter *>( m_parameters[i] )->getNumericValue();
							static_cast<IntParameter *>( m_parameters[i] )->numericDefaultValue();
							break;
						case FloatParameterTypeId :
							static_cast<FloatParameter *>( m_parameters[i] )->getNumericValue();
							static_cast<FloatParameter *>( m_parameters[i] )->numericDefaultValue();
							break;
						case V3fParameterTypeId :
							static_cast<V3fParameter *>( m_parameters[i] )->getTypedValue();
							static_cast<V3fParameter *>( m_parameters[i] )->typedDefaultValue();
							break;
						case V3iParameterTypeId :
							static_cast<V3iParameter *>( m_parameters[i] )->getTypedValue();
							static_cast<V3iParameter *>( m_parameters[i] )->typedDefaultValue();
							break;
						default :
							;
					}
				}
			}

		private :

			std::vector<Parameter *> &m_parameters;
	};

	void testReading()
	{
		std::vector<ParameterPtr> parameters;
		parameters.push_back( new FloatParameter( "a", "" ) );
		parameters.push_back( new IntParameter( "b", "" ) );
		parameters.push_back( new V3fParameter( "c", "" ) );
		parameters.push_back( new V3iParameter( "d", "" ) );
		parameters.push_back( new V3dParameter( "e", "" ) );
		parameters.push_back( new Box3fParameter( "f", "" ) );
		parameters.push_back( new Box3fParameter( "g", "" ) );

		Imath::Rand32 rand;
		const size_t permutationSize = 1000000;
		std::vector<Parameter *> parameterPermutation;
		parameterPermutation.resize( permutationSize );
		for( size_t i=0; i<permutationSize; i++ )
		{
			parameterPermutation[i] = parameters[rand.nexti()%parameters.size()].get();
		}
		tbb::task_group_context taskGroupContext( tbb::task_group_context::isolated );
		parallel_for( blocked_range<size_t>( 0, permutationSize ), ReadParameters( parameterPermutation ), taskGroupContext );
	}

	struct CreateAndDestroyOp
	{
		void operator()( const blocked_range<size_t> &r ) const
		{
			for( size_t i=r.begin(); i!=r.end(); ++i )
			{
				DataCastOpPtr o = new DataCastOp();
			}
		}
	};

	void testOpCreationAndDestruction()
	{
		tbb::task_group_context taskGroupContext( tbb::task_group_context::isolated );
		parallel_for( blocked_range<size_t>( 0, 100000 ), CreateAndDestroyOp(), taskGroupContext );
	}

};


struct ParameterThreadingTestSuite : public boost::unit_test::test_suite
{

	ParameterThreadingTestSuite() : boost::unit_test::test_suite( "ParameterThreadingTestSuite" )
	{
		boost::shared_ptr<ParameterThreadingTest> instance( new ParameterThreadingTest() );

		add( BOOST_CLASS_TEST_CASE( &ParameterThreadingTest::testReading, instance ) );
		add( BOOST_CLASS_TEST_CASE( &ParameterThreadingTest::testReadingCompoundChildren, instance ) );
		add( BOOST_CLASS_TEST_CASE( &ParameterThreadingTest::testOpCreationAndDestruction, instance ) );
	}
};

void addParameterThreadingTest(boost::unit_test::test_suite* test)
{
	test->add( new ParameterThreadingTestSuite( ) );
}

} // namespace IECore
