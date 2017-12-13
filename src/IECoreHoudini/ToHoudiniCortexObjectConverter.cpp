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

#include "SYS/SYS_Types.h"
#include "UT/UT_StringMMPattern.h"

#include "IECoreHoudini/Convert.h"
#include "IECoreHoudini/GEO_CortexPrimitive.h"
#include "IECoreHoudini/ToHoudiniCortexObjectConverter.h"
#include "IECoreHoudini/ToHoudiniStringAttribConverter.h"

using namespace IECore;
using namespace IECoreHoudini;

#if UT_MAJOR_VERSION_INT >= 14

typedef GEO_CortexPrimitive CortexPrimitive;

#else

#include "IECoreHoudini/GU_CortexPrimitive.h"

typedef GU_CortexPrimitive CortexPrimitive;

#endif

IE_CORE_DEFINERUNTIMETYPED( ToHoudiniCortexObjectConverter );

ToHoudiniGeometryConverter::Description<ToHoudiniCortexObjectConverter> ToHoudiniCortexObjectConverter::m_description( ObjectTypeId );

ToHoudiniCortexObjectConverter::ToHoudiniCortexObjectConverter( const Object *object ) :
	ToHoudiniGeometryConverter( object, "Converts an IECore::Object to a Houdini GU_Detail." )
{
}

ToHoudiniCortexObjectConverter::~ToHoudiniCortexObjectConverter()
{
}

bool ToHoudiniCortexObjectConverter::doConversion( const Object *object, GU_Detail *geo ) const
{
	ConstObjectPtr result = filterAttribs( object );

	GA_Size numPrims = geo->getNumPrimitives();

	CortexPrimitive::build( geo, result.get() );
	
	GA_OffsetList offsets;
	offsets.append( geo->primitiveOffset( numPrims ) );
	GA_Range newPrims( geo->getPrimitiveMap(), offsets );
	
	if ( nameParameter()->getTypedValue() != "" )
	{
		setName( geo, newPrims );
	}
	// backwards compatibility with older data
	else if ( const BlindDataHolder *holder = IECore::runTimeCast<const BlindDataHolder>( object ) )
	{
		if ( const StringData *nameData = holder->blindData()->member<StringData>( "name" ) )
		{
			ToHoudiniStringVectorAttribConverter::convertString( "name", nameData->readable(), geo, newPrims );
		}
	}

	return ( (GA_Size)geo->getNumPrimitives() > numPrims );
}

void ToHoudiniCortexObjectConverter::transferAttribs( GU_Detail *geo, const GA_Range &points, const GA_Range &prims ) const
{
	GA_Primitive *hPrim = geo->getPrimitiveList().get( prims.begin().getOffset() );

	if ( hPrim->getTypeId() != CortexPrimitive::typeId() )
	{
		return;
	}
	
	const Primitive *input = IECore::runTimeCast<const Primitive>( srcParameter()->getValue() );

	Primitive *output = IECore::runTimeCast<Primitive>( ((CortexPrimitive *)hPrim)->getObject() );

	if ( !input || !output )
	{
		return;
	}
	
	const char *filter = attributeFilterParameter()->getTypedValue().c_str();
	for ( PrimitiveVariableMap::const_iterator it = input->variables.begin() ; it != input->variables.end(); ++it )
	{
		if ( !UT_String( it->first ).multiMatch( filter ) )
		{
			continue;
		}
		
		if ( output->isPrimitiveVariableValid( it->second ) )
		{
			output->variables[it->first] = it->second;
		}
	}
	
	if ( UT_String( "P" ).multiMatch( filter ) )
	{
		geo->setPos3( points.begin().getOffset(), IECore::convert<UT_Vector3>( input->bound().center() ) );
	}
}

ConstObjectPtr ToHoudiniCortexObjectConverter::filterAttribs( const Object *object ) const
{
	const Primitive *primitive = IECore::runTimeCast<const Primitive>( object );
	if ( !primitive )
	{
		return object;
	}
	
	const char *filter = attributeFilterParameter()->getTypedValue().c_str();
	
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
		return object;
	}
	
	PrimitivePtr result = primitive->copy();
	PrimitiveVariableMap &resultVariables = result->variables;
	for ( size_t i = 0; i < variablesToErase.size(); ++i )
	{
		resultVariables.erase( variablesToErase[i] );
	}
	
	return result;
}
