//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2013, Image Engine Design Inc. All rights reserved.
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

#include "UT/UT_StringMMPattern.h"

#include "IECore/CompoundData.h"
#include "IECore/CompoundParameter.h"
#include "IECore/MessageHandler.h"

#include "IECoreHoudini/Convert.h"
#include "IECoreHoudini/ToHoudiniAttribConverter.h"
#include "IECoreHoudini/ToHoudiniGeometryConverter.h"
#include "IECoreHoudini/ToHoudiniStringAttribConverter.h"

using namespace IECore;
using namespace IECoreHoudini;

IE_CORE_DEFINERUNTIMETYPED( ToHoudiniGeometryConverter );

ToHoudiniGeometryConverter::ToHoudiniGeometryConverter( const VisibleRenderable *renderable, const std::string &description )
	:	ToHoudiniConverter( description, VisibleRenderableTypeId )
{
	srcParameter()->setValue( (VisibleRenderable *)renderable );
	
	m_attributeFilterParameter = new StringParameter(
		"attributeFilter",
		"A list of attribute names to convert, if they exist. Uses Houdini matching syntax.",
		"*"
	);
	
	parameters()->addParameter( m_attributeFilterParameter );
}

ToHoudiniGeometryConverter::~ToHoudiniGeometryConverter()
{
}

StringParameter *ToHoudiniGeometryConverter::attributeFilterParameter()
{
	return m_attributeFilterParameter;
}

const StringParameter *ToHoudiniGeometryConverter::attributeFilterParameter() const
{
	return m_attributeFilterParameter;
}

bool ToHoudiniGeometryConverter::convert( GU_DetailHandle handle ) const
{
	ConstCompoundObjectPtr operands = parameters()->getTypedValidatedValue<CompoundObject>();
	GU_DetailHandleAutoWriteLock writeHandle( handle );
	
	GU_Detail *geo = writeHandle.getGdp();
	if ( !geo )
	{
		return false;
	}
	
	const VisibleRenderable *renderable = IECore::runTimeCast<const VisibleRenderable>( srcParameter()->getValidatedValue() );
	if ( !renderable )
	{
		return false;
	}
	
	return doConversion( renderable, geo );
}

GA_Range ToHoudiniGeometryConverter::appendPoints( GA_Detail *geo, const IECore::V3fVectorData *positions ) const
{
	if ( !positions )
	{
		return GA_Range();
	}
	
	const std::vector<Imath::V3f> &pos = positions->readable();
	GA_OffsetList offsets;
	offsets.reserve( pos.size() );
	
	for ( size_t i=0; i < pos.size(); i++ )
	{
		GA_Offset offset = geo->appendPoint();
		geo->setPos3( offset, IECore::convert<UT_Vector3>( pos[i] ) );
		offsets.append( offset );
	}
	
	return GA_Range( geo->getPointMap(), offsets );
}

PrimitiveVariable ToHoudiniGeometryConverter::processPrimitiveVariable( const IECore::Primitive *primitive, const PrimitiveVariable &primVar ) const
{
	return primVar;
}

void ToHoudiniGeometryConverter::transferAttribs( GU_Detail *geo, const GA_Range &points, const GA_Range &prims ) const
{
	const Primitive *primitive = IECore::runTimeCast<const Primitive>( srcParameter()->getValidatedValue() );
	if ( primitive )
	{
		transferAttribValues( primitive, geo, points, prims );
	}
}

