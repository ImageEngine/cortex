//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#include <cassert>

#include "boost/format.hpp"

#include "OpenEXR/ImathBoxAlgo.h"
#include "OpenEXR/ImathLineAlgo.h"

#include "IECore/BoxOps.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/PrimitiveVariable.h"
#include "IECore/Exception.h"
#include "IECore/ImagePrimitiveEvaluator.h"
#include "IECore/Deleter.h"

using namespace IECore;
using namespace Imath;

static PrimitiveEvaluator::Description< ImagePrimitiveEvaluator > g_registraar = PrimitiveEvaluator::Description< ImagePrimitiveEvaluator >();

ImagePrimitiveEvaluator::Result::Result( const Imath::Box3f &bound, const Imath::Box2i &dataWindow ) : m_bound( bound )
{
	m_dataWindow = dataWindow;
}

ImagePrimitiveEvaluator::Result::~Result()
{
}

V3f ImagePrimitiveEvaluator::Result::point() const
{
	return m_p;
}

V3f ImagePrimitiveEvaluator::Result::normal() const
{
	return V3f( 0.0f, 0.0f, 1.0f );
}

V2f ImagePrimitiveEvaluator::Result::uv() const
{	
	return V2f(
		( m_p.x - m_bound.min.x ) / ( m_bound.max.x - m_bound.min.x ),
		( m_p.y - m_bound.min.y ) / ( m_bound.max.y - m_bound.min.y )
	);
}

V3f ImagePrimitiveEvaluator::Result::uTangent() const
{
	return V3f( 1.0f, 0.0f, 0.0f );
}

V3f ImagePrimitiveEvaluator::Result::vTangent() const
{
	return V3f( 0.0f, 1.0f, 0.0f );
}

V2i ImagePrimitiveEvaluator::Result::pixel() const
{
	/// Round to nearest integer
	return V2i(
		static_cast<int>(0.5f + m_p.x - m_bound.min.x - 1.0),
		static_cast<int>(0.5f + m_p.y - m_bound.min.y - 1.0)
	);
}

V3f ImagePrimitiveEvaluator::Result::vectorPrimVar( const PrimitiveVariable &pv ) const
{
	return getPrimVar< V3f >( pv );
}

float ImagePrimitiveEvaluator::Result::floatPrimVar( const PrimitiveVariable &pv ) const
{
	return getPrimVar< float >( pv );
}

int ImagePrimitiveEvaluator::Result::intPrimVar( const PrimitiveVariable &pv ) const
{
	return getPrimVar< int >( pv );
}

unsigned int ImagePrimitiveEvaluator::Result::uintPrimVar( const PrimitiveVariable &pv ) const
{
	return getPrimVar< unsigned int >( pv );
}

const std::string &ImagePrimitiveEvaluator::Result::stringPrimVar( const PrimitiveVariable &pv ) const
{
	StringDataPtr data = runTimeCast< StringData >( pv.data );
		
	if (data)
	{
		return data->readable();
	}	
	else
	{
		StringVectorDataPtr data = runTimeCast< StringVectorData >( pv.data );
		
		if (data)
		{		
			return data->readable()[0];
		}
	}
	
	throw InvalidArgumentException( "Could not retrieve primvar data for ImagePrimitiveEvaluator" );
}

Color3f ImagePrimitiveEvaluator::Result::colorPrimVar( const PrimitiveVariable &pv ) const
{
	return getPrimVar< Color3f >( pv );
}

half ImagePrimitiveEvaluator::Result::halfPrimVar( const PrimitiveVariable &pv ) const
{
	return getPrimVar< half >( pv );
}

short ImagePrimitiveEvaluator::Result::shortPrimVar ( const PrimitiveVariable &pv ) const
{
	return getPrimVar< short >( pv );
}

unsigned short ImagePrimitiveEvaluator::Result::ushortPrimVar( const PrimitiveVariable &pv ) const
{
	return getPrimVar< unsigned short >( pv );
}

