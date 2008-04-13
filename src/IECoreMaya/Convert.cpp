//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#include "IECoreMaya/Convert.h"

using namespace Imath;

namespace IECore
{

template<>
std::string convert( const MString &from )
{
	return from.asChar();
}

template<>
MString convert( const std::string &from )
{
	return from.c_str();
}

template<>
Imath::V3f convert( const MVector &from )
{
	return Imath::V3f( from[0], from[1], from[2] );
}

template<>
Imath::V3f convert( const MFloatVector &from )
{
	return Imath::V3f( from[0], from[1], from[2] );
}

template<>
Imath::V3d convert( const MVector &from )
{
	return Imath::V3d( from[0], from[1], from[2] );
} 

template<>
Imath::V3d convert( const MFloatVector &from )
{
	return Imath::V3d( from[0], from[1], from[2] );
} 

template<>
Imath::V3f convert( const MPoint &from )
{
	return Imath::V3f( from[0], from[1], from[2] );
}

template<>
Imath::V3f convert( const MFloatPoint &from )
{
	return Imath::V3f( from[0], from[1], from[2] );
}

template<>
Imath::V3d convert( const MPoint &from )
{
	return Imath::V3f( from[0], from[1], from[2] );
}

template<>
Imath::V3d convert( const MFloatPoint &from )
{
	return Imath::V3f( from[0], from[1], from[2] );
}

template<>
MVector convert( const Imath::V3f &from )
{
	return MVector( from[0], from[1], from[2] );
}

template<>
MVector convert( const Imath::V3d &from )
{
	return MVector( from[0], from[1], from[2] );
}

template<>
MFloatVector convert( const Imath::V3f &from )
{
	return MFloatVector( from[0], from[1], from[2] );
}

template<>
MFloatVector convert( const Imath::V3d &from )
{
	return MFloatVector( from[0], from[1], from[2] );
}

template<>
MPoint convert( const Imath::V3f &from )
{
	return MPoint( from[0], from[1], from[2] );
}

template<>
MPoint convert( const Imath::V3d &from )
{
	return MPoint( from[0], from[1], from[2] );
}

template<>
MFloatPoint convert( const Imath::V3f &from )
{
	return MFloatPoint( from[0], from[1], from[2] );
}

template<>
MFloatPoint convert( const Imath::V3d &from )
{
	return MFloatPoint( from[0], from[1], from[2] );
}

template<>
Imath::Color3f convert( const MVector &from )
{
	return Imath::Color3f( from[0], from[1], from[2] );
}

template<>
Imath::Color3f convert( const MColor &from )
{
	return Imath::Color3f( from[0], from[1], from[2] );
}

template<>
Imath::Color4f convert( const MColor &from )
{
	return Imath::Color4f( from[0], from[1], from[2], 1.0f );
}

template<>
MBoundingBox convert( const Imath::Box3f &from )
{
	if( from.isEmpty() )
	{
		return MBoundingBox();
	}
	return MBoundingBox( convert<MPoint>( from.min ), convert<MPoint>( from.max ) );
}

template<>
Imath::Box3f convert( const MBoundingBox &from )
{
	return Imath::Box3f( convert<V3f>( from.min() ), convert<V3f>( from.max() ) );
}

template<>
Imath::Quatf convert( const MQuaternion &from )
{
	return Imath::Quatf( static_cast<float>(from[3]), static_cast<float>(from[0]), static_cast<float>(from[1]), static_cast<float>(from[2]) );
}

template<>
MQuaternion convert( const Imath::Quatf &from )
{
	return MQuaternion( static_cast<double>(from[1]), static_cast<double>(from[2]), static_cast<double>(from[3]), static_cast<double>(from[0]) );
}

template<>
Imath::Quatd convert( const MQuaternion &from )
{
	return Imath::Quatd( from[3], from[0], from[1], from[2] );
}

template<>
MQuaternion convert( const Imath::Quatd &from )
{
	return MQuaternion( from[1], from[2], from[3], from[0] );
}

template<>
Imath::M44f convert( const MMatrix &from )
{
	return Imath::M44f( from[0][0], from[0][1], from[0][2], from[0][3], from[1][0], from[1][1], from[1][2], from[1][3],
						from[2][0], from[2][1], from[2][2], from[2][3], from[3][0], from[3][1], from[3][2], from[3][3]  );
}

template<>
MMatrix convert( const Imath::M44f &from )
{
	return MMatrix( from.x );
}

template< typename T >
IECore::TransformationMatrix<T> convertTransf( const MTransformationMatrix &from )
{
	IECore::TransformationMatrix<T> to;
	double vector[3];
	to.scalePivot = convert< Imath::Vec3<T>, MPoint >( from.scalePivot( MSpace::kTransform ) );
	from.getScale( vector, MSpace::kTransform );
	to.scale = Imath::Vec3<T>( static_cast<T>(vector[0]), static_cast<T>(vector[1]), static_cast<T>(vector[2]) );
	from.getShear( vector, MSpace::kTransform );
	to.shear = Imath::Vec3<T>( static_cast<T>(vector[0]), static_cast<T>(vector[1]), static_cast<T>(vector[2]) );
	to.scalePivotTranslation = convert< Imath::Vec3<T>, MVector>( from.scalePivotTranslation( MSpace::kTransform ) );
	to.rotatePivot = convert< Imath::Vec3<T>, MPoint>( from.rotatePivot( MSpace::kTransform ) );
	to.rotationOrientation = convert< Imath::Quat<T>, MQuaternion>( from.rotationOrientation() );
	to.rotate = convert< Imath::Quat<T>, MQuaternion>( from.rotation() );
	to.rotatePivotTranslation = convert< Imath::Vec3<T>, MVector >( from.rotatePivotTranslation( MSpace::kTransform ) );
	to.translate = convert< Imath::Vec3<T>, MVector >( from.getTranslation( MSpace::kTransform ) );
	return to;
}

template< typename T >
MTransformationMatrix convertTransf( const IECore::TransformationMatrix<T> &from )
{
	MTransformationMatrix to;
	double vector[3];
	to.setScalePivot( convert< MPoint, Imath::Vec3<T> >( from.scalePivot ), MSpace::kTransform, false );
	vector[0] = static_cast<double>( from.scale[0] );
	vector[1] = static_cast<double>( from.scale[1] );
	vector[2] = static_cast<double>( from.scale[2] );
	to.setScale( vector, MSpace::kTransform );
	vector[0] = static_cast<double>( from.shear[0] );
	vector[1] = static_cast<double>( from.shear[1] );
	vector[2] = static_cast<double>( from.shear[2] );
	to.setShear( vector, MSpace::kTransform );
	to.setScalePivotTranslation( convert< MVector, Imath::Vec3<T> >( from.scalePivotTranslation ), MSpace::kTransform );
	to.setRotatePivot( convert< MPoint, Imath::Vec3<T> >( from.rotatePivot ), MSpace::kTransform, false );
	to.setRotationOrientation( convert< MQuaternion, Imath::Quat<T> >( from.rotationOrientation ) );
	to.rotateTo( convert< MQuaternion, Imath::Quat<T> >( from.rotate ) );
	to.setRotatePivotTranslation( convert< MVector, Imath::Vec3<T> >( from.rotatePivotTranslation ), MSpace::kTransform );
	to.setTranslation( convert< MVector, Imath::Vec3<T> >( from.translate ), MSpace::kTransform );
	return to;
}

template<>
IECore::TransformationMatrixf convert( const MTransformationMatrix &from )
{
	return convertTransf< float >( from );
}

template<>
MTransformationMatrix convert( const IECore::TransformationMatrixf &from )
{
	return convertTransf< float >( from );
}

template<>
IECore::TransformationMatrixd convert( const MTransformationMatrix &from )
{
	return convertTransf< double >( from );
}

template<>
MTransformationMatrix convert( const IECore::TransformationMatrixd &from )
{
	return convertTransf< double >( from );
}


}	// namespace IECore
