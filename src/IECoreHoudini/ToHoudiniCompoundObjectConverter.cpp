//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

#include "IECoreHoudini/ToHoudiniCompoundObjectConverter.h"

#include "IECoreHoudini/ToHoudiniCortexObjectConverter.h"

#include "IECore/CompoundObject.h"

using namespace IECore;
using namespace IECoreHoudini;

IE_CORE_DEFINERUNTIMETYPED( ToHoudiniCompoundObjectConverter );

ToHoudiniGeometryConverter::Description<ToHoudiniCompoundObjectConverter> ToHoudiniCompoundObjectConverter::m_description( CompoundObjectTypeId );

ToHoudiniCompoundObjectConverter::ToHoudiniCompoundObjectConverter( const Object *object ) :
	ToHoudiniGeometryConverter( object, "Converts the members of an IECore::CompoundObject to a Houdini GU_Detail." )
{
}

ToHoudiniCompoundObjectConverter::~ToHoudiniCompoundObjectConverter()
{
}

bool ToHoudiniCompoundObjectConverter::doConversion( const Object *object, GU_Detail *geo ) const
{
	const CompoundObject *compound = IECore::runTimeCast<const CompoundObject>( object );
	if ( !compound )
	{
		return false;
	}

	GU_DetailHandle handle;
	handle.allocateAndSet( geo, false );
	size_t numPrims = geo->getNumPrimitives();

	std::string name = nameParameter()->getTypedValue();
	if ( name != "" )
	{
		name += "/";
	}

	ToHoudiniCortexObjectConverterPtr converter = new ToHoudiniCortexObjectConverter( object );

	const CompoundObject::ObjectMap &members = compound->members();
	for ( CompoundObject::ObjectMap::const_iterator it = members.begin(); it != members.end(); ++it )
	{
		converter->nameParameter()->setTypedValue( name + it->first.string() );
		converter->srcParameter()->setValue( it->second );
		converter->convert( handle );
	}

	return ( (size_t)geo->getNumPrimitives() > numPrims );
}

void ToHoudiniCompoundObjectConverter::transferAttribs( GU_Detail *geo, const GA_Range &points, const GA_Range &prims ) const
{
}
