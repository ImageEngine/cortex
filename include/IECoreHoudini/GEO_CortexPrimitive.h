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

#ifndef IECOREHOUDINI_GEOCORTEXPRIMITIVE_H
#define IECOREHOUDINI_GEOCORTEXPRIMITIVE_H

#include "GA/GA_Defines.h"
#include "GA/GA_LoadMap.h"
#include "GEO/GEO_Primitive.h"
#include "GU/GU_Detail.h"
#include "GEO/GEO_Point.h"
#include "OP/OP_Context.h"
#include "OP/OP_NodeInfoParms.h"
#include "UT/UT_Version.h"
#include "UT/UT_ParallelUtil.h"
#include "GA/GA_Primitive.h"
#if UT_MAJOR_VERSION_INT >= 14

typedef GEO_ConvertParms ConvertParms;

#else

#include "GU/GU_Prim.h"

typedef GU_Primitive::NormalComp NormalComp;
typedef GU_ConvertParms ConvertParms;

#endif

#include "IECoreHoudini/CoreHoudiniVersion.h"
#include "IECore/Object.h"

namespace IECoreHoudini
{

/// Wrapper for hosting IECore::Objects natively in Houdini
class GEO_CortexPrimitive : public GEO_Primitive
{
	public :
		
		GEO_CortexPrimitive( GA_Detail *detail, GA_Offset offset = GA_INVALID_OFFSET );
		GEO_CortexPrimitive( GEO_Detail *detail, GA_Offset offset = GA_INVALID_OFFSET );
#if UT_MAJOR_VERSION_INT < 16
		GEO_CortexPrimitive( const GA_MergeMap &map, GA_Detail &detail, GA_Offset offset, const GA_Primitive &src );
#endif
		virtual ~GEO_CortexPrimitive();
		
#if UT_MAJOR_VERSION_INT < 16
		virtual void swapVertexOffsets( const GA_Defragment &defrag );
		virtual GA_Size getVertexCount() const;
		virtual GA_Offset getVertexOffset( GA_Size index ) const;
#endif
		virtual GA_DereferenceStatus dereferencePoint( GA_Offset point, bool dry_run = false );
		virtual GA_DereferenceStatus dereferencePoints( const GA_RangeMemberQuery &pt_q, bool dry_run = false );

#if UT_MAJOR_VERSION_INT >= 13

		virtual void stashed( bool beingstashed, GA_Offset offset=GA_INVALID_OFFSET );

#endif

		// \todo: This signature is for Houdini 12.5 and earlier. Remove when we drop support
		virtual void stashed( int onoff, GA_Offset offset=GA_INVALID_OFFSET );
		virtual void clearForDeletion();
		virtual bool isDegenerate() const;
		virtual void copyUnwiredForMerge( const GA_Primitive *src, const GA_MergeMap &map );
		virtual void transform( const UT_Matrix4 &xform );
		virtual const GA_PrimitiveJSON* getJSON() const;
		virtual void reverse();
		
		virtual GEO_Primitive * copy( int preserve_shared_pts ) const;
		virtual void copyPrimitive( const GEO_Primitive *src, GEO_Point **ptredirect );
		virtual int getBBox( UT_BoundingBox *bbox ) const;
		virtual void enlargePointBounds( UT_BoundingBox &box ) const;
		virtual UT_Vector3 computeNormal() const;
		virtual int detachPoints( GA_PointGroup &grp );
#if UT_MAJOR_VERSION_INT >= 16
 
		bool saveVertexArray( UT_JSONWriter &w,	const GA_SaveMap &map ) const;
		bool loadVertexArray( UT_JSONParser &p, const GA_LoadMap &map );

#endif

		static const char *typeName;

#if MIN_HOU_VERSION(16, 5, 0)
		static void create(GA_Primitive **new_prims, GA_Size nprimitives, GA_Detail &detail, GA_Offset start_offset, const GA_PrimitiveDefinition &def, bool allowed_to_parallelize);
#elif UT_MAJOR_VERSION_INT >=16
        static void create(GA_Primitive **new_prims, GA_Size nprimitives, GA_Detail &detail, GA_Offset start_offset, const GA_PrimitiveDefinition &def);
#elif UT_MAJOR_VERSION_INT >= 14
		
		static GA_Primitive *create( GA_Detail &detail, GA_Offset offset, const GA_PrimitiveDefinition &definition );		
		// merge constructor
		static GA_Primitive *create( const GA_MergeMap &map, GA_Detail &detail, GA_Offset offset, const GA_Primitive &src );

#endif

		// factory
		static GEO_CortexPrimitive *build( GU_Detail *geo, const IECore::Object *object );
		
		virtual int64 getMemoryUsage() const;
		virtual void countMemory( UT_MemoryCounter &counter ) const;
		virtual void copyPrimitive( const GEO_Primitive *src );
		
		virtual const GA_PrimitiveDefinition &getTypeDef() const;
		static void registerDefinition(GA_PrimitiveFactory *factory);
		static GA_PrimitiveTypeId typeId();
		
		virtual GEO_Primitive *convert( ConvertParms &parms, GA_PointGroup *usedpts = 0 );
		virtual GEO_Primitive *convertNew( ConvertParms &parms );
		virtual void normal( NormalComp &output ) const;
		virtual int intersectRay( const UT_Vector3 &o, const UT_Vector3 &d, float tmax=1E17F, float tol=1E-12F, float *distance=0, UT_Vector3 *pos=0, UT_Vector3 *nml=0, int accurate=0, float *u=0, float *v=0, int ignoretrim=1 ) const;

		/// Set the IECore::Object contained by this GEO_Primitive. Note that in most situations
		/// this method takes a copy of the object. However, for ParameterisedProcedurals it does
		/// not, and it is the users responsibility to treat the contained object as const.
		void setObject( const IECore::Object *object );
		/// Get the IECore::Object contained by this GEO_Primitive
		const IECore::Object *getObject() const;
		/// Allowing non-const access to the IECore::Object so it can be updated in-place.
		/// Most users should prefer the const method above.
		IECore::Object *getObject();
		
		/// Convenience method to inspect a GU_Detail and return some information about
		/// the GU_CortexPrimitives within, if there are any.
		static void infoText( const GU_Detail *geo, OP_Context &context, OP_NodeInfoParms &parms );
	
	protected :

#if (UT_VERSION_INT >= 0x0c050132) // 12.5.306 or later
		virtual void copyOffsetPrimitive( const GEO_Primitive *src, GA_Index basept );
#else
		virtual void copyOffsetPrimitive( const GEO_Primitive *src, int basept );
#endif

		virtual bool evaluatePointRefMap( GA_Offset result_vtx, GA_AttributeRefMap &map, fpreal u, fpreal v=0, uint du=0, uint dv=0 ) const;
		
		IECore::ObjectPtr m_object;
#if UT_MAJOR_VERSION_INT < 16
		// offset for the representative vertex
		GA_Offset m_offset;
#endif

	private :
		
		class geo_CortexPrimitiveJSON;

		GEO_Primitive *doConvert( ConvertParms &parms );
		
		static GA_PrimitiveDefinition *m_definition;

};

} // namespace IECoreHoudini

#endif // IECOREHOUDINI_GEOCORTEXPRIMITIVE_H

