//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013-2015, Image Engine Design Inc. All rights reserved.
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

#include "IECore/CompoundObject.h"

#include "IECoreHoudini/FromHoudiniCortexObjectConverter.h"
#include "IECoreHoudini/GEO_CortexPrimitive.h"

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreHoudini;

#if UT_MAJOR_VERSION_INT >= 14

typedef GEO_CortexPrimitive CortexPrimitive;

#else

#include "IECoreHoudini/GU_CortexPrimitive.h"

typedef GU_CortexPrimitive CortexPrimitive;

#endif

IE_CORE_DEFINERUNTIMETYPED( FromHoudiniCortexObjectConverter );

FromHoudiniGeometryConverter::Description<FromHoudiniCortexObjectConverter> FromHoudiniCortexObjectConverter::m_description( ObjectTypeId );
FromHoudiniGeometryConverter::Description<FromHoudiniCortexObjectConverter> FromHoudiniCortexObjectConverter::m_universalDescription( InvalidTypeId );

FromHoudiniCortexObjectConverter::FromHoudiniCortexObjectConverter( const GU_DetailHandle &handle ) :
	FromHoudiniGeometryConverter( handle, "Converts a Houdini GU_Detail to an IECore::Object." )
{
}

FromHoudiniCortexObjectConverter::FromHoudiniCortexObjectConverter( const SOP_Node *sop ) :
	FromHoudiniGeometryConverter( sop, "Converts a Houdini GU_Detail to an IECore::Object." )
{
}

FromHoudiniCortexObjectConverter::~FromHoudiniCortexObjectConverter()
{
}

FromHoudiniGeometryConverter::Convertability FromHoudiniCortexObjectConverter::canConvert( const GU_Detail *geo )
{
	size_t numPrims = geo->getNumPrimitives();
	if ( numPrims == 1 )
	{
		const GA_Primitive *prim = geo->getPrimitiveList().get( geo->getPrimitiveRange().begin().getOffset() );

		if ( prim->getTypeId() == CortexPrimitive::typeId() )
		{
			return Ideal;
		}
	}

	return Inapplicable;
}

ObjectPtr FromHoudiniCortexObjectConverter::doDetailConversion( const GU_Detail *geo, const CompoundObject *operands ) const
{
	const GA_Primitive *prim = geo->getPrimitiveList().get( geo->getPrimitiveRange().begin().getOffset() );

	if ( prim->getTypeId() != CortexPrimitive::typeId() )
	{
		throw std::runtime_error( "FromHoudiniCortexObjectConverter: Geometry does not contain a single CortexObject primitive" );
	}

	ConstObjectPtr object = ((CortexPrimitive *)prim)->getObject();

	ObjectPtr result = filterAttribs( object.get(), operands->member<StringData>( "attributeFilter" )->readable().c_str() );

	if ( result )
	{
		return result;
	}

	if ( object )
	{
		return object->copy();
	}

	return 0;
}

ObjectPtr FromHoudiniCortexObjectConverter::filterAttribs( const Object *object, const char *filter ) const
{
	const Primitive *primitive = IECore::runTimeCast<const Primitive>( object );
	if ( !primitive )
	{
		return 0;
	}

	std::vector<InternedString> variablesToErase;
	const PrimitiveVariableMap &variables = primitive->variables;
	for ( PrimitiveVariableMap::const_iterator it = variables.begin(); it != variables.end(); ++it )
	{
		if ( !UT_String( it->first ).multiMatch( filter ) )
		{
			variablesToErase.push_back( it->first );
		}
	}

	if ( variablesToErase.empty() )
	{
		return 0;
	}

	PrimitivePtr result = primitive->copy();
	PrimitiveVariableMap &resultVariables = result->variables;
	for ( size_t i = 0; i < variablesToErase.size(); ++i )
	{
		resultVariables.erase( variablesToErase[i] );
	}

	return result;
}
