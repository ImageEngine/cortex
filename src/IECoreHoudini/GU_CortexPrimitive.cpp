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

#include "IECoreHoudini/Convert.h"
#include "IECoreHoudini/GU_CortexPrimitive.h"
#include "IECoreHoudini/ToHoudiniPolygonsConverter.h"

#include "IECoreScene/CoordinateSystem.h"
#include "IECoreScene/Transform.h"

#include "IECore/Object.h"

#include "GU/GU_ConvertParms.h"
#include "GU/GU_RayIntersect.h"
#include "UT/UT_MemoryCounter.h"

using namespace IECoreHoudini;

const char *GU_CortexPrimitive::typeName = "CortexObject";

GU_CortexPrimitive::GU_CortexPrimitive( GU_Detail *gdp, GA_Offset offset )
	: GEO_CortexPrimitive( gdp, offset ), GU_Primitive()
{
}

GU_CortexPrimitive::GU_CortexPrimitive( const GA_MergeMap &map, GA_Detail &detail, GA_Offset offset, const GA_Primitive &src )
	: GEO_CortexPrimitive( map, detail, offset, src )
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

GA_Primitive *GU_CortexPrimitive::create( const GA_MergeMap &map, GA_Detail &detail, GA_Offset offset, const GA_Primitive &src )
{
	return new GU_CortexPrimitive( map, detail, offset, src );
}

const GA_PrimitiveDefinition &GU_CortexPrimitive::getTypeDef() const
{
	return GEO_CortexPrimitive::getTypeDef();
}

int64 GU_CortexPrimitive::getMemoryUsage() const
{
	return GEO_CortexPrimitive::getMemoryUsage();
}

#if UT_MAJOR_VERSION_INT >= 13

void GU_CortexPrimitive::countMemory( UT_MemoryCounter &counter ) const
{
	GEO_CortexPrimitive::countMemory( counter );
}

void GU_CortexPrimitive::copyPrimitive( const GEO_Primitive *src )
{
	GEO_CortexPrimitive::copyPrimitive( src );
}

#endif

GEO_Primitive *GU_CortexPrimitive::convert( GU_ConvertParms &parms, GA_PointGroup *usedpts )
{
	return GEO_CortexPrimitive::convert( parms, usedpts );
}

GEO_Primitive *GU_CortexPrimitive::convertNew( GU_ConvertParms &parms )
{
	return GEO_CortexPrimitive::convertNew( parms );
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
	GEO_CortexPrimitive::normal( output );
}

/// \todo: build ray cache and intersect properly
int GU_CortexPrimitive::intersectRay( const UT_Vector3 &o, const UT_Vector3 &d, float tmax, float tol, float *distance, UT_Vector3 *pos, UT_Vector3 *nml, int accurate, float *u, float *v, int ignoretrim ) const
{
	return GEO_CortexPrimitive::intersectRay( o, d, tmax, tol, distance, pos, nml, accurate, u, v, ignoretrim );
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

#endif // excluded in Houdini 14 and newer
