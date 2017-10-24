//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2012, Image Engine Design Inc. All rights reserved.
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

#include "IECoreGL/ToGLConverter.h"

#include "IECore/ObjectParameter.h"
#include "IECore/CompoundParameter.h"

using namespace IECoreGL;
using namespace IECore;

IE_CORE_DEFINERUNTIMETYPED( ToGLConverter );

ToGLConverter::ToGLConverter( const std::string &description, IECore::TypeId supportedType )
	:	FromCoreConverter( description, supportedType )
{
}

ToGLConverter::~ToGLConverter()
{
}

IECore::RunTimeTypedPtr ToGLConverter::convert()
{
	ConstCompoundObjectPtr operands = parameters()->getTypedValidatedValue<CompoundObject>();
	return doConversion( srcParameter()->getValue(), operands );
}

ToGLConverterPtr ToGLConverter::create( IECore::ConstObjectPtr object, IECore::TypeId resultType )
{
	Registrations &r = registrations();

	IECore::TypeId objectTypeId = object->typeId();
	while( objectTypeId != InvalidTypeId )
	{
		Registrations::const_iterator low = r.lower_bound( objectTypeId );
		Registrations::const_iterator high = r.upper_bound( objectTypeId );
		for( Registrations::const_iterator it = low; it != high; it++ )
		{
			if( it->second.resultType == resultType ||
				IECore::RunTimeTyped::inheritsFrom( it->second.resultType, resultType )
			)
			{
				return it->second.creator( object );
			}
		}
		objectTypeId = IECore::RunTimeTyped::baseTypeId( objectTypeId );
	}
	return nullptr;
}

ToGLConverter::Registrations &ToGLConverter::registrations()
{
	static Registrations r;
	return r;
}
