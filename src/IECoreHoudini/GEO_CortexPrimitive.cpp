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

#include "IECoreHoudini/GEO_CortexPrimitive.h"

#include "IECoreHoudini/Convert.h"
#include "IECoreHoudini/CoreHoudiniVersion.h"

#ifdef IECOREHOUDINI_WITH_GL
#include "IECoreHoudini/GUI_CortexPrimitiveHook.h"
#endif

#include "IECoreHoudini/SOP_OpHolder.h"
#include "IECoreHoudini/ToHoudiniPolygonsConverter.h"
#include "IECoreHoudini/UT_ObjectPoolCache.h"


#include "IECoreScene/CoordinateSystem.h"
#include "IECoreScene/Group.h"
#include "IECoreScene/MatrixTransform.h"
#include "IECoreScene/Primitive.h"
#include "IECoreScene/TransformOp.h"
#include "IECoreScene/VisibleRenderable.h"

#include "IECore/HexConversion.h"
#include "IECore/MemoryIndexedIO.h"

#ifdef IECOREHOUDINI_WITH_GL
#include "DM/DM_RenderTable.h"
#endif

#include "GA/GA_Defragment.h"
#include "GA/GA_ElementWrangler.h"
#include "GA/GA_IndexMap.h"
#include "GA/GA_MergeMap.h"
#include "GA/GA_Primitive.h"
#include "GA/GA_PrimitiveJSON.h"
#include "GA/GA_RangeMemberQuery.h"
#include "GA/GA_SaveMap.h"
#include "GEO/GEO_Detail.h"
#include "GU/GU_RayIntersect.h"
#include "UT/UT_JSONParser.h"
#include "UT/UT_JSONWriter.h"
#include "UT/UT_MemoryCounter.h"
#include "UT/UT_StringHolder.h"

#if UT_MAJOR_VERSION_INT < 14

	#include "GU/GU_ConvertParms.h"
	#include "IECoreHoudini/GU_CortexPrimitive.h"

#else

	#include "GEO/GEO_ConvertParms.h"

#endif

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreHoudini;

GEO_CortexPrimitive::GEO_CortexPrimitive( GA_Detail *detail, GA_Offset offset )
	: GEO_Primitive( detail, offset )
{
#if UT_MAJOR_VERSION_INT < 16
	m_offset = allocateVertex();
#endif

}

// \todo remove this in cortex 10
GEO_CortexPrimitive::GEO_CortexPrimitive( GEO_Detail *detail, GA_Offset offset )
	: GEO_Primitive( detail, offset )
{
#if UT_MAJOR_VERSION_INT < 16
	m_offset = allocateVertex();
#endif
}

// in H16 and later vertex lists are managed by GA_Primitive, and merge constructor is no longer allowed, so these functions aren't necessary
#if UT_MAJOR_VERSION_INT < 16
GEO_CortexPrimitive::GEO_CortexPrimitive( const GA_MergeMap &map, GA_Detail &detail, GA_Offset offset, const GA_Primitive &src )
	: GEO_Primitive( static_cast<GEO_Detail *>( &detail ), offset )
{
	const GEO_CortexPrimitive *orig = static_cast<const GEO_CortexPrimitive *>( &src );

	m_offset = ( map.isIdentityMap( GA_ATTRIB_VERTEX ) ) ? orig->m_offset : map.mapDestFromSource( GA_ATTRIB_VERTEX, orig->m_offset );

	m_object = orig->m_object->copy();
}
#endif

GEO_CortexPrimitive::~GEO_CortexPrimitive()
{
#if UT_MAJOR_VERSION_INT < 16
	if ( GAisValid( m_offset ) )
	{
		destroyVertex( m_offset );
	}
#endif
}

#if UT_MAJOR_VERSION_INT < 16
void GEO_CortexPrimitive::swapVertexOffsets( const GA_Defragment &defrag )
{
	if ( defrag.hasOffsetChanged( m_offset ) )
	{
		m_offset = defrag.mapOffset( m_offset );
	}
}

