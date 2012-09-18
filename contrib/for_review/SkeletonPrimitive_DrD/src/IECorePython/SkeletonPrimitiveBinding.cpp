//////////////////////////////////////////////////////////////////////////
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

#include "IECore/SkeletonPrimitive.h"
#include "IECorePython/SkeletonPrimitiveBinding.h"
#include "IECorePython/RunTimeTypedBinding.h"

using namespace boost::python;
using namespace IECore;

namespace IECorePython
{
	void bindSkeletonPrimitive()
	{
		RunTimeTypedClass<SkeletonPrimitive> skeletonPrimitiveWrap = RunTimeTypedClass<SkeletonPrimitive>();

		scope skeletonPrimitiveScope(skeletonPrimitiveWrap);

			enum_<SkeletonPrimitive::Space>("Space")
				.value("Local", SkeletonPrimitive::Local)
				.value("Reference", SkeletonPrimitive::Reference)
				.value("World", SkeletonPrimitive::World)
				;

		skeletonPrimitiveWrap
			.def( init<>() )
			.def( init< ConstSkeletonPrimitivePtr >() )
			.def( init< ConstM44fVectorDataPtr, ConstIntVectorDataPtr, SkeletonPrimitive::Space >() )

			.def( "isEqualTo", &SkeletonPrimitive::isEqualTo )
			.def( "bound", &SkeletonPrimitive::bound )
			.def( "variableSize", &SkeletonPrimitive::variableSize )

			.def( "setAsCopyOf", &SkeletonPrimitive::setAsCopyOf )
			.def( "isSimilarTo", &SkeletonPrimitive::isSimilarTo )

			.def( "addJoint", make_function(&SkeletonPrimitive::addJoint, default_call_policies(), (arg("parentId"), arg("name")="joint")) )
			.def( "numJoints", &SkeletonPrimitive::numJoints )

			.def( "setJointPoses", make_function(&SkeletonPrimitive::setJointPoses, default_call_policies(), (arg("poses"), arg("space")=SkeletonPrimitive::World) ) )
			.def( "setJointPose", make_function(&SkeletonPrimitive::setJointPose, default_call_policies(), (arg("jointId"), arg("pose"), arg("space")=SkeletonPrimitive::World) ) )

			.def( "getJointPoses", make_function(&SkeletonPrimitive::getJointPoses, default_call_policies(), arg("space")=SkeletonPrimitive::World) )
			.def( "getJointPose", make_function(&SkeletonPrimitive::getJointPose, default_call_policies(), (arg("jointId"), arg("space")=SkeletonPrimitive::World) ) )

			.def( "getParentIds", &SkeletonPrimitive::getParentIds )
			.def( "getParentId", &SkeletonPrimitive::getParentId )
			.def( "getChildrenIds", &SkeletonPrimitive::getChildrenIds )

			.def( "setJointNames", &SkeletonPrimitive::setJointNames )
			.def( "setJointName", &SkeletonPrimitive::setJointName )
			.def( "getJointNames", &SkeletonPrimitive::getJointNames )
			.def( "getJointName", &SkeletonPrimitive::getJointName )

			.def( "shareStaticData", &SkeletonPrimitive::shareStaticData )
			.def( "shareAnimatableData", &SkeletonPrimitive::shareAnimatableData )

			.def( "pullUpdate", &SkeletonPrimitive::pullUpdate )
			.def( "pushUpdate", &SkeletonPrimitive::pushUpdate )
			.def( "getRootJointId", make_function(&SkeletonPrimitive::getRootJointId, default_call_policies(), arg("fromId")=0) )
			.def( "update", &SkeletonPrimitive::update )

			.def( "setRadius", &SkeletonPrimitive::setRadius )
			.def( "setDebug", &SkeletonPrimitive::setDebug )
			.def( "getRadius", &SkeletonPrimitive::getRadius )
			.def( "getDebug", &SkeletonPrimitive::getDebug )

			.def( "createHuman", &SkeletonPrimitive::createHuman  ).staticmethod( "createHuman" )
		;
	}

}
