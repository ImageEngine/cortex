//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2016, Esteban Tovagliari. All rights reserved.
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

#include "IECoreAppleseed/ParameterAlgo.h"

#include "IECore/SimpleTypedData.h"
#include "IECore/SplineData.h"
#include "IECore/VectorTypedData.h"

#include "boost/algorithm/string.hpp"
#include "boost/lexical_cast.hpp"

#include "renderer/api/color.h"

using namespace IECore;
using namespace Imath;

using namespace boost;
using namespace std;

namespace asf = foundation;
namespace asr = renderer;

//////////////////////////////////////////////////////////////////////////
// Internal utilities
//////////////////////////////////////////////////////////////////////////

namespace
{

template<typename Spline>
void declareSplineBasisAndPositions( const InternedString &name, const Spline &spline, asr::ParamArray &params )
{
	string basisParamName = name.string() + "Basis";

	if( spline.basis == Spline::Basis::bezier() )
	{
		params.insert( basisParamName.c_str(), "string bezier" );
	}
	else if( spline.basis == Spline::Basis::bSpline() )
	{
		params.insert( basisParamName.c_str(), "string bspline" );
	}
	else if( spline.basis == Spline::Basis::linear() )
	{
		params.insert( basisParamName.c_str(), "string linear" );
	}
	else
	{
		params.insert( basisParamName.c_str(), "string catmull-rom" );
	}

	stringstream ss;
	ss << "float[] ";
	for( typename Spline::PointContainer::const_iterator it = spline.points.begin(), eIt = spline.points.end(); it != eIt; ++it )
	{
		ss << it->first << " ";
	}

	string positionsParamName = name.string() + "Positions";
	params.insert( positionsParamName.c_str(), ss.str().c_str() );
}

void declareSpline( const InternedString &name, const Splineff &spline, asr::ParamArray &params )
{
	declareSplineBasisAndPositions( name, spline, params );

	stringstream ss;
	ss << "float[] ";
	for( Splineff::PointContainer::const_iterator it = spline.points.begin(), eIt = spline.points.end(); it != eIt; ++it )
	{
		ss << it->second << " ";
	}

	string valuesParamName = name.string() + "Values";
	params.insert( valuesParamName.c_str(), ss.str().c_str() );
}

void declareSpline( const InternedString &name, const SplinefColor3f &spline, asr::ParamArray &params )
{
	declareSplineBasisAndPositions( name, spline, params );

	stringstream ss;
	ss << "color[] ";
	for( SplinefColor3f::PointContainer::const_iterator it = spline.points.begin(), eIt = spline.points.end(); it != eIt; ++it )
	{
		const Color3f &c = it->second;
		ss << c.x << " " << c.y << " " << c.z << " ";
	}

	string valuesParamName = name.string() + "Values";
	params.insert( valuesParamName.c_str(), ss.str().c_str() );
}

}

//////////////////////////////////////////////////////////////////////////
// Implementation of public API.
//////////////////////////////////////////////////////////////////////////