GA_Size GEO_CortexPrimitive::getVertexCount() const
{
	return GAisValid( m_offset ) ? 1 : 0;
}

GA_Offset GEO_CortexPrimitive::getVertexOffset( GA_Size index ) const
{
	return ( index == 0 ) ? m_offset : GA_INVALID_OFFSET;
}
#endif

GA_Primitive::GA_DereferenceStatus GEO_CortexPrimitive::dereferencePoint( GA_Offset point, bool dry_run )
{
	if ( isDegenerate() )
	{
		return GA_DEREFERENCE_DEGENERATE;
	}

	return GA_DEREFERENCE_FAIL;
}

GA_Primitive::GA_DereferenceStatus GEO_CortexPrimitive::dereferencePoints( const GA_RangeMemberQuery &pt_q, bool dry_run )
{
	if ( isDegenerate() )
	{
		return GA_DEREFERENCE_DEGENERATE;
	}

	return GA_DEREFERENCE_FAIL;
}

#if UT_MAJOR_VERSION_INT >= 13

void GEO_CortexPrimitive::stashed( bool beingstashed, GA_Offset offset )
{
	GEO_Primitive::stashed( beingstashed, offset );
	// remove reference to the m_object when being stashed
	m_object = 0;

#if UT_MAJOR_VERSION_INT < 16
	if( beingstashed )
	{
		m_offset = GA_INVALID_OFFSET;
	}
	else
	{
		m_offset = allocateVertex();
	}
#endif
}

#endif

// \todo remove in Cortex 10
void GEO_CortexPrimitive::stashed( int onoff, GA_Offset offset )
{
	GEO_Primitive::stashed( onoff, offset );

	m_object = 0;
#if UT_MAJOR_VERSION_INT < 16
	if ( onoff )
	{
		m_offset = GA_INVALID_OFFSET;
	}
	else
	{
		m_offset = allocateVertex();
	}
#endif
}

void GEO_CortexPrimitive::clearForDeletion()
{
	m_object = 0;
#if UT_MAJOR_VERSION_INT < 16
	m_offset = GA_INVALID_OFFSET;
#endif
	GEO_Primitive::clearForDeletion();
}

bool GEO_CortexPrimitive::isDegenerate() const
{
	return false;
}

void GEO_CortexPrimitive::copyUnwiredForMerge( const GA_Primitive *src, const GA_MergeMap &map )
{
	const GEO_CortexPrimitive *orig = static_cast<const GEO_CortexPrimitive *>( src );

	m_object = orig->m_object->copy();

#if UT_MAJOR_VERSION_INT >= 16
	GEO_Primitive::copyUnwiredForMerge(src, map);
#else
	if ( GAisValid( m_offset ) )
	{
		destroyVertex(  m_offset );
	}

	m_offset = ( map.isIdentityMap( GA_ATTRIB_VERTEX ) ) ? orig->m_offset : map.mapDestFromSource( GA_ATTRIB_VERTEX, orig->m_offset );
#endif
}

#if UT_MAJOR_VERSION_INT >= 18
void GEO_CortexPrimitive::copySubclassData(const GA_Primitive *src)
{
	const GEO_CortexPrimitive *orig = static_cast<const GEO_CortexPrimitive *>( src );
	m_object = orig->m_object->copy();
}
#endif

void GEO_CortexPrimitive::transform( const UT_Matrix4 &xform )
{
	if ( xform.isIdentity() )
	{
		return;
	}

	Imath::M44f transform = IECore::convert<Imath::M44f>( xform );

	if ( Primitive *primitive = IECore::runTimeCast<Primitive>( m_object.get() ) )
	{
		TransformOpPtr transformer = new TransformOp();
		transformer->inputParameter()->setValue( primitive );
		transformer->copyParameter()->setTypedValue( false );
		transformer->matrixParameter()->setValue( new M44fData( transform ) );
		transformer->operate();
	}
	else if ( Group *group = IECore::runTimeCast<Group>( m_object.get() ) )
	{
		if ( MatrixTransform *matTransform = IECore::runTimeCast<MatrixTransform>( group->getTransform() ) )
		{
			matTransform->matrix = transform * matTransform->matrix;
		}
	}
	else if ( CoordinateSystem *coord = IECore::runTimeCast<CoordinateSystem>( m_object.get() ) )
	{
		if ( MatrixTransform *matTransform = IECore::runTimeCast<MatrixTransform>( coord->getTransform() ) )
		{
			matTransform->matrix = transform * matTransform->matrix;
		}
	}
}

