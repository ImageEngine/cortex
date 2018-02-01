//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

#include "IECoreScene/SampledSceneInterface.h"

#include "IECore/ObjectInterpolator.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/TransformationMatrixData.h"

using namespace IECore;
using namespace IECoreScene;

IE_CORE_DEFINERUNTIMETYPEDDESCRIPTION( SampledSceneInterface )

SampledSceneInterface::~SampledSceneInterface()
{
}

Imath::Box3d SampledSceneInterface::readBound( double time ) const
{
	size_t sample1, sample2;
	double x = boundSampleInterval( time, sample1, sample2 );

	if( x == 0 )
	{
		return readBoundAtSample( sample1 );
	}
	if( x == 1 )
	{
		return readBoundAtSample( sample2 );
	}

	Imath::Box3d box1 = readBoundAtSample( sample1 );
	Imath::Box3d box2 = readBoundAtSample( sample2 );
	Imath::Box3d box;
	LinearInterpolator<Imath::Box3d>()(box1, box2, x, box);
	return box;
}

ConstDataPtr SampledSceneInterface::readTransform( double time ) const
{
	size_t sample1, sample2;
	double x = transformSampleInterval( time, sample1, sample2 );

	if( x == 0 )
	{
		return readTransformAtSample( sample1 );
	}
	if( x == 1 )
	{
		return readTransformAtSample( sample2 );
	}

	ConstDataPtr transformData1 = readTransformAtSample( sample1 );
	ConstDataPtr transformData2 = readTransformAtSample( sample2 );
	DataPtr transformData = runTimeCast< Data >( linearObjectInterpolation( transformData1.get(), transformData2.get(), x ) );
	if( !transformData )
	{
		// failed to interpolate, return the closest one
		return ( x >= 0.5 ? transformData2 : transformData1 );
	}
	return transformData;
}

Imath::M44d SampledSceneInterface::readTransformAsMatrix( double time ) const
{
	ConstDataPtr d = readTransform( time );
	switch( d->typeId() )
	{
		case M44dDataTypeId :
			return static_cast<const M44dData *>( d.get() )->readable();
		case TransformationMatrixdDataTypeId :
			return static_cast<const TransformationMatrixdData *>( d.get() )->readable().transform();
		default :
			throw Exception( "Unsupported transform data type" );
	}
}

ConstObjectPtr SampledSceneInterface::readAttribute( const Name &name, double time ) const
{
	size_t sample1, sample2;
	double x = attributeSampleInterval( name, time, sample1, sample2 );

	if( x == 0 )
	{
		return readAttributeAtSample( name, sample1 );
	}
	if( x == 1 )
	{
		return readAttributeAtSample( name, sample2 );
	}

	ConstObjectPtr attributeObj1 = readAttributeAtSample( name, sample1 );
	ConstObjectPtr attributeObj2 = readAttributeAtSample( name, sample2 );

	ObjectPtr attributeObj = linearObjectInterpolation( attributeObj1.get(), attributeObj2.get(), x );
	if( !attributeObj )
	{
		// failed to interpolate, return the closest one
		return ( x >= 0.5 ? attributeObj2 : attributeObj1 );
	}
	return attributeObj;
}

ConstObjectPtr SampledSceneInterface::readObject( double time ) const
{
	size_t sample1, sample2;
	double x = objectSampleInterval( time, sample1, sample2 );

	if( x == 0 )
	{
		return readObjectAtSample( sample1 );
	}
	if( x == 1 )
	{
		return readObjectAtSample( sample2 );
	}

	ConstObjectPtr object1 = readObjectAtSample( sample1 );
	ConstObjectPtr object2 = readObjectAtSample( sample2 );

	ObjectPtr object = linearObjectInterpolation( object1.get(), object2.get(), x );
	if( !object )
	{
		// failed to interpolate, return the closest one
		return ( x >= 0.5 ? object2 : object1 );
	}

	return object;
}
