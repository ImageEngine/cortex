//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_FRAMELISTPARAMETER_H
#define IE_CORE_FRAMELISTPARAMETER_H

#include "IECore/FrameList.h"
#include "IECore/SimpleTypedParameter.h"

namespace IECore
{

class FrameListParameter : public StringParameter
{
	public:		
	
		IE_CORE_DECLAREOBJECT( FrameListParameter, StringParameter );
		
		FrameListParameter( const std::string &name, const std::string &description, const std::string &defaultValue = std::string(), bool allowEmptyList = true,
			const PresetsContainer &presets = PresetsContainer(), bool presetsOnly = false, ConstCompoundObjectPtr userData = 0 );
		
		FrameListParameter( const std::string &name, const std::string &description, StringDataPtr defaultValue, bool allowEmptyList = true,
			const ObjectPresetsContainer &presets = ObjectPresetsContainer(), bool presetsOnly = false, ConstCompoundObjectPtr userData = 0 );
			
		virtual ~FrameListParameter();
		
		virtual bool valueValid( ConstObjectPtr value, std::string *reason = 0 ) const;
		
		void setFrameListValue( ConstFrameListPtr frameList );		
		FrameListPtr getFrameListValue() const;
		
	protected :
	
		// for io and copying
		FrameListParameter();
		friend class TypeDescription<FrameListParameter>;
	
		bool m_allowEmptyList;
	
	private :
	
		static const unsigned int g_ioVersion;
			
};

IE_CORE_DECLAREPTR( FrameListParameter );
	
} // namespace IECore

#endif // IE_CORE_FRAMELISTPARAMETER_H
