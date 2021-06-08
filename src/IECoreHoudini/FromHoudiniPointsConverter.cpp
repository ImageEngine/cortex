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

#include "IECoreHoudini/FromHoudiniPointsConverter.h"

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreHoudini;

IE_CORE_DEFINERUNTIMETYPED( FromHoudiniPointsConverter );

FromHoudiniGeometryConverter::Description<FromHoudiniPointsConverter> FromHoudiniPointsConverter::m_description( PointsPrimitive::staticTypeId() );

FromHoudiniPointsConverter::FromHoudiniPointsConverter( const GU_DetailHandle &handle ) :
	FromHoudiniGeometryConverter( handle, "Converts a Houdini GU_Detail to an IECoreScene::PointsPrimitive." )
{
}

FromHoudiniPointsConverter::FromHoudiniPointsConverter( const SOP_Node *sop ) :
	FromHoudiniGeometryConverter( sop, "Converts a Houdini GU_Detail to an IECoreScene::PointsPrimitive." )
{
}

FromHoudiniPointsConverter::~FromHoudiniPointsConverter()
{
}

FromHoudiniGeometryConverter::Convertability FromHoudiniPointsConverter::canConvert( const GU_Detail *geo )
{
	size_t numPrims = geo->getNumPrimitives();
	if ( !numPrims )
	{
		/// \todo: An alternative to consider would be to register a `FromHoudiniNullConverter` as the `Ideal`
		/// converter when there are also no points in the detail.
		return Ideal;
	}

	if ( numPrims == 1 )
	{
		const GA_PrimitiveList &primitives = geo->getPrimitiveList();
		const GA_Primitive *prim = primitives.get( geo->getPrimitiveRange().begin().getOffset() );
		if ( prim->getTypeId() == GEO_PRIMPART )
		{
			return Ideal;
		}
	}

	return Inapplicable;
}

ObjectPtr FromHoudiniPointsConverter::doDetailConversion( const GU_Detail *geo, const CompoundObject *operands ) const
{
	PointsPrimitivePtr result = new PointsPrimitive( geo->getNumPoints() );

	transferAttribs( geo, result.get(), operands, PrimitiveVariable::Vertex );

	return result;
}
