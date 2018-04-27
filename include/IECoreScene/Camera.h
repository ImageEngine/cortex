//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORESCENE_CAMERA_H
#define IECORESCENE_CAMERA_H

#include "IECoreScene/Export.h"
#include "IECoreScene/PreWorldRenderable.h"

namespace IECoreScene
{

class IECORESCENE_API Camera : public PreWorldRenderable
{
	public:
		enum FilmFit
		{
			Horizontal,
			Vertical,
			Fit,
			Fill,
			Distort,
		};

		Camera( IECore::CompoundDataPtr parameters = new IECore::CompoundData );
		~Camera() override;

		IE_CORE_DECLAREEXTENSIONOBJECT( Camera, CameraTypeId, PreWorldRenderable );

		IECore::CompoundDataMap &parameters();
		const IECore::CompoundDataMap &parameters() const;

		/// This is mostly of use for the binding - the parameters()
		/// function gives more direct access to the contents of the CompoundData
		/// (it calls readable() or writable() for you).
		IECore::CompoundData *parametersData();
		const IECore::CompoundData *parametersData() const;

		/// Camera parameters
		/// ------------------------------
		///
		/// These are the fundamental parameters of the camera.
		/// They are stored in the parameters of key/value pairs, but you can always
		/// just use these accessors.  get* behaves as if parameters that have not
		/// been set yet had been stored with a default value.

		/// The supported standard values of projection are "perspective" or "orthographic".
		/// Specific renderer backends may support other options
		std::string getProjection() const;
		void setProjection( const std::string &projection );

		/// When using an "orthographic" projection, the aperture is the size
		/// of the frustum in world units.
		///
		/// When using a "perspective" projection, aperture is defines the film back size, and
		/// the units of aperture are arbitrary, as long as they are the same as the units of focal
		/// length.  By convention, usually millimeters are used, regardless of world units, so
		/// that the values make sense to art who are used to using a "35mm lens" to define a field
		/// of view.  When matching a physical camera, you should set aperture and focal length
		/// based on the film back size and focal length of the physical camera.  If you are using
		/// depth of field, you will need to set focalLengthWorldScale to the ratio between
		/// world units and aperture units.
		Imath::V2f getAperture() const;
		void setAperture( const Imath::V2f &aperture );

		/// Aperture offset a horizontal and vertical offset of the frustum in the same units
		/// as aperture.  For perspective cameras, it can be used to create skewed frustums.
		Imath::V2f getApertureOffset() const;
		void setApertureOffset( const Imath::V2f &apertureOffset );

		/// For perspective cameras, specify the focal length, in the same units as aperture.
		float getFocalLength() const;
		void setFocalLength( const float &focalLength );

		/// Near and far clipping planes, in world units.
		Imath::V2f getClippingPlanes() const;
		void setClippingPlanes( const Imath::V2f &clippingPlanes );

		/// Ratio of focal length to the diameter of the lens opening, for use in depth of field
		/// blur calculations.  Setting to 0 disable depth of field, otherwise setting it lower
		/// produces more blur.
		float getFStop() const;
		void setFStop( const float &fStop );

		/// In order to use the focal length for computing depth of field, we need the focal
		/// length in world units.  This parameter specifies the scale from focal length to
		/// world units.  The default of 0.1 is correct for a focal length in millimeters and
		/// world units in centimeters ( these are the default units of Alembic and USD ).
		float getFocalLengthWorldScale() const;
		void setFocalLengthWorldScale( const float &focalLengthWorldScale );

		/// World unit distance to the plane which the camera sees in perfect focus
		float getFocusDistance() const;
		void setFocusDistance( const float &focusDistance );


		/// Rendering parameters
		/// ------------------------------
		///
		/// These specify additional optional overrides of rendering settings.
		/// Usually these setting should be controlled from the render globals,
		/// but we include the option of overriding them on the camera so that :
		/// - A user can set up a projection camera, where it is important that
		///   the aspect ratio not vary with the render globals
		/// - A user can override overscan or crop on just one camera in a
		///   multi-camera setup
		/// - So that the `Renderer::camera()` method receives everything
		///   related to a particular camera in a single call, simplifying IPR edits.
		///   This means that settings from the render globals must be baked into
		///   the camera before passing to Renderer.

		/// Determines how the size of the rendered image relates to the camera aperture.
		/// You can fit based on horizontal aperture, vertical aperture, or the min or max.
		bool hasFilmFit() const;
		FilmFit getFilmFit() const;
		void setFilmFit( const FilmFit &filmFit );
		void removeFilmFit();

