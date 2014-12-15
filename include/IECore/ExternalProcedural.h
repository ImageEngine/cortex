//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2014, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_EXTERNALPROCEDURAL_H
#define IECORE_EXTERNALPROCEDURAL_H

#include "IECore/Export.h"
#include "IECore/VisibleRenderable.h"

namespace IECore
{

/// \ingroup renderingGroup
/// \ingroup coreGroup
class IECORE_API ExternalProcedural : public VisibleRenderable
{

	public :

		ExternalProcedural( const std::string &fileName = "", const Imath::Box3f &bound = Imath::Box3f(), const CompoundData *parameters = NULL );
		virtual ~ExternalProcedural();

		IE_CORE_DECLAREOBJECT( ExternalProcedural, VisibleRenderable );

		void setFileName( const std::string &fileName );
		const std::string &getFileName() const;

		void setBound( const Imath::Box3f &bound );
		const Imath::Box3f &getBound() const;

		CompoundData *parameters();
		const CompoundData *parameters() const;

		virtual void render( Renderer *renderer ) const;
		virtual Imath::Box3f bound() const;

	private :

		std::string m_fileName;
		Imath::Box3f m_bound;
		CompoundDataPtr m_parameters;

};

IE_CORE_DECLAREPTR( ExternalProcedural );

} // namespace IECore

#endif // IECORE_EXTERNALPROCEDURAL_H
