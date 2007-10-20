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

#include <boost/python.hpp>

#include "IECore/PerlinNoise.h"
#include "IECore/TypedData.h"

using namespace boost;
using namespace boost::python;
using namespace std;

namespace IECore {

template<typename T>
static intrusive_ptr<TypedData<vector<typename T::Value> > > noiseVector( const T &n, intrusive_ptr<TypedData<vector<typename T::Point> > > p, intrusive_ptr<TypedData<vector<typename T::Value> > > v = 0 )
{
	if( !v )
	{
		v = new TypedData<vector<typename T::Value> >;
	}
	vector<typename T::Value> &vv = v->writable();
	const vector<typename T::Point> &pp = p->readable();
	vv.resize( pp.size() );
	for( typename vector<typename T::Point>::size_type i=0; i<pp.size(); i++ )
	{
		vv[i] = n.noise( pp[i] );
	}
	return v;
}

template<typename T>
static intrusive_ptr<TypedData<vector<typename T::Value> > > noiseVector2( const T &n, intrusive_ptr<TypedData<vector<typename T::Point> > > p )
{
	return noiseVector<T>( n, p );
}

template<typename T>
void bindNoise( const char *name )
{
	class_<T>( name )
		.def( init<unsigned long int>() )
		.def( "noise", &T::noise )
		.def( "noiseVector", &noiseVector<T>, "Returns an array of noise values when given an array of points. Optionally the values array to be filled may be passed as the last argument - if not specified then a new array is created." )
		.def( "noiseVector", &noiseVector2<T> )
		.def( "initGradients", &T::initGradients )
	;
}

void bindPerlinNoise()
{
	bindNoise<PerlinNoiseV3ff>( "PerlinNoiseV3ff" );
	bindNoise<PerlinNoiseV2ff>( "PerlinNoiseV2ff" );
	bindNoise<PerlinNoiseff>( "PerlinNoiseff" );
	
	bindNoise<PerlinNoiseV3fV2f>( "PerlinNoiseV3fV2f" );
	bindNoise<PerlinNoiseV2fV2f>( "PerlinNoiseV2fV2f" );
	bindNoise<PerlinNoisefV2f>( "PerlinNoisefV2f" );
	
	bindNoise<PerlinNoiseV3fV3f>( "PerlinNoiseV3fV3f" );
	bindNoise<PerlinNoiseV2fV3f>( "PerlinNoiseV2fV3f" );
	bindNoise<PerlinNoisefV3f>( "PerlinNoisefV3f" );
	
	bindNoise<PerlinNoiseV3fColor3f>( "PerlinNoiseV3fColor3f" );
	bindNoise<PerlinNoiseV2fColor3f>( "PerlinNoiseV2fColor3f" );
	bindNoise<PerlinNoisefColor3f>( "PerlinNoisefColor3f" );
}

} // namespace IECore