namespace IECoreAppleseed
{

namespace ParameterAlgo
{

string dataToString( const Data *value )
{
	assert( value );

	stringstream ss;

	switch( value->typeId() )
	{
		case IntDataTypeId :
		{
			int x = static_cast<const IntData*>( value )->readable();
			ss << x;
		}
		break;

		case FloatDataTypeId :
		{
			float x = static_cast<const FloatData*>( value )->readable();
			ss << x;
		}
		break;

		case StringDataTypeId :
		{
			const string &x = static_cast<const StringData*>( value )->readable();
			ss << x;
		}
		break;

		case V2iDataTypeId :
		{
			const V2i &x = static_cast<const V2iData*>( value )->readable();
			ss << x.x << ", " << x.y;
		}
		break;

		case Color3fDataTypeId :
		{
			const Color3f &x = static_cast<const Color3fData*>( value )->readable();
			ss << x.x << ", " << x.y << ", " << x.z;
		}
		break;

		case BoolDataTypeId :
		{
			bool x = static_cast<const BoolData*>( value )->readable();
			ss << x;
		}
		break;

		default:
		break;
	}

	return ss.str();
}

string dataToString( ConstDataPtr value )
{
	return dataToString( value.get() );
}

void setParam( const string &name, const Data *value, asr::ParamArray &params )
{
	switch( value->typeId() )
	{
		case IntDataTypeId :
		{
			int x = static_cast<const IntData*>( value )->readable();
			params.insert( name, x );
		}
		break;

		case FloatDataTypeId :
		{
			float x = static_cast<const FloatData*>( value )->readable();
			params.insert( name, x );
		}
		break;

		case StringDataTypeId :
		{
			const string &x = static_cast<const StringData*>( value )->readable();
			params.insert( name, x.c_str() );
		}
		break;

		case BoolDataTypeId :
		{
			bool x = static_cast<const BoolData*>( value )->readable();
			params.insert( name, x );
		}
		break;

		default:
		break;
	}
}

asr::ParamArray convertParams( const CompoundDataMap &parameters )
{
	asr::ParamArray result;

	for( CompoundDataMap::const_iterator it=parameters.begin(); it!=parameters.end(); ++it )
		setParam( it->first.value(), it->second.get(), result );

	return result;
}

asr::ParamArray convertShaderParameters( const CompoundDataMap &parameters )
{
	asr::ParamArray params;

	for( CompoundDataMap::const_iterator it = parameters.begin(), eIt = parameters.end(); it != eIt; ++it )
	{
		std::stringstream ss;

		const Data *data = it->second.get();
		switch( data->typeId() )
		{
			case FloatDataTypeId :
			{
				const float *p = static_cast<const FloatData *>( data )->baseReadable();
				ss << "float " << *p;
			}
			break;

			case IntDataTypeId :
			{
				const int *p = static_cast<const IntData *>( data )->baseReadable();
				ss << "int " << *p;
			}
			break;

			case V3fDataTypeId :
			{
				const float *p = static_cast<const V3fData *>( data )->baseReadable();
				ss << "vector " << p[0] << " " << p[1] << " " << p[2];
			}
			break;

			case Color3fDataTypeId :
			{
				const float *p = static_cast<const Color3fData *>( data )->baseReadable();
				ss << "color " << p[0] << " " << p[1] << " " << p[2];
			}
			break;

			case StringDataTypeId :
			{
				const std::string *p = &(static_cast<const StringData *>( data )->readable() );
				ss << "string " << *p;
			}
			break;

			case M44fDataTypeId:
			{
				const float *p = static_cast<const M44fData *>( data )->baseReadable();
				ss << "matrix ";
				ss << p[ 0] << " " << p[ 1] << " " << p[ 2] << " " << p[ 3] << " ";
				ss << p[ 4] << " " << p[ 5] << " " << p[ 6] << " " << p[ 7] << " ";
				ss << p[ 8] << " " << p[ 9] << " " << p[10] << " " << p[11] << " ";
				ss << p[12] << " " << p[13] << " " << p[14] << " " << p[15];
			}
			break;

			case IntVectorDataTypeId:
			{
				const std::vector<int> &p = static_cast<const IntVectorData *>( data )->readable();
				ss << "int[] ";
				for( size_t i = 0, e = p.size(); i < e; ++i )
				{
					ss << p[i] << " ";
				}
			}
			break;

			case FloatVectorDataTypeId:
			{
				const std::vector<float> &p = static_cast<const FloatVectorData *>( data )->readable();
				ss << "float[] ";
				for( size_t i = 0, e = p.size(); i < e; ++i )
				{
					ss << p[i] << " ";
				}
			}
			break;

			case SplineffDataTypeId:
			{
				const SplineffData *splineData = static_cast<const SplineffData *>( data );
				declareSpline( it->first, splineData->readable(), params );
				continue;
			}
			break;

			case SplinefColor3fDataTypeId:
			{
				const SplinefColor3fData *splineData = static_cast<const SplinefColor3fData *>( data );
				declareSpline( it->first, splineData->readable(), params );
				continue;
			}
			break;

			default:
			break;
		}

		if( !ss.str().empty() )
		{
			params.insert( it->first.c_str(), ss.str() );
		}
	}

	return params;
}

} // namespace ParameterAlgo

} // namespace IECoreAppleseed
