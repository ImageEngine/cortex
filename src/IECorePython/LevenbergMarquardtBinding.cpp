//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2010, Image Engine Design Inc. All rights reserved.
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

#include "boost/python.hpp"

#include "IECorePython/LevenbergMarquardtBinding.h"

#include "IECorePython/RefCountedBinding.h"
#include "IECorePython/ScopedGILLock.h"

#include "IECore/LevenbergMarquardt.h"

using namespace boost::python;
using namespace IECore;

namespace IECorePython
{

template<typename T>
class LevenbergMarquardtErrorFn : public RefCounted
{
	public :

		IE_CORE_DECLAREMEMBERPTR( LevenbergMarquardtErrorFn<T> );

		LevenbergMarquardtErrorFn()
		{
		}

		~LevenbergMarquardtErrorFn() override
		{
		}

		void operator()(
		        typename TypedData< std::vector<T> >::Ptr parameters,
		        typename TypedData< std::vector<T> >::Ptr errors
		)
		{
			computeErrors( parameters, errors );
			assert( errors->readable().size() == numErrors() );
		}

		virtual unsigned numErrors() = 0;

		virtual void computeErrors(
		        typename TypedData< std::vector<T> >::Ptr parameters,
		        typename TypedData< std::vector<T> >::Ptr errors
		) = 0;

};

template<typename T>
class LevenbergMarquardtErrorFnWrapper : public RefCountedWrapper< LevenbergMarquardtErrorFn<T> >
{
	public:

		IE_CORE_DECLAREMEMBERPTR( LevenbergMarquardtErrorFnWrapper<T> );

		LevenbergMarquardtErrorFnWrapper( PyObject *self )
			: RefCountedWrapper< LevenbergMarquardtErrorFn<T> >( self )
		{
		}

		~LevenbergMarquardtErrorFnWrapper() override
		{
		}

		unsigned numErrors() override
		{
			ScopedGILLock gilLock;
			object o = this->methodOverride( "numErrors" );
			if( o )
			{
				try
				{
					return extract<unsigned>( o() );
				}
				catch( const error_already_set & )
				{
					PyErr_Print();
					return 0;
				}
			}
			else
			{
				throw Exception( "LevenbergMarquardt: Error function does not define 'numErrors' instance method" );
			}
		}

		void computeErrors(
		        typename TypedData< std::vector<T> >::Ptr parameters,
		        typename TypedData< std::vector<T> >::Ptr errors
		) override
		{
			ScopedGILLock gilLock;
			object o = this->methodOverride( "computeErrors" );
			if( o )
			{
				try
				{
					o( parameters, errors );
				}
				catch( const error_already_set & )
				{
					PyErr_Print();
					return;
				}
			}
			else
			{
				throw Exception( "LevenbergMarquardt: Error function does not define 'computeErrors' instance method" );
			}
		}
};

template< typename T>
class LevenbergMarquardtWrapper : public LevenbergMarquardt< T, LevenbergMarquardtErrorFnWrapper<T> >
{
	public:

		tuple getParameters()
		{
			T ftol, xtol, gtol, epsilon, stepBound;
			LevenbergMarquardt< T, LevenbergMarquardtErrorFnWrapper<T> >::getParameters( ftol, xtol, gtol, epsilon, stepBound );
			return make_tuple(ftol, xtol, gtol, epsilon, stepBound);
		}
};

template<typename T>
void bindLevenbergMarquardt( const char *name )
{
	object lm = class_< LevenbergMarquardtWrapper<T>, boost::noncopyable >( name, no_init )
		.def( init<>() )
		.def( "setParameters", &LevenbergMarquardtWrapper<T>::setParameters )
		.def( "getParameters", &LevenbergMarquardtWrapper<T>::getParameters )
		.def( "solve", &LevenbergMarquardtWrapper<T>::solve )
		.def( "setMaxCalls", &LevenbergMarquardtWrapper<T>::setMaxCalls )
		.def( "getMaxCalls", &LevenbergMarquardtWrapper<T>::getMaxCalls )
	;

	{
		scope lmS( lm );

		enum_< typename LevenbergMarquardtWrapper<T>::Status >( "Status" )
			.value( "Success", LevenbergMarquardtWrapper<T>::Success )
		;

		RefCountedClass<LevenbergMarquardtErrorFn<T>, RefCounted, LevenbergMarquardtErrorFnWrapper<T> >( "ErrorFn" )
			.def( init<>() )
			.def( "numErrors", pure_virtual( &LevenbergMarquardtErrorFnWrapper<T>::numErrors ) )
			.def( "computeErrors", pure_virtual( &LevenbergMarquardtErrorFnWrapper<T>::computeErrors ) )
		;
	}
}

void bindLevenbergMarquardt()
{
	bindLevenbergMarquardt< float >( "LevenbergMarquardtf" );
	bindLevenbergMarquardt< double >( "LevenbergMarquardtd" );
}

} // namespace IECorePython
