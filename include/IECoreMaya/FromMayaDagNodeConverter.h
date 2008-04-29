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

#ifndef IECOREMAYA_FROMMAYADAGNODECONVERTER_H
#define IECOREMAYA_FROMMAYADAGNODECONVERTER_H

#include "IECoreMaya/FromMayaObjectConverter.h"

#include "maya/MDagPath.h"

namespace IECoreMaya
{

IE_CORE_FORWARDDECLARE( FromMayaDagNodeConverter );

/// The FromMayaShapeConverter class forms an abstract base class for converting
/// maya dag node objects into IECore objects.
class FromMayaDagNodeConverter : public FromMayaObjectConverter
{
	
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( FromMayaDagNodeConverter, FromMayaDagNodeConverterTypeId, FromMayaObjectConverter );
		
		
		//! @name Factory
		/// The functions allow the creation of a specific converter subclass appropriate
		/// to a particular MDagPath instance.
		/////////////////////////////////////////////////////////////////////////////////
		//@{
		/// Creates a converter which will convert the given object to an IECore::Object
		/// of any relevant type. Returns 0 if no such converter can be found.
		static FromMayaDagNodeConverterPtr create( const MDagPath &dagPath );
		/// Creates a converter which will convert the given object to an IECore::Object
		/// of the specified type. Returns 0 if no such converter can be found.
		static FromMayaDagNodeConverterPtr create( const MDagPath &dagPath, IECore::TypeId resultType );
		//@}
		
	protected :
	
		FromMayaDagNodeConverter( const std::string &name, const std::string &description, const MDagPath &dagPath );

		/// No need to override this in derived classes - override the function below instead.
		virtual IECore::ObjectPtr doConversion( const MObject &object, IECore::ConstCompoundObjectPtr operands ) const;
		/// Must be implemented by derived classes. Will only be called if dagPath is valid.
		virtual IECore::ObjectPtr doConversion( const MDagPath &dagPath, IECore::ConstCompoundObjectPtr operands ) const = 0;

		/// Creating a static instance of one of these (templated on your Converter type)
		/// within your class will register your converter with the factory mechanism.
		template<class T>
		class Description
		{
			public :
				/// fromTypes should be an array terminated by MFn::kInvalid and resultTypes should
				/// be an array terminated by IECore::InvalidTypeId. resultTypes is only provided as an array
				/// so you can register all the subclasses of the actual resultType - ideally this would be done
				/// automatically using as yet unavailable functionality in RunTimeTyped.
				Description( const MFn::Type *fromTypes, const IECore::TypeId *resultTypes );
			private :
				static FromMayaDagNodeConverterPtr creator( const MDagPath &dagPath );
		};

	private :

		MDagPath m_dagPath;
		
		typedef FromMayaDagNodeConverterPtr (*CreatorFn)( const MDagPath &dagPath );
		typedef std::pair<MFn::Type, IECore::TypeId> Types;
		typedef std::map<Types, CreatorFn> TypesToFnsMap;
		
		static TypesToFnsMap *typesToFns();
		static void registerConverter( const MFn::Type fromType, IECore::TypeId resultType, CreatorFn creator );

};

} // namespace IECoreMaya

#include "IECoreMaya/FromMayaDagNodeConverter.inl"

#endif // IECOREMAYA_FROMMAYADAGNODECONVERTER_H
