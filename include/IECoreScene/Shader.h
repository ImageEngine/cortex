//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORESCENE_SHADER_H
#define IECORESCENE_SHADER_H

#include "IECoreScene/Export.h"
#include "IECoreScene/Renderable.h"

namespace IECoreScene
{

/// A class to represent shaders.
/// \ingroup renderingGroup
class IECORESCENE_API Shader : public Renderable
{
	public:

		Shader( const std::string &name="defaultsurface", const std::string &type="surface", const IECore::CompoundDataMap &parameters = IECore::CompoundDataMap() );

		// Special constructor if you already have a CompoundData allocated.  We usually don't expect shaders
		// to share parameter data, so if you use this form you need to be careful about avoiding reuse of this
		// CompoundData
		Shader( const std::string &name, const std::string &type, const IECore::CompoundDataPtr &parametersData );

		~Shader() override;

		IE_CORE_DECLAREEXTENSIONOBJECT( Shader, ShaderTypeId, Renderable );

		const std::string &getName() const;
		void setName( const std::string &name );

		const std::string &getType() const;
		void setType( const std::string &type );

		IECore::CompoundDataMap &parameters();
		const IECore::CompoundDataMap &parameters() const;
		/// This is mostly of use for the binding - the parameters()
		/// function gives more direct access to the contents of the CompoundData
		/// (it calls readable() or writable() for you).
		IECore::CompoundData *parametersData();
		const IECore::CompoundData *parametersData() const;

	private:

		std::string m_name;
		std::string m_type;
		IECore::CompoundDataPtr m_parameters;

		static const unsigned int m_ioVersion;
};

IE_CORE_DECLAREPTR( Shader );

} // namespace IECoreScene


#endif // IECORESCENE_SHADER_H
