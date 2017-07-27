//////////////////////////////////////////////////////////////////////////
//
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

#include "Alembic/AbcGeom/IGeomBase.h"
#include "Alembic/AbcGeom/ICamera.h"
#include "Alembic/AbcGeom/IXForm.h"

#include "IECoreAlembic/ObjectAlgo.h"

using namespace Alembic;
using namespace IECore;
using namespace IECoreAlembic;

namespace IECoreAlembic
{

namespace ObjectAlgo
{

namespace Detail
{

RegistrationVector &registrations()
{
	static RegistrationVector r;
	return r;
}

} // namespace Detail

IECore::ObjectPtr convert( const Alembic::Abc::IObject &object, const Alembic::Abc::ISampleSelector &sampleSelector, IECore::TypeId resultType )
{
	const Abc::MetaData &md = object.getMetaData();
	const Detail::RegistrationVector &r = Detail::registrations();
	for( Detail::RegistrationVector::const_reverse_iterator it = r.rbegin(), eIt = r.rend(); it!=eIt; it++ )
	{
		const bool resultTypeMatches = resultType == IECore::InvalidTypeId || resultType == it->resultType || RunTimeTyped::inheritsFrom( it->resultType, resultType );
		if( resultTypeMatches && it->matcher( md, Abc::kStrictMatching ) )
		{
			return it->converter( object, sampleSelector );
		}
	}
	return NULL;
}

Alembic::AbcCoreAbstract::TimeSamplingPtr timeSampling( const Alembic::Abc::IObject &object, size_t &numSamples )
{
	const Abc::MetaData &md = object.getMetaData();

	// \todo Is there really no generic way of querying this from
	// Alembic generically, without trying every type one by one?

	if( !object.getParent() )
	{
		// Top of archive
		Alembic::Abc::IBox3dProperty boundsProperty( object.getProperties(), ".childBnds" );
		numSamples = boundsProperty.getNumSamples();
		return boundsProperty.getTimeSampling();
	}
	else if( AbcGeom::IXform::matches( md ) )
	{
		AbcGeom::IXform iXForm( object, Abc::kWrapExisting );
		numSamples = iXForm.getSchema().getNumSamples();
		return iXForm.getSchema().getTimeSampling();
	}
	else if( AbcGeom::ICamera::matches( md ) )
	{
		AbcGeom::ICamera iCamera( object, Abc::kWrapExisting );
		numSamples = iCamera.getSchema().getNumSamples();
		return iCamera.getSchema().getTimeSampling();
	}
	else
	{
		AbcGeom::IGeomBaseObject geomBase( object, Abc::kWrapExisting );
		numSamples = geomBase.getSchema().getNumSamples();
		return geomBase.getSchema().getTimeSampling();
	}

	return nullptr;
}

} // namespace ObjectAlgo

} // namespace IECoreAlembic
