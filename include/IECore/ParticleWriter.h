//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_PARTICLEWRITER_H
#define IE_CORE_PARTICLEWRITER_H

#include "IECore/Writer.h"
#include "IECore/TypedParameter.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( PointsPrimitive );

/// The ParticleWriter class defines an abstract base class
/// for classes able to write particle cache file formats.
/// Its main purpose is to define a standard set of parameters
/// which all ParticleWriters should obey.
class ParticleWriter : public Writer
{

	public :

		IE_CORE_DECLARERUNTIMETYPED( ParticleWriter, Writer )
				
		/// Default implementation just checks that object is a PointsPrimitive instance.
		static bool canWrite( ConstObjectPtr object, const std::string &fileName );
		
	protected :
	
		ParticleWriter( const std::string &name, const std::string &description );
	
		/// ParticleWriters only write objects of the PointsPrimitive type,
		/// so this function returns object() already cast and ready.
		ConstPointsPrimitivePtr particleObject();
	
		/// Convenience functions to access the values held in parameters().
		/// Fills names with the unions of the attributes requested to be
		/// saved and those actually present in the object being saved.
		/// It also omits any attributes with an incorrect number of elements.
		void particleAttributes( std::vector<std::string> &names );
		StringVectorParameterPtr m_attributesParameter;
		/// Returns the number of particles in particleObject()
		size_t particleCount();
		
};

IE_CORE_DECLAREPTR( ParticleWriter );

} // namespace IECore

#endif // IE_CORE_PARTICLEWRITER_H
