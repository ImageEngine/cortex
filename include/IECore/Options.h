//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, John Haddon. All rights reserved.
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

#ifndef IECORE_OPTIONS_H
#define IECORE_OPTIONS_H

#include "IECore/Export.h"
#include "IECore/PreWorldRenderable.h"

namespace IECore
{

/// A class to manipulate the options of a Renderer.
/// \ingroup renderingGroup
class IECORE_API Options : public PreWorldRenderable
{
	public:

		/// Options is referenced directly rather than copied.
		Options( CompoundDataPtr options );
		Options( const CompoundDataMap &options = CompoundDataMap() );
		~Options() override;

		IE_CORE_DECLAREOBJECT( Options, PreWorldRenderable );

		CompoundDataMap &options();
		const CompoundDataMap &options() const;
		/// This is mostly of use for the binding - the options()
		/// function gives more direct access to the contents of the CompoundData
		/// (it calls readable() or writable() for you).
		CompoundDataPtr optionsData();

		/// Calls renderer->setOptions() for each name, data pair in options()
		void render( Renderer *renderer ) const override;

	private:

		CompoundDataPtr m_options;

		static const unsigned int m_ioVersion;
};

IE_CORE_DECLAREPTR( Options );

} // namespace IECore

#endif // IECORE_OPTIONS_H
