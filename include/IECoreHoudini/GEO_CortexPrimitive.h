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

#ifndef IECOREHOUDINI_GEOCORTEXPRIMITIVE_H
#define IECOREHOUDINI_GEOCORTEXPRIMITIVE_H

#include "GA/GA_Defines.h"
#include "GEO/GEO_Primitive.h"
#include "UT/UT_Version.h"

#include "IECore/Object.h"

namespace IECoreHoudini
{

/// Wrapper for hosting IECore::Objects natively in Houdini
class GEO_CortexPrimitive : public GEO_Primitive
{
	public :
		
		GEO_CortexPrimitive( GEO_Detail *detail, GA_Offset offset = GA_INVALID_OFFSET );
		virtual ~GEO_CortexPrimitive();
		
		virtual void swapVertexOffsets( const GA_Defragment &defrag );
		virtual GA_Size getVertexCount() const;
		virtual GA_Offset getVertexOffset( GA_Size index ) const;
		virtual GA_DereferenceStatus dereferencePoint( GA_Offset point, bool dry_run = false );
		virtual GA_DereferenceStatus dereferencePoints( const GA_RangeMemberQuery &pt_q, bool dry_run = false );
		virtual void stashed( int onoff, GA_Offset offset=GA_INVALID_OFFSET );
		virtual void clearForDeletion();
		virtual bool isDegenerate() const;
		virtual void copyUnwiredForMerge( const GA_Primitive *src, const GA_MergeMap &map );
		virtual const GA_PrimitiveJSON* getJSON() const;
		virtual void reverse();
		
		virtual void copyPrimitive( const GEO_Primitive *src, GEO_Point **ptredirect );
		virtual int getBBox( UT_BoundingBox *bbox ) const;
		virtual void enlargePointBounds( UT_BoundingBox &box ) const;
		virtual UT_Vector3 computeNormal() const;
		virtual int detachPoints( GA_PointGroup &grp );
		
		const IECore::Object *getObject() const;
		void setObject( const IECore::Object *object );
	
	protected :

#if (UT_VERSION_INT >= 0x0c050132) // 12.5.306 or later
		virtual void copyOffsetPrimitive( const GEO_Primitive *src, GA_Index basept );
#else
		virtual void copyOffsetPrimitive( const GEO_Primitive *src, int basept );
#endif

		virtual bool evaluatePointRefMap( GA_Offset result_vtx, GA_AttributeRefMap &map, fpreal u, fpreal v=0, uint du=0, uint dv=0 ) const;
		
		IECore::ObjectPtr m_object;
		// offset for the representative vertex
		GA_Offset m_offset;
	
	private :
		
		class geo_CortexPrimitiveJSON;

};

} // namespace IECoreHoudini

#endif // IECOREHOUDINI_GEOCORTEXPRIMITIVE_H

