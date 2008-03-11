//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREGL_TOGLCONVERTER_H
#define IECOREGL_TOGLCONVERTER_H

#include "IECoreGL/TypeIds.h"

#include "IECore/FromCoreConverter.h"
#include "IECore/CompoundObject.h"

namespace IECoreGL
{

/// The ToGLConverter class is a to be used as a base for all classes able to perform
/// some kind of conversion from an IECore datatype to an IECoreGL datatype.
/// \todo Some sort of factory mechanism accepting the desired source and destination types.
class ToGLConverter : public IECore::FromCoreConverter
{

	public :
	
		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( ToGLConverter, ToGLConverterTypeId, IECore::FromCoreConverter );
	
		/// Returns the object in srcParameter() converted to an appropriate IECoreGL
		/// type.
		IECore::RunTimeTypedPtr convert();
		
	protected :
	
		ToGLConverter( const std::string &name, const std::string &description, IECore::TypeId supportedType );
		virtual ~ToGLConverter();

		/// Called by convert() to actually perform the operation.
		/// operands contains the result of parameters()->getValidatedValue() -
		/// this function will never be called when the contents of the parameters
		/// are in a bad state. Must be implemented in derived classes.
		virtual IECore::RunTimeTypedPtr doConversion( IECore::ConstObjectPtr src, IECore::ConstCompoundObjectPtr operands ) const = 0;
			
};

IE_CORE_DECLAREPTR( ToGLConverter );

} // namespace IECoreGL

#endif // IECOREGL_TOGLCONVERTER_H
