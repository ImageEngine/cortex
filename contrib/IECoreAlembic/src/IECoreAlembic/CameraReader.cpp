//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, John Haddon. All rights reserved.
//  Copyright (c) 2017, Image Engine Design Inc. All rights reserved.
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

#include "IECoreAlembic/ObjectReader.h"

#include "IECoreScene/Camera.h"
#include "IECoreScene/Transform.h"

#include "IECore/SimpleTypedData.h"

#include "Alembic/AbcGeom/ICamera.h"

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreAlembic;
using namespace Alembic::AbcGeom;

namespace
{

class CameraReader : public IECoreAlembic::ObjectReader
{

	public :

		CameraReader( const ICamera &camera )
			:	m_camera( camera )
		{
		}

		const Alembic::Abc::IObject &object() const override
		{
			return m_camera;
		}

		Alembic::Abc::IBox3dProperty readBoundProperty() const override
		{
			return IBox3dProperty();
		}

		size_t readNumSamples() const override
		{
			return m_camera.getSchema().getNumSamples();
		}

		Alembic::AbcCoreAbstract::TimeSamplingPtr readTimeSampling() const override
		{
			return m_camera.getSchema().getTimeSampling();
		}

		IECore::ObjectPtr readSample( const Alembic::Abc::ISampleSelector &sampleSelector ) const override
		{
			const ICameraSchema &cameraSchema = m_camera.getSchema();
			CameraSample sample;
			cameraSchema.get( sample, sampleSelector );

			CameraPtr result = new Camera;
			result->setProjection( "perspective" );

			result->setFocalLength( sample.getFocalLength() );
			result->setAperture( 10.0f *
				Imath::V2f( sample.getHorizontalAperture(), sample.getVerticalAperture() )
			);
			result->setApertureOffset( 10.0f *
				Imath::V2f( sample.getHorizontalFilmOffset(), sample.getVerticalFilmOffset() )
			);

			result->setClippingPlanes(
				Imath::V2f( sample.getNearClippingPlane(), sample.getFarClippingPlane() )
			);
			result->setFStop( sample.getFStop() );
			result->setFocalLengthWorldScale( 0.1 ); // Alembic stores focal length in tenths of world units
			result->setFocusDistance( sample.getFocusDistance() );

			return result;
		}

	private :

		const ICamera m_camera;

		static Description<CameraReader, ICamera> g_description;
};

IECoreAlembic::ObjectReader::Description<CameraReader, ICamera> CameraReader::g_description( Camera::staticTypeId() );

} // namespace
