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

#include "DDImage/ParticleOp.h"

using namespace IECoreNuke;
using namespace IECore;
using namespace IECoreScene;
using namespace DD::Image;

FromNukePointsConverter::FromNukePointsConverter( const DD::Image::GeoInfo *geo, DD::Image::Op* op )
	:	FromNukeConverter( "Converts nuke ParticleSprites to IECore PointsPrimitive." ), m_geo( geo ), m_op( op )
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
	if( colorAttr && colorAttr->size() == result->getNumPoints() )
	{
		Color4fVectorDataPtr colorData = new Color4fVectorData();
		colorData->writable().resize( result->getNumPoints() );
		std::transform( colorAttr->vector4_list->begin(), colorAttr->vector4_list->end(), colorData->writable().begin(), IECore::convert<Imath::Color4f, DD::Image::Vector4> );
		result->variables["Cs"] = PrimitiveVariable( PrimitiveVariable::Vertex, colorData );

		// Adding a separate alpha primvar as according to my test
		// Cs is a Color3f in Gaffer. While we could also use 3f here, I think it is reasonable
		// to combine alpha inside Cs and hope it gets supported by Gaffer and then we can remove 
		// the alpha primvar.
		FloatVectorDataPtr alphaData = new FloatVectorData();
		auto& alpha = alphaData->writable();
		alpha.resize( result->getNumPoints() );
	
		for( size_t i=0; i < result->getNumPoints(); i++ )
		{
			alpha[i] = colorAttr->vector4( i ).w;
		}
		result->variables["alpha"] = PrimitiveVariable( PrimitiveVariable::Vertex, alphaData );
	}

	// get pid
	const DD::Image::Attribute *idAttr = m_geo->get_typed_attribute( "id", DD::Image::INT_ATTRIB );
	if( idAttr && idAttr->size() == result->getNumPoints() )
	{
		IntVectorDataPtr idData = new IntVectorData();
		auto& id = idData->writable();
		id.resize( result->getNumPoints() );
		for( size_t i=0; i < result->getNumPoints(); i++ )
		{
			id[i] = idAttr->integer( i );
		}
		result->variables["pid"] = PrimitiveVariable( PrimitiveVariable::Vertex, idData );
	}

	// get size/width
	const DD::Image::Attribute *sizeAttr = m_geo->get_typed_attribute( "size", DD::Image::FLOAT_ATTRIB );
	if( sizeAttr && sizeAttr->size() == result->getNumPoints() )
	{
		FloatVectorDataPtr widthData = new FloatVectorData();
		auto& width = widthData->writable();
		width.resize( result->getNumPoints() );
	
		for( size_t i=0; i < result->getNumPoints(); i++ )
		{
			width[i] = sizeAttr->flt( i );
		}
		result->variables["width"] = PrimitiveVariable( PrimitiveVariable::Vertex, widthData );
	}

	// get vel
	// Nuke's particle system seems to be a bit ad-hock rather than integrated in the 3D sub-system.
	// To get the particle velocity, we mix the API ( ParticleOp and GeoOp ). Arguably, we could switch
	// to use the ParticleOp API for everything but for now I decided to keep accessing what can be using the
	// GeoOp/GeoInfo API, hoping that Foundry will ultimately get the particle to geo fully supported.
	//
	// Another important detail here is that we are using the m_op->input0, this is based on the expectation
	// that the LiveScene will always be the client of this kind of converter.
	// In which case, the LiveScene is always internal to a GeoOp derived node (LiveSceneHolder or WriteGeo)
	auto particleOp = m_op->particleOp();
	if ( particleOp )
	{
		OutputContext oc;
		oc.setFrame( m_op->outputContext().frame() );
		particleOp->setOutputContext( oc );
		float prevTime, outTime;
		const auto particleSystem = particleOp->getParticleSystem(prevTime, outTime, true, nullptr);

		V3fVectorDataPtr velData = new V3fVectorData();
		auto& vel = velData->writable();
		vel.resize( result->getNumPoints() );
		for( size_t i=0; i < result->getNumPoints(); i++ )
		{
			// velocity seems to be calculated per time step so we need to multiply by the frames per second to get velocity compatible with motion blur rendering.
			vel[i] = IECore::convert<Imath::V3f>( particleSystem->particleVelocity( i ) ) * DD::Image::root_real_fps();
		}

		result->variables["velocity"] = PrimitiveVariable( PrimitiveVariable::Vertex, velData );
	}
	// \todo Other primitive variables

	return result;
}
