//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_COREMAYA_MOBJECTBINDING_H
#define IE_COREMAYA_MOBJECTBINDING_H

#include "maya/MObjectHandle.h"

#include "IECoreMaya/FromMayaConverter.h"

namespace IECoreMaya
{

class MObjectWrapper
{
	public :
	
		MObjectWrapper( const MObject &object );
		/// Throws an exception if no maya object of that name
		/// exists.
		MObjectWrapper( const char *name );
		
		/// Always throws an exception if the object
		/// is not alive. Also throws an exception if the
		/// object is not valid (ie alive but in the undo
		/// queue) and throwIfNotValid is true.
		MObject object( bool throwIfNotValid=true );
		/// Returns a reference to the internal MObjectHandle.
		const MObjectHandle &objectHandle();
	
		/// Utility function returning FromMayaObjectConverter::create( object() )
		FromMayaConverterPtr converter();
		/// Utility function returning FromMayaObjectConverter::create( object(), resultType )
		FromMayaConverterPtr converter( IECore::TypeId resultType );
		/// Utility function returning converter()->convert(), assuming an appropriate converter
		/// is found.
		IECore::ObjectPtr convert();
		/// Utility function returning converter( resultType )->convert(), assuming an appropriate converter
		/// is found.
		IECore::ObjectPtr convert( IECore::TypeId resultType );
	
	private :
	
		MObjectHandle m_objectHandle;
		
};

void bindMObject();

}

#endif //  IE_COREMAYA_MOBJECTBINDING_H