		/// Override the render resolution.
		bool hasResolution() const;
		Imath::V2i getResolution() const;
		void setResolution( const Imath::V2i &resolution );
		void removeResolution();

		/// Override the render pixel aspect ratio.
		bool hasPixelAspectRatio() const;
		float getPixelAspectRatio() const;
		void setPixelAspectRatio( const float &pixelAspectRatio );
		void removePixelAspectRatio();

		/// Override the render multiplier.
		bool hasResolutionMultiplier() const;
		float getResolutionMultiplier() const;
		void setResolutionMultiplier( const float &resolutionMultiplier );
		void removeResolutionMultiplier();

		/// Override the overscan toggle
		bool hasOverscan() const;
		bool getOverscan() const;
		void setOverscan( const bool &overscan );
		void removeOverscan();

		/// Override the left overscan. Specified as a 0-1 proportion of the original image width.
		bool hasOverscanLeft() const;
		float getOverscanLeft() const;
		void setOverscanLeft( const float &overscanLeft );
		void removeOverscanLeft();

		/// Override the right overscan. Specified as a 0-1 proportion of the original image width.
		bool hasOverscanRight() const;
		float getOverscanRight() const;
		void setOverscanRight( const float &overscanRight );
		void removeOverscanRight();

		/// Override the top overscan. Specified as a 0-1 proportion of the original image height.
		bool hasOverscanTop() const;
		float getOverscanTop() const;
		void setOverscanTop( const float &overscanTop );
		void removeOverscanTop();

		/// Override the bottom overscan. Specified as a 0-1 proportion of the original image height.
		bool hasOverscanBottom() const;
		float getOverscanBottom() const;
		void setOverscanBottom( const float &overscanBottom );
		void removeOverscanBottom();

		/// Override the crop window. Specified as a 0-1 proportion of the original image height.
		bool hasCropWindow() const;
		Imath::Box2f getCropWindow() const;
		void setCropWindow( const Imath::Box2f &overscanBottom );
		void removeCropWindow();

		/// Override the shutter, stored in frames.
		/// If this camera is stored in a file or being used for processing in Gaffer, this is relative
		/// to the frame ( eg. -0.25, 0.25 ).
		/// If this camera is being used in the renderer backend, this is absolute ( eg. 1000.75, 1001.25 ).
		bool hasShutter() const;
		Imath::V2f getShutter() const;
		void setShutter( const Imath::V2f &shutter );
		void removeShutter();

		/// Given window with an arbitrary aspect, compute a box that fits it with a particular fit mode,
		/// to achieve a desired target aspect ratio.
		static Imath::Box2f fitWindow( const Imath::Box2f &window, Camera::FilmFit fitMode, float targetAspect );

		/// This method returns a screen window at a distance of 1 unit from the camera.  This
		/// canonical form is a concise way to characterize the frustum of the camera.
		///
		/// The first form computes the filmFit and aspect ratio based on the render overrides
		/// set on the camera, or the default values.
		///
		/// The second form overrides the filmFit, and the third mode overrides both filmFit and
		/// aspect ratio.
		Imath::Box2f frustum() const;
		Imath::Box2f frustum( FilmFit filmFit ) const;
		Imath::Box2f frustum( FilmFit filmFit, float aspectRatio ) const;

		/// Return the render resolution, based on resolution and resolutionMultiplier overrides.
		Imath::V2i renderResolution() const;

		/// Return the render region, based on resolution, resolutionMultiplier, overscan, and
		/// cropWindow overrides.  The render region is represented in Gaffer image coordinates,
		/// with +Y up and an exclusive upper bound.
		Imath::Box2i renderRegion() const;

		/// Based on the focal length and aperture, compute the horizontal and vertical field of
		/// view, in degrees.
		Imath::V2f calculateFieldOfView() const;

		/// Set the focal length so that based on the current aperture, we get the specified
		/// horizontal field of view ( in degrees )
		void setFocalLengthFromFieldOfView( float horizontalFOV );

		void render( Renderer *renderer ) const override;


	private:

		Imath::Box2f defaultApertureRect() const;

		IECore::CompoundDataPtr m_parameters;

		static const unsigned int m_ioVersion;
};

IE_CORE_DECLAREPTR( Camera );

}

#endif // IECORESCENE_CAMERA_H
