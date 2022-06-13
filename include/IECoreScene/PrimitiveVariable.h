//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORESCENE_PRIMITIVEVARIABLE_H
#define IECORESCENE_PRIMITIVEVARIABLE_H

#include "IECoreScene/Export.h"

#include "IECore/VectorTypedData.h"

namespace IECoreScene
{

/// The PrimitiveVariable defines a simple
/// structure to store primitive variables to
/// be used by the Renderer and Primitive classes.
struct IECORESCENE_API PrimitiveVariable
{
	/// The Interpolation enum is used to describe how the
	/// values of a Variable are to be interpolated
	/// across the surface of a Primitive. The types are
	/// essentially those defined in the RenderMan standard.
	enum Interpolation {
		Invalid,
		Constant,
		Uniform,
		Vertex,
		Varying,
		FaceVarying
	};

	/// Constructs a PrimitiveVariable with Interpolation type Invalid
	/// and a null data pointer. This allows the [] operator to work
	/// in the PrimitiveVariableMap, but you must be careful to use it
	/// only for assignment or reading of entries you /know/ exist,
	/// otherwise you're inadvertently populating the map with invalid
	/// PrimitiveVariables. This is a problem with the map [] operator
	/// generally but it's worth noting as it's likely to appear here.
	PrimitiveVariable();
	/// Constructor - Data is not copied but referenced directly.
	PrimitiveVariable( Interpolation i, IECore::DataPtr d );
	/// Constructor - Data is not copied but referenced directly.
	PrimitiveVariable( Interpolation i, IECore::DataPtr d, IECore::IntVectorDataPtr indices );
	/// It's OK to make shallow copies where the data is not copied just rereferenced,
	/// so we can use default copy and assignment
	PrimitiveVariable( const PrimitiveVariable &other ) = default;
	PrimitiveVariable & operator= ( const PrimitiveVariable &rhs ) = default;
	~PrimitiveVariable() = default;

	/// Copy constructor which optionally allows a deep copy of data
	/// to be taken.
	PrimitiveVariable( const PrimitiveVariable &other, bool deepCopy );

	bool operator==( const PrimitiveVariable &other ) const;
	bool operator!=( const PrimitiveVariable &other ) const;

	/// Use expandedData() to expand indices if they exist. If the variable
	/// is not indexed, a direct copy will be returned.
	IECore::DataPtr expandedData() const;

	/// The interpolation type for this PrimitiveVariable.
	Interpolation interpolation;
	/// The Data for this PrimitiveVariable. Unless Interpolation is Constant,
	/// Variable data is expected to be one of the types defined in VectorTypedData.h.
	/// Constant interpolated data can be represented by any type of Data.
	IECore::DataPtr data;
	/// Optional indices array into the primary Data. This can be used
	/// to store a more compact representation of the data, for example
	/// a constant list of strings which are mapped to Vertex, Uniform,
	/// or FaceVarying via the indices. Similarly, UV sets can store an
	/// array of unique UVs in data and map them to FaceVarying using
	/// the indices.
	IECore::IntVectorDataPtr indices;

	template<typename T>
	class IndexedView;

};

/// Utility class for iterating the `data` field from
/// a PrimitiveVariable, using the `indices` field
/// appropriately if it exists.
///
/// > Note : This intentionally only provides `const`
/// > access because it does not make sense to modify
/// > the `data` elements via the `indices`, since
/// > each element will be visited an unknown number of
/// > times.
template<typename T>
class PrimitiveVariable::IndexedView
{

	public :

		/// Uninitialised.
		IndexedView();

		/// Throws if the PrimitiveVariable doesn't contain
		/// `TypedData<vector<T>>`.
		///
		/// > Note : the IndexedView does not own any data.
		/// > It is the caller's responsibility to keep
		/// > `variable` alive for the lifetime for the view.
		IndexedView( const PrimitiveVariable &variable );

		IndexedView( const std::vector<T> &data, const std::vector<int> *indices );

		class Iterator;

		Iterator begin();
		Iterator end();

		typename std::vector<T>::const_reference operator[]( size_t i ) const
		{
			return (*m_data)[index(i)];
		}

		size_t size() const
		{
			return m_indices ? m_indices->size() : m_data->size();
		}

		size_t index( size_t i ) const
		{
			return m_indices ? (*m_indices)[i] : i;
		}

		const std::vector<T> &data() const
		{
			return *m_data;
		}

		const std::vector<int> *indices() const
		{
			return m_indices;
		}

		explicit operator bool() const
		{
			return m_data != nullptr;
		}

		bool isValid() const
		{
			return static_cast< bool >( *this );
		}

	private :

		static const std::vector<T> *data( const PrimitiveVariable &variable );

		const std::vector<T> *m_data;
		const std::vector<int> *m_indices;

};

/// A simple type to hold named PrimitiveVariables.
typedef std::map<std::string, PrimitiveVariable> PrimitiveVariableMap;

} // namespace IECoreScene

#include "IECoreScene/PrimitiveVariable.inl"

#endif // IECORESCENE_PRIMITIVEVARIABLE_H
