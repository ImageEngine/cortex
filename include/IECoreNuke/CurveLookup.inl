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

#ifndef IECORENUKE_CURVELOOKUP_INL
#define IECORENUKE_CURVELOOKUP_INL

#include "OpenEXR/ImathFun.h"

#include <vector>
#include <cassert>

namespace IECoreNuke
{

template<class T>
CurveLookup<T>::CurveLookup( const std::string &name, const std::string &label, const std::string &toolTip )
	:	m_name( name ), m_label( label ), m_toolTip( toolTip )
{
#ifdef IECORENUKE_NO_ANIMATION
	m_namesAndDefaultsStrings = new std::vector<std::string>;
	m_curveDescriptions = new std::vector<DD::Image::CurveDescription>;
	m_curves = 0;
#endif
}

template<class T>
CurveLookup<T>::~CurveLookup()
{
#ifdef IECORENUKE_NO_ANIMATION
	delete m_curves;
	// Note that we're not deliberately not deleting m_namesAndDefaultsStrings
	// or m_curveDescriptions - the nuke docs say that they have to remain forever, so
	// we're effectively forced into leaking memory here.
#endif
}

template<class T>
CurveLookup<T>::CurveLookup( const CurveLookup &other )
{
	assert( 0 );
}

template<class T>
const CurveLookup<T> &CurveLookup<T>::operator=( const CurveLookup<T> &other )
{
	assert( 0 );
	return *this;
}

template<class T>
int CurveLookup<T>::addCurve( const std::string &name, const std::string &defaultCurve )
{
#ifdef IECORENUKE_NO_ANIMATION
	m_namesAndDefaultsStrings->push_back( name );
	m_namesAndDefaultsStrings->push_back( defaultCurve );
	return m_namesAndDefaultsStrings->size()/2 - 1;
#else
	m_namesAndDefaultsStrings.push_back( name );
	m_namesAndDefaultsStrings.push_back( defaultCurve );
	return m_namesAndDefaultsStrings.size()/2 - 1;
#endif
}

template<class T>
void CurveLookup<T>::knob( DD::Image::Knob_Callback f )
{
#ifdef IECORENUKE_NO_ANIMATION
	if( !m_curves )
	{
		for( unsigned i=0; i<m_namesAndDefaultsStrings->size(); i+=2 )
		{
			DD::Image::CurveDescription d;
			d.name = (*m_namesAndDefaultsStrings)[i].c_str();
			d.defaultValue = (*m_namesAndDefaultsStrings)[i+1].c_str();
			m_curveDescriptions->push_back( d );
		}
		DD::Image::CurveDescription endMarker = { 0 };
		m_curveDescriptions->push_back( endMarker );
		m_curves = new DD::Image::LookupCurves( &*(m_curveDescriptions->begin()) );
	}
	LookupCurves_knob( f, m_curves, m_name.c_str(), m_label.c_str() ); 	
#else
	if( !m_namesAndDefaultsPtrs.size() )
	{
		for( std::vector<std::string>::const_iterator it=m_namesAndDefaultsStrings.begin(); it!=m_namesAndDefaultsStrings.end(); it++ )
		{
			m_namesAndDefaultsPtrs.push_back( it->c_str() );
		}
		m_curves.resize( m_namesAndDefaultsPtrs.size()/2 );
		m_namesAndDefaultsPtrs.push_back( 0 );
		m_namesAndDefaultsPtrs.push_back( 0 );
	}
	Animation_knob( f, &(m_curves[0]), &(m_namesAndDefaultsPtrs[0]), m_name.c_str(), m_label.c_str() );
#endif
	Tooltip( f, m_toolTip.c_str() );
}

template<class T>
unsigned CurveLookup<T>::numCurves() const
{
#ifdef IECORENUKE_NO_ANIMATION
	return m_curves->size();
#else
	return m_curves.size();
#endif
}

template<class T>
void CurveLookup<T>::validate( T xMin, T xMax, unsigned numSamples )
{
	for( unsigned i=0; i<numCurves(); i++ )
	{
		validate( i, xMin, xMax, numSamples );
	}
}
		
template<class T>
void CurveLookup<T>::validate( unsigned curveIndex, T xMin, T xMax, unsigned numSamples )
{
	assert( numSamples>=2 );
	assert( curveIndex<numCurves() );
	m_lookups.resize( numCurves() );
	
	Lookup &lookup = m_lookups[curveIndex];
	lookup.values.resize( numSamples );
	T xStep = (xMax - xMin) / (numSamples-1);
	T x = xMin;
	for( unsigned i=0; i<numSamples; i++ )
	{
#ifdef IECORENUKE_NO_ANIMATION
		lookup.values[i] = m_curves->getValue( curveIndex, x );
#else	
		lookup.values[i] = m_curves[curveIndex]->evaluate( x );
#endif
		x += xStep;
	}
	lookup.xMin = xMin;
	lookup.xMax = xMax;
	lookup.xMult = (numSamples - 1) / (xMax - xMin);
}

template<class T>
void CurveLookup<T>::append( DD::Image::Hash &hash ) const
{
#ifdef IECORENUKE_NO_ANIMATION
	m_curves->append( hash );
#else
	for( unsigned i=0; i<m_curves.size(); i++ )
	{
		append( i, hash );
	}
#endif
}

template<class T>
void CurveLookup<T>::append( unsigned curveIndex, DD::Image::Hash &hash ) const
{
#ifdef IECORENUKE_NO_ANIMATION
	// we can't do them one by one with the new nuke version.
	m_curves->append( hash );
#else
	m_curves[curveIndex]->append( hash );
#endif
}

template<class T>
inline T CurveLookup<T>::evaluate( unsigned curveIndex, T x ) const
{
	const Lookup &lookup = m_lookups[curveIndex];
	x = Imath::clamp( x, lookup.xMin, lookup.xMax );
	T f = (x - lookup.xMin) * lookup.xMult;
	int fi = fast_floor( f );
	T ff;
	if( fi<0 )
	{
		ff = f;
		fi = 0;
	}
	else if( fi>(int)lookup.values.size()-2)
	{
		fi = lookup.values.size()-2;
		ff = f - fi;
	}
	else
	{
		 ff = f - fi;
	}
	
	assert( fi >= 0 );
	assert( fi + 1 < (int)lookup.values.size() );
	return Imath::lerp( lookup.values[fi], lookup.values[fi+1], ff );
}
		
} // namespace IECoreNuke

#endif // IECORENUKE_CURVELOOKUP_INL
