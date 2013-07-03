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

#include "GA/GA_Defragment.h"
#include "GA/GA_ElementWrangler.h"
#include "GA/GA_IndexMap.h"
#include "GA/GA_MergeMap.h"

#include "GEO/GEO_Detail.h"

#include "IECore/VisibleRenderable.h"

#include "IECoreHoudini/Convert.h"
#include "IECoreHoudini/GEO_CortexPrimitive.h"

using namespace IECoreHoudini;

GEO_CortexPrimitive::GEO_CortexPrimitive( GEO_Detail *detail, GA_Offset offset )
	: GEO_Primitive( detail, offset )
{
	m_offset = allocateVertex();
}

GEO_CortexPrimitive::~GEO_CortexPrimitive()
{
	if ( GAisValid( m_offset ) )
	{
		destroyVertex( m_offset );
	}
}

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

void GEO_CortexPrimitive::stashed( int onoff, GA_Offset offset )
{
	if ( onoff )
	{
		m_object = 0;
		m_offset = GA_INVALID_OFFSET;
	}
	else
	{
		m_object = 0;
		m_offset = allocateVertex();
	}
	
	GEO_Primitive::stashed( onoff, offset );
}

void GEO_CortexPrimitive::clearForDeletion()
{
	m_object = 0;
	m_offset = GA_INVALID_OFFSET;
	GEO_Primitive::clearForDeletion();
}

bool GEO_CortexPrimitive::isDegenerate() const
{
	return false;
}

void GEO_CortexPrimitive::copyUnwiredForMerge( const GA_Primitive *src, const GA_MergeMap &map )
{
	const GEO_CortexPrimitive *orig = static_cast<const GEO_CortexPrimitive *>( src );
	
	if ( GAisValid( m_offset ) )
	{
		destroyVertex(  m_offset );
	}
	
	m_offset = ( map.isIdentityMap( GA_ATTRIB_VERTEX ) ) ? orig->m_offset : map.mapDestFromSource( GA_ATTRIB_VERTEX, orig->m_offset );
	
	/// \todo: should we make a shallow or a deep copy?
	m_object = orig->m_object;
}

const GA_PrimitiveJSON *GEO_CortexPrimitive::getJSON() const
{
	/// \todo: implement me
	return 0;
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
	
	const IECore::VisibleRenderable *renderable = IECore::runTimeCast<const IECore::VisibleRenderable>( m_object );
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
	if ( grp.containsOffset( getDetail().vertexPoint( 0 ) ) )
	{
		return -2;
	}
	
	return 0;
}

void GEO_CortexPrimitive::copyPrimitive( const GEO_Primitive *src, GEO_Point **ptredirect )
{
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
	GEO_Point *point = ptredirect[ srcPoints.indexFromOffset( orig->getDetail().vertexPoint( 0 ) ) ];
	wireVertex( v, point ? point->getMapOffset() : GA_INVALID_OFFSET );
	vertexWrangler.copyAttributeValues( v, orig->m_offset );
}

#if (UT_VERSION_INT >= 0x0c050132) // 12.5.306 or later
void GEO_CortexPrimitive::copyOffsetPrimitive( const GEO_Primitive *src, GA_Index basept )
#else
void GEO_CortexPrimitive::copyOffsetPrimitive( const GEO_Primitive *src, int basept )
#endif
{
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
}

bool GEO_CortexPrimitive::evaluatePointRefMap( GA_Offset result_vtx, GA_AttributeRefMap &map, fpreal u, fpreal v, uint du, uint dv ) const
{
	return false;
}

const IECore::Object *GEO_CortexPrimitive::getObject() const
{
	return m_object;
}

void GEO_CortexPrimitive::setObject( const IECore::Object *object )
{
	/// \todo: should this be a deep copy?
	m_object = object->copy();
}
