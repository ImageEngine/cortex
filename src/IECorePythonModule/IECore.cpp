//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2014, Image Engine Design Inc. All rights reserved.
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
//
//  Copyright (c) 2012, John Haddon. All rights reserved.
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

#include "boost/python.hpp"

#include "TBBBinding.h"

#include "OpenEXR/ImathEuler.h"

#include "IECorePython/RefCountedBinding.h"
#include "IECorePython/RunTimeTypedBinding.h"
#include "IECorePython/ExceptionBinding.h"
#include "IECorePython/KDTreeBinding.h"
#include "IECorePython/IndexedIOBinding.h"
#include "IECorePython/DataBinding.h"
#include "IECorePython/GeometricTypedDataBinding.h"
#include "IECorePython/SimpleTypedDataBinding.h"
#include "IECorePython/VectorTypedDataBinding.h"
#include "IECorePython/ObjectBinding.h"
#include "IECorePython/TypeIdBinding.h"
#include "IECorePython/CompoundDataBinding.h"
#include "IECorePython/MessageHandlerBinding.h"
#include "IECorePython/ReaderBinding.h"
#include "IECorePython/BlindDataHolderBinding.h"
#include "IECorePython/WriterBinding.h"
#include "IECorePython/ParameterBinding.h"
#include "IECorePython/NumericParameterBinding.h"
#include "IECorePython/SimpleTypedParameterBinding.h"
#include "IECorePython/VecTypedParameterBinding.h"
#include "IECorePython/ColorTypedParameterBinding.h"
#include "IECorePython/BoxTypedParameterBinding.h"
#include "IECorePython/MatrixTypedParameterBinding.h"
#include "IECorePython/LineTypedParameterBinding.h"
#include "IECorePython/VectorTypedParameterBinding.h"
#include "IECorePython/SplineParameterBinding.h"
#include "IECorePython/DateTimeParameterBinding.h"
#include "IECorePython/CompoundParameterBinding.h"
#include "IECorePython/ValidatedStringParameterBinding.h"
#include "IECorePython/PathParameterBinding.h"
#include "IECorePython/FileNameParameterBinding.h"
#include "IECorePython/DirNameParameterBinding.h"
#include "IECorePython/CompoundObjectBinding.h"
#include "IECorePython/ObjectReaderBinding.h"
#include "IECorePython/ObjectWriterBinding.h"
#include "IECorePython/TimerBinding.h"
#include "IECorePython/TurbulenceBinding.h"
#include "IECorePython/SearchPathBinding.h"
#include "IECorePython/CachedReaderBinding.h"
#include "IECorePython/ParameterisedBinding.h"
#include "IECorePython/OpBinding.h"
#include "IECorePython/ObjectParameterBinding.h"
#include "IECorePython/ModifyOpBinding.h"
#include "IECorePython/PerlinNoiseBinding.h"
#include "IECorePython/HalfBinding.h"
#include "IECorePython/NullObjectBinding.h"
#include "IECorePython/ObjectInterpolatorBinding.h"
#include "IECorePython/TransformationMatrixBinding.h"
#include "IECorePython/TransformationMatrixDataBinding.h"
#include "IECorePython/BoundedKDTreeBinding.h"
#include "IECorePython/VectorDataFilterOpBinding.h"
#include "IECorePython/TypedObjectParameterBinding.h"
#include "IECorePython/HeaderGeneratorBinding.h"
#include "IECorePython/DataCastOpBinding.h"
#include "IECorePython/DataPromoteOpBinding.h"
#include "IECorePython/MatrixMultiplyOpBinding.h"
#include "IECorePython/RandomRotationOpBinding.h"
#include "IECorePython/InternedStringBinding.h"
#include "IECorePython/InverseDistanceWeightedInterpolationBinding.h"
#include "IECorePython/MathBinding.h"
#include "IECorePython/PathVectorParameterBinding.h"
#include "IECorePython/TriangleAlgoBinding.h"
#include "IECorePython/ConverterBinding.h"
#include "IECorePython/FromCoreConverterBinding.h"
#include "IECorePython/LineSegmentBinding.h"
#include "IECorePython/CubicBasisBinding.h"
#include "IECorePython/BezierAlgoBinding.h"
#include "IECorePython/ToCoreConverterBinding.h"
#include "IECorePython/PolygonAlgoBinding.h"
#include "IECorePython/UnicodeToStringBinding.h"
#include "IECorePython/RadixSortBinding.h"
#include "IECorePython/AngleConversionBinding.h"
#include "IECorePython/SplineBinding.h"
#include "IECorePython/SplineDataBinding.h"
#include "IECorePython/ObjectVectorBinding.h"
#include "IECorePython/HenyeyGreensteinBinding.h"
#include "IECorePython/OversamplesCalculatorBinding.h"
#include "IECorePython/DateTimeDataBinding.h"
#include "IECorePython/FrameListBinding.h"
#include "IECorePython/EmptyFrameListBinding.h"
#include "IECorePython/FrameRangeBinding.h"
#include "IECorePython/CompoundFrameListBinding.h"
#include "IECorePython/ReorderedFrameListBinding.h"
#include "IECorePython/BinaryFrameListBinding.h"
#include "IECorePython/ReversedFrameListBinding.h"
#include "IECorePython/ExclusionFrameListBinding.h"
#include "IECorePython/FrameListParameterBinding.h"
#include "IECorePython/FileSequenceBinding.h"
#include "IECorePython/FileSequenceFunctionsBinding.h"
#include "IECorePython/FileSequenceParameterBinding.h"
#include "IECorePython/FileSequenceVectorParameterBinding.h"
#include "IECorePython/LevenbergMarquardtBinding.h"
#include "IECorePython/PointDistributionBinding.h"
#include "IECorePython/HexConversionBinding.h"
#include "IECorePython/LookupBinding.h"
#include "IECorePython/CamelCaseBinding.h"
#include "IECorePython/LRUCacheBinding.h"
#include "IECorePython/DataInterleaveOpBinding.h"
#include "IECorePython/DataConvertOpBinding.h"
#include "IECorePython/MurmurHashBinding.h"
#include "IECorePython/ImfBinding.h"
#include "IECorePython/TimeCodeDataBinding.h"
#include "IECorePython/LensModelBinding.h"
#include "IECorePython/StandardRadialLensModelBinding.h"
#include "IECorePython/ObjectPoolBinding.h"
#include "IECorePython/DataAlgoBinding.h"
#include "IECorePython/BoxAlgoBinding.h"
#include "IECorePython/RandomAlgoBinding.h"
#include "IECorePython/StringAlgoBinding.h"
#include "IECorePython/PathMatcherBinding.h"
#include "IECorePython/CancellerBinding.h"
#include "IECorePython/IndexedIOAlgoBinding.h"

