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

#ifndef IE_CORE_OBJECTPARAMETER_H
#define IE_CORE_OBJECTPARAMETER_H

#include "IECore/Export.h"
#include "IECore/Parameter.h"

#include <set>

namespace IECore
{

/// The ObjectParameter class implements a Parameter which validates
/// based on a list of acceptable TypeIds.
class IECORE_API ObjectParameter : public Parameter
{
	public :

		IE_CORE_DECLARERUNTIMETYPED( ObjectParameter, Parameter )

		typedef std::set<TypeId> TypeIdSet;

		ObjectParameter( const std::string &name, const std::string &description, ObjectPtr defaultValue, TypeId type, const PresetsContainer &presets = PresetsContainer(), bool presetsOnly = false,ConstCompoundObjectPtr userData=0 );
		ObjectParameter( const std::string &name, const std::string &description, ObjectPtr defaultValue, const TypeIdSet &types, const PresetsContainer &presets = PresetsContainer(), bool presetsOnly = false, ConstCompoundObjectPtr userData=0 );
		/// types must be an array of TypeIds terminated with InvalidTypeId.
		ObjectParameter( const std::string &name, const std::string &description, ObjectPtr defaultValue, const TypeId *types, const PresetsContainer &presets = PresetsContainer(), bool presetsOnly = false, ConstCompoundObjectPtr userData=0 );

		/// Returns false if isn't an instance of any type in validTypes(), or any type derived from those types
		virtual bool valueValid( const Object *value, std::string *reason = 0 ) const;

		/// Returns a set containing the valid types.
		const TypeIdSet &validTypes() const;

	private :

		TypeIdSet m_validTypes;

};

IE_CORE_DECLAREPTR( ObjectParameter );

} // namespace IECore

#endif // IE_CORE_OBJECTPARAMETER_H