void GEO_CortexPrimitive::reverse()
{
}

int GEO_CortexPrimitive::getBBox( UT_BoundingBox *bbox ) const
{
	if ( !m_object )
	{
		return 0;
	}

	const IECoreScene::VisibleRenderable *renderable = IECore::runTimeCast<const IECoreScene::VisibleRenderable>( m_object.get() );
	if ( !renderable )
	{
		return 0;
	}

	Imath::Box3f bound = renderable->bound();
	bbox->setBounds( bound.min.x, bound.min.y, bound.min.z, bound.max.x, bound.max.y, bound.max.z );

	return 1;
}

void GEO_CortexPrimitive::enlargePointBounds( UT_BoundingBox &box ) const
{
	UT_BoundingBox bounds;
	if ( getBBox( &bounds ) )
	{
		box.enlargeBounds( bounds );
	}

	GEO_Primitive::enlargePointBounds( box );
}

UT_Vector3 GEO_CortexPrimitive::computeNormal() const
{
	return UT_Vector3( 0, 0, 0 );
}

int GEO_CortexPrimitive::detachPoints( GA_PointGroup &grp )
{
#if UT_MAJOR_VERSION_INT >= 16
	if ( grp.containsOffset( getPointOffset( 0 ) ) )
#else
	if ( grp.containsOffset( getDetail().vertexPoint( 0 ) ) )
#endif
	{
		return -2;
	}

	return 0;
}

// \todo: remove this in Cortex 10, not used since H12
void GEO_CortexPrimitive::copyPrimitive( const GEO_Primitive *src, GEO_Point **ptredirect )
{
#if UT_MAJOR_VERSION_INT < 16
	if ( src == this )
	{
		return;
	}

	const GEO_CortexPrimitive *orig = (const GEO_CortexPrimitive *)src;

	const GA_IndexMap &srcPoints = orig->getParent()->getPointMap();

	/// \todo: should we make a shallow or a deep copy?
	m_object = orig->m_object;

	GA_VertexWrangler vertexWrangler( *getParent(),	*orig->getParent() );

	GA_Offset v = m_offset;

#if UT_MAJOR_VERSION_INT >= 14

	GA_Offset p = srcPoints.offsetFromIndex( srcPoints.indexFromOffset( orig->getDetail().vertexPoint( 0 ) ) );
	wireVertex( v, p );

#else

	GEO_Point *point = ptredirect[ srcPoints.indexFromOffset( orig->getDetail().vertexPoint( 0 ) ) ];
	wireVertex( v, point ? point->getMapOffset() : GA_INVALID_OFFSET );

#endif

	vertexWrangler.copyAttributeValues( v, orig->m_offset );

#endif
}

// \todo: remove this in Cortex 10, not used since H12
#if (UT_VERSION_INT >= 0x0c050132) // 12.5.306 or later
void GEO_CortexPrimitive::copyOffsetPrimitive( const GEO_Primitive *src, GA_Index basept )
#else
void GEO_CortexPrimitive::copyOffsetPrimitive( const GEO_Primitive *src, int basept )
#endif
{
#if UT_MAJOR_VERSION_INT < 16

	if ( src == this )
	{
		return;
	}

	const GEO_CortexPrimitive *orig = (const GEO_CortexPrimitive *)src;

	const GA_IndexMap &points = getParent()->getPointMap();
	const GA_IndexMap &srcPoints = orig->getParent()->getPointMap();

	/// \todo: should we make a shallow or a deep copy?
	m_object = orig->m_object;

	GA_VertexWrangler vertexWrangler( *getParent(),	*orig->getParent() );

	GA_Offset v = m_offset;
	GA_Offset point = points.offsetFromIndex( srcPoints.indexFromOffset( orig->getDetail().vertexPoint( 0 ) ) + basept );
	wireVertex( v, point );
	vertexWrangler.copyAttributeValues( v, orig->m_offset );

#endif
}

