//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORESCENE_CAMERA_H
#define IECORESCENE_CAMERA_H

#include "IECoreScene/Export.h"
#include "IECoreScene/PreWorldRenderable.h"

namespace IECoreScene
{

IE_CORE_FORWARDDECLARE( Transform )

class IECORESCENE_API Camera : public PreWorldRenderable
{
	public:

		Camera( const std::string &name = "default",
			TransformPtr transform = nullptr, IECore::CompoundDataPtr parameters = new IECore::CompoundData );
		~Camera() override;

		IE_CORE_DECLAREEXTENSIONOBJECT( Camera, CameraTypeId, PreWorldRenderable );

		void setName( const std::string &name );
		const std::string &getName() const;

		void setTransform( TransformPtr transform );
		/// May return 0 if no transform has been applied.
		Transform *getTransform();
		const Transform *getTransform() const;

		IECore::CompoundDataMap &parameters();
		const IECore::CompoundDataMap &parameters() const;
		/// This is mostly of use for the binding - the parameters()
		/// function gives more direct access to the contents of the CompoundData
		/// (it calls readable() or writable() for you).
		IECore::CompoundData *parametersData();
		const IECore::CompoundData *parametersData() const;
		/// Adds the standard parameters documented as part of Renderer::camera(),
		/// giving them the appropriate default values as documented there. Note that
		/// this function will only modify existing parameters if they are of the incorrect
		/// datatype or have invalid values - in all other cases the values of missing
		/// parameters will be computed based on the existing parameters.
		void addStandardParameters();

		void render( Renderer *renderer ) const override;

	private:

		std::string m_name;
		TransformPtr m_transform;
		IECore::CompoundDataPtr m_parameters;

		static const unsigned int m_ioVersion;
};

IE_CORE_DECLAREPTR( Camera );

}

#endif // IECORESCENE_CAMERA_H
