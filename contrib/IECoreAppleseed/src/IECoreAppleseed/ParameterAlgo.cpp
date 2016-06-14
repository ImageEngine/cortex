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

#include "boost/lexical_cast.hpp"

#include "renderer/api/color.h"

#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"

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

} // namespace

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
			msg( MessageHandler::Warning, "IECoreAppleseed::dataToString", format( "Unknown data typeid \"%s\"." ) % value->typeName() );
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
			msg( MessageHandler::Warning, "IECoreAppleseed::setParam", format( "Unknown data typeid \"%s\"." ) % value->typeName() );
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

} // namespace ParameterAlgo

} // namespace IECoreAppleseed
