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

#ifndef IECOREHOUDINI_GUCORTEXPRIMITIVE_H
#define IECOREHOUDINI_GUCORTEXPRIMITIVE_H

#include "GU/GU_Prim.h"
#include "GU/GU_Detail.h"

#include "IECore/Object.h"

#include "IECoreHoudini/GEO_CortexPrimitive.h"
#include "ieHoudini.h"

namespace IECoreHoudini
{

/// Wrapper for hosting IECore::Objects natively in Houdini
class CortexHOUAPI GU_CortexPrimitive : public GEO_CortexPrimitive, GU_Primitive
{
	public :
		
		/// \todo: We should probably provide a merge constructor as well
		GU_CortexPrimitive( GU_Detail *gdp, GA_Offset offset = GA_INVALID_OFFSET );
		virtual ~GU_CortexPrimitive();
		
		static const char *typeName;
		
		static GA_Primitive *create( GA_Detail &detail, GA_Offset offset );
		static GU_CortexPrimitive *build( GU_Detail *geo, const IECore::Object *object );
		
		virtual int64 getMemoryUsage() const;
		virtual const GA_PrimitiveDefinition &getTypeDef() const;
		/// \todo: setTypeDef is called once by the plugin. Seems quite silly to expose.
		/// Maybe we should just give up registration in the plugin and do it all here.
		static void setTypeDef( GA_PrimitiveDefinition *def );
		static GA_PrimitiveTypeId typeId();
		
		virtual GEO_Primitive *convert( GU_ConvertParms &parms, GA_PointGroup *usedpts = 0 );
		virtual GEO_Primitive *convertNew( GU_ConvertParms &parms );
		virtual void *castTo() const;
		virtual const GEO_Primitive *castToGeo() const;
		virtual void normal( NormalComp &output ) const;
		virtual int intersectRay( const UT_Vector3 &o, const UT_Vector3 &d, float tmax=1E17F, float tol=1E-12F, float *distance=0, UT_Vector3 *pos=0, UT_Vector3 *nml=0, int accurate=0, float *u=0, float *v=0, int ignoretrim=1 ) const;
		virtual GU_RayIntersect *createRayCache( int &persistent );
	
	private :
		
		GEO_Primitive *doConvert( GU_ConvertParms &parms );
		
		static GA_PrimitiveDefinition *m_definition;

};

} // namespace IECoreHoudini

#endif // IECOREHOUDINI_GUCORTEXPRIMITIVE_H


