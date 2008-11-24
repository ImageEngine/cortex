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

#ifndef IE_CORE_TYPEDPRIMITIVEOP_H
#define IE_CORE_TYPEDPRIMITIVEOP_H

#include "IECore/PrimitiveOp.h"
#include "IECore/MeshPrimitive.h"
#include "IECore/ImagePrimitive.h"

namespace IECore
{

/// The TypedPrimitiveOp template defines a base class for Ops which modify primitives of a specific type.
template<typename T>
class TypedPrimitiveOp : public ModifyOp
{
	public :
	
		IE_CORE_DECLAREMEMBERPTR( TypedPrimitiveOp<T> )
		
		typedef T PrimitiveType;		
		
		TypedPrimitiveOp( const std::string name, const std::string description );
		virtual ~TypedPrimitiveOp();
		
		//! @name RunTimeTyped functions
		////////////////////////////////////
		//@{
		virtual TypeId typeId() const;
		virtual std::string typeName() const;
		virtual bool isInstanceOf( TypeId typeId ) const;
		virtual bool isInstanceOf( const std::string &typeName ) const;
		static TypeId staticTypeId();
		static std::string staticTypeName();
		static bool inheritsFrom( TypeId typeId );
		static bool inheritsFrom( const std::string &typeName );
		//@}
		
	protected :
		
		/// Must be implemented by all subclasses.
		virtual void modifyTypedPrimitive( typename T::Ptr typedPrimitive, ConstCompoundObjectPtr operands ) = 0;
		
	private :
	
		/// Implemented to call modifyTypedPrimitive
		void modify( ObjectPtr primitive, ConstCompoundObjectPtr operands );
	
};

#define IE_CORE_DEFINETYPEDPRIMITIVEOP( TNAME ) \
	typedef TypedPrimitiveOp<TNAME> (TNAME ## Op); \
	typedef TypedPrimitiveOp<TNAME>::Ptr (TNAME ## OpPtr); \
	typedef TypedPrimitiveOp<TNAME>::ConstPtr (Const ## TNAME ## OpPtr);
	
IE_CORE_DEFINETYPEDPRIMITIVEOP( MeshPrimitive )	
IE_CORE_DEFINETYPEDPRIMITIVEOP( ImagePrimitive )	

} // namespace IECore

#endif // IE_CORE_TYPEDPRIMITIVEOP_H
