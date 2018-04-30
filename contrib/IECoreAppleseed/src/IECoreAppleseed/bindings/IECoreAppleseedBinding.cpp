//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2014, Esteban Tovagliari. All rights reserved.
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

#include "boost/python.hpp"
#include "boost/python/suite/indexing/container_utils.hpp"

#include "IECoreAppleseed/CameraAlgo.h"
#include "IECoreAppleseed/ObjectAlgo.h"
#include "IECoreAppleseed/TransformAlgo.h"

using namespace boost::python;
using namespace IECoreAppleseed;

namespace
{

foundation::auto_release_ptr<renderer::Camera> cameraConvertWrapper( const IECoreScene::Camera *camera )
{
	return foundation::auto_release_ptr<renderer::Camera>(
		CameraAlgo::convert( camera )
	);
}

void bindCameraAlgo()
{
	object objectAlgoModule( handle<>( borrowed( PyImport_AddModule( "IECoreAppleseed.CameraAlgo" ) ) ) );
	scope().attr( "CameraAlgo" ) = objectAlgoModule;
	scope objectAlgoModuleScope( objectAlgoModule );

	def( "convert", &cameraConvertWrapper );
}

foundation::auto_release_ptr<renderer::Object> convertWrapper( const IECore::Object *primitive )
{
	return foundation::auto_release_ptr<renderer::Object>(
		ObjectAlgo::convert( primitive )
	);
}

foundation::auto_release_ptr<renderer::Object> convertWrapper2( object pythonSamples, object pythonTimes, float shutterOpenTime, float shutterCloseTime )
{
	std::vector<const IECore::Object *> samples;
	container_utils::extend_container( samples, pythonSamples );

	std::vector<float> times;
	container_utils::extend_container( times, pythonTimes );

	return foundation::auto_release_ptr<renderer::Object>(
		ObjectAlgo::convert( samples, times, shutterOpenTime, shutterCloseTime )
	);
}

void bindObjectAlgo()
{

	object objectAlgoModule( handle<>( borrowed( PyImport_AddModule( "IECoreAppleseed.ObjectAlgo" ) ) ) );
	scope().attr( "ObjectAlgo" ) = objectAlgoModule;
	scope objectAlgoModuleScope( objectAlgoModule );

	def( "isPrimitiveSupported", &ObjectAlgo::isPrimitiveSupported );
	def( "convert", &convertWrapper );
	def( "convert", &convertWrapper2 );

}

void makeTransformSequenceWrapper1( const Imath::M44f &m, renderer::TransformSequence &xformSeq )
{
	TransformAlgo::makeTransformSequence( m, xformSeq );
}

void makeTransformSequenceWrapper2( object pythonTimes, object pythonTransforms, renderer::TransformSequence &xformSeq )
{
	std::vector<float> times;
	container_utils::extend_container( times, pythonTimes );

	std::vector<Imath::M44f> transforms;
	container_utils::extend_container( transforms, pythonTransforms );

	TransformAlgo::makeTransformSequence( times, transforms, xformSeq );
}

void bindTransformAlgo()
{

	object transformAlgoModule( handle<>( borrowed( PyImport_AddModule( "IECoreAppleseed.TransformAlgo" ) ) ) );
	scope().attr( "TransformAlgo" ) = transformAlgoModule;
	scope transformAlgoModuleScope( transformAlgoModule );

	def( "makeTransformSequence", &makeTransformSequenceWrapper1 );
	def( "makeTransformSequence", &makeTransformSequenceWrapper2 );

}

} // namespace

BOOST_PYTHON_MODULE( _IECoreAppleseed )
{
	bindCameraAlgo();
	bindObjectAlgo();
	bindTransformAlgo();
}
