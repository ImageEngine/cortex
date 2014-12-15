//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2013, Image Engine Design Inc. All rights reserved.
//  Copyright (c) 2012, John Haddon. All rights reserved.
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

#ifndef IECORE_CAMERACONTROLLER_H
#define IECORE_CAMERACONTROLLER_H

#include "IECore/Export.h"
#include "IECore/SimpleTypedData.h"

#include "OpenEXR/ImathVec.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( Camera )
IE_CORE_FORWARDDECLARE( MatrixTransform )

/// The CameraController class provides methods to aid in the use
/// of the Camera class within the context of a GUI.
class IECORE_API CameraController : public boost::noncopyable
{

	public:

		CameraController( CameraPtr camera );
		~CameraController();

		void setCamera( CameraPtr camera );
		CameraPtr getCamera();
		ConstCameraPtr getCamera() const;

		/// Positive.
		void setCentreOfInterest( float centreOfInterest );
		float getCentreOfInterest();

		enum ScreenWindowAdjustment
		{
			/// Crop/extend the screen window to accommodate
			/// the new resolution without scaling the content.
			CropScreenWindow,
			/// Preserve the horizontal framing and change the
			/// vertical framing to maintain aspect ratio.
			ScaleScreenWindow
		};

		/// \todo Remove this form, and add a default value for adjustment
		/// in the form below.
		void setResolution( const Imath::V2i &resolution );
		/// Changes the camera resolution, modifying the screen window
		/// in a manner determined by the adjustment argument.
		void setResolution( const Imath::V2i &resolution, ScreenWindowAdjustment adjustment );
		const Imath::V2i &getResolution() const;

		/// Moves the camera to frame the specified box, keeping the
		/// current viewing direction unchanged.
		void frame( const Imath::Box3f &box );
		/// Moves the camera to frame the specified box, viewing it from the
		/// specified direction, and with the specified up vector.
		void frame( const Imath::Box3f &box, const Imath::V3f &viewDirection,
			const Imath::V3f &upVector = Imath::V3f( 0, 1, 0 ) );

		/// Computes the points on the near and far clipping planes that correspond
		/// with the specified raster position. Points are computed in world space.
		/// \todo Accept a V2f to provide extra precision and make this method const.
		void unproject( const Imath::V2f rasterPosition, Imath::V3f &near, Imath::V3f &far );
		/// Projects the point in world space into a raster space position.
		Imath::V2f project( const Imath::V3f &worldPosition ) const;
		
		//! @name Motion
		/// These functions facilitate the implementation of maya style
		/// camera movement controls within a UI. All coordinates passed
		/// are mouse coordinates in raster space (0,0 at top left).
		//////////////////////////////////////////////////////////////////
		//@{
		enum MotionType
		{
			None,
			Track,
			Tumble,
			Dolly
		};
		/// Starts a motion of the specified type.
		void motionStart( MotionType motion, const Imath::V2f &startPosition );
		/// Updates the camera position based on a changed mouse position. Can only
		/// be called after motionStart() and before motionEnd().
		void motionUpdate( const Imath::V2f &newPosition );
		/// End the current motion, ready to call motionStart() again if required.
		void motionEnd( const Imath::V2f &endPosition );
		//@}

	private:

		void track( const Imath::V2f &p );
		void tumble( const Imath::V2f &p );
		void dolly( const Imath::V2f &p );

		IE_CORE_FORWARDDECLARE( MemberData );

		MemberDataPtr m_data;

};

}

#endif // IECORE_CAMERACONTROLLER_H
