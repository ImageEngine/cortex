//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#include "IECoreNuke/FromNukePointsConverter.h"
#include "IECoreNuke/Convert.h"

#include "IECoreScene/PointsPrimitive.h"

using namespace IECoreNuke;
using namespace IECore;
using namespace IECoreScene;

FromNukePointsConverter::FromNukePointsConverter( const DD::Image::GeoInfo *geo )
	:	FromNukeConverter( "Converts nuke meshes to IECore meshes." ), m_geo( geo )
{
}

FromNukePointsConverter::~FromNukePointsConverter()
{
}

IECore::ObjectPtr FromNukePointsConverter::doConversion( IECore::ConstCompoundObjectPtr operands ) const
{

	// get points
	V3fVectorDataPtr p = new V3fVectorData();
	if( const DD::Image::PointList *pl = m_geo->point_list() )
	{
		p->writable().resize( pl->size() );
		std::transform( pl->begin(), pl->end(), p->writable().begin(), IECore::convert<Imath::V3f, DD::Image::Vector3> );
	}

	PointsPrimitivePtr result = new PointsPrimitive( p );

	// get colour
	const DD::Image::Attribute *colorAttr = m_geo->get_typed_attribute( "Cf", DD::Image::VECTOR4_ATTRIB );
	if( colorAttr && colorAttr->size()==result->getNumPoints() )
	{
		Color3fVectorDataPtr colorData = new Color3fVectorData();
		colorData->writable().resize( result->getNumPoints() );
		std::transform( colorAttr->vector4_list->begin(), colorAttr->vector4_list->end(), colorData->writable().begin(), IECore::convert<Imath::Color3f, DD::Image::Vector4> );
		result->variables["Cs"] = PrimitiveVariable( PrimitiveVariable::Vertex, colorData );
	}

	/// \todo Other primitive variables

	return result;
}
