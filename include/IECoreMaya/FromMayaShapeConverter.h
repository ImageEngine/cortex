//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2011, Image Engine Design Inc. All rights reserved.
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

namespace IECoreScene
{

IE_CORE_FORWARDDECLARE( Primitive );

} // namespace IECoreScene

namespace IECoreMaya
{

IE_CORE_FORWARDDECLARE( FromMayaShapeConverter );

/// The FromMayaShapeConverter class forms an abstract base class for converting
/// maya shape objects into IECoreScene::Primitive objects. Note that Maya is
/// quite restrictive when it comes to surface variation, so by default only a few
/// PrimitiveVariables are exported: generally P, N, and any uv sets. Users can
/// customize the export to generate extra PrimitiveVarialbes using dynamic attributes
/// on the shape node in Maya. Any attribute names beginning with "iePrimVar" are
/// considered to represent PrimitiveVariables and are converted as such. The
/// interpolation type of the variable is guessed, unless the attribute name begins
/// with iePrimVar_?_, in which case the ? is used to specify interpolation type:
/// 	C for Constant
/// 	U for Uniform
/// 	V for Vertex
/// 	Y for Varying
/// 	F for FaceVarying
class IECOREMAYA_API FromMayaShapeConverter : public FromMayaObjectConverter
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

		//! @name Factory
		/// These functions allow the creation of a specific converter subclass appropriate
		/// to a particular MDagPath instance.
		/////////////////////////////////////////////////////////////////////////////////
		//@{
		/// Creates a converter which will convert the given object to an IECore::Object
		/// of the type specified by resultType - the default value specifies that any result
		/// will do. Returns 0 if no suitable converter can be found. Where possible this should be used in
		/// preference to the FromMayaObjectConverter factory function, as constructing
		/// a converter from an MDagPath provides additional functionality (for instance
		/// correct world space queries).
		static FromMayaShapeConverterPtr create( const MDagPath &dagPath, IECore::TypeId resultType=IECore::InvalidTypeId );
		//@}

	protected :

		FromMayaShapeConverter( const std::string &description, const MObject &object );
		/// This form is necessary if people want to get the shape in world space - world space queries only ever
		/// work with an MDagPath.
		FromMayaShapeConverter( const std::string &description, const MDagPath &dagPath );

		/// Implemented to call doPrimitiveConversion(), and then add on primitive variables specified as dynamic attributes
		/// on the object begin converted. Derived classes need not reimplement this function, but should instead
		/// implement doPrimitiveConversion().
		IECore::ObjectPtr doConversion( const MObject &object, IECore::ConstCompoundObjectPtr operands ) const override;
		/// Must be implemented by derived classes to return a Primitive created to represent the specified object.
		virtual IECoreScene::PrimitivePtr doPrimitiveConversion( const MObject &object, IECore::ConstCompoundObjectPtr operands ) const = 0;
		/// Must be implemented by derived classes to return a Primitive created to represent the specified object.
		virtual IECoreScene::PrimitivePtr doPrimitiveConversion( const MDagPath &dagPath, IECore::ConstCompoundObjectPtr operands ) const = 0;

		/// The space in which derived classes should convert the object.
		MSpace::Space space() const;
		/// Returns the dag path for the shape to be converted. This may return false, in which case object() should
		/// be converted instead. Generally derived classes shouldn't need this method as they can just implement
		/// the two doPrimitiveConversion() methods above, and this class will call the appropriate one. If emitSpaceWarnings
		/// is true, then a warning is emitted if there is no valid dag path available and the space parameter is set to world.
		const MDagPath *dagPath( bool emitSpaceWarnings=false ) const;

		/// Creating a static instance of one of these (templated on your Converter type)
		/// within your class will register your converter with the factory mechanism.
		template<class T>
		class Description
		{
			public :
				/// \param fromType The maya type which can be converted.
				/// \param resultType The cortex type which will result from the conversion.
				/// \param defaultConversion Should be true if this conversion is the "best" for a given fromType. If
				/// this is true then this is the converter that will be used when create() is called without specifying
				/// a resultType.
				Description( MFn::Type fromType, IECore::TypeId resultType, bool defaultConversion );
			private :
				static FromMayaShapeConverterPtr creator( const MDagPath &dagPath );
		};

	private :

		void constructCommon(); // does stuff both constructors need.
		void addPrimVars( const MObject &object, IECoreScene::PrimitivePtr primitive ) const;

		MDagPath m_dagPath;

		IECore::IntParameterPtr m_spaceParameter;

		typedef FromMayaShapeConverterPtr (*ShapeCreatorFn)( const MDagPath &dagPath );
		typedef std::pair<MFn::Type, IECore::TypeId> ShapeTypes;
		typedef std::map<ShapeTypes, ShapeCreatorFn> ShapeTypesToFnsMap;
		typedef std::map<MFn::Type, ShapeTypesToFnsMap::const_iterator> DefaultConvertersMap;

		static ShapeTypesToFnsMap &shapeTypesToFns();
		static DefaultConvertersMap &defaultConverters();
		static void registerShapeConverter( const MFn::Type fromType, IECore::TypeId resultType, bool defaultConverter, ShapeCreatorFn creator );

};

} // namespace IECoreMaya

#include "IECoreMaya/FromMayaShapeConverter.inl"

#endif // IECOREMAYA_FROMMAYASHAPECONVERTER_H