#include "IECore/IECore.h"

using namespace IECorePython;
using namespace IECorePythonModule;
using namespace boost::python;


namespace
{

bool isDebug()
{
#ifdef NDEBUG
	return false;
#else
	return true;
#endif
}

std::string defaultRepr( object &o )
{
	return extract<std::string>( o.attr( "__repr__" )() );
}

} // namespace

// Module declaration

BOOST_PYTHON_MODULE(_IECore)
{
	bindRefCounted();
	bindRunTimeTyped();
	bindException();
	bindKDTree();
	bindObject();
	bindCompoundObject();
	bindTypeId();
	bindData();
	bindGeometricTypedData();
	bindAllSimpleTypedData();
	bindAllVectorTypedData();
	bindCompoundData();
	bindIndexedIO();
	bindMessageHandler();
	bindParameterised();
	bindOp();
	bindReader();
	bindBlindDataHolder();
	bindWriter();
	bindObjectReader();
	bindObjectWriter();
	bindParameter();
	bindNumericParameter();
	bindSimpleTypedParameter();
	bindVecTypedParameter();
	bindColorTypedParameter();
	bindBoxTypedParameter();
	bindMatrixTypedParameter();
	bindLineTypedParameter();
	bindVectorTypedParameter();
	bindSplineParameter();
	bindDateTimeParameter();
	bindCompoundParameter();
	bindValidatedStringParameter();
	bindPathParameter();
	bindFileNameParameter();
	bindDirNameParameter();
	bindPerlinNoise();
	bindHalf();
	bindTimer();
	bindTurbulence();
	bindSearchPath();
	bindCachedReader();
	bindObjectParameter();
	bindModifyOp();
	bindNullObject();
	bindObjectInterpolator();
	bindOversamplesCalculator();
	bindTransformationMatrix();
	bindTransformationMatrixData();
	bindBoundedKDTree();
	bindVectorDataFilterOp();
	bindTypedObjectParameter();
	bindHeaderGenerator();
	bindDataCastOp();
	bindDataPromoteOp();
	bindMatrixMultiplyOp();
	bindRandomRotationOp();
	bindInternedString();
	bindInverseDistanceWeightedInterpolation();
	bindMath();
	bindPathVectorParameter();
	bindTriangleAlgo();
	bindConverter();
	bindFromCoreConverter();
	bindLineSegment();
	bindCubicBasis();
	bindBezierAlgo();
	bindToCoreConverter();
	bindPolygonAlgo();
	bindUnicodeToString();
	bindRadixSort();
	bindAngleConversion();
	bindSpline();
	bindSplineData();
	bindObjectVector();
	bindHenyeyGreenstein();
	bindDateTimeData();
	bindFrameList();
	bindEmptyFrameList();
	bindFrameRange();
	bindCompoundFrameList();
	bindReorderedFrameList();
	bindBinaryFrameList();
	bindReversedFrameList();
	bindExclusionFrameList();
	bindFrameListParameter();
	bindFileSequence();
	bindFileSequenceFunctions();
	bindFileSequenceParameter();
	bindFileSequenceVectorParameter();
	bindLevenbergMarquardt();
	bindPointDistribution();
	bindHexConversion();
	bindLookup();
	bindCamelCase();
	bindLRUCache();
	bindDataInterleaveOp();
	bindDataConvertOp();
	bindMurmurHash();
	bindImf();
	bindTimeCodeData();
	bindLensModel();
	bindStandardRadialLensModel();
	bindObjectPool();
	bindDataAlgo();
	bindBoxAlgo();
	bindRandomAlgo();
	bindStringAlgo();
	bindPathMatcher();
	bindCanceller();
	bindIndexedIOAlgo();
	bindTBB();

	def( "majorVersion", &IECore::majorVersion );
	def( "minorVersion", &IECore::minorVersion );
	def( "patchVersion", &IECore::patchVersion );
	def( "versionString", &IECore::versionString, return_value_policy<copy_const_reference>() );
	def( "isDebug", &::isDebug );
	def( "withFreeType", &IECore::withFreeType );
	def( "initThreads", &PyEval_InitThreads );

	// Expose our own implementation of `repr()` for all the Imath
	// types, along with a fallback version for all other types. This
	// gives us a way of reliably round-tripping values through
	// `eval( repr( value) )`, which is needed in Gaffer's serialisation
	// among other places. The standard imath versions aren't suitable
	// for this because they don't include the module prefix.

	def( "repr", &defaultRepr );
	def( "repr", &repr<Imath::V2i> );
	def( "repr", &repr<Imath::V2f> );
	def( "repr", &repr<Imath::V2d> );
	def( "repr", &repr<Imath::V3i> );
	def( "repr", &repr<Imath::V3f> );
	def( "repr", &repr<Imath::V3d> );
	def( "repr", &repr<Imath::Box2i> );
	def( "repr", &repr<Imath::Box3i> );
	def( "repr", &repr<Imath::Box2f> );
	def( "repr", &repr<Imath::Box3f> );
	def( "repr", &repr<Imath::Box2d> );
	def( "repr", &repr<Imath::Box3d> );
	def( "repr", &repr<Imath::Color3f> );
	def( "repr", &repr<Imath::Color4f> );
	def( "repr", &repr<Imath::Eulerf> );
	def( "repr", &repr<Imath::Eulerd> );
	def( "repr", &repr<Imath::M33f> );
	def( "repr", &repr<Imath::M33d> );
	def( "repr", &repr<Imath::M44f> );
	def( "repr", &repr<Imath::M44d> );
	def( "repr", &repr<Imath::Plane3f> );
	def( "repr", &repr<Imath::Plane3d> );
	def( "repr", &repr<Imath::Quatf> );
	def( "repr", &repr<Imath::Quatd> );

}
