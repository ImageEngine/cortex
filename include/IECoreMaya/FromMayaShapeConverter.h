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

#include "maya/MDagPath.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( Primitive );
	
} // namespace IECore

namespace IECoreMaya
{

IE_CORE_FORWARDDECLARE( FromMayaShapeConverter );

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
		
		//! @name Factory
		/// The functions allow the creation of a specific converter subclass appropriate
		/// to a particular MDagPath instance. Where possible these should be used in
		/// preference to the FromMayaObjectConverter factory functions, as constructing
		/// a converter from an MDagPath provides additional functionality (for instance
		/// correct world space queries).
		/////////////////////////////////////////////////////////////////////////////////
		//@{
		/// Creates a converter which will convert the given object to an IECore::Object
		/// of any relevant type. Returns 0 if no such converter can be found.
		static FromMayaShapeConverterPtr create( const MDagPath &dagPath );
		/// Creates a converter which will convert the given object to an IECore::Object
		/// of the specified type. Returns 0 if no such converter can be found.
		static FromMayaShapeConverterPtr create( const MDagPath &dagPath, IECore::TypeId resultType );
		//@}
		
	protected :
	
		FromMayaShapeConverter( const std::string &name, const std::string &description, const MObject &object );
		/// This form is necessary if people want to get the shape in world space - world space queries only ever
		/// work with an MDagPath.
		FromMayaShapeConverter( const std::string &name, const std::string &description, const MDagPath &dagPath );

		/// Implemented to call doPrimitiveConversion(), and then add on primitive variables specified as dynamic attributes
		/// on the object begin converted. Derived classes need not reimplement this function, but should instead
		/// implement doPrimitiveConversion().
		virtual IECore::ObjectPtr doConversion( const MObject &object, IECore::ConstCompoundObjectPtr operands ) const;
		/// Must be implemented by derived classes to return a Primitive created to represent the specified object.
		virtual IECore::PrimitivePtr doPrimitiveConversion( const MObject &object, IECore::ConstCompoundObjectPtr operands ) const = 0;
		/// Must be implemented by derived classes to return a Primitive created to represent the specified object.
		virtual IECore::PrimitivePtr doPrimitiveConversion( const MDagPath &dagPath, IECore::ConstCompoundObjectPtr operands ) const = 0;

		/// The space in which derived classes should convert the object.
		MSpace::Space space() const;
		

		/// Creating a static instance of one of these (templated on your Converter type)
		/// within your class will register your converter with the factory mechanism.
		template<class T>
		class Description
		{
			public :
				Description( MFn::Type fromType, IECore::TypeId resultType );
				/// fromTypes should be an array terminated by MFn::kInvalid and resultTypes should
				/// be an array terminated by IECore::InvalidTypeId. resultTypes is only provided as an array
				/// so you can register all the subclasses of the actual resultType - ideally this would be done
				/// automatically using as yet unavailable functionality in RunTimeTyped.
				Description( const MFn::Type *fromTypes, const IECore::TypeId *resultTypes );
			private :
				static FromMayaShapeConverterPtr creator( const MDagPath &dagPath );
		};

	private :
	
		void constructCommon(); // does stuff both constructors need.
		void addPrimVars( const MObject &object, IECore::PrimitivePtr primitive ) const;

		MDagPath m_dagPath;

		IECore::IntParameterPtr m_spaceParameter;
		IECore::StringParameterPtr m_primVarAttrPrefixParameter;
		
		typedef FromMayaShapeConverterPtr (*ShapeCreatorFn)( const MDagPath &dagPath );
		typedef std::pair<MFn::Type, IECore::TypeId> ShapeTypes;
		typedef std::map<ShapeTypes, ShapeCreatorFn> ShapeTypesToFnsMap;
		
		static ShapeTypesToFnsMap *shapeTypesToFns();
		static void registerShapeConverter( const MFn::Type fromType, IECore::TypeId resultType, ShapeCreatorFn creator );

};

} // namespace IECoreMaya

#include "IECoreMaya/FromMayaShapeConverter.inl"

#endif // IECOREMAYA_FROMMAYASHAPECONVERTER_H
