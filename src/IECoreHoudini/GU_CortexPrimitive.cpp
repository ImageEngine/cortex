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

#include "GU/GU_ConvertParms.h"
#include "GU/GU_RayIntersect.h"
#include "UT/UT_Version.h"

#if UT_MAJOR_VERSION_INT >= 13

	#include "UT/UT_MemoryCounter.h"

#endif

#include "IECore/CoordinateSystem.h"
#include "IECore/Object.h"
#include "IECore/Transform.h"

#include "IECoreHoudini/Convert.h"
#include "IECoreHoudini/GU_CortexPrimitive.h"
#include "IECoreHoudini/ToHoudiniPolygonsConverter.h"

using namespace IECoreHoudini;

const char *GU_CortexPrimitive::typeName = "CortexObject";

GU_CortexPrimitive::GU_CortexPrimitive( GU_Detail *gdp, GA_Offset offset )
	: GEO_CortexPrimitive( gdp, offset ), GU_Primitive()
{
}

GU_CortexPrimitive::~GU_CortexPrimitive()
{
}

#if UT_MAJOR_VERSION_INT >= 13

GA_Primitive *GU_CortexPrimitive::create( GA_Detail &detail, GA_Offset offset, const GA_PrimitiveDefinition &definition )
{
	return new GU_CortexPrimitive( static_cast<GU_Detail *>( &detail ), offset );
}

#else

GA_Primitive *GU_CortexPrimitive::create( GA_Detail &detail, GA_Offset offset )
{
	return new GU_CortexPrimitive( static_cast<GU_Detail *>( &detail ), offset );
}

#endif

GU_CortexPrimitive *GU_CortexPrimitive::build( GU_Detail *geo, const IECore::Object *object )
{
	GU_CortexPrimitive *result = (GU_CortexPrimitive *)geo->appendPrimitive( m_definition->getId() );
	
	GA_Offset point = geo->appendPointOffset();
	result->wireVertex( result->m_offset, point );
	result->setObject( object );
	
	if ( const IECore::VisibleRenderable *renderable = IECore::runTimeCast<const IECore::VisibleRenderable>( object ) )
	{
		geo->setPos3( point, IECore::convert<UT_Vector3>( renderable->bound().center() ) );
		return result;
	}
	
	if ( const IECore::CoordinateSystem *coord = IECore::runTimeCast<const IECore::CoordinateSystem>( object ) )
	{
		if ( const IECore::Transform *transform = coord->getTransform() )
		{
			geo->setPos3( point, IECore::convert<UT_Vector3>( transform->transform().translation() ) );
		}
		
		return result;
	}
	
	return result;
}

GA_PrimitiveDefinition *GU_CortexPrimitive::m_definition = 0;

const GA_PrimitiveDefinition &GU_CortexPrimitive::getTypeDef() const
{
	return *m_definition;
}

void GU_CortexPrimitive::setTypeDef( GA_PrimitiveDefinition *def )
{
	if ( !m_definition )
	{
		m_definition = def;
	}
}

GA_PrimitiveTypeId GU_CortexPrimitive::typeId()
{
	return m_definition->getId();
}

int64 GU_CortexPrimitive::getMemoryUsage() const
{
	size_t total = sizeof( this );
	
	if ( m_object )
	{
		total += m_object->memoryUsage();
	}
	
	return total;
}

#if UT_MAJOR_VERSION_INT >= 13

void GU_CortexPrimitive::countMemory( UT_MemoryCounter &counter ) const
{
	/// \todo: its unclear how we're supposed to count objects which are held by multiple
	/// GU_CortexPrimitives, so we're just counting them every time for now.
	counter.countUnshared( getMemoryUsage() );
}

void GU_CortexPrimitive::copyPrimitive( const GEO_Primitive *src )
{
	if ( src == this )
	{
		return;
	}
	
	const GU_CortexPrimitive *orig = (const GU_CortexPrimitive *)src;
	const GA_IndexMap &srcPoints = orig->getParent()->getPointMap();
	
	/// \todo: should we make a shallow or a deep copy?
	m_object = orig->m_object;
	
	GA_VertexWrangler vertexWrangler( *getParent(),	*orig->getParent() );
	
	GA_Offset v = m_offset;
	GA_Offset p = srcPoints.indexFromOffset( orig->getDetail().vertexPoint( 0 ) );
	
	wireVertex( v, p );
	vertexWrangler.copyAttributeValues( v, orig->m_offset );
}

#endif

