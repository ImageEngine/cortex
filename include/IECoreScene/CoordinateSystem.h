//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORESCENE_COORDINATESYSTEM_H
#define IECORESCENE_COORDINATESYSTEM_H

#include "IECoreScene/Export.h"
#include "IECoreScene/StateRenderable.h"

namespace IECoreScene
{

IE_CORE_FORWARDDECLARE( Transform )

/// This calls allows the specification of coordinate systems to
/// Renderers.
/// \ingroup renderingGroup
class IECORESCENE_API CoordinateSystem : public StateRenderable
{
	public:

		CoordinateSystem();
		CoordinateSystem( const std::string &name, TransformPtr transform=nullptr );
		~CoordinateSystem() override;

		IE_CORE_DECLAREEXTENSIONOBJECT( CoordinateSystem, CoordinateSystemTypeId, StateRenderable );

		const std::string &getName() const;
		void setName( const std::string &name );

		/// Returns the Transform applied to the coordinate system.
		/// This is the local transform relative to the parent of
		/// the coordinate system (usually a Group). May return 0
		/// if no transform has been applied.
		Transform *getTransform();
		const Transform *getTransform() const;
		/// Sets the Transform applied to the coordinate system.
		void setTransform( TransformPtr transform );

	private:

		std::string m_name;
		TransformPtr m_transform;

		static const unsigned int m_ioVersion;
};

IE_CORE_DECLAREPTR( CoordinateSystem );

} // namespace IECoreScene

#endif // IECORESCENE_COORDINATESYSTEM_H
