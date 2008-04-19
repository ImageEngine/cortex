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

#ifndef IECOREMAYA_FROMMAYASHAPECONVERTER_H
#define IECOREMAYA_FROMMAYASHAPECONVERTER_H

#include "IECoreMaya/FromMayaObjectConverter.h"

#include "IECore/NumericParameter.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( Primitive );
	
} // namespace IECore

namespace IECoreMaya
{

/// The FromMayaShapeConverter class forms an abstract base class for converting
/// maya shape objects into IECore::Primitive objects.
class FromMayaShapeConverter : public FromMayaObjectConverter
{
	
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( FromMayaShapeConverter, FromMayaShapeConverterTypeId, FromMayaObjectConverter );
		
		enum Space
		{
			Object = 0,
			World = 1
		};
		
		IECore::IntParameterPtr spaceParameter();
		IECore::ConstIntParameterPtr spaceParameter() const;
		
		IECore::StringParameterPtr primVarAttrPrefixParameter();
		IECore::ConstStringParameterPtr primVarAttrPrefixParameter() const;
		
	protected :
	
		FromMayaShapeConverter( const std::string &name, const std::string &description, const MObject &object );

		/// Implemented to call doPrimitiveConversion(), and then add on primitive variables specified as dynamic attributes
		/// on the object begin converted. Derived classes need not reimplement this function, but should instead
		/// implement doPrimitiveConversion().
		virtual IECore::ObjectPtr doConversion( const MObject &object, IECore::ConstCompoundObjectPtr operands ) const;
		/// Must be implemented by derived classes to return a Primitive created to represent the specified object.
		virtual IECore::PrimitivePtr doPrimitiveConversion( const MObject &object, IECore::ConstCompoundObjectPtr operands ) const = 0;

		/// The space in which derived classes should convert the object.
		MSpace::Space space() const;
		
	private :
	
		void addPrimVars( const MObject &object, IECore::PrimitivePtr primitive ) const;

		IECore::IntParameterPtr m_spaceParameter;
		IECore::StringParameterPtr m_primVarAttrPrefixParameter;
		
};

IE_CORE_DECLAREPTR( FromMayaShapeConverter );

} // namespace IECoreMaya

#endif // IECOREMAYA_FROMMAYASHAPECONVERTER_H