char ImagePrimitiveEvaluator::Result::charPrimVar ( const PrimitiveVariable &pv ) const
{
	return getPrimVar< char >( pv );
}

unsigned char ImagePrimitiveEvaluator::Result::ucharPrimVar( const PrimitiveVariable &pv ) const
{
	return getPrimVar< unsigned char >( pv );
}

template<typename T>
T ImagePrimitiveEvaluator::Result::getPrimVar( const PrimitiveVariable &pv ) const
{
	if ( pv.interpolation == PrimitiveVariable::Constant )
	{
		typedef TypedData<T> Data;
		typedef typename Data::Ptr DataPtr;
		
		DataPtr data = runTimeCast< Data >( pv.data );
		
		if (data)
		{
			return data->readable();
		}
	}
	
	typedef TypedData< std::vector<T> > VectorData;
	typedef typename VectorData::Ptr VectorDataPtr;
	VectorDataPtr data = runTimeCast< VectorData >( pv.data );

	if (!data)
	{
		throw InvalidArgumentException( ( boost::format("ImagePrimitiveEvaluator: Could not retrieve primvar data of type %s or %s " ) % TypedData<T>::staticTypeName() % VectorData::staticTypeName() ).str() );
	}
	
	switch ( pv.interpolation )
	{
		case PrimitiveVariable::Uniform :
		case PrimitiveVariable::Constant :
			assert( data->readable().size() == 1 );
			
			return data->readable()[0];

		case PrimitiveVariable::Vertex :
		case PrimitiveVariable::Varying:
		case PrimitiveVariable::FaceVarying:
		{	
			if ( m_dataWindow.isEmpty() )
			{
				/// \todo Perhaps use a traits class here to specify some "zero" value
				return T();
			}
			
			// \todo Use UV coord instead, and perform bilinear interpolation
			V2i p = pixel() ;	
			
			p = p - m_dataWindow.min;
			
			int dataWidth = static_cast<int>( m_dataWindow.size().x + 1 );
			int dataHeight = static_cast<int>( m_dataWindow.size().y + 1 );
			
			if ( p.x < 0 || p.y < 0 || p.x >= dataWidth || p.y >= dataHeight )
			{
				/// \todo Perhaps use a traits class here to specify some "zero" value
				return T();
			}
						
			int idx = ( p.y * dataWidth ) + p.x;								
			assert( idx >= 0 );
			assert( idx < (int)data->readable().size() );

			return data->readable()[idx];							
		}
			
		default :
			/// Unimplemented primvar interpolation
			assert( false );
			return T();			
	}
}

PrimitiveEvaluatorPtr ImagePrimitiveEvaluator::create( ConstPrimitivePtr primitive )
{
	ConstImagePrimitivePtr image = runTimeCast< const ImagePrimitive >( primitive );
	assert ( image );

	return new ImagePrimitiveEvaluator( image );
}

ImagePrimitiveEvaluator::ImagePrimitiveEvaluator( ConstImagePrimitivePtr image )
{
	if (! image )
	{
		throw InvalidArgumentException( "No image given to ImagePrimitiveEvaluator");
	}
	
	if (! image->arePrimitiveVariablesValid() )
	{
		throw InvalidArgumentException( "Image with invalid primitive variables given to ImagePrimitiveEvaluator");
	}
	
	m_image = image->copy();
}

ImagePrimitiveEvaluator::~ImagePrimitiveEvaluator()
{
}

ConstPrimitivePtr ImagePrimitiveEvaluator::primitive() const
{
	return m_image;
}

PrimitiveEvaluator::ResultPtr ImagePrimitiveEvaluator::createResult() const
{
	return new Result( m_image->bound(), m_image->getDataWindow() );
}

bool ImagePrimitiveEvaluator::closestPoint( const V3f &p, const PrimitiveEvaluator::ResultPtr &result ) const
{	
	ResultPtr r = boost::dynamic_pointer_cast< Result >( result );
	assert( r );
	
	r->m_p = closestPointOnBox( p, m_image->bound() );
	
	return true;
}

