//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2016, Image Engine Design Inc. All rights reserved.
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

#include "boost/format.hpp"

#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/MessageHandler.h"

#include "IECoreRI/NSI/AttributeAlgo.h"

using namespace IECore;

namespace
{

NSIType_t convertGeometricInterpretation( GeometricData::Interpretation interpretation )
{
	switch( interpretation )
	{
		case GeometricData::None :
		case GeometricData::UV :
			/// \todo This isn't really right - we should probably treat
			/// this as float[3] instead. We need to be using Interpretation
			/// consistently across the board before this becomes an option
			/// though.
			return NSITypeVector;
		case GeometricData::Point :
			return NSITypePoint;
		case GeometricData::Normal :
			return NSITypeNormal;
		case GeometricData::Vector :
			return NSITypeVector;
		case GeometricData::Color :
			return NSITypeColor;
	}
	return NSITypeInvalid;
}

} // namespace

namespace IECoreRI
{

namespace NSI
{

void setAttribute( NSIContext_t context, NSIHandle_t object, const char *name, const IECore::Data *value )
{
	NSIParam_t param = {
		name,
		NULL, // data
		NSITypeInvalid,
		0, // arraylength
		1  // count
	};

	const char *charPtr = NULL;
	switch( value->typeId() )
	{
		case IntDataTypeId :
			param.type = NSITypeInteger;
			param.data = static_cast<const IntData *>( value )->baseReadable();
			break;
		case FloatDataTypeId :
			param.type = NSITypeFloat;
			param.data = static_cast<const FloatData *>( value )->baseReadable();
			break;
		case Color3fDataTypeId :
			param.type = NSITypeColor;
			param.data = static_cast<const Color3fData *>( value )->baseReadable();
			break;
		case V3fDataTypeId :
			param.type = convertGeometricInterpretation( static_cast<const V3fData *>( value )->getInterpretation() );
			param.data = static_cast<const Color3fData *>( value )->baseReadable();
			break;
		case StringDataTypeId :
			param.type = NSITypeString;
			charPtr = static_cast<const StringData *>( value )->readable().c_str();
			param.data = &charPtr;
			break;
		default :
			msg( Msg::Warning, "NSI::setAttribute", boost::format( "Attribute \"%s\" has unsupported datatype \"%s\"." ) % name % value->typeName() );
			break;
	}

	if( param.type && param.data )
	{
		NSISetAttribute( context, object, 1, &param );
	}
}

void setAttribute( NSIContext_t context, NSIHandle_t object, const IECore::CompoundDataMap &values )
{
	for( CompoundDataMap::const_iterator it = values.begin(), eIt = values.end(); it != eIt; ++it )
	{
		setAttribute( context, object, it->first.c_str(), it->second.get() );
	}
}

} // namespace NSI

} // namespace IECoreRI
