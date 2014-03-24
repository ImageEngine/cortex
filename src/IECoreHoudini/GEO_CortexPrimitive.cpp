//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013-2014, Image Engine Design Inc. All rights reserved.
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
#include "GA/GA_PrimitiveJSON.h"
#include "GA/GA_SaveMap.h"
#include "UT/UT_JSONParser.h"
#include "UT/UT_JSONWriter.h"

#include "GEO/GEO_Detail.h"

#include "IECore/CoordinateSystem.h"
#include "IECore/Group.h"
#include "IECore/HexConversion.h"
#include "IECore/MatrixTransform.h"
#include "IECore/MemoryIndexedIO.h"
#include "IECore/ParameterisedProcedural.h"
#include "IECore/Primitive.h"
#include "IECore/TransformOp.h"
#include "IECore/VisibleRenderable.h"

#include "IECoreHoudini/Convert.h"
#include "IECoreHoudini/GEO_CortexPrimitive.h"

using namespace IECore;
using namespace IECoreHoudini;

GEO_CortexPrimitive::GEO_CortexPrimitive( GEO_Detail *detail, GA_Offset offset )
	: GEO_Primitive( detail, offset )
{
	m_offset = allocateVertex();
}

GEO_CortexPrimitive::GEO_CortexPrimitive( const GA_MergeMap &map, GA_Detail &detail, GA_Offset offset, const GA_Primitive &src )
	: GEO_Primitive( static_cast<GEO_Detail *>( &detail ), offset )
{
	const GEO_CortexPrimitive *orig = static_cast<const GEO_CortexPrimitive *>( &src );
	
	m_offset = ( map.isIdentityMap( GA_ATTRIB_VERTEX ) ) ? orig->m_offset : map.mapDestFromSource( GA_ATTRIB_VERTEX, orig->m_offset );
	
	m_object = orig->m_object->copy();
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

#if UT_MAJOR_VERSION_INT >= 13

void GEO_CortexPrimitive::stashed( bool beingstashed, GA_Offset offset )
{
	GEO_Primitive::stashed( beingstashed, offset );
	
	if ( beingstashed )
	{
		m_object = 0;
		m_offset = GA_INVALID_OFFSET;
	}
	else
	{
		m_object = 0;
		m_offset = allocateVertex();
	}	
}

#endif

void GEO_CortexPrimitive::stashed( int onoff, GA_Offset offset )
{
	GEO_Primitive::stashed( onoff, offset );
	
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
	
	m_object = orig->m_object->copy();
}

void GEO_CortexPrimitive::transform( const UT_Matrix4 &xform )
{
	if ( xform.isIdentity() )
	{
		return;
	}
	
	Imath::M44f transform = IECore::convert<Imath::M44f>( xform );
	
	if ( Primitive *primitive = IECore::runTimeCast<Primitive>( m_object ) )
	{
		TransformOpPtr transformer = new TransformOp();
		transformer->inputParameter()->setValue( primitive );
		transformer->copyParameter()->setTypedValue( false );
		transformer->matrixParameter()->setValue( new M44fData( transform ) );
		transformer->operate();
	}
	else if ( Group *group = IECore::runTimeCast<Group>( m_object ) )
	{
		if ( MatrixTransform *matTransform = IECore::runTimeCast<MatrixTransform>( group->getTransform() ) )
		{
			matTransform->matrix = transform * matTransform->matrix;
		}
	}
	else if ( CoordinateSystem *coord = IECore::runTimeCast<CoordinateSystem>( m_object ) )
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

IECore::Object *GEO_CortexPrimitive::getObject()
{
	return m_object;
}

const IECore::Object *GEO_CortexPrimitive::getObject() const
{
	return m_object;
}

void GEO_CortexPrimitive::setObject( const IECore::Object *object )
{
	if ( object->isInstanceOf( IECore::ParameterisedProcedural::staticTypeId() ) )
	{
		m_object = const_cast<IECore::Object *>( object );
	}
	else
	{
		/// \todo: should this be a deep copy?
		m_object = object->copy();
	}
}

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
					GA_Offset offset = object( pr )->getVertexOffset( 0 );
					return w.jsonInt( int64( map.getVertexIndex( offset ) ) );
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
						object( pr )->setObject( IECore::Object::load( io, "object" ) );
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
