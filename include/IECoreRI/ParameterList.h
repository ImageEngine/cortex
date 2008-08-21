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

#ifndef IECORERI_PARAMETERLIST_H
#define IECORERI_PARAMETERLIST_H

#include "IECore/Renderer.h"

namespace IECoreRI
{

/// This class provides a means of converting from an IECore::CompoundDataMap
/// to the nasty form passable through the Renderman Interface.
class ParameterList
{

	public :

		/// Construct a new ParameterList given some parameters
		/// in a map. Note that the ParameterList refers to data within
		/// the parameters object, and expects that data to exist for as long
		/// as you still use the ParameterList (it doesn't copy the list
		/// for efficiency reasons). If provided, the typeHints map is used
		/// to resolve the type of ambiguous types such as V3fData, which
		/// could represent points, normals or vectors. The typeHints map
		/// simply maps from the name of the primitive variable to the
		/// Renderman type token. It too is expected to exist for as long
		/// as you still use the ParameterList.
		ParameterList( const IECore::CompoundDataMap &parameters,
			const std::map<std::string, std::string> *typeHints = 0 );
		/// As above but uses only parameters beginning with the specified
		/// prefix, and strips the prefix off when creating the parameter names.
		ParameterList( const IECore::CompoundDataMap &parameters, const std::string &prefix,
			const std::map<std::string, std::string> *typeHints = 0 );
		/// Convenience function for constructing a ParameterList of length 1.	
		ParameterList( const std::string &name, IECore::ConstDataPtr parameter,
			const std::map<std::string, std::string> *typeHints = 0 );
		
		/// Returns the number of parameters, to be passed as the
		/// n argument to the Ri call.
		int n();
		/// Returns the tokens to be passed to the Ri call. Note that these
		/// have been cast to a non-const form to fit the Ri interface, but
		/// you must absolutely not modify the data.
		char **tokens();
		/// Returns the values to be passed to the Ri call. Note that these
		/// have been cast to a non-const form to fit the Ri interface, but
		/// you must absolutely not modify the data.
		void **values();
	
	private :
	
		const char *type( const std::string &name, IECore::ConstDataPtr d, size_t &arraySize, const std::map<std::string, std::string> *typeHints );
		const void *value( IECore::ConstDataPtr d );
		
		// Call one of these to reserve enough space in the arrays below to represent all the parameters.
		void reserve( const IECore::CompoundDataMap &parameters );
		void reserve( IECore::ConstDataPtr &parameter );
		void accumulateReservations( const IECore::ConstDataPtr d, size_t &numStrings, size_t &numCharPtrs, size_t &numInts, size_t &numFloats );
		// Then call this n times to populate the member data
		void appendParameter( const std::string &name, IECore::ConstDataPtr d, const std::map<std::string, std::string> *typeHints );
		std::vector<const char *> m_charPtrs;
		std::vector<std::string> m_strings;
		std::vector<int> m_ints;
		std::vector<float> m_floats;
		std::vector<const char *> m_tokens;
		std::vector<const void *> m_values;
	
};

} // namespace IECoreRI

#endif // IECORERI_PARAMETERLIST_H
