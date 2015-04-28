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

#include "UT/UT_Version.h"

#if UT_MAJOR_VERSION_INT < 14

#ifndef IECOREHOUDINI_GUCORTEXPRIMITIVE_H
#define IECOREHOUDINI_GUCORTEXPRIMITIVE_H

#include "GU/GU_Prim.h"
#include "GU/GU_Detail.h"
#include "OP/OP_Context.h"
#include "OP/OP_NodeInfoParms.h"

#include "IECore/Object.h"

#include "IECoreHoudini/GEO_CortexPrimitive.h"

namespace IECoreHoudini
{

/// Wrapper for hosting IECore::Objects natively in Houdini
class GU_CortexPrimitive : public GEO_CortexPrimitive, GU_Primitive
{
	public :
		
		GU_CortexPrimitive( GU_Detail *gdp, GA_Offset offset = GA_INVALID_OFFSET );
		GU_CortexPrimitive( const GA_MergeMap &map, GA_Detail &detail, GA_Offset offset, const GA_Primitive &src );
		virtual ~GU_CortexPrimitive();
		
		static const char *typeName;
		
#if UT_MAJOR_VERSION_INT >= 13

		static GA_Primitive *create( GA_Detail &detail, GA_Offset offset, const GA_PrimitiveDefinition &definition );
#else

		static GA_Primitive *create( GA_Detail &detail, GA_Offset offset );

#endif
		
		// merge constructor
		static GA_Primitive *create( const GA_MergeMap &map, GA_Detail &detail, GA_Offset offset, const GA_Primitive &src );

		virtual int64 getMemoryUsage() const;
		
#if UT_MAJOR_VERSION_INT >= 13

		virtual void countMemory( UT_MemoryCounter &counter ) const;
		virtual void copyPrimitive( const GEO_Primitive *src );

#endif
		
		virtual const GA_PrimitiveDefinition &getTypeDef() const;
		
		virtual GEO_Primitive *convert( GU_ConvertParms &parms, GA_PointGroup *usedpts = 0 );
		virtual GEO_Primitive *convertNew( GU_ConvertParms &parms );
		virtual void *castTo() const;
		virtual const GEO_Primitive *castToGeo() const;
		virtual void normal( NormalComp &output ) const;
		virtual int intersectRay( const UT_Vector3 &o, const UT_Vector3 &d, float tmax=1E17F, float tol=1E-12F, float *distance=0, UT_Vector3 *pos=0, UT_Vector3 *nml=0, int accurate=0, float *u=0, float *v=0, int ignoretrim=1 ) const;

#if UT_MAJOR_VERSION_INT < 13

		virtual GU_RayIntersect *createRayCache( int &persistent );

#endif

};

} // namespace IECoreHoudini

#endif // IECOREHOUDINI_GUCORTEXPRIMITIVE_H

#endif // excluded in Houdini 14 and newer
