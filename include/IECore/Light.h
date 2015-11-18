//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_LIGHT_H
#define IE_CORE_LIGHT_H

#include "IECore/Export.h"
#include "IECore/StateRenderable.h"

namespace IECore
{

/// A class to represent lights.
/// \ingroup renderingGroup
class IECORE_API Light : public StateRenderable
{
	public:

		Light( const std::string &name="distantlight", const std::string &handle="", const CompoundDataMap &parameters = CompoundDataMap() );
		virtual ~Light();

		IE_CORE_DECLAREABSTRACTOBJECT( Light, StateRenderable );

		const std::string &getName() const;
		void setName( const std::string &name );

		const std::string &getHandle() const;
		void setHandle( const std::string &handle );

		CompoundDataMap &parameters();
		const CompoundDataMap &parameters() const;
		/// This is mostly of use for the binding - the parameters()
		/// function gives more direct access to the contents of the CompoundData
		/// (it calls readable() or writable() for you).
		/// \todo: Should return raw pointer to CompoundData, and we should probably remove the
		/// parameters() methods since the helper methods on ParametersData make it more useful
		CompoundDataPtr parametersData();
		const CompoundDataPtr parametersData() const;

		/// Sets this light in Renderer.
		virtual void render( Renderer *renderer ) const;

	private:

		static TypeDescription<Light> m_description;

		std::string m_name;
		std::string m_handle;
		CompoundDataPtr m_parameters;

		static const unsigned int m_ioVersion;
};

IE_CORE_DECLAREPTR( Light );

} // namespace IECore


#endif // IE_CORE_LIGHT_H