bool ImagePrimitiveEvaluator::pointAtUV( const V2f &uv, const PrimitiveEvaluator::ResultPtr &result ) const
{
	ResultPtr r = boost::dynamic_pointer_cast< Result >( result );
	assert( r );
	
	if ( uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0 )
	{
		return false;
	}
	
	r->m_p = V3f(
		m_image->bound().min.x + uv.x * ( m_image->bound().max.x - m_image->bound().min.x ),
		m_image->bound().min.y + uv.y * ( m_image->bound().max.y - m_image->bound().min.y ),
		0.0
	);
	
	return true;	
}

bool ImagePrimitiveEvaluator::pointAtPixel( const Imath::V2i &pixel, const PrimitiveEvaluator::ResultPtr &result ) const
{
	ResultPtr r = boost::dynamic_pointer_cast< Result >( result );
	assert( r );
	
	V3f imageSize = boxSize( m_image->bound() );
	
	if ( pixel.x < 0 || pixel.x > (int)imageSize.x || pixel.y < 0 || pixel.y > (int)imageSize.y )
	{
		return false;
	}
	
	V2f uv(
		( 1 + pixel.x ) / imageSize.x,
		( 1 + pixel.y ) / imageSize.y		
	);
	
	return pointAtUV( uv, r );
}

bool ImagePrimitiveEvaluator::intersectionPoint( const V3f &origin, const V3f &direction,
                const PrimitiveEvaluator::ResultPtr &result, float maxDistance ) const
{
	ResultPtr r = boost::dynamic_pointer_cast< Result >( result );
	assert( r );
	
	std::vector<PrimitiveEvaluator::ResultPtr> results;
	
	int numIntersections = intersectionPoints( origin, direction, results, maxDistance );
	assert( numIntersections == 0 || numIntersections == 1 );
	
	if ( ! numIntersections )
	{
		return false;
	}
	else
	{
		assert( numIntersections == 1 );
		assert( results.size() == 1 );
		
		ResultPtr intersectionResult = boost::dynamic_pointer_cast< Result >( results[0] );
		assert( intersectionResult );
		r->m_p = intersectionResult->m_p;
		
		return true;
	}
}

int ImagePrimitiveEvaluator::intersectionPoints( const V3f &origin, const V3f &direction,
                std::vector<PrimitiveEvaluator::ResultPtr> &results, float maxDistance ) const
{
	results.clear();
	
	V3f hitPoint;
	Box3f bound = m_image->bound();
	bool hit = boxIntersects( bound , origin, direction.normalized(), hitPoint );
	
	if ( hit )
	{
		if ( ( origin - hitPoint ).length2() < maxDistance * maxDistance )
		{	
			ResultPtr result = boost::dynamic_pointer_cast< Result >( createResult() );
			result->m_p = hitPoint;
		
			results.push_back( result );			
		}		
	}
	
	return results.size();
}

float ImagePrimitiveEvaluator::volume() const
{
	return 0.0f;
}

V3f ImagePrimitiveEvaluator::centerOfGravity() const
{
	return Imath::V3f( 0.0, 0.0, 0.0 );
}

float ImagePrimitiveEvaluator::surfaceArea() const
{
	V3f size = boxSize( m_image->bound() );
	
	return 2.0f * ( size.x*size.y + size.x*size.z + size.y*size.z );
}

PrimitiveVariableMap::const_iterator ImagePrimitiveEvaluator::R() const
{
	return m_image->variables.find( "R" );
}

PrimitiveVariableMap::const_iterator ImagePrimitiveEvaluator::G() const
{
	return m_image->variables.find( "G" );
}

PrimitiveVariableMap::const_iterator ImagePrimitiveEvaluator::B() const
{
	return m_image->variables.find( "B" );
}

PrimitiveVariableMap::const_iterator ImagePrimitiveEvaluator::A() const
{
	return m_image->variables.find( "A" );
}

PrimitiveVariableMap::const_iterator ImagePrimitiveEvaluator::Y() const
{
	return m_image->variables.find( "Y" );
}
