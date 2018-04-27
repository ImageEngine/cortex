//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2018, Image Engine Design Inc. All rights reserved.
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

#include "IECoreScene/Camera.h"

#include "IECore/AngleConversion.h"

#include "IECoreScene/Renderer.h"

#include "IECore/MurmurHash.h"
#include "IECore/SimpleTypedData.h"

using namespace IECore;
using namespace IECoreScene;
using namespace Imath;
using namespace std;

namespace {

	// \todo: Remove once we have similar utilities on CompoundData
	template <class T>
	typename T::ValueType accessCompoundDataMapWithDefault( const CompoundDataMap &c, const InternedString &key, typename T::ValueType def )
	{
		CompoundDataMap::const_iterator it=c.find( key );
		if( it != c.end() )
		{
			const T *data = runTimeCast<const T>( it->second.get() );
			if( data )
			{
				return data->readable();
			}
		}
		return def;
	}

	template <class T>
	bool checkCompoundDataMap( const CompoundDataMap &c, const InternedString &key )
	{
		CompoundDataMap::const_iterator it=c.find( key );
		if( it != c.end() )
		{
			const T *data = runTimeCast<const T>( it->second.get() );
			if( data )
			{
				return true;
			}
		}
		return false;
	}
}

IE_CORE_DEFINEOBJECTTYPEDESCRIPTION(Camera);

static IndexedIO::EntryID g_parametersEntry("parameters");
const unsigned int Camera::m_ioVersion = 0;

Camera::Camera( CompoundDataPtr parameters )
	:	m_parameters( parameters )
{
}

Camera::~Camera()
{
}

void Camera::copyFrom( const Object *other, CopyContext *context )
{
	PreWorldRenderable::copyFrom( other, context );
	const Camera *tOther = static_cast<const Camera *>( other );
	m_parameters = context->copy<CompoundData>( tOther->m_parameters.get() );
}

void Camera::save( SaveContext *context ) const
{
	PreWorldRenderable::save( context );
	IndexedIOPtr container = context->container( staticTypeName(), m_ioVersion );
	context->save( m_parameters.get(), container.get(), g_parametersEntry );
}

void Camera::load( LoadContextPtr context )
{
	PreWorldRenderable::load( context );
	unsigned int v = m_ioVersion;
	ConstIndexedIOPtr container = context->container( staticTypeName(), v );

	m_parameters = context->load<CompoundData>( container.get(), g_parametersEntry );
}

bool Camera::isEqualTo( const Object *other ) const
{
	if( !PreWorldRenderable::isEqualTo( other ) )
	{
		return false;
	}

	const Camera *tOther = static_cast<const Camera *>( other );

	// check parameters
	if( !m_parameters->isEqualTo( tOther->m_parameters.get() ) )
	{
		return false;
	}

	return true;
}

void Camera::memoryUsage( Object::MemoryAccumulator &a ) const
{
	PreWorldRenderable::memoryUsage( a );
	a.accumulate( m_parameters.get() );
}

void Camera::hash( MurmurHash &h ) const
{
	PreWorldRenderable::hash( h );
	m_parameters->hash( h );
}

CompoundDataMap &Camera::parameters()
{
	return m_parameters->writable();
}

const CompoundDataMap &Camera::parameters() const
{
	return m_parameters->readable();
}

CompoundData *Camera::parametersData()
{
	return m_parameters.get();
}

const CompoundData *Camera::parametersData() const
{
	return m_parameters.get();
}

// This builds a default aperture, used by the accessors if no aperture has been set.
// In the future, it will be trivial, but for the moment, in order to support old caches,
// it builds an aperture from the deprecated logic for screenWindow and projection:fov parameters.
Imath::Box2f Camera::defaultApertureRect() const
{
	float scale = 1.0f;
	if( getProjection() == "perspective" )
	{
		float fov = accessCompoundDataMapWithDefault<FloatData>( parameters(), "projection:fov", -1.0f );
		if( fov == -1.0f )
		{
			// No obselete stuff to worry about
			return Imath::Box2f( Imath::V2f(-1), Imath::V2f(1) );
		}
		scale = tan( 0.5f * IECore::degreesToRadians( fov ) ) * getFocalLength();
	}

	Imath::Box2f screenWindow = accessCompoundDataMapWithDefault<Box2fData>( parameters(), "screenWindow", Imath::Box2f() );
	if( screenWindow.isEmpty() )
	{
		const Imath::V2i &resolution = accessCompoundDataMapWithDefault<V2iData>( parameters(), "resolution", V2i( 512 ) );
		float pixelAspectRatio = accessCompoundDataMapWithDefault<FloatData>( parameters(), "pixelAspectRatio", 1 );
		float aspectRatio = ((float)resolution.x * pixelAspectRatio)/(float)resolution.y;
		if( aspectRatio < 1.0f )
		{
			screenWindow.min.x = -1;
			screenWindow.max.x = 1;
			screenWindow.min.y = -1.0f / aspectRatio;
			screenWindow.max.y = 1.0f / aspectRatio;
		}
		else
		{
			screenWindow.min.y = -1;
			screenWindow.max.y = 1;
			screenWindow.min.x = -aspectRatio;
			screenWindow.max.x = aspectRatio;
		}
	}

	screenWindow.min *= scale;
	screenWindow.max *= scale;

	return screenWindow;
}