bool GEO_CortexPrimitive::evaluatePointRefMap( GA_Offset result_vtx, GA_AttributeRefMap &map, fpreal u, fpreal v, uint du, uint dv ) const
{
	return false;
}

IECore::Object *GEO_CortexPrimitive::getObject()
{
	return m_object.get();
}

const IECore::Object *GEO_CortexPrimitive::getObject() const
{
	return m_object.get();
}

void GEO_CortexPrimitive::setObject( const IECore::Object *object )
{
	/// \todo: should this be a deep copy?
	m_object = object->copy();
}

const char *GEO_CortexPrimitive::typeName = "CortexObject";
GA_PrimitiveDefinition *GEO_CortexPrimitive::m_definition = 0;

const GA_PrimitiveDefinition &GEO_CortexPrimitive::getTypeDef() const
{
	return *m_definition;
}

GA_PrimitiveTypeId GEO_CortexPrimitive::typeId()
{
	// m_definition is set by calling setTypeDef above & this is performed by the houdiniPlugin.
	// setting the id to -1 defines the GA_PrimitiveTypeId as invalid
	if( !m_definition )
	{
		return GA_PrimitiveTypeId( -1 );
	}

	return m_definition->getId();
}

#if UT_MAJOR_VERSION_INT >= 16

void GEO_CortexPrimitive::create(
		GA_Primitive **newPrims,
		GA_Size numPrimitives,
		GA_Detail &detail,
		GA_Offset startOffset,
		const GA_PrimitiveDefinition &def
#if MIN_HOU_VERSION(16, 5, 0)
		, bool allowed_to_parallelize
#endif
		)
{

	// allocate all the points and vertices at the same time
	GA_Offset pointBlock = detail.appendPointBlock( numPrimitives );
#if MIN_HOU_VERSION(16, 5, 0)
	if ( allowed_to_parallelize && numPrimitives >= 4*GA_PAGE_SIZE )
#else
	if ( numPrimitives >= 4*GA_PAGE_SIZE )
#endif
	{
		// Allocate them in parallel if we're allocating many.
		// This is using the C++11 lambda syntax to make a functor.
		UTparallelForLightItems(UT_BlockedRange<GA_Offset>( startOffset, startOffset + numPrimitives ),
				[ newPrims, &detail, startOffset, &pointBlock ]( const UT_BlockedRange<GA_Offset> &r ){
			GA_Offset primOffset( r.begin() );
			GA_Primitive **pprims = newPrims + ( primOffset - startOffset );
			GA_Offset endOffset( r.end() );
			for ( ; primOffset != endOffset; ++primOffset, ++pprims, ++pointBlock )
			{
				GEO_CortexPrimitive * newPrim = new GEO_CortexPrimitive( &detail , primOffset );
				GA_Offset vertex = newPrim->allocateVertex( pointBlock );
				newPrim->myVertexList.setTrivial( vertex, 1 );
				*pprims = newPrim;
			}
		});
	}
	else
	{
		// Allocate them serially if we're only allocating a few.
		GA_Offset endOffset( startOffset + numPrimitives );
		for ( GA_Offset primOffset( startOffset ); primOffset != endOffset; ++primOffset, ++newPrims, ++pointBlock )
		{
			GEO_CortexPrimitive * newPrim = new GEO_CortexPrimitive( &detail , primOffset );
			GA_Offset vertex = newPrim->allocateVertex( pointBlock );
			newPrim->myVertexList.setTrivial( vertex, 1 );
			*newPrims = newPrim;
		}
	}
}

#elif UT_MAJOR_VERSION_INT >= 14