GEO_Primitive *GU_CortexPrimitive::convert( GU_ConvertParms &parms, GA_PointGroup *usedpts )
{
	GEO_Primitive *prim = doConvert( parms );
	if ( !prim )
	{
		return 0;
	}
	
	if ( usedpts )
	{
		addPointRefToGroup( *usedpts );
	}
	
	if ( GA_PrimitiveGroup *group = parms.getDeletePrimitives() )
	{
		group->add( this );
	}
	else
	{
		getParent()->deletePrimitive( *this, usedpts != NULL );
	}
	
	return prim;
}

GEO_Primitive *GU_CortexPrimitive::convertNew( GU_ConvertParms &parms )
{
	return doConvert( parms );
}

GEO_Primitive *GU_CortexPrimitive::doConvert( GU_ConvertParms &parms )
{
	if ( !m_object )
	{
		return 0;
	}
	
#if UT_MAJOR_VERSION_INT >= 13

	GA_PrimCompat::TypeMask type = parms.toType();

#else

	GA_PrimCompat::TypeMask type = parms.toType;

#endif

	/// \todo: should the GEO_PrimTypeCompat be registered with the converters?
	if ( m_object->isInstanceOf( IECore::MeshPrimitiveTypeId ) && type == GEO_PrimTypeCompat::GEOPRIMPOLY )
	{
		GU_DetailHandle handle;
		handle.allocateAndSet( (GU_Detail*)getParent(), false );
		ToHoudiniPolygonsConverterPtr converter = new ToHoudiniPolygonsConverter( IECore::runTimeCast<const IECore::MeshPrimitive>( m_object ) );
		if ( !converter->convert( handle ) )
		{
			return 0;
		}
	}
	
	/// \todo: support for CurvesPrimitive, PointsPrimitive, and any other existing converters
	
	return 0;
}

void *GU_CortexPrimitive::castTo() const
{
	return (GU_Primitive*)this;
}

const GEO_Primitive *GU_CortexPrimitive::castToGeo() const
{
	return this;
}

void GU_CortexPrimitive::normal( NormalComp &output ) const
{
}

/// \todo: build ray cache and intersect properly
int GU_CortexPrimitive::intersectRay( const UT_Vector3 &o, const UT_Vector3 &d, float tmax, float tol, float *distance, UT_Vector3 *pos, UT_Vector3 *nml, int accurate, float *u, float *v, int ignoretrim ) const
{
	UT_BoundingBox bbox;
	getBBox( &bbox );
	
	float dist;
	int result = bbox.intersectRay( o, d, tmax, &dist, nml );
	if ( result )
	{
		if ( distance )
		{
			*distance = dist;
		}
		
		if ( pos )
		{
			*pos = o + dist * d;
		}
	}
	
	return result;
}

#if UT_MAJOR_VERSION_INT < 13

GU_RayIntersect *GU_CortexPrimitive::createRayCache( int &persistent )
{
	GU_Detail *gdp = (GU_Detail *)getParent();
	
	persistent = 0;
	if ( gdp->cacheable() )
	{
		buildRayCache();
	}
	
	GU_RayIntersect	*intersect = getRayCache();
	if ( !intersect )
	{
		intersect = new GU_RayIntersect( gdp, this );
		persistent = 1;
	}
	
	return intersect;
}

#endif

void GU_CortexPrimitive::infoText( const GU_Detail *geo, OP_Context &context, OP_NodeInfoParms &parms )
{
	if ( !geo )
	{
		return;
	}
	
	std::map<std::string, int> typeMap;
	const GA_PrimitiveList &primitives = geo->getPrimitiveList();
	for ( GA_Iterator it=geo->getPrimitiveRange().begin(); !it.atEnd(); ++it )
	{
		const GA_Primitive *prim = primitives.get( it.getOffset() );
		if ( prim->getTypeId() == GU_CortexPrimitive::typeId() )
		{
			const IECore::Object *object = ((GU_CortexPrimitive *)prim)->getObject();
			if ( object )
			{
				typeMap[object->typeName()] += 1;
			}
		}
	}
	
	if ( typeMap.empty() )
	{
		return;
	}
	
	parms.append( "Cortex Object Details:\n" );
	for ( std::map<std::string, int>::iterator it = typeMap.begin(); it != typeMap.end(); ++it )
	{
		parms.append( ( boost::format( "  %d " + it->first + "s\n" ) % it->second ).str().c_str() );
	}
	parms.append( "\n" );
}
