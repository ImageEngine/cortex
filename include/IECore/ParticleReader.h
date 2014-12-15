//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_PARTICLEREADER_H
#define IE_CORE_PARTICLEREADER_H

#include "IECore/Export.h"
#include "IECore/Reader.h"
#include "IECore/SimpleTypedParameter.h"
#include "IECore/VectorTypedParameter.h"
#include "IECore/NumericParameter.h"

namespace IECore
{

/// The ParticleReader class defines an abstract base class
/// for classes able to read particle cache file formats.
/// Its main purpose is to define a standard set of parameters
/// which all ParticleReaders should obey.
/// \ingroup ioGroup
class IECORE_API ParticleReader : public Reader
{

	public :

		IE_CORE_DECLARERUNTIMETYPED( ParticleReader, Reader );

		ParticleReader( const std::string &description );

		/// An enum for the values accepted by realTypeParameter().
		enum RealType
		{
			Native = 0,
			Float = 1,
			Double = 2,
		};

		//! @name Parameter accessors
		/// These provide more convenient access to parameters than
		/// by searching the parameters() structure by hand.
		/////////////////////////////////////////////////////////
		//@{
		FloatParameter * percentageParameter();
		const FloatParameter * percentageParameter() const;
		IntParameter * percentageSeedParameter();
		const IntParameter * percentageSeedParameter() const;
		StringVectorParameter * attributesParameter();
		const StringVectorParameter * attributesParameter() const;
		IntParameter * realTypeParameter();
		const IntParameter * realTypeParameter() const;
		BoolParameter * convertPrimVarNamesParameter();
		const BoolParameter * convertPrimVarNamesParameter() const;
		//@}

		//! @name Particle specific reading functions.
		/// These allow more controlled reading than the read()
		/// method alone. These functions are still affected
		/// by the parameters() settings, so that things like
		/// percentage filtering are still available.
		/////////////////////////////////////////////////////////
		//@{
		/// Returns the number of particles in the file. This returns
		/// the absolute total, rather than one affected by the percentage
		/// parameter.
		virtual unsigned long numParticles() = 0;
		/// Fills the passed vector with the names of all attributes
		/// in the file.
		virtual void attributeNames( std::vector<std::string> &names ) = 0;
		/// Reads the specified attribute, filtered by the percentage specified
		/// in parameters(). Returns 0 if the attribute doesn't
		/// exist. The type of Data is chosen automatically to best represent the
		/// particle data.
		virtual DataPtr readAttribute( const std::string &name ) = 0;
		//@}

	protected :

		/// Returns a PointsPrimitive object containing all the
		/// attributes requested filtered by the percentage
		/// requested. This is implemented using the virtual methods
		/// defined above - there is no need to reimplement it
		/// in derived classes.
		virtual ObjectPtr doOperation( const CompoundObject * operands );

		/// Convenience functions to access the values held in parameters().
		/// If called from within doOperation they will never throw, but if
		/// called at any other time they may, due to invalid values in the
		/// corresponding Parameter.
		float particlePercentage() const;
		FloatParameterPtr m_percentageParameter;
		int particlePercentageSeed() const;
		IntParameterPtr m_percentageSeedParameter;
		void particleAttributes( std::vector<std::string> &names );
		StringVectorParameterPtr m_attributesParameter;
		RealType realType() const;
		IntParameterPtr m_realTypeParameter;
		bool convertPrimVarNames() const;
		BoolParameterPtr m_convertPrimVarNamesParameter;

		/// Convenience function to filter prim vars at a given percentage.
		/// The filtering is based on the particle id or based on the 
		/// particle index if no id is provided.
		template<typename T, typename F>
		typename T::Ptr filterAttr( const F * attr, float percentage, const Data *idAttr ) const;
		
		/// Returns the name of the original position primVar should we need to convert it to "P"
		virtual std::string positionPrimVarName() = 0;

	private :

		template<typename T, typename F, typename U >
		typename T::Ptr filterAttr( const F * attr, float percentage, const std::vector< U > &ids ) const;


};

IE_CORE_DECLAREPTR( ParticleReader );

} // namespace IECore

#endif // IE_CORE_PARTICLEREADER_H
