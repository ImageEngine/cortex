//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2017, Image Engine Design Inc. All rights reserved.
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

#include "IECorePython/IECoreBinding.h"

#include "IECore/Export.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "OpenEXR/ImathBox.h"
#include "OpenEXR/ImathColor.h"
#include "OpenEXR/ImathEuler.h"
#include "OpenEXR/ImathMatrix.h"
#include "OpenEXR/ImathPlane.h"
#include "OpenEXR/ImathVec.h"
IECORE_POP_DEFAULT_VISIBILITY

using namespace std;
using namespace Imath;

namespace IECorePython
{

#define DEFINEVECSTRSPECIALISATION( VEC )\
\
template<> IECORE_EXPORT \
std::string repr<VEC>( VEC &x )\
{\
	std::stringstream s;\
	s << "imath." << #VEC << "( ";\
	for( unsigned i=0; i<VEC::dimensions(); i++ )\
	{\
		s << boost::lexical_cast<string>( x[i] );\
		if( i!=VEC::dimensions()-1 )\
		{\
			s << ", ";\
		}\
	}\
	s << " )";\
	return s.str();\
}\
\
template<> IECORE_EXPORT \
std::string str<VEC>( VEC &x )\
{\
	std::stringstream s;\
	for( unsigned i=0; i<VEC::dimensions(); i++ )\
	{\
		s << boost::lexical_cast<string>( x[i] );\
		if( i!=VEC::dimensions()-1 )\
		{\
			s << " ";\
		}\
	}\
	return s.str();\
}\

DEFINEVECSTRSPECIALISATION( V2i );
DEFINEVECSTRSPECIALISATION( V2f );
DEFINEVECSTRSPECIALISATION( V2d );
DEFINEVECSTRSPECIALISATION( V3i );
DEFINEVECSTRSPECIALISATION( V3f );
DEFINEVECSTRSPECIALISATION( V3d );

#define DEFINEBOXSTRSPECIALISATION( BOX )\
\
template<> IECORE_EXPORT \
std::string repr<BOX>( BOX &x )\
{\
	std::stringstream s;\
	s << "imath." << #BOX << "(";\
	if( !x.isEmpty() )\
	{\
		s << " " << repr( x.min ) << ", ";\
		s << repr( x.max ) << " ";\
	}\
	s << ")";\
	return s.str();\
}\
\
template<> IECORE_EXPORT \
std::string str<BOX>( BOX &x )\
{\
	std::stringstream s;\
	s << str( x.min ) << " " << str( x.max );\
	return s.str();\
}\

DEFINEBOXSTRSPECIALISATION( Box2i );
DEFINEBOXSTRSPECIALISATION( Box3i );
DEFINEBOXSTRSPECIALISATION( Box2f );
DEFINEBOXSTRSPECIALISATION( Box3f );
DEFINEBOXSTRSPECIALISATION( Box2d );
DEFINEBOXSTRSPECIALISATION( Box3d );

#define DEFINECOLSTRSPECIALISATION( COL )\
\
template<> IECORE_EXPORT \
std::string repr<COL>( COL &x )\
{\
	std::stringstream s;\
	s << "imath." << #COL << "( ";\
	for( unsigned i=0; i<COL::dimensions(); i++ )\
	{\
		s << boost::lexical_cast<std::string>( x[i] );\
		if( i!=COL::dimensions()-1 )\
		{\
			s << ", ";\
		}\
	}\
	s << " )";\
	return s.str();\
}\
\
template<> IECORE_EXPORT \
std::string str<COL>( COL &x )\
{\
	std::stringstream s;\
	for( unsigned i=0; i<COL::dimensions(); i++ )\
	{\
		s << boost::lexical_cast<std::string>( x[i] );\
		if( i!=COL::dimensions()-1 )\
		{\
			s << " ";\
		}\
	}\
	return s.str();\
}\

DEFINECOLSTRSPECIALISATION( Color3f );
DEFINECOLSTRSPECIALISATION( Color4f );

/// \todo Handle rotation order
#define DEFINEEULERSTRSPECIALISATION( EULER )\
\
template<> IECORE_EXPORT \
std::string repr<EULER>( EULER &x )\
{\
	std::stringstream s;\
	s << "imath." << #EULER << "( ";\
	for( unsigned i=0; i<EULER::dimensions(); i++ )\
	{\
		s << x[i];\
		if( i!=EULER::dimensions()-1 )\
		{\
			s << ", ";\
		}\
	}\
	s << " )";\
	return s.str();\
}\
\
template<> IECORE_EXPORT \
std::string str<EULER>( EULER &x )\
{\
	std::stringstream s;\
	for( unsigned i=0; i<EULER::dimensions(); i++ )\
	{\
		s << x[i];\
		if( i!=EULER::dimensions()-1 )\
		{\
			s << " ";\
		}\
	}\
	return s.str();\
}\

DEFINEEULERSTRSPECIALISATION( Eulerf );
DEFINEEULERSTRSPECIALISATION( Eulerd );

#define DEFINEMATRIXSTRSPECIALISATION( TYPE, D )\
template<> IECORE_EXPORT \
string repr<TYPE>( TYPE &x )\
{\
	stringstream s;\
	s << "imath." << #TYPE << "( ";\
	for( int i=0; i<D; i++ )\
	{\
		for( int j=0; j<D; j++ )\
		{\
			s << x[i][j];\
			if( !(i==D-1 && j==D-1) )\
			{\
				s << ", ";\
			}\
		}\
	}\
	s << " )";\
	return s.str();\
}\
\
template<> IECORE_EXPORT \
string str<TYPE>( TYPE &x )\
{\
	stringstream s;\
	for( int i=0; i<D; i++ )\
	{\
		for( int j=0; j<D; j++ )\
		{\
			s << x[i][j];\
			if( !(i==D-1 && j==D-1) )\
			{\
				s << " ";\
			}\
		}\
	}\
	return s.str();\
}

DEFINEMATRIXSTRSPECIALISATION( M33f, 3 );
DEFINEMATRIXSTRSPECIALISATION( M33d, 3 );
DEFINEMATRIXSTRSPECIALISATION( M44f, 4 );
DEFINEMATRIXSTRSPECIALISATION( M44d, 4 );

#define DEFINEPLANEPECIALISATION( PLANE )\
\
template<> IECORE_EXPORT \
std::string repr<PLANE>( PLANE &x )\
{\
	std::stringstream s;\
	s << "imath." << #PLANE << "( ";\
	s << repr( x.normal ) << ", ";\
	s << x.distance;\
	s << " )";\
	return s.str();\
}\
\
template<> IECORE_EXPORT \
std::string str<PLANE>( PLANE &x )\
{\
	std::stringstream s;\
	s << str( x.normal ) << " " << str( x.distance );\
	return s.str();\
}\

DEFINEPLANEPECIALISATION( Plane3f );
DEFINEPLANEPECIALISATION( Plane3d );

#define DEFINEQUATSTRSPECIALISATION( QUAT )\
\
template<> IECORE_EXPORT \
std::string repr<QUAT>( QUAT &x )\
{\
	std::stringstream s;\
	s << "imath." << #QUAT << "( ";\
	for( unsigned i=0; i<4; i++ )\
	{\
		s << x[i];\
		if( i!=3 )\
		{\
			s << ", ";\
		}\
	}\
	s << " )";\
	return s.str();\
}\
\
template<> IECORE_EXPORT \
std::string str<QUAT>( QUAT &x )\
{\
	std::stringstream s;\
	for( unsigned i=0; i<4; i++ )\
	{\
		s << x[i];\
		if( i!=3 )\
		{\
			s << " ";\
		}\
	}\
	return s.str();\
}\

DEFINEQUATSTRSPECIALISATION( Quatf );
DEFINEQUATSTRSPECIALISATION( Quatd );

} // namespace IECorePython
