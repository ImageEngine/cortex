//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2018, Image Engine Design Inc. All rights reserved.
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


#include "IECoreMaya/FromMayaInstancerConverter.h"

#include "IECoreMaya/FromMayaArrayDataConverter.h"

#include "IECoreScene/PointsPrimitive.h"

#include "IECore/NullObject.h"
#include "IECore/AngleConversion.h"

#include "maya/MDataHandle.h"
#include "maya/MDoubleArray.h"
#include "maya/MIntArray.h"
#include "maya/MFnArrayAttrsData.h"
#include "maya/MFnDoubleArrayData.h"
#include "maya/MFnInstancer.h"
#include "maya/MFnIntArrayData.h"
#include "maya/MFnVectorArrayData.h"
#include "maya/MFnStringArrayData.h"
#include "maya/MPlug.h"
#include "maya/MPlugArray.h"
#include "maya/MStringArray.h"
#include "maya/MVectorArray.h"

#include "ImathEuler.h"

#include "boost/algorithm/string/replace.hpp"

using namespace IECoreMaya;
using namespace IECore;
using namespace IECoreScene;
using namespace std;
using namespace Imath;

IE_CORE_DEFINERUNTIMETYPED( FromMayaInstancerConverter );

namespace
{

DataPtr convert( const MString &attrName, MFnArrayAttrsData &attrsData )
{
	MFnArrayAttrsData::Type t = MFnArrayAttrsData::kInvalid;
	attrsData.checkArrayExist( attrName, t );

	switch( t )
	{
		case MFnArrayAttrsData::kVectorArray:
		{
			MFnVectorArrayData vectorArrayData;

			FromMayaArrayDataConverterVV3f::Ptr converter = new FromMayaArrayDataConverterVV3f(
				vectorArrayData.create(
					attrsData.vectorArray(
						attrName
					)
				)
			);
			return runTimeCast<V3fVectorData>( converter->convert() );
		}
		case MFnArrayAttrsData::kDoubleArray:
		{
			MFnDoubleArrayData doubleArrayData;
			FromMayaArrayDataConverterdd::Ptr converter = new FromMayaArrayDataConverterdd(
				doubleArrayData.create(
					attrsData.doubleArray(
						attrName
					)
				)
			);
			return runTimeCast<DoubleVectorData>( converter->convert() );

		}
		case MFnArrayAttrsData::kIntArray:
		{
			MFnIntArrayData intArrayData;
			FromMayaArrayDataConverterii::Ptr converter = new FromMayaArrayDataConverterii( intArrayData.create( attrsData.intArray( attrName ) ) );
			return runTimeCast<IntVectorData>( converter->convert() );

		}
		case MFnArrayAttrsData::kStringArray:
		{
			MFnStringArrayData stringArrayData;
			FromMayaArrayDataConverterss::Ptr converter = new FromMayaArrayDataConverterss(
				stringArrayData.create(
					attrsData.stringArray(
						attrName
					)
				)
			);
			return runTimeCast<StringVectorData>( converter->convert() );

		}
		case MFnArrayAttrsData::kInvalid:
		case MFnArrayAttrsData::kLast:
			break;
	}

	return nullptr;
}

IECore::QuatfVectorDataPtr eulerToQuat( IECore::V3fVectorData *eulerData, Imath::Eulerf::Order order, bool isDegrees )
{
	auto &readableEulerData = eulerData->readable();

	IECore::QuatfVectorDataPtr quatData = new IECore::QuatfVectorData();
	auto &writableQuatData = quatData->writable();

	writableQuatData.reserve( readableEulerData.size() );

	for( const auto rotation : readableEulerData )
	{
		float x = isDegrees ? IECore::degreesToRadians( rotation.x ) : rotation.x;
		float y = isDegrees ? IECore::degreesToRadians( rotation.y ) : rotation.y;
		float z = isDegrees ? IECore::degreesToRadians( rotation.z ) : rotation.z;

		Imath::Eulerf euler( x, y, z, order );
		writableQuatData.push_back( euler.toQuat() );
	}

	return quatData;
}

IECore::IntVectorDataPtr doubleToInt( IECore::DoubleVectorData *doubleData )
{
	auto &readableDoubleData = doubleData->readable();

	IECore::IntVectorDataPtr intData = new IECore::IntVectorData();
	auto &writableIntData = intData->writable();

	writableIntData.reserve( readableDoubleData.size() );

	for( const auto value : readableDoubleData )
	{
		writableIntData.push_back( (int) value );
	}

	return intData;
}

Imath::Eulerf::Order getRotationOrder( int instancerRotationOrder )
{
	using namespace Imath;
	static const std::vector<Eulerf::Order> orders = {Eulerf::XYZ, Eulerf::XZY, Eulerf::YZX, Eulerf::YXZ, Eulerf::ZXY, Eulerf::ZYX};
	if( instancerRotationOrder >= (int) orders.size() )
	{
		return Eulerf::Default;
	}
	return orders[instancerRotationOrder];
}
} // namespace

