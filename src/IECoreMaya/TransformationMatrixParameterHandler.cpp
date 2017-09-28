//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#include "IECoreMaya/NumericTraits.h"
#include "IECoreMaya/ToMayaObjectConverter.h"
#include "IECoreMaya/FromMayaObjectConverter.h"
#include "IECoreMaya/TransformationMatrixParameterHandler.h"

#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/TransformationMatrix.h"

#include "maya/MFnCompoundAttribute.h"
#include "maya/MFnNumericAttribute.h"
#include "maya/MFnUnitAttribute.h"

using namespace IECoreMaya;
using namespace Imath;

// T & R should be kept first, to make iteration easier later on
// as they are the only two with special attribute types.
#define TRANSLATE_INDEX 0
#define ROTATE_INDEX 1
#define SCALE_INDEX 2
#define SHEAR_INDEX 3
#define SCALEPIVOT_INDEX 4
#define SCALEPIVOTTRANS_INDEX 5
#define ROTATEPIVOT_INDEX 6
#define ROTATEPIVOTTRANS_INDEX 7

// We keep this in an array to allow a bunch of tasks to be done in a loop later.
// The order here should correlate with the defines above.
template<typename T>
MString TransformationMatrixParameterHandler<T>::g_attributeNames[] = {
	"translate",
	"rotate",
	"scale",
	"shear",
	"scalePivot",
	"scalePivotTranslation",
	"rotatePivot",
	"rotatePivotTranslation"
};

static ParameterHandler::Description< TransformationMatrixParameterHandler<float> > floatRegistrar( IECore::TransformationMatrixfParameter::staticTypeId() );
static ParameterHandler::Description< TransformationMatrixParameterHandler<double> > doubleRegistrar( IECore::TransformationMatrixdParameter::staticTypeId() );