#define DECLARE_ACCESSORS( NAME, PROPERTY, DATATYPE, DEFAULT )\
void Camera::set##NAME( const typename DATATYPE::ValueType &value ) { parameters()[PROPERTY] = new DATATYPE( value ); }\
typename DATATYPE::ValueType Camera::get##NAME() const { return accessCompoundDataMapWithDefault<DATATYPE>( parameters(), PROPERTY, DEFAULT ); }

#define DECLARE_ACCESSORS_FOR_OPTIONAL( NAME, PROPERTY, DATATYPE, DEFAULT )\
DECLARE_ACCESSORS( NAME, PROPERTY, DATATYPE, DEFAULT )\
bool Camera::has##NAME() const { return checkCompoundDataMap<DATATYPE>( parameters(), PROPERTY ); }\
void Camera::remove##NAME() { parameters().erase(PROPERTY); }


DECLARE_ACCESSORS( Projection, "projection", StringData, "orthographic" );
// The complexity of the default values for these 4 parameters is just due to backwards compatibility for
// old cameras with screenWindow baked in - once we're free of those, these will be constants
DECLARE_ACCESSORS( Aperture, "aperture", V2fData, defaultApertureRect().size() );
DECLARE_ACCESSORS( ApertureOffset, "apertureOffset", V2fData, defaultApertureRect().center() );
DECLARE_ACCESSORS( FocalLength, "focalLength", FloatData, 1.0f );
DECLARE_ACCESSORS( ClippingPlanes, "clippingPlanes", V2fData, V2f( 0.01f, 100000.0f ) );
DECLARE_ACCESSORS( FStop, "fStop", FloatData, 0.0f );
// The default value of focalLengthWorldScale is for a focalLength in mm, and world units in cm
// This matches Alembic and USD's convention
DECLARE_ACCESSORS( FocalLengthWorldScale, "focalLengthWorldScale", FloatData, 0.1f );
DECLARE_ACCESSORS( FocusDistance, "focusDistance", FloatData, 1.0f );

// Film fit mode requires a specialized accessor, to convert to the enum
bool Camera::hasFilmFit() const
{
	return checkCompoundDataMap<IntData>( parameters(), "filmFit" );
}
void Camera::setFilmFit( const Camera::FilmFit &value )
{
	parameters()["filmFit"] = new IntData( value );
}
Camera::FilmFit Camera::getFilmFit() const
{
	return (FilmFit)accessCompoundDataMapWithDefault<IntData>( parameters(), "filmFit", Horizontal );
}
void Camera::removeFilmFit()
{
	parameters().erase("filmFit");
}

DECLARE_ACCESSORS_FOR_OPTIONAL( Resolution, "resolution", V2iData, V2i( 640, 480 ) );
DECLARE_ACCESSORS_FOR_OPTIONAL( PixelAspectRatio, "pixelAspectRatio", FloatData, 1.0f );
DECLARE_ACCESSORS_FOR_OPTIONAL( ResolutionMultiplier, "resolutionMultiplier", FloatData, 1.0f );
DECLARE_ACCESSORS_FOR_OPTIONAL( Overscan, "overscan", BoolData, false );
DECLARE_ACCESSORS_FOR_OPTIONAL( OverscanLeft, "overscanLeft", FloatData, 0.0f );
DECLARE_ACCESSORS_FOR_OPTIONAL( OverscanRight, "overscanRight", FloatData, 0.0f );
DECLARE_ACCESSORS_FOR_OPTIONAL( OverscanTop, "overscanTop", FloatData, 0.0f );
DECLARE_ACCESSORS_FOR_OPTIONAL( OverscanBottom, "overscanBottom", FloatData, 0.0f );
DECLARE_ACCESSORS_FOR_OPTIONAL( CropWindow, "cropWindow", Box2fData, Box2f() );
DECLARE_ACCESSORS_FOR_OPTIONAL( Shutter, "shutter", V2fData, V2f( -0.5f, 0.5f ) );

namespace {

//pxr/imaging/lib/cameraUtil/conformWindow.cpp : _ResolveConformWindowPolicy
Camera::FilmFit resolveFitMode(const Imath::V2f &size, Camera::FilmFit fitMode, float targetAspect)
{
	if( (fitMode == Camera::Horizontal) || (fitMode == Camera::Vertical ) )
	{
		return fitMode;
	}

	const float aspect =
		(size[1] != 0.0) ? size[0] / size[1] : 1.0;

	if( (fitMode == Camera::Fit ) != (aspect > targetAspect) )
	{
		return Camera::Vertical;
	}

	return Camera::Horizontal;
}

}

