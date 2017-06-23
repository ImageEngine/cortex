//////////////////////////////////////////////////////////////////////////
//
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

#include "IECore/SimpleTypedData.h"
#include "IECore/Transform.h"

#include "IECoreAlembic/FromAlembicCameraConverter.h"

using namespace Imath;
using namespace Alembic::AbcGeom;
using namespace IECore;
using namespace IECoreAlembic;

FromAlembicCameraConverter::ConverterDescription<FromAlembicCameraConverter> FromAlembicCameraConverter::g_description;

IE_CORE_DEFINERUNTIMETYPED( FromAlembicCameraConverter );

FromAlembicCameraConverter::FromAlembicCameraConverter( Alembic::Abc::IObject iCamera )
	:	FromAlembicConverter( "Converts AbcGeom::ICamera objects to IECore::Camera objects", iCamera )
{
}

IECore::ObjectPtr FromAlembicCameraConverter::doAlembicConversion( const Alembic::Abc::IObject &iObject, const Alembic::Abc::ISampleSelector &sampleSelector, const IECore::CompoundObject *operands ) const
{
	ICamera iCamera( iObject, kWrapExisting );
	ICameraSchema &iCameraSchema = iCamera.getSchema();
	CameraSample sample;
	iCameraSchema.get( sample, sampleSelector );

	CameraPtr result = new Camera;
	result->parameters()["projection"] = new StringData( "perspective" );

	double top, bottom, left, right;
	sample.getScreenWindow( top, bottom, left, right );
	result->parameters()["screenWindow"] = new Box2fData( Box2f( V2f( left, bottom ), V2f( right, top ) ) );
	result->parameters()["projection:fov"] = new FloatData( sample.getFieldOfView() );

	return result;
}