GA_Primitive *GEO_CortexPrimitive::create( GA_Detail &detail, GA_Offset offset, const GA_PrimitiveDefinition &definition )
{
	return new GEO_CortexPrimitive( static_cast<GU_Detail *>( &detail ), offset );
}

GA_Primitive *GEO_CortexPrimitive::create( const GA_MergeMap &map, GA_Detail &detail, GA_Offset offset, const GA_Primitive &src )
{
	return new GEO_CortexPrimitive( map, detail, offset, src );
}
#endif

GEO_CortexPrimitive *GEO_CortexPrimitive::build( GU_Detail *geo, const IECore::Object *object )
{
	GEO_CortexPrimitive *result = (GEO_CortexPrimitive *)geo->appendPrimitive( m_definition->getId() );

#if UT_MAJOR_VERSION_INT >= 16
	GA_Offset point = result->getPointOffset(0 );
#else
	GA_Offset point = geo->appendPointOffset();
	result->wireVertex( result->m_offset, point );
#endif

	result->setObject( object );

	if ( const IECoreScene::VisibleRenderable *renderable = IECore::runTimeCast<const IECoreScene::VisibleRenderable>( object ) )
	{
		geo->setPos3( point, IECore::convert<UT_Vector3>( renderable->bound().center() ) );
		return result;
	}

	if ( const IECoreScene::CoordinateSystem *coord = IECore::runTimeCast<const IECoreScene::CoordinateSystem>( object ) )
	{
		if ( const IECoreScene::Transform *transform = coord->getTransform() )
		{
			geo->setPos3( point, IECore::convert<UT_Vector3>( transform->transform().translation() ) );
		}

		return result;
	}

	return result;
}

int64 GEO_CortexPrimitive::getMemoryUsage() const
{
#if UT_MAJOR_VERSION_INT >= 16
	size_t total = GEO_Primitive::getMemoryUsage();
#else
	size_t total = sizeof( this );
#endif

	if ( m_object )
	{
		total += m_object->memoryUsage();
	}

	return total;
}

void GEO_CortexPrimitive::countMemory( UT_MemoryCounter &counter ) const
{
	/// \todo: its unclear how we're supposed to count objects which are held by multiple
	/// GEO_CortexPrimitives, so we're just counting them every time for now.
	counter.countUnshared( getMemoryUsage() );
}

void GEO_CortexPrimitive::copyPrimitive( const GEO_Primitive *src )
{
	if ( src == this )
	{
		return;
	}

	const GEO_CortexPrimitive *orig = (const GEO_CortexPrimitive *)src;

	/// \todo: should we make a shallow or a deep copy?
	m_object = orig->m_object;

#if UT_MAJOR_VERSION_INT >= 16
	GEO_Primitive::copyPrimitive(src);
#else
	// this will also copy the attribute versions, but according to the header, it shouldn't
	const GA_IndexMap &srcPoints = orig->getParent()->getPointMap();

	GA_VertexWrangler vertexWrangler( *getParent(),	*orig->getParent() );

	GA_Offset v = m_offset;
	GA_Offset p = srcPoints.indexFromOffset( orig->getDetail().vertexPoint( 0 ) );

	wireVertex( v, p );
	vertexWrangler.copyAttributeValues( v, orig->m_offset );
#endif
}

GEO_Primitive * GEO_CortexPrimitive::copy(int preserve_shared_pts) const
{
	GEO_CortexPrimitive *clone = static_cast<GEO_CortexPrimitive *>( GEO_Primitive::copy( preserve_shared_pts ) );
	if( !clone )
	{
		return nullptr;
	}

	/// \todo: should we make a shallow or a deep copy?
	clone->m_object = m_object;

	return clone;
}

GEO_Primitive *GEO_CortexPrimitive::convert( ConvertParms &parms, GA_PointGroup *usedpts )
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

GEO_Primitive *GEO_CortexPrimitive::convertNew( ConvertParms &parms )
{
	return doConvert( parms );
}