template<typename T>
MStatus TransformationMatrixParameterHandler<T>::doUpdate( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	typename IECore::TypedParameter<IECore::TransformationMatrix<T> >::ConstPtr p = IECore::runTimeCast<const IECore::TypedParameter<IECore::TransformationMatrix<T> > >( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	MObject attribute = plug.attribute();
	MFnCompoundAttribute fnCAttr( attribute );
	if( !fnCAttr.hasObj( attribute ) )
	{
		return MS::kFailure;
	}

	if( plug.numChildren() != 8 )
	{
		return MS::kFailure;
	}

	MStatus stat;
	MPlug tmpPlug;

	for( unsigned int i=0; i<8; i++ )
	{
		tmpPlug = plug.child( i, &stat );

		// Verify the naming of the child plugs.

		const MString name = tmpPlug.partialName();
		const unsigned int len = name.length() - 1;
		const unsigned int nlen = g_attributeNames[i].length() - 1;
		const MString nameEnd = name.substringW( len - nlen, len );

		if( !stat || nameEnd != g_attributeNames[i] )
		{
			return MS::kFailure;
		}

		MObject attr = tmpPlug.attribute();
		fnCAttr.setObject( attr );
		if( !fnCAttr.hasObj( attr ) )
		{
			return MS::kFailure;
		}
	}

	IECore::TransformationMatrix<T> tMatrix = p->typedDefaultValue();

	if( !setUnitVecDefaultValues( plug.child( TRANSLATE_INDEX ), tMatrix.translate ) )
		return MS::kFailure;

	if( !setUnitVecDefaultValues( plug.child( ROTATE_INDEX ), tMatrix.rotate ) )
		return MS::kFailure;

	if( !setVecDefaultValues( plug.child( SCALE_INDEX ), tMatrix.scale ) )
		return MS::kFailure;

	if( !setVecDefaultValues( plug.child( SHEAR_INDEX ), tMatrix.shear ) )
		return MS::kFailure;

	if( !setVecDefaultValues( plug.child( SCALEPIVOT_INDEX ), tMatrix.scalePivot ) )
		return MS::kFailure;

	if( !setVecDefaultValues( plug.child( SCALEPIVOTTRANS_INDEX ), tMatrix.scalePivotTranslation ) )
		 return MS::kFailure;

	if( !setVecDefaultValues( plug.child( ROTATEPIVOT_INDEX ), tMatrix.rotatePivot ) )
		return MS::kFailure;

	if( !setVecDefaultValues( plug.child( ROTATEPIVOTTRANS_INDEX ), tMatrix.rotatePivotTranslation ) )
		return MS::kFailure;

	return finishUpdating( parameter, plug );
}

template<typename T>
MPlug TransformationMatrixParameterHandler<T>::doCreate( IECore::ConstParameterPtr parameter, const MString &plugName, MObject &node ) const
{
	typename IECore::TypedParameter<IECore::TransformationMatrix<T> >::ConstPtr p = IECore::runTimeCast<const IECore::TypedParameter<IECore::TransformationMatrix<T> > >( parameter );
	if( !p )
	{
		return MPlug();
	}

	MFnCompoundAttribute fnCAttr;
	MObject attribute = fnCAttr.create( plugName, plugName );

	MFnNumericAttribute fnNAttr;
	MFnUnitAttribute fnUAttr;

	// As TransformationMatrix embodies a fairly comprehensive rotation model, were going to be a little
	// more basic here, and just supply a V3f rotation and pretend that the Quartonean isn't there.
	///\todo Expose rotation order and rotationOrientation.

	// These use the '0', '1' and '2' child suffixes instead of 'X' 'Y' and 'Z' to match those created by fnNAttr.create() @ L188.
	MObject p1 = fnUAttr.create( plugName + g_attributeNames[ TRANSLATE_INDEX ] + "0", plugName + g_attributeNames[ TRANSLATE_INDEX ] + "0", MFnUnitAttribute::kDistance );
	MObject p2 = fnUAttr.create( plugName + g_attributeNames[ TRANSLATE_INDEX ] + "1", plugName + g_attributeNames[ TRANSLATE_INDEX ] + "1", MFnUnitAttribute::kDistance );
	MObject p3 = fnUAttr.create( plugName + g_attributeNames[ TRANSLATE_INDEX ] + "2", plugName + g_attributeNames[ TRANSLATE_INDEX ] + "2", MFnUnitAttribute::kDistance );
	fnCAttr.addChild( fnNAttr.create( plugName + g_attributeNames[ TRANSLATE_INDEX ], plugName + g_attributeNames[ TRANSLATE_INDEX ], p1, p2, p3 ) );

	p1 = fnUAttr.create( plugName + g_attributeNames[ ROTATE_INDEX ] + "0", plugName + g_attributeNames[ ROTATE_INDEX ] + "0", MFnUnitAttribute::kAngle );
	p2 = fnUAttr.create( plugName + g_attributeNames[ ROTATE_INDEX ] + "1", plugName + g_attributeNames[ ROTATE_INDEX ] + "1", MFnUnitAttribute::kAngle );
	p3 = fnUAttr.create( plugName + g_attributeNames[ ROTATE_INDEX ] + "2", plugName + g_attributeNames[ ROTATE_INDEX ] + "2", MFnUnitAttribute::kAngle );
	fnCAttr.addChild( fnNAttr.create( plugName + g_attributeNames[ ROTATE_INDEX ], plugName + g_attributeNames[ ROTATE_INDEX ], p1, p2, p3 ) );

	for( unsigned int i=2; i<8; i++ )
	{
		fnCAttr.addChild( fnNAttr.create( plugName + g_attributeNames[i], plugName + g_attributeNames[i], NumericTraits<Imath::Vec3<T> >::dataType() ) );
	}

	MPlug result = finishCreating( parameter, attribute, node );

	if( !doUpdate( parameter, result ) )
	{
		return MPlug(); // failure
	}

	if( !finishUpdating( parameter, result ) )
	{
		return MPlug(); // failure
	}

	return result;
}

template<typename T>
MStatus TransformationMatrixParameterHandler<T>::doSetValue( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	typename IECore::TypedParameter<IECore::TransformationMatrix<T> >::ConstPtr p = IECore::runTimeCast<const IECore::TypedParameter<IECore::TransformationMatrix<T> > >( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	IECore::TransformationMatrix<T> tMatrix = p->getTypedValue();

	if( tMatrix.rotate.order() != Imath::Euler<T>::XYZ )
	{
		IECore::msg(
			IECore::Msg::Error,
			"TransformationMatrixParameterHandler::doSetValue",
			"The rotation order of the parameter '"+parameter->name()+"' is not XYZ, unable to set value."
		);
		return MS::kFailure;
	}

	if( !setVecValues( plug.child( TRANSLATE_INDEX ), tMatrix.translate ) )
		return MS::kFailure;

	if( !setVecValues( plug.child( ROTATE_INDEX ), tMatrix.rotate ) )
		return MS::kFailure;

	if( !setVecValues( plug.child( SCALE_INDEX ), tMatrix.scale ) )
		return MS::kFailure;

	if( !setVecValues( plug.child( SHEAR_INDEX ), tMatrix.shear ) )
		return MS::kFailure;

	if( !setVecValues( plug.child( SCALEPIVOT_INDEX ), tMatrix.scalePivot ) )
		return MS::kFailure;

	if( !setVecValues( plug.child( SCALEPIVOTTRANS_INDEX ), tMatrix.scalePivotTranslation ) )
		return MS::kFailure;

	if( !setVecValues( plug.child( ROTATEPIVOT_INDEX ), tMatrix.rotatePivot ) )
		return MS::kFailure;

	if( !setVecValues( plug.child( ROTATEPIVOTTRANS_INDEX ), tMatrix.rotatePivotTranslation ) )
		return MS::kFailure;

	return MS::kSuccess;
}

template<typename T>
MStatus TransformationMatrixParameterHandler<T>::doSetValue( const MPlug &plug, IECore::ParameterPtr parameter ) const
{
	typename IECore::TypedParameter<IECore::TransformationMatrix<T> >::Ptr p = IECore::runTimeCast<IECore::TypedParameter<IECore::TransformationMatrix<T> > >( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	IECore::TransformationMatrix<T> tMatrix = p->getTypedValue();

	if( tMatrix.rotate.order() != Imath::Euler<T>::XYZ )
	{
		IECore::msg(
			IECore::Msg::Error,
			"TransformationMatrixParameterHandler::doSetValue",
			"The rotation order of the parameter '"+parameter->name()+"' is not XYZ, unable to set value."
		);
		return MS::kFailure;
	}

	std::vector<Imath::Vec3<T> > v;
	v.resize(8);

	for( unsigned int i=0; i<8; i++ )
	{
		if( !getVecValues( plug.child(i), v[i] ) )
			return MS::kFailure;
	}

	tMatrix.translate = v[ TRANSLATE_INDEX ];
	tMatrix.rotate = Imath::Euler<T>(v[ ROTATE_INDEX ]);
	tMatrix.scale = v[ SCALE_INDEX ];
	tMatrix.shear = v[ SHEAR_INDEX ];
	tMatrix.scalePivot = v[ SCALEPIVOT_INDEX ];
	tMatrix.scalePivotTranslation = v[ SCALEPIVOTTRANS_INDEX ];
	tMatrix.rotatePivot = v[ ROTATEPIVOT_INDEX ];
	tMatrix.rotatePivotTranslation = v[ ROTATEPIVOTTRANS_INDEX ];

	p->setTypedValue( tMatrix );

	return MS::kSuccess;
}

template<typename T>
MStatus TransformationMatrixParameterHandler<T>::setVecValues( MPlug vecPlug, Imath::Vec3<T> &values ) const
{
	if( vecPlug.numChildren() != 3 )
	{
		return MS::kFailure;
	}

	for( unsigned int i=0; i<3; i++ )
	{
		if( !vecPlug.child(i).setValue( values[i] ) )
		{
			return MS::kFailure;
		}
	}

	return MS::kSuccess;
}

template<typename T>
MStatus TransformationMatrixParameterHandler<T>::getVecValues( MPlug vecPlug, Imath::Vec3<T> &values ) const
{
	if( vecPlug.numChildren() != 3 )
	{
		return MS::kFailure;
	}

	for( unsigned int i=0; i<3; i++ )
	{
		if( !vecPlug.child(i).getValue( values[i] ) )
		{
			return MS::kFailure;
		}
	}

	return MS::kSuccess;
}


template<typename T>
MStatus TransformationMatrixParameterHandler<T>::setVecDefaultValues( MPlug vecPlug, Imath::Vec3<T> &defaultValue ) const
{
	if( vecPlug.numChildren() != 3 )
	{
		return MS::kFailure;
	}

	MFnNumericAttribute fnN;
	for( unsigned int i=0; i<3; i++ )
	{
		fnN.setObject( vecPlug.child(i).attribute() );
		if( !fnN.setDefault( defaultValue[i] ) )
		{
			return MS::kFailure;
		}
	}

	return MS::kSuccess;
}

template<typename T>
MStatus TransformationMatrixParameterHandler<T>::setUnitVecDefaultValues( MPlug vecPlug, Imath::Vec3<T> &defaultValue ) const
{
	if( vecPlug.numChildren() != 3 )
	{
		return MS::kFailure;
	}

	MFnUnitAttribute fnU;
	for( unsigned int i=0; i<3; i++ )
	{
		fnU.setObject( vecPlug.child(i).attribute() );
		if( !fnU.setDefault( defaultValue[i] ) )
		{
			return MS::kFailure;
		}
	}

	return MS::kSuccess;
}
