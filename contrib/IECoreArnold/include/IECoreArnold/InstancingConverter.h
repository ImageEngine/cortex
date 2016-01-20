//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREARNOLD_INSTANCINGCONVERTER_H
#define IECOREARNOLD_INSTANCINGCONVERTER_H

#include "ai.h"

#include "IECore/Primitive.h"

#include "IECoreArnold/Export.h"

namespace IECoreArnold
{

/// A class for managing the conversion of a series of IECore::Primitives to
/// AtNodes, automatically returning ginstances when a previously converted
/// primitive is processed again.
class IECOREARNOLD_API InstancingConverter : public IECore::RefCounted
{

	public :

		IE_CORE_DECLAREMEMBERPTR( InstancingConverter );

		/// Constructs a new converter. The converter expects that any
		/// AtNodes it creates will remain alive for the lifetime of the
		/// InstancingConverter itself - it is the responsibility of the
		/// caller to ensure that this is the case. This means that the
		/// InstancingConverter lifespan should be contained within a
		/// UniverseBlock.
		InstancingConverter();
		virtual ~InstancingConverter();

		/// Returns the Primitive converted to an appropriate AtNode type, returning
		/// a ginstance if an identical primitive has already been processed. This version
		/// of the function simply calls the version below using primitive->hash()
		/// for the hash.
		AtNode *convert( const IECore::Primitive *primitive );
		/// As above, but allowing the user to explicitly pass a hash uniquely
		/// identifying the primitive. This is useful for systems which intend to
		/// manipulate the primary (non-instanced) AtNodes which are returned - they
		/// can append to the primitive hash to represent the post-processing they intend
		/// to do.
		AtNode *convert( const IECore::Primitive *primitive, const IECore::MurmurHash &hash );

	private :

		struct MemberData;
		MemberData *m_data;

};

IE_CORE_DECLAREPTR( InstancingConverter );

} // namespace IECoreArnold

#endif // IECOREARNOLD_INSTANCINGCONVERTER_H
