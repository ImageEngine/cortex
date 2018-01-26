//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2015, Esteban Tovagliari. All rights reserved.
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

#ifndef IECOREAPPLESEED_MOTIONBLOCKHANDLER_H
#define IECOREAPPLESEED_MOTIONBLOCKHANDLER_H

#include "IECoreAppleseed/private/AttributeState.h"
#include "IECoreAppleseed/private/PrimitiveConverter.h"
#include "IECoreAppleseed/private/TransformStack.h"

#include "IECoreScene/Primitive.h"

#include "IECore/Export.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "OpenEXR/ImathMatrix.h"
IECORE_POP_DEFAULT_VISIBILITY

#include "boost/noncopyable.hpp"

#include "renderer/api/scene.h"

#include <set>
#include <string>

namespace IECoreAppleseed
{

/// The MotionBlockHandler class saves the required state
/// between motionBegin / End calls and creates motion
/// blurred appleseed entities when the motion block finishes.
class MotionBlockHandler : boost::noncopyable
{

	public :

		MotionBlockHandler( TransformStack &transformStack,
			PrimitiveConverter &primitiveConverter );

		void setShutterInterval( float openTime, float closeTime );

		bool insideMotionBlock() const;

		void motionBegin( const std::set<float> &times );
		void motionEnd( const AttributeState &attrState,
			renderer::Assembly *mainAssembly );

		void setTransform( const Imath::M44f &m );
		void concatTransform( const Imath::M44f &m );

		void primitive( IECoreScene::PrimitivePtr primitive,
			const std::string &materialName );

	private :

		float m_shutterOpenTime;
		float m_shutterCloseTime;

		enum BlockType
		{
			NoBlock,
			SetTransformBlock,
			ConcatTransformBlock,
			PrimitiveBlock
		};

		BlockType m_blockType;
		std::set<float> m_times;

		// transform
		TransformStack &m_transformStack;
		std::vector<Imath::M44f> m_transforms;

		// primitives
		PrimitiveConverter &m_primitiveConverter;
		std::vector<IECoreScene::PrimitivePtr> m_primitives;
		IECore::TypeId m_primitiveType;
		std::string m_materialName;
};

} // namespace IECoreAppleseed

#endif // IECOREAPPLESEED_MOTIONBLOCKHANDLER_H