FromMayaDagNodeConverter::Description<FromMayaInstancerConverter> FromMayaInstancerConverter::m_description(
	MFn::kInstancer, PointsPrimitive::staticTypeId(), true
);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// constructors
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


FromMayaInstancerConverter::FromMayaInstancerConverter( const MDagPath &dagPath ) : FromMayaDagNodeConverter(
	"Converts Instancer to IECoreScene::PointsPrimitive objects.", dagPath
)
{
}

FromMayaInstancerConverter::~FromMayaInstancerConverter()
{
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// conversion
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IECore::ObjectPtr FromMayaInstancerConverter::doConversion( const MDagPath &dagPath, IECore::ConstCompoundObjectPtr operands ) const
{
	MFnInstancer instancer;

	if( !instancer.setObject( dagPath ) )
	{
		return nullptr;
	}

	MStatus status;
	Imath::Eulerf::Order order;
	MPlug rotationOrderPlug = instancer.findPlug( "rotationOrder", true, &status );
	order = status ? getRotationOrder( rotationOrderPlug.asInt() ) : Imath::Eulerf::Default;

	MPlug rotationUnits = instancer.findPlug( "rotationAngleUnits", true, &status );
	bool isDegrees = status ? rotationUnits.asInt() == 0 : false; // if isDegrees == false then we have radians

	auto emptyPositions = new V3fVectorData();
	emptyPositions->setInterpretation( GeometricData::Interpretation::Point );
	PointsPrimitivePtr pointsPrimitive = new PointsPrimitive( emptyPositions, nullptr );

	MPlug inputPointsPlug = instancer.findPlug( "inputPoints", true, &status );
	if( !status )
	{
		return pointsPrimitive;
	}

	MFnArrayAttrsData attrsData( inputPointsPlug.asMDataHandle().data(), &status );
	if( !status )
	{
		return pointsPrimitive;
	}

	MStringArray attributeNames = attrsData.list();

	for( size_t a = 0; a < attributeNames.length(); ++a )
	{
		const MString &attrName = attributeNames[a];

		DataPtr d = ::convert( attrName, attrsData );
		if( d )
		{
			std::string cortexAttributeName( attrName.asChar() );

			if( attrName == "position" )
			{
				cortexAttributeName = "P";
				auto pointData = runTimeCast<V3fVectorData>( d ).get();
				pointsPrimitive->setNumPoints( pointData->readable().size() );
				pointData->setInterpretation( IECore::GeometricData::Interpretation::Point );
			}
			else if( attrName == "rotation" )
			{
				cortexAttributeName = "orient";
				d = eulerToQuat( runTimeCast<V3fVectorData>( d.get() ), order, isDegrees );
			}
			else if (attrName == "objectIndex")
			{
				cortexAttributeName = "instanceType";
				d = doubleToInt( runTimeCast<DoubleVectorData>( d.get() ) );
			}
			else if( attrName == "visibility" )
			{
				d = doubleToInt( runTimeCast<DoubleVectorData>( d.get() ) );
			}
			else if ( attrName == "id")
			{
				d = doubleToInt( runTimeCast<DoubleVectorData>( d.get() ) );
			}

			pointsPrimitive->variables[cortexAttributeName] = PrimitiveVariable( PrimitiveVariable::Vertex, d );
		}
	}

	MPlug inputHierarchy = instancer.findPlug( "inputHierarchy", true, &status );

	if( status )
	{
		StringVectorDataPtr instancePathsData = new StringVectorData();
		auto &writableInstancePaths = instancePathsData->writable();

		for( unsigned int i = 0; i < inputHierarchy.numElements(); ++i )
		{
			MPlug e = inputHierarchy.elementByLogicalIndex( i );
			MPlugArray inputs;
			e.connectedTo( inputs, true, false );
			if( inputs.length() )
			{
				MStatus s;
				MFnDagNode dag( inputs[0].node(), &s );
				if( s )
				{
					std::string instancePath = boost::algorithm::replace_all_copy(
						std::string( dag.fullPathName().asChar() ), std::string( "|" ), std::string( "/" )
					);
					writableInstancePaths.push_back( instancePath );
				}
			}
		}

		pointsPrimitive->variables["instances"] = PrimitiveVariable( PrimitiveVariable::Constant, instancePathsData );
	}

	return pointsPrimitive;

}

