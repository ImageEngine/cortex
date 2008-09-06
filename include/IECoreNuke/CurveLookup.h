//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORENUKE_CURVELOOKUP_H
#define IECORENUKE_CURVELOOKUP_H

#if IECORENUKE_NUKE_MAJOR_VERSION >= 5 && IECORENUKE_NUKE_MINOR_VERSION > 0 
#define IECORENUKE_NO_ANIMATION
#endif

#ifdef IECORENUKE_NO_ANIMATION
#include "DDImage/LookupCurves.h"
#else
#include "DDImage/Animation.h"
#endif

#include "DDImage/Knobs.h"

#include <vector>

namespace IECoreNuke
{

/// The CurveLookup class provides a useful wrapper around a bunch of DD::Image::Animation pointers,
/// which can be used to provide lookup curves to a node. It contains everything necessary to declare
/// the interface, sample the curve in _evaluate(), and then interpolate those sample values
/// to perform quick evaluations in engine(). It's templated on the type you want to be returned
/// from the evaluate() method. It also deals with the fact that the API for using curve lookups
/// changed completely between Nuke5 and Nuke5.1.
template<class T>
class CurveLookup
{

	public :
	
		typedef T BaseType;
	
		CurveLookup( const std::string &name, const std::string &label, const std::string &toolTip = "" );
		~CurveLookup();
		/// Call this in the constructor for a node, to add as many curves as required. Returns the index
		/// which should be passed to the validate and evaluate calls below, but this is guaranteed to
		/// be 0 for the first curve, 1 for the second etc. You cannot add more curves once knobs()
		/// has been called.
		int addCurve( const std::string &name, const std::string &defaultCurve = "y C 0 1" );
		/// Call this in the knobs() method for a node, to build the interface for the curves.
		void knob( DD::Image::Knob_Callback f );
		/// Call this in the _validate method for a node, to sample the curves into a lookup
		/// which can be evaluated quickly in engine().
		void validate( T xMin = 0, T xMax = 1, unsigned numSamples=100 );
		/// Append all the curves to the specified Hash.
		void append( DD::Image::Hash &hash ) const;
		/// Append an individual curve to the specified Hash.
		void append( unsigned curveIndex, DD::Image::Hash &hash ) const;
		/// As above, but samples just one curve - this can be used if you wish to sample different
		/// curves over different ranges. curveIndex is 0 for the first curve added, and increments
		/// by 1 for each subsequent curve.
		void validate( unsigned curveIndex, T xMin = 0, T xMax = 1, unsigned numSamples=100 );
		/// Calculate the y value for the specified curve at the specified position.
		/// validate() must have been called for that curve before this method is called.
		inline T evaluate( unsigned curveIndex, T x ) const;

	private :
	
		/// No copying please
		CurveLookup( const CurveLookup<T> &other );
		const CurveLookup<T> &operator=( const CurveLookup &other );
	
		unsigned numCurves() const;
	
		std::string m_name;
		std::string m_label;
		std::string m_toolTip;
		
#ifdef IECORENUKE_NO_ANIMATION	
		std::vector<std::string> *m_namesAndDefaultsStrings;
		std::vector<DD::Image::CurveDescription> *m_curveDescriptions;
		DD::Image::LookupCurves *m_curves;
#else
		std::vector<std::string> m_namesAndDefaultsStrings;
		std::vector<const char *> m_namesAndDefaultsPtrs;
		std::vector<const DD::Image::Animation *> m_curves;
#endif

		struct Lookup
		{
			std::vector<T> values;
			float xMin;
			float xMax;
			float xMult;
		};
		
		std::vector<Lookup>	m_lookups;

};

} // namespace IECoreNuke

#include "IECoreNuke/CurveLookup.inl"

#endif // IECORENUKE_CURVELOOKUP_H