void ToHoudiniGeometryConverter::transferAttribValues(
	const Primitive *primitive, GU_Detail *geo,
	const GA_Range &points, const GA_Range &prims,
	PrimitiveVariable::Interpolation vertexInterpolation,
	PrimitiveVariable::Interpolation primitiveInterpolation,
	PrimitiveVariable::Interpolation pointInterpolation,
	PrimitiveVariable::Interpolation detailInterpolation
) const
{
	GA_OffsetList offsets;
	if ( prims.isValid() )
	{
		const GA_PrimitiveList &primitives = geo->getPrimitiveList();
		for ( GA_Iterator it=prims.begin(); !it.atEnd(); ++it )
		{
			const GA_Primitive *prim = primitives.get( it.getOffset() );
			size_t numPrimVerts = prim->getVertexCount();
			for ( size_t v=0; v < numPrimVerts; v++ )
			{
				if ( prim->getTypeId() == GEO_PRIMPOLY )
				{
					offsets.append( prim->getVertexOffset( numPrimVerts - 1 - v ) );
				}
				else
				{
					offsets.append( prim->getVertexOffset( v ) );
				}
			}
		}
	}

	GA_Range vertRange( geo->getVertexMap(), offsets );
	
	UT_String filter( attributeFilterParameter()->getTypedValue() );
	// P should already have been added as points
	/// \todo: we can't be ignoring P anymore
	filter += " ^P";
	
	// match all the string variables to each associated indices variable
	/// \todo: replace all this logic with IECore::IndexedData once it exists...
	PrimitiveVariableMap stringsToIndices;
	for ( PrimitiveVariableMap::const_iterator it=primitive->variables.begin() ; it != primitive->variables.end(); it++ )
	{
		if ( !primitive->isPrimitiveVariableValid( it->second ) )
		{
			IECore::msg( IECore::MessageHandler::Warning, "ToHoudiniGeometryConverter", "PrimitiveVariable " + it->first + " is invalid. Ignoring." );
			filter += UT_String( " ^" + it->first );
			continue;
		}

		ToHoudiniAttribConverterPtr converter = ToHoudiniAttribConverter::create( it->second.data );
		if ( !converter )
		{
			continue;
		}
		
		if ( it->second.data->isInstanceOf( StringVectorDataTypeId ) )
		{
			std::string indicesVariableName = it->first + "Indices";
			PrimitiveVariableMap::const_iterator indices = primitive->variables.find( indicesVariableName );
			if ( indices != primitive->variables.end() && indices->second.data->isInstanceOf( IntVectorDataTypeId ) && primitive->isPrimitiveVariableValid( indices->second ) )
			{
				stringsToIndices[it->first] = indices->second;
				filter += UT_String( " ^" + indicesVariableName );
			}
		}
	}
	
	UT_StringMMPattern attribFilter;
	attribFilter.compile( filter );
	
	/// \todo: should we convert s and t to uv automatically?
	
 	// add the primitive variables to the various GEO_AttribDicts based on interpolation type
	for ( PrimitiveVariableMap::const_iterator it=primitive->variables.begin() ; it != primitive->variables.end(); it++ )
	{
		UT_String varName( it->first );
		if ( !varName.multiMatch( attribFilter ) )
		{
			continue;
		}
		
		PrimitiveVariable primVar = processPrimitiveVariable( primitive, it->second );
		ToHoudiniAttribConverterPtr converter = ToHoudiniAttribConverter::create( primVar.data );
		if ( !converter )
		{
			continue;
		}
		
		PrimitiveVariable::Interpolation interpolation = primVar.interpolation;
		
		if ( converter->isInstanceOf( (IECore::TypeId)ToHoudiniStringVectorAttribConverterTypeId ) )
		{
			PrimitiveVariableMap::const_iterator indices = stringsToIndices.find( it->first );
			if ( indices != stringsToIndices.end() )
			{
				ToHoudiniStringVectorAttribConverter *stringVectorConverter = IECore::runTimeCast<ToHoudiniStringVectorAttribConverter>( converter );
				PrimitiveVariable indicesPrimVar = processPrimitiveVariable( primitive, indices->second );
				stringVectorConverter->indicesParameter()->setValidatedValue( indicesPrimVar.data );
				interpolation = indices->second.interpolation;
			}
		}
		
		if ( interpolation == detailInterpolation )
 		{
			// add detail attribs
			converter->convert( it->first, geo );
	 	}
		else if ( interpolation == pointInterpolation )
		{
			// add point attribs
 			converter->convert( it->first, geo, points );
		}
		else if ( interpolation == primitiveInterpolation )
		{
			// add primitive attribs
			converter->convert( it->first, geo, prims );
		}
		else if ( interpolation == vertexInterpolation )
		{
			// add vertex attribs
			converter->convert( it->first, geo, vertRange );
		}
	}
	
	// add the name attribute based on blindData
	const StringData *nameData = primitive->blindData()->member<StringData>( "name" );
	if ( nameData )
	{
		if ( prims.isValid() )
		{
			StringVectorDataPtr nameVectorData = new StringVectorData();
			nameVectorData->writable().push_back( nameData->readable() );
			std::vector<int> indexValues( prims.getEntries(), 0 );
			IntVectorDataPtr indexData = new IntVectorData( indexValues );
			ToHoudiniStringVectorAttribConverterPtr converter = new ToHoudiniStringVectorAttribConverter( nameVectorData );
			converter->indicesParameter()->setValidatedValue( indexData );
			converter->convert( "name", geo, prims );
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////
// Factory
/////////////////////////////////////////////////////////////////////////////////

ToHoudiniGeometryConverterPtr ToHoudiniGeometryConverter::create( const VisibleRenderable *renderable )
{
	const TypesToFnsMap *m = typesToFns();
	TypesToFnsMap::const_iterator it = m->find( Types( renderable->typeId() ) );
	if( it!=m->end() )
	{
		return it->second( renderable );
	}
	
	return 0;
}

void ToHoudiniGeometryConverter::registerConverter( IECore::TypeId fromType, CreatorFn creator )
{
	TypesToFnsMap *m = typesToFns();
	m->insert( TypesToFnsMap::value_type( Types( fromType ), creator ) );
}

ToHoudiniGeometryConverter::TypesToFnsMap *ToHoudiniGeometryConverter::typesToFns()
{
	static TypesToFnsMap *m = new TypesToFnsMap;
	return m;
}

void ToHoudiniGeometryConverter::supportedTypes( std::set<IECore::TypeId> &types )
{
	types.clear();
	
	const TypesToFnsMap *m = typesToFns();
	for ( TypesToFnsMap::const_iterator it=m->begin(); it != m->end(); it ++ )
	{
		types.insert( it->first.fromType );
	}
}

/////////////////////////////////////////////////////////////////////////////////
// Implementation of nested Types class
/////////////////////////////////////////////////////////////////////////////////

ToHoudiniGeometryConverter::Types::Types( IECore::TypeId from ) : fromType( from )
{
}

bool ToHoudiniGeometryConverter::Types::operator < ( const Types &other ) const
{
	return fromType < other.fromType;
}
