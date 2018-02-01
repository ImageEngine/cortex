//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_GEOMETRICTYPEDDATA_H
#define IECORE_GEOMETRICTYPEDDATA_H

#include "IECore/TypedData.h"

namespace IECore
{

namespace GeometricData
{

/// \todo - we now call the default interpretation None
///        It means there is no special geometric interpretation, numeric data is just numeric,
///        but it could also apply to data like strings, which are just interpreted as strings
///        Once we have removed old code using Numeric, we can remove the duplicate declaration
///        as Numeric, and just have None.
enum Interpretation
{
	None = 0,
	Numeric = 0,
	Point,
	Normal,
	Vector,
	Color,
	UV
};

} // namespace GeometricData

/// A templated class which can be used to wrap useful data types and
/// containers that may be interpreted in several ways, conforming to
/// the values in GeometricData::Interpretation. GeometricTypedData
/// extends TypedData by adding get/setInterpretation methods, which
/// may be used to differentiate use cases for individual instances of
/// a given data type. See IECore::TransformOp for an example use case.
///
/// Note that to use instantiations of this class you should utilise the
/// appropriate typedefs as provided by headers such as SimpleTypedData.h
/// or VectorTypedData.h, rather than include this header directly.
/// \ingroup coreGroup
template <class T>
class IECORE_EXPORT GeometricTypedData : public TypedData<T>
{
	public :

		typedef typename TypedData<T>::ValueType ValueType;

		GeometricTypedData();
		GeometricTypedData( const ValueType &data );
		GeometricTypedData( const ValueType &data, GeometricData::Interpretation interpretation );

		IECORE_RUNTIMETYPED_DECLARETEMPLATE( GeometricTypedData<T>, TypedData<T> );

		//! @name Object interface
		////////////////////////////////////////////////////////////
		//@{
		typename GeometricTypedData<T>::Ptr copy() const;
		bool isEqualTo( const Object *other ) const override;
		void hash( MurmurHash &h ) const override;
		//@}

		/// Get/Set Interpretation may be used to distinguish different uses of common data types.
		/// For example, one V3fVectorData may be interpreted as points, another as normals, and
		/// yet another as colors.
		GeometricData::Interpretation getInterpretation() const;
		void setInterpretation( GeometricData::Interpretation interpretation );

	protected :

		~GeometricTypedData() override;

		static Object::TypeDescription<GeometricTypedData<T> > m_typeDescription;

		void copyFrom( const Object *other, Object::CopyContext *context ) override;
		void save( Object::SaveContext *context ) const override;
		void load( Object::LoadContextPtr context ) override;
		void memoryUsage( Object::MemoryAccumulator &accumulator ) const override;

	private :

		GeometricData::Interpretation m_interpretation;

};

/// Macro for the declaration of a new GeometricTypedData instantiation, holding
/// a type T. TYPENAME is the name of the new GeometricTypedData class, T is the
/// held type, BASETYPE is the base type or void if there is no base, and
/// DataHolder is either SimpleDataHolder or ShareableDataHolder. A TypedData<T>
/// named TYPENAME + Base will be declared as the parent type.
/// SimpleDataHolder is appropriate where T is of a small fixed size and
/// SharedDataHolder is appropriate where T is larger and/or varies
/// in size. The difference being that ShareableDataHolder implements
/// a lazy-copy-on-write behaviour useful for large types, but also incurs
/// the cost of an extra allocation and reference count inappropriate
/// for small types.
#define IECORE_DECLARE_GEOMETRICTYPEDDATA( TYPENAME, T, BASETYPE, DATAHOLDER ) \
	IECORE_DECLARE_TYPEDDATA( TYPENAME ## Base, T, BASETYPE, DATAHOLDER ) \
	\
	typedef GeometricTypedData<T> TYPENAME; \
	\
	IE_CORE_DECLAREPTR( TYPENAME ); \

} // namespace IECore

#endif // IECORE_GEOMETRICTYPEDDATA_H
