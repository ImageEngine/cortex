//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_CAMERA_H
#define IECORE_CAMERA_H

#include "IECore/PreWorldRenderable.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( Transform )

class Camera : public PreWorldRenderable
{
	public:
	
		Camera( const std::string &name = "default",
			TransformPtr transform = 0, CompoundDataPtr parameters = new CompoundData );
		virtual ~Camera();
				
		IE_CORE_DECLAREOBJECT( Camera, PreWorldRenderable );

		void setName( const std::string &name );
		const std::string &getName() const;

		void setTransform( TransformPtr transform );
		/// May return 0 if no transform has been applied.
		TransformPtr getTransform();
		ConstTransformPtr getTransform() const;
		
		CompoundDataMap &parameters();
		const CompoundDataMap &parameters() const;
		/// This is mostly of use for the binding - the parameters()
		/// function gives more direct access to the contents of the CompoundData
		/// (it calls readable() or writable() for you).
		CompoundDataPtr parametersData();

		virtual void render( RendererPtr renderer );
			
	private:
	
		std::string m_name;
		TransformPtr m_transform;
		CompoundDataPtr m_parameters;
	
		static const unsigned int m_ioVersion;
};

IE_CORE_DECLAREPTR( Camera );

}

#endif // IECORE_CAMERA_H
