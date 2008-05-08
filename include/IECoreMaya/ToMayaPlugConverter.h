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

#ifndef IE_COREMAYA_TOMAYAPLUGCONVERTER_H
#define IE_COREMAYA_TOMAYAPLUGCONVERTER_H

#include "IECoreMaya/ToMayaConverter.h"

#include "IECore/Object.h"

#include "maya/MPlug.h"

namespace IECoreMaya
{

IE_CORE_FORWARDDECLARE( ToMayaPlugConverter );

/// The ToMayaPlugConverter class allows conversion from an IECore Object to MPlug values.
class ToMayaPlugConverter : public ToMayaConverter
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( ToMayaPlugConverter, ToMayaPlugConverterTypeId, ToMayaConverter );

		/// Converts the srcParameter() value to an MPlug value.
		/// \todo Replace this function with one that calls a pure virtual doConversion
		/// function taking the contents of parameters(), like the other converters. Also
		/// consider that MStatus might be a better return type, and that we might also
		/// want a converter to create a new plug rather than just fill an existing one.
		virtual bool convert( MPlug &plug ) const;
		
		/// \todo Implement this as a genuine factory which creates subclasses.
		static ToMayaPlugConverterPtr create( const IECore::ObjectPtr src );
		
	private :
	
		ToMayaPlugConverter( IECore::ConstObjectPtr obj );

};

} // namespace IECoreMaya

#endif // IE_COREMAYA_TOMAYAPLUGCONVERTER_H