//pxr/imaging/lib/cameraUtil/conformWindow.cpp : CameraUtilConformedWindow
Imath::Box2f Camera::fitWindow( const Imath::Box2f &window, Camera::FilmFit fitMode, float targetAspect)
{
	if( fitMode == Camera::Distort )
	{
		return window; // Just return the original window, even if this produces non-square pixels
	}

	const Camera::FilmFit resolvedFitMode =
		resolveFitMode( window.size(), fitMode, targetAspect);

	if( resolvedFitMode == Camera::Horizontal )
	{
		const double height =
			window.size()[0] / (targetAspect != 0.0 ? targetAspect : 1.0);

		return Imath::Box2f(
			Imath::V2f( window.min[0], window.center()[1] - 0.5f * height ),
			Imath::V2f( window.max[0], window.center()[1] + 0.5f * height )
		);
	}
	else
	{
		const double width = window.size()[1] * targetAspect;

		return Imath::Box2f(
			Imath::V2f(window.center()[0] - 0.5f * width, window.min[1]),
			Imath::V2f(window.center()[0] + 0.5f * width, window.max[1])
		);
	}
}

Imath::Box2f Camera::frustum() const
{
	return frustum( getFilmFit() );
}

Imath::Box2f Camera::frustum( FilmFit filmFit ) const
{
	Imath::V2i resolution = renderResolution();
	float aspectRatio = float( std::max( 1, resolution.x ) ) / std::max( 1, resolution.y ) * getPixelAspectRatio();
	return frustum( filmFit, aspectRatio );
}

Imath::Box2f Camera::frustum( FilmFit filmFit, float aspectRatio ) const
{
	Imath::V2f corner( 0.5f * getAperture() );

	// A degenerate screen window can cause weird problems ( for example it currently causes
	// crashes in our GL backend ), so lets make totally sure that doesn't happen.
	if( !( isfinite( corner.x ) && corner.x > 0.0f ) )
	{
		corner.x = 1e-16;
	}
	if( !( isfinite( corner.y ) && corner.y > 0.0f ) )
	{
		corner.y = 1e-16;
	}

	Imath::Box2f window( -corner, corner );

	// Apply the aperture offset
	Imath::V2f offset( getApertureOffset() );
	if( !isfinite( offset.x ) )
	{
		offset.x = 0;
	}
	if( !isfinite( offset.y ) )
	{
		offset.y = 0;
	}
	window.min += offset;
	window.max += offset;

	if ( getProjection() == "perspective" && getFocalLength() != 0.0f ) {
		window.min /= getFocalLength();
		window.max /= getFocalLength();
	}

	return fitWindow( window, filmFit, aspectRatio );
}


Imath::V2i Camera::renderResolution() const
{
	V2i origResolution = getResolution();
	float resolutionMultiplier = getResolutionMultiplier();
	return V2i(
		std::max( 1, int((float)origResolution.x * resolutionMultiplier) ),
		std::max( 1, int((float)origResolution.y * resolutionMultiplier) )
	);
}

Imath::Box2i Camera::renderRegion() const
{
	V2i resolution = renderResolution();
	Box2i renderRegion = Box2i( V2i( 0 ), resolution );

	if( getOverscan() )
	{
		// convert offsets into pixel values and apply them to the render region
		renderRegion.min -= V2i(
			int( getOverscanLeft() * (float)resolution.x),
			int( getOverscanBottom() * (float)resolution.y)
		);

		renderRegion.max += V2i(
			int( getOverscanRight() * (float)resolution.x),
			int( getOverscanTop() * (float)resolution.y)
		);
	}


	if( hasCropWindow() )
	{
		const Box2f &cropWindow = getCropWindow();

		// Note the flip in Y because cropWindow uses a RenderMan convention, but renderRegion
		// uses the Gaffer image convention
		Box2i cropRegion(
			V2i(
				(int)( round( resolution.x * cropWindow.min.x ) ),
				resolution.y - (int)( round( resolution.y * cropWindow.max.y ) )
			),
			V2i(
				(int)( round( resolution.x * cropWindow.max.x ) ),
				resolution.y - (int)( round( resolution.y * cropWindow.min.y ) )
			)
		);

		renderRegion.min.x = std::max( renderRegion.min.x, cropRegion.min.x );
		renderRegion.max.x = std::min( renderRegion.max.x, cropRegion.max.x );
		renderRegion.min.y = std::max( renderRegion.min.y, cropRegion.min.y );
		renderRegion.max.y = std::min( renderRegion.max.y, cropRegion.max.y );
	}

	return renderRegion;
}

Imath::V2f Camera::calculateFieldOfView() const
{
	return V2f(
		2.0f * IECore::radiansToDegrees( atan( 0.5f * getAperture()[0] / getFocalLength() ) ),
		2.0f * IECore::radiansToDegrees( atan( 0.5f * getAperture()[1] / getFocalLength() ) )
	);
}

void Camera::setFocalLengthFromFieldOfView( float horizontalFOV )
{
	setFocalLength( getAperture()[0] * 0.5f / tan( 0.5f * IECore::degreesToRadians( horizontalFOV ) ) );
}

void Camera::render( Renderer *renderer ) const
{
	// The old renderer interface takes unused name parameter
	renderer->camera( "", m_parameters->readable() );
}
