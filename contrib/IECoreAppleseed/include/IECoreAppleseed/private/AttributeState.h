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

#ifndef IECOREAPPLESEED_ATTRIBUTESTATE_H
#define IECOREAPPLESEED_ATTRIBUTESTATE_H

#include "IECoreAppleseed/private/ShadingState.h"

#include "IECore/CompoundData.h"

#include "renderer/api/utility.h"

namespace IECoreAppleseed
{

class AttributeState
{

	public :

		AttributeState();
		AttributeState( const AttributeState &other );

		IECore::ConstDataPtr getAttribute( const std::string &name ) const;
		void setAttribute( const std::string &name, IECore::ConstDataPtr value );

		const std::string &name() const;

		const foundation::Dictionary &visibilityDictionary() const;

		const std::string &alphaMap() const;

		bool photonTarget() const;

		int mediumPriority() const;

		void attributesHash( IECore::MurmurHash &hash ) const;

		void addOSLShader( IECoreScene::ConstShaderPtr shader );
		void setOSLSurface( IECoreScene::ConstShaderPtr surface );

		bool shadingStateValid() const;

		void shaderGroupHash( IECore::MurmurHash &hash ) const;
		void materialHash( IECore::MurmurHash &hash ) const;

		std::string createShaderGroup( renderer::Assembly &assembly );
		void editShaderGroup( renderer::Assembly &assembly, const std::string &name );

		std::string createMaterial( renderer::Assembly &assembly, const std::string &shaderGroupName );

	private :

		IECore::CompoundDataPtr m_attributes;
		ShadingState m_shadingState;
		std::string m_name;
		std::string m_alphaMap;
		bool m_photonTarget;
		foundation::Dictionary m_visibilityDictionary;
		int m_mediumPriority;

};

} // namespace IECoreAppleseed

#endif // IECOREAPPLESEED_ATTRIBUTESTATE_H
