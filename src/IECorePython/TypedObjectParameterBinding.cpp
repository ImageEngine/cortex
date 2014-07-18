//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
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

#include "IECorePython/TypedObjectParameterBinding.h"
#include "IECorePython/Wrapper.h"
#include "IECore/TypedObjectParameter.h"
#include "IECore/CompoundObject.h"
#include "IECorePython/RunTimeTypedBinding.h"
#include "IECorePython/Wrapper.h"

#include "IECore/Renderable.h"
#include "IECore/StateRenderable.h"
#include "IECore/AttributeState.h"
#include "IECore/Shader.h"
#include "IECore/Transform.h"
#include "IECore/MatrixMotionTransform.h"
#include "IECore/MatrixTransform.h"
#include "IECore/VisibleRenderable.h"
#include "IECore/Group.h"
#include "IECore/MotionPrimitive.h"
#include "IECore/Primitive.h"
#include "IECore/MeshPrimitive.h"
#include "IECore/CurvesPrimitive.h"
#include "IECore/PointsPrimitive.h"
#include "IECore/ObjectVector.h"
#include "IECore/SmoothSkinningData.h"

using namespace std;
using namespace boost;
using namespace boost::python;
using namespace IECore;

namespace IECorePython
{

template<typename T>
static void bindTypedObjectParameter()
{
	using boost::python::arg;

	RunTimeTypedClass<TypedObjectParameter<T>, TypedObjectParameterWrap<T> >()
		.def(
			init< const std::string &, const std::string &, typename T::Ptr, boost::python::optional<const object &, bool, CompoundObjectPtr > >
			(
				(
					arg( "name" ),
					arg( "description" ),
					arg( "defaultValue" ),
					arg( "presets" ) = boost::python::tuple(),
					arg( "presetsOnly" ) = false ,
					arg( "userData" ) = CompoundObject::Ptr( 0 )
				)
			)
		)
		.IECOREPYTHON_DEFPARAMETERWRAPPERFNS( TypedObjectParameter<T> )
	;
}

void bindTypedObjectParameter()
{
	bindTypedObjectParameter<Renderable>();
	bindTypedObjectParameter<StateRenderable>();
	bindTypedObjectParameter<AttributeState>();
	bindTypedObjectParameter<Shader>();
	bindTypedObjectParameter<Transform>();
	bindTypedObjectParameter<MatrixMotionTransform>();
	bindTypedObjectParameter<MatrixTransform>();
	bindTypedObjectParameter<VisibleRenderable>();
	bindTypedObjectParameter<Group>();
	bindTypedObjectParameter<MotionPrimitive>();
	bindTypedObjectParameter<Primitive>();
	bindTypedObjectParameter<MeshPrimitive>();
	bindTypedObjectParameter<CurvesPrimitive>();
	bindTypedObjectParameter<PointsPrimitive>();
	bindTypedObjectParameter<CompoundObject>();
	bindTypedObjectParameter<ObjectVector>();
	bindTypedObjectParameter<SmoothSkinningData>();
}

} // namespace IECorePython
