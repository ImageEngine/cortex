//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_COREHOUDINI_FROMHOUDININODECONVERTER_H
#define IE_COREHOUDINI_FROMHOUDININODECONVERTER_H

#include <OP/OP_Node.h>

#include "TypeIds.h"

#include "NodeHandle.h"
#include "FromHoudiniConverter.h"

namespace IECoreHoudini
{

IE_CORE_FORWARDDECLARE( FromHoudiniNodeConverter );

/// The FromHoudiniNodeConverter class forms a base class for
/// all classes able to perform some kind of conversion
/// from a Houdini OP_Node to an IECore datatype.
class FromHoudiniNodeConverter : public FromHoudiniConverter
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( FromHoudiniNodeConverter, FromHoudiniNodeConverterTypeId, IECore::ToCoreConverter );
		
		//! @name Factory
		//////////////////////////////////////////////////////////////////////////////////////////
		//@{
		/// Creates a converter which will convert the given Houdini OP_Node to an IECore datatype.
		/// If resultType is specified then only converters which create objects of that
		/// type will be returned - the default value allows any suitable converter to be
		/// created. If no matching converters exist then returns 0.
		static FromHoudiniNodeConverterPtr create( const OP_Node *node, IECore::TypeId resultType=IECore::InvalidTypeId );
		//@}
	
	protected:

		FromHoudiniNodeConverter( const OP_Node *node, const std::string &description );

		virtual ~FromHoudiniNodeConverter();

		typedef FromHoudiniNodeConverterPtr (*CreatorFn)( const OP_Node *node );

		static void registerConverter( const OP_OpTypeId fromType, IECore::TypeId resultType, bool isDefault, CreatorFn creator );

		/// Creating a static instance of one of these (templated on your Converter type)
		/// within your class will register your converter with the factory mechanism.
		/// To use this mechanism, your class must typedef the OP_Node class it takes as
		/// an initializer to FromType (e.g. "typedef SOP_Node FromType")
		template<class T>
		class Description
		{
			public :
				Description( OP_OpTypeId fromType, IECore::TypeId resultType, bool isDefault = false );
			private :
				static FromHoudiniNodeConverterPtr creator( const OP_Node *node );
		};
		
		/// retrieves the OP_Node held by the converter
		OP_Node *node() const;
			
	private :
		
		// the handle to the OP_Node
		NodeHandle m_handle;
		
		struct Types
		{
			Types( OP_OpTypeId from, IECore::TypeId result );
			OP_OpTypeId fromType;
			IECore::TypeId resultType;
			bool operator < ( const Types &other ) const;
		};

		typedef std::map<Types, CreatorFn> TypesToFnsMap;
		static TypesToFnsMap *typesToFns();

};

}

#include "FromHoudiniNodeConverter.inl"

#endif // IE_COREHOUDINI_FROMHOUDININODECONVERTER_H