GEO_Primitive *GEO_CortexPrimitive::doConvert( ConvertParms &parms )
{
	if ( !m_object )
	{
		return 0;
	}

	GA_PrimCompat::TypeMask type = parms.toType();

	/// \todo: should the GEO_PrimTypeCompat be registered with the converters?
	if ( m_object->isInstanceOf( IECoreScene::MeshPrimitive::staticTypeId() ) && type == GEO_PrimTypeCompat::GEOPRIMPOLY )
	{
		GU_DetailHandle handle;
		handle.allocateAndSet( (GU_Detail*)getParent(), false );
		ToHoudiniPolygonsConverterPtr converter = new ToHoudiniPolygonsConverter( IECore::runTimeCast<const IECoreScene::MeshPrimitive>( m_object.get() ) );
		if ( !converter->convert( handle ) )
		{
			return 0;
		}
	}

	/// \todo: support for CurvesPrimitive, PointsPrimitive, and any other existing converters

	return 0;
}

void GEO_CortexPrimitive::normal( NormalComp &output ) const
{
}

#ifdef GA_PRIMITIVE_VERTEXLIST

bool GEO_CortexPrimitive::saveVertexArray( UT_JSONWriter &w, const GA_SaveMap &map ) const
{
	return myVertexList.jsonVertexArray( w, map );
}

bool GEO_CortexPrimitive::loadVertexArray( UT_JSONParser &p, const GA_LoadMap &map )
{
	GA_Offset startVtxOff = map.getVertexOffset();

	int64 vtxOffs[1];
	int nVertex = p.parseUniformArray( vtxOffs, 1 );
	if ( startVtxOff != GA_Offset( 0 ) )
	{
		for ( int i = 0; i < nVertex; i++ )
		{
			if ( vtxOffs[i] >= 0 )
				vtxOffs[i] += GA_Size( startVtxOff );
		}
	}
	for ( int i = nVertex; i < 1; ++i )
	{
		vtxOffs[i] = GA_INVALID_OFFSET;
	}
	myVertexList.set( vtxOffs, 1, GA_Offset( 0 ) );
	if ( nVertex < 1 )
	{
		return false;
	}
	return true;
}

#endif

/// \todo: build ray cache and intersect properly
int GEO_CortexPrimitive::intersectRay( const UT_Vector3 &o, const UT_Vector3 &d, float tmax, float tol, float *distance, UT_Vector3 *pos, UT_Vector3 *nml, int accurate, float *u, float *v, int ignoretrim ) const
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

