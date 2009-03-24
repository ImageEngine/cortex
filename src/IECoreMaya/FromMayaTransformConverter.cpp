//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#include "IECoreMaya/FromMayaTransformConverter.h"
#include "IECoreMaya/Convert.h"

#include "IECore/CompoundParameter.h"
#include "IECore/TransformationMatrixData.h"

#include "maya/MTransformationMatrix.h"
#include "maya/MFnMatrixData.h"
#include "maya/MFnTransform.h"
#include "maya/MPlug.h"

using namespace IECoreMaya;

static const MFn::Type fromTypes[] = { MFn::kTransform };
static const IECore::TypeId toTypes[] = { IECore::TransformationMatrixdData::staticTypeId() };

FromMayaDagNodeConverter::Description<FromMayaTransformConverter> FromMayaTransformConverter::g_description( fromTypes, toTypes );

FromMayaTransformConverter::FromMayaTransformConverter( const MDagPath &dagPath )
	:	FromMayaDagNodeConverter( staticTypeName(), "Converts transform nodes.",  dagPath )
{
	
	IECore::IntParameter::PresetsMap spacePresets;
	spacePresets["Local"] = Local;	
	spacePresets["World"] = World;		
	m_spaceParameter = new IECore::IntParameter(
		"space",
		"The space in which the transform is converted.",
		World,
		Local,
		World,
		spacePresets,
		true
	);

	parameters()->addParameter( m_spaceParameter );
	
	m_lastRotationValid = false;
	m_eulerFilterParameter = new IECore::BoolParameter(
		"eulerFilter",
		"If this parameter is on, then rotations are filtered so as to be as "
		"close as possible to the previously converted rotation. This allows "
		"the reuse of the same converter over a series of frames to produce a series "
		"of transformations which will interpolate smoothly.",
		false
	);
	
	parameters()->addParameter( m_eulerFilterParameter );
	
	/// \todo We need this parameter because we're finding that our conversion of the maya
	/// MTransformationMatrix class to our TransformationMatrix classes isn't yielding the same
	/// results when the pivot is non-zero. We should figure out the real reason for that rather
	/// than use this parameter as a crutch.
	m_zeroPivotsParameter = new IECore::BoolParameter(
		"zeroPivots",
		"If this parameter is on, then the scale and rotate pivots are reset to zero, "
		"adjusting the transform to maintain the same positioning.",
		false
	);
	
	parameters()->addParameter( m_zeroPivotsParameter );
}

IECore::IntParameterPtr FromMayaTransformConverter::spaceParameter()
{
	return m_spaceParameter;
}

IECore::ConstIntParameterPtr FromMayaTransformConverter::spaceParameter() const
{
	return m_spaceParameter;
}

IECore::BoolParameterPtr FromMayaTransformConverter::eulerFilterParameter()
{
	return m_eulerFilterParameter;
}

IECore::ConstBoolParameterPtr FromMayaTransformConverter::eulerFilterParameter() const
{
	return m_eulerFilterParameter;
}

IECore::BoolParameterPtr FromMayaTransformConverter::zeroPivotsParameter()
{
	return m_zeroPivotsParameter;
}

IECore::ConstBoolParameterPtr FromMayaTransformConverter::zeroPivotsParameter() const
{
	return m_zeroPivotsParameter;
}
				
IECore::ObjectPtr FromMayaTransformConverter::doConversion( const MDagPath &dagPath, IECore::ConstCompoundObjectPtr operands ) const
{	
	MTransformationMatrix transform;
	
	if( m_spaceParameter->getNumericValue()==Local )
	{
		MFnTransform fnT( dagPath );
		transform = fnT.transformation();
	}
	else
	{
		unsigned instIndex = dagPath.instanceNumber();
	
		MObject dagNode = dagPath.node();
		MFnDependencyNode fnN( dagNode );
		
		MPlug plug = fnN.findPlug( "worldMatrix" );
		MPlug instPlug = plug.elementByLogicalIndex( instIndex );
		
		MObject matrix;
		instPlug.getValue( matrix );
		
		MFnMatrixData fnM( matrix );
		transform = fnM.transformation();
	}
	
	if( m_zeroPivotsParameter->getTypedValue() )
	{
		transform.setScalePivot( MPoint( 0, 0, 0 ), MSpace::kTransform, true );
		transform.setRotatePivot( MPoint( 0, 0, 0 ), MSpace::kTransform, true );
	}
	
	if( m_eulerFilterParameter->getTypedValue() && m_lastRotationValid )
	{
		transform.rotateTo( transform.eulerRotation().closestSolution( m_lastRotation ) );
	}
	
	m_lastRotation = transform.eulerRotation();
	m_lastRotationValid = true;
	
	return new IECore::TransformationMatrixdData( IECore::convert<IECore::TransformationMatrixd, MTransformationMatrix>( transform ) );
}
			
