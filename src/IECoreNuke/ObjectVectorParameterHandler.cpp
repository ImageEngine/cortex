//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

#include "IECore/ObjectVector.h"
#include "IECore/TypedObjectParameter.h"

#include "IECoreNuke/ObjectVectorParameterHandler.h"
#include "IECoreNuke/OpHolder.h"

using namespace IECoreNuke;

ParameterHandler::Description<ObjectVectorParameterHandler> ObjectVectorParameterHandler::g_description( IECore::ObjectVectorParameter::staticTypeId() );

ObjectVectorParameterHandler::ObjectVectorParameterHandler()
{
}

int ObjectVectorParameterHandler::minimumInputs( const IECore::Parameter *parameter )
{
	return 0;
}

int ObjectVectorParameterHandler::maximumInputs( const IECore::Parameter *parameter )
{
	return 100;
}

bool ObjectVectorParameterHandler::testInput( const IECore::Parameter *parameter, int input, const DD::Image::Op *op )
{
	if( dynamic_cast<const OpHolder *>( op ) )
	{
		return 1;
	}
	return 0;
}

void ObjectVectorParameterHandler::setParameterValue( IECore::Parameter *parameter, InputIterator first, InputIterator last )
{
	assert( last == first + 1 );

	IECore::ObjectVectorPtr value = new IECore::ObjectVector;
	for( InputIterator it=first; it!=last; it++ )
	{
		OpHolder *opHolder = static_cast<OpHolder *>( *it );
		if( opHolder )
		{
			IECore::ObjectPtr o = opHolder->engine();
			value->members().push_back( o );
		}
	}

	parameter->setValue( value );
}