void GEO_CortexPrimitive::infoText( const GU_Detail *geo, OP_Context &context, OP_NodeInfoParms &parms )
{
	if ( !geo )
	{
		return;
	}

	std::map<std::string, int> typeMap;
	const GA_PrimitiveList &primitives = geo->getPrimitiveList();

	GA_Offset start, end;
	for( GA_Iterator it( geo->getPrimitiveRange() ); it.blockAdvance( start, end ); )
	{
		for( GA_Offset offset = start; offset < end; ++offset )
		{
			const GA_Primitive *prim = primitives.get( offset );

			if( prim->getTypeId() == GEO_CortexPrimitive::typeId() )
			{
				if( const IECore::Object *object = ( (GEO_CortexPrimitive *) prim )->getObject() )
				{
					typeMap[object->typeName()] += 1;
				}
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

namespace
{

static UT_StringHolder vertex_sh = "vertex";
static UT_StringHolder cortex_sh = "cortex";

} // namespace

class GEO_CortexPrimitive::geo_CortexPrimitiveJSON : public GA_PrimitiveJSON
{
	public :

		geo_CortexPrimitiveJSON()
		{
		}

		virtual ~geo_CortexPrimitiveJSON()
		{
		}

		enum
		{
			geo_TBJ_VERTEX,
			geo_TBJ_CORTEX,
			geo_TBJ_ENTRIES
		};

		const GEO_CortexPrimitive *object( const GA_Primitive *p ) const
		{
			return static_cast<const GEO_CortexPrimitive *>(p);
		}

		GEO_CortexPrimitive *object( GA_Primitive *p ) const
		{
			return static_cast<GEO_CortexPrimitive *>(p);
		}

		virtual int getEntries() const
		{
			return geo_TBJ_ENTRIES;
		}


#if UT_MAJOR_VERSION_INT < 17

		virtual const char *getKeyword( int i ) const
		{
			switch ( i )
			{
				case geo_TBJ_VERTEX :
				{
					return "vertex";
				}
				case geo_TBJ_CORTEX :
				{
					return "cortex";
				}
				case geo_TBJ_ENTRIES :
				{
					break;
				}
			}

			return 0;
		}

#else

		virtual const UT_StringHolder &getKeyword( int i ) const
		{
			switch ( i )
			{
				case geo_TBJ_VERTEX :
				{
					return vertex_sh;
				}
				case geo_TBJ_CORTEX :
				{
					return cortex_sh;
				}
				case geo_TBJ_ENTRIES :
				{
					break;
				}
			}

			return UT_StringHolder::theEmptyString;
		}

#endif

		virtual bool shouldSaveField( const GA_Primitive *prim, int i, const GA_SaveMap &sm ) const
		{
			switch ( i )
			{
				case geo_TBJ_VERTEX :
				{
					return true;
				}
				case geo_TBJ_CORTEX :
				{
					return true;
				}
				case geo_TBJ_ENTRIES :
				{
					break;
				}
			}

			return false;
		}

		virtual bool saveField( const GA_Primitive *pr, int i, UT_JSONWriter &w, const GA_SaveMap &map ) const
		{
			switch ( i )
			{
				case geo_TBJ_VERTEX :
				{
#ifdef GA_PRIMITIVE_VERTEXLIST

					return object(pr)->saveVertexArray(w, map);

#else

					GA_Offset offset = object( pr )->getVertexOffset( 0 );
					return w.jsonInt( int64( map.getVertexIndex( offset ) ) );

#endif
				}
				case geo_TBJ_CORTEX :
				{
					const IECore::Object *obj = object( pr )->getObject();
					if ( !obj )
					{
						return false;
					}

					try
					{
						IECore::MemoryIndexedIOPtr io = new IECore::MemoryIndexedIO( IECore::ConstCharVectorDataPtr(), IECore::IndexedIO::rootPath, IECore::IndexedIO::Exclusive | IECore::IndexedIO::Write );

						obj->save( io, "object" );

						IECore::ConstCharVectorDataPtr buf = io->buffer();
						const IECore::CharVectorData::ValueType &data = buf->readable();

						if ( w.getBinary() )
						{
							int64 length = data.size();
							w.jsonValue( length );

							UT_JSONWriter::TiledStream out( w );
							out.write( &data[0], length );
						}
						else
						{
							std::string str = IECore::decToHex( data.begin(), data.end() );
							w.jsonString( str.c_str(), str.size() );
						}
					}
					catch ( std::exception &e )
					{
						std::cerr << e.what() << std::endl;
						return false;
					}

					return true;
				}
				case geo_TBJ_ENTRIES :
				{
					break;
				}
			}

			return false;
		}

		virtual bool loadField( GA_Primitive *pr, int i, UT_JSONParser &p, const GA_LoadMap &map ) const
		{
			switch ( i )
			{
				case geo_TBJ_VERTEX :
				{
#ifdef GA_PRIMITIVE_VERTEXLIST
					 return object(pr)->loadVertexArray(p, map);
#else
					int64 vId;
					if ( !p.parseInt( vId ) )
					{
						return false;
					}

					GEO_CortexPrimitive *prim = object( pr );
					GA_Offset offset = map.getVertexOffset( GA_Index( vId ) );
					if ( prim->m_offset != offset )
					{
						prim->destroyVertex( prim->m_offset );
						prim->m_offset = offset;
					}

					return true;

#endif
				}
				case geo_TBJ_CORTEX :
				{
					try
					{
						IECore::CharVectorDataPtr buf = new IECore::CharVectorData();

						if ( p.getBinary() )
						{
							int64 length;
							if ( !p.parseValue( length ) )
							{
								return false;
							}

							UT_JSONParser::TiledStream in( p );
							buf->writable().resize( length );
							in.read( &buf->writable()[0], length );
						}
						else
						{
							UT_WorkBuffer workBuffer;
							if ( !p.parseString( workBuffer ) )
							{
								return false;
							}

							buf->writable().resize( workBuffer.length() / 2 );
							IECore::hexToDec<char>( workBuffer.buffer(), workBuffer.buffer() + workBuffer.length(), buf->writable().begin() );
						}

						IECore::MemoryIndexedIOPtr io = new IECore::MemoryIndexedIO( buf, IECore::IndexedIO::rootPath, IECore::IndexedIO::Exclusive | IECore::IndexedIO::Read );
						object( pr )->setObject( IECore::Object::load( io, "object" ).get() );
					}
					catch ( std::exception &e )
					{
						std::cerr << e.what() << std::endl;
						return false;
					}

					return true;
				}
				case geo_TBJ_ENTRIES :
				{
					break;
				}
			}

			return false;
		}

		virtual bool isEqual( int i, const GA_Primitive *p0, const GA_Primitive *p1 ) const
		{
			switch ( i )
			{
				case geo_TBJ_VERTEX :
				{
					return ( p0->getVertexOffset( 0 ) == p1->getVertexOffset( 0 ) );
				}
				case geo_TBJ_CORTEX :
				{
					/// \todo: should this be returning object( p0 )->getPrimitive()->isSame( object( p1 )->getPrimitive() )?
					return false;
				}
				case geo_TBJ_ENTRIES :
				{
					break;
				}
			}

			UT_ASSERT(0);
			return false;
		}

/// These methods were pure virtual in Houdini 12.1
#if UT_MAJOR_VERSION_INT >= 12 && UT_MINOR_VERSION_INT <= 1

		virtual bool saveField( const GA_Primitive *pr, int i, UT_JSONValue &val, const GA_SaveMap &map ) const
		{
			UT_AutoJSONWriter w( val );
			return saveField( pr, i, *w, map );
		}

		virtual bool loadField( GA_Primitive *pr, int i, UT_JSONParser &p, const UT_JSONValue &jval, const GA_LoadMap &map ) const
		{
			UT_AutoJSONParser parser( jval );
			bool ok = loadField( pr, i, *parser, map );
			p.stealErrors( *parser );
			return ok;
		}

#endif

};

const GA_PrimitiveJSON *GEO_CortexPrimitive::getJSON() const
{
	static GA_PrimitiveJSON *jsonPrim = 0;
	if ( !jsonPrim )
	{
		jsonPrim = new geo_CortexPrimitiveJSON();
	}

	return jsonPrim;
}

void GEO_CortexPrimitive::registerDefinition(GA_PrimitiveFactory *factory) {
	GA_PrimitiveDefinition *primDef = factory->registerDefinition(
			GEO_CortexPrimitive::typeName, GEO_CortexPrimitive::create,
			GA_FAMILY_NONE, (std::string(GEO_CortexPrimitive::typeName ) + "s" ).c_str()
		);

	if ( !primDef )
		{
			std::cerr << "Warning: Duplicate definition for CortexPrimitive. Make sure only 1 version of the ieCoreHoudini plugin is on your path." << std::endl;
			return;
		}

	// merge constructors removed in H16
#if UT_MAJOR_VERSION_INT < 16
	primDef->setMergeConstructor( CortexPrimitive::create );
#endif
	primDef->setHasLocalTransform( true );

	// this will put the proper cortex primitive type into the intrinsic attribute table
	GEO_CortexPrimitive::registerIntrinsics(*primDef);

	m_definition = primDef;

	/// Create the default ObjectPool cache
	UT_ObjectPoolCache::defaultObjectPoolCache();
	/// Declare our new Render Hook if IECoreGL is enabled.
#ifdef IECOREHOUDINI_WITH_GL

	DM_RenderTable::getTable()->registerGEOHook( new GUI_CortexPrimitiveHook, primDef->getId(), 0 );

#endif

}
