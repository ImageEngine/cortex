//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2020, Cinesite VFX Ltd. All rights reserved.
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

#include "IECoreAlembic/ObjectWriter.h"

#include "IECoreScene/Camera.h"

#include "Alembic/AbcGeom/OCamera.h"

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreAlembic;
using namespace Alembic::AbcGeom;

namespace
{

class CameraWriter : public IECoreAlembic::ObjectWriter
{

	public :

		CameraWriter( Alembic::Abc::OObject &parent, const std::string &name )
			:	m_camera( parent, name )
		{
		}

		void writeSample( const IECore::Object *object ) override
		{
			const Camera *camera = runTimeCast<const Camera>( object );
			if( !camera )
			{
				throw IECore::Exception( "CameraWriter expected a Camera" );
			}

			CameraSample sample;

			// Alembic doesn't have an equivalent to `focalLengthWorldScale`,
			// so we must bake that into the values. Alembic stores focal
			// length in tenths of world units.

			sample.setFocalLength( camera->getFocalLength() * camera->getFocalLengthWorldScale() * 10.0f );
			sample.setHorizontalAperture( camera->getAperture().x * camera->getFocalLengthWorldScale() );
			sample.setVerticalAperture( camera->getAperture().y * camera->getFocalLengthWorldScale()  );
			sample.setHorizontalFilmOffset( camera->getApertureOffset().x * camera->getFocalLengthWorldScale() );
			sample.setVerticalFilmOffset( camera->getApertureOffset().y * camera->getFocalLengthWorldScale() );

			sample.setNearClippingPlane( camera->getClippingPlanes()[0] );
			sample.setFarClippingPlane( camera->getClippingPlanes()[1] );

			sample.setFStop( camera->getFStop() );
			sample.setFocusDistance( camera->getFocusDistance() );

			m_camera.getSchema().set( sample );
		}

		void writeTimeSampling( const Alembic::AbcCoreAbstract::TimeSamplingPtr &timeSampling ) override
		{
			m_camera.getSchema().setTimeSampling( timeSampling );
		}

	private :

		Alembic::AbcGeom::OCamera m_camera;

		static Description<CameraWriter> g_description;

};

IECoreAlembic::ObjectWriter::Description<CameraWriter> CameraWriter::g_description( IECoreScene::Camera::staticTypeId() );

} // namespace

