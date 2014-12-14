//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_PRIMITIVE_H
#define IE_CORE_PRIMITIVE_H

#include "IECore/Export.h"
#include "IECore/VisibleRenderable.h"
#include "IECore/PrimitiveVariable.h"

namespace IECore
{

/// The Primitive class defines an abstract base for Renderable
/// primitives. Primitives are expected to be objects which are
/// visible in final rendered images rather than Renderables which
/// just change some part of the renderer state (such as an attribute).
/// Primitives may hold "primitive variables" which are simply values
/// which vary over the surface of the Primitive and can be used by
/// the renderer to define various aspects of its appearance.
/// \todo Validation of variableSizes before rendering.
/// \ingroup geometryGroup
class IECORE_API Primitive : public VisibleRenderable
{
	public:

		Primitive();
		virtual ~Primitive();

		IE_CORE_DECLAREABSTRACTOBJECT( Primitive, VisibleRenderable );

		/// Variables a stored as a public map for easy manipulation.
		PrimitiveVariableMap variables;
		
		/// Convenience function to find name in variables, and returning a runTimeCast to the requested type. If requiredInterpolation is
		/// specified then 0 is returned if the interpolation doesn't match.
		template<typename T>
		T *variableData( const std::string &name, PrimitiveVariable::Interpolation requiredInterpolation=PrimitiveVariable::Invalid );
		template<typename T>
		const T *variableData( const std::string &name, PrimitiveVariable::Interpolation requiredInterpolation=PrimitiveVariable::Invalid ) const;

		/// Returns true if the given primitive variable has the correct size for its interpolation type
		bool isPrimitiveVariableValid( const PrimitiveVariable &pv ) const;

		/// Returns true if all primitive variables have the correct size for their interpolation type
		bool arePrimitiveVariablesValid() const;

		/// Guesses a suitable interpolation type for a PrimitiveVariable containing
		/// the specified number of data elements. Returns PrimitiveVariable::Invalid
		/// if no such interpolation exists. Note that for a given size multiple
		/// interpolation types may well be valid, so this method may not always give
		/// the desired results. In the case of multiple suitable types, interpolations
		/// are given the following priority (highest first) :
		///
		/// Constant
		/// Uniform
		/// Vertex
		/// Varying
		/// FaceVarying
		PrimitiveVariable::Interpolation inferInterpolation( size_t numElements ) const;
		/// Convenience function which finds the size of data and calls the above
		/// method.
		PrimitiveVariable::Interpolation inferInterpolation( const Data *data ) const;

		/// Implemented to return a box containing all the points in the variable
		/// "P" if it exists.
		virtual Imath::Box3f bound() const;

		/// Returns the number of values a piece of data must provide for the given
		/// interpolation type. Must be implemented in all derived classes.
		virtual size_t variableSize( PrimitiveVariable::Interpolation interpolation ) const = 0;
		
		/// Hash representing the topology only
		virtual void topologyHash( MurmurHash &h ) const = 0;

		/// Utility function that can be used in place of Object::load() to load only the primitive variables from a Primitive object stored in a IndexedIO file.
		/// The function tries to load the requested primitive variables and will ignore the ones that do not exist in the file.
		/// \param ioInterface File handle where the Primitive is stored.
		/// \param name Name of the entry where the Primitive is stored under the file location.
		/// \param primVarNames List of primitive variable names that will be attempted to be loaded.
		static PrimitiveVariableMap loadPrimitiveVariables( const IndexedIO *ioInterface, const IndexedIO::EntryID &name, const IndexedIO::EntryIDList &primVarNames );

	private:

		static const unsigned int m_ioVersion;

};

IE_CORE_DECLAREPTR( Primitive );

} // namespace IECore

#include "IECore/Primitive.inl"

#endif // IE_CORE_PRIMITIVE_H
