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

#ifndef IEMAYAFUR_V3MANIPULATOR_H
#define IEMAYAFUR_V3MANIPULATOR_H

#include <IECoreMaya/ParameterManipContainer.h>

#include <maya/MPxCommand.h>
#include <maya/MFnNumericData.h>
#include <maya/MMatrix.h>
#include <maya/MPoint.h>
#include <maya/MFnDagNode.h>

#include <vector>

namespace IECoreMaya
{

/// This class provides a manipulator for V3f and V3d Parameters.
/// Curently the node is only tested/registered for V3f Parameter types.
///
/// Behavior can be further customised by adding the StringData member
/// "manipSpace" to the "UI" CompoundObject in there Parameters userData().
/// If this member exists, valid values are "world" and "object". When using
/// object space (default), on-screen controls are transformed along with the node.
///
class V3Manipulator : public ParameterManipContainer
{
	public:

		V3Manipulator();
		virtual ~V3Manipulator();

		static void *creator();
    	static MStatus initialize();

		virtual MStatus createChildren();
    	virtual MStatus connectToDependNode( const MObject &node);

		virtual void draw( M3dView & view,
						   const MDagPath & path,
						   M3dView::DisplayStyle style,
						   M3dView::DisplayStatus status );

		MManipData vectorPlugToManipConversion( unsigned int manipIndex );
		MManipData vectorManipToPlugConversion( unsigned int plugIndex );

		static MTypeId id;

	private:

		MPoint getPlugValues( MPlug &plug );
		void getPlugValues( MPlug &plug, double *values );
		void getPlugValues( MPlug &plug, MFnNumericData &data );

		void readParameterOptions( MFnDagNode &nodeFn );
		bool m_worldSpace;

		MDagPath m_translateManip;
		MPlug m_translatePlug;
		MMatrix m_localMatrix;
		MMatrix m_localMatrixInv;
};


}
#endif // IEMAYAFUR_V3MANIPULATOR_H
