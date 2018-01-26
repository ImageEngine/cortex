//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORESCENE_MOTIONBLOCK_H
#define IECORESCENE_MOTIONBLOCK_H

#include "IECoreScene/Export.h"
#include "IECoreScene/Renderer.h"

#include "boost/noncopyable.hpp"

namespace IECoreScene
{

/// The MotionBlock class provides a simple means of ensuring that renderer->motionBegin()
/// calls are matched by renderer->motionEnd() calls, even in the face of exceptions and
/// multiple return statements from a function.
/// \ingroup renderingGroup
class IECORESCENE_API MotionBlock : public boost::noncopyable
{
	public :

		/// Starts a new motion block, calling renderer->motionBegin(). If active is false
		/// or times.size() < 2 then nothing is done.
		MotionBlock( RendererPtr renderer, const std::set<float> &times, bool active=true );
		/// Closes the motion block by calling renderer->motionEnd(), unless no block was
		/// created in the constructor for any reason.
		~MotionBlock();

	private :

		RendererPtr m_renderer;

};

} // namespace IECoreScene

#endif // IECORESCENE_MOTIONBLOCK_H
