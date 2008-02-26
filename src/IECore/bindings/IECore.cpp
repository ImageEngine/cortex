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

// System includes

// External includes
#include <boost/python.hpp>

// Local includes
#include "IECore/bindings/RefCountedBinding.h"
#include "IECore/bindings/RunTimeTypedBinding.h"
#include "IECore/bindings/ExceptionBinding.h"
#include "IECore/bindings/ImathBinding.h"
#include "IECore/bindings/KDTreeBinding.h"
#include "IECore/bindings/IndexedIOInterfaceBinding.h"
#include "IECore/bindings/DataBinding.h"
#include "IECore/bindings/SimpleTypedDataBinding.h"
#include "IECore/bindings/VectorTypedDataBinding.h"
#include "IECore/bindings/ObjectBinding.h"
#include "IECore/bindings/TypeIdBinding.h"
#include "IECore/bindings/CompoundDataBinding.h"
#include "IECore/bindings/MessageHandlerBinding.h"
#include "IECore/bindings/AttributeCacheBinding.h"
#include "IECore/bindings/ReaderBinding.h"
#include "IECore/bindings/ParticleReaderBinding.h"
#include "IECore/bindings/PDCParticleReaderBinding.h"
#include "IECore/bindings/BlindDataHolderBinding.h"
#include "IECore/bindings/RenderableBinding.h"
#include "IECore/bindings/VisibleRenderableBinding.h"
#include "IECore/bindings/StateRenderableBinding.h"
#include "IECore/bindings/RendererBinding.h"
#include "IECore/bindings/WriterBinding.h"
#include "IECore/bindings/ParticleWriterBinding.h"
#include "IECore/bindings/PDCParticleWriterBinding.h"
#include "IECore/bindings/ParameterBinding.h"
#include "IECore/bindings/NumericParameterBinding.h"
#include "IECore/bindings/TypedParameterBinding.h"
#include "IECore/bindings/CompoundParameterBinding.h"
#include "IECore/bindings/ValidatedStringParameterBinding.h"
#include "IECore/bindings/PathParameterBinding.h"
#include "IECore/bindings/FileNameParameterBinding.h"
#include "IECore/bindings/DirNameParameterBinding.h"
#include "IECore/bindings/CompoundObjectBinding.h"
#include "IECore/bindings/ObjectReaderBinding.h"
#include "IECore/bindings/ObjectWriterBinding.h"
#include "IECore/bindings/PrimitiveBinding.h"
#include "IECore/bindings/PrimitiveVariableBinding.h"
#include "IECore/bindings/PointsPrimitiveBinding.h"
#include "IECore/bindings/TimerBinding.h"
#include "IECore/bindings/TurbulenceBinding.h"
#include "IECore/bindings/ShaderBinding.h"
#include "IECore/bindings/SearchPathBinding.h"
#include "IECore/bindings/CachedReaderBinding.h"
#include "IECore/bindings/ParameterisedBinding.h"
#include "IECore/bindings/OpBinding.h"
#include "IECore/bindings/ObjectParameterBinding.h"
#include "IECore/bindings/ModifyOpBinding.h"
#include "IECore/bindings/PrimitiveOpBinding.h"
#include "IECore/bindings/ImagePrimitiveBinding.h"
#include "IECore/bindings/ImageReaderBinding.h"
#include "IECore/bindings/ImageWriterBinding.h"
#include "IECore/bindings/PerlinNoiseBinding.h"
#include "IECore/bindings/EXRImageReaderBinding.h"
#include "IECore/bindings/EXRImageWriterBinding.h"
#include "IECore/bindings/HalfBinding.h"
#include "IECore/bindings/TIFFImageReaderBinding.h"
#include "IECore/bindings/TIFFImageWriterBinding.h"
#include "IECore/bindings/CINImageReaderBinding.h"
#include "IECore/bindings/CINImageWriterBinding.h"
#include "IECore/bindings/DPXImageReaderBinding.h"
#include "IECore/bindings/DPXImageWriterBinding.h"
#include "IECore/bindings/JPEGImageReaderBinding.h"
#include "IECore/bindings/JPEGImageWriterBinding.h"
#include "IECore/bindings/MeshPrimitiveBinding.h"
#include "IECore/bindings/MotionPrimitiveBinding.h"
#include "IECore/bindings/TransformBinding.h"
#include "IECore/bindings/MatrixTransformBinding.h"
#include "IECore/bindings/GroupBinding.h"
#include "IECore/bindings/AttributeStateBinding.h"
#include "IECore/bindings/MatrixMotionTransformBinding.h"
#include "IECore/bindings/OBJReaderBinding.h"
#include "IECore/bindings/NullObjectBinding.h"
#include "IECore/bindings/ObjectInterpolatorBinding.h"
#include "IECore/bindings/PointNormalsOpBinding.h"
#include "IECore/bindings/PointDensitiesOpBinding.h"
#include "IECore/bindings/InterpolatedCacheBinding.h"
#include "IECore/bindings/TransformationMatrixBinding.h"
#include "IECore/bindings/TransformationMatrixDataBinding.h"
#include "IECore/bindings/HierarchicalCacheBinding.h"
#include "IECore/bindings/BoundedKDTreeBinding.h"
#include "IECore/bindings/VectorDataFilterOpBinding.h"
#include "IECore/bindings/TypedObjectParameterBinding.h"
#include "IECore/bindings/HeaderGeneratorBinding.h"
#include "IECore/bindings/PreWorldRenderableBinding.h"
#include "IECore/bindings/CameraBinding.h"
#include "IECore/bindings/NURBSPrimitiveBinding.h"
#include "IECore/bindings/DataCastOpBinding.h"
#include "IECore/bindings/DataPromoteOpBinding.h"
#include "IECore/bindings/MatrixMultiplyOpBinding.h"
#include "IECore/bindings/PointBoundsOpBinding.h"
#include "IECore/bindings/ImathRandomBinding.h"
#include "IECore/bindings/RandomRotationOpBinding.h"
#include "IECore/bindings/ImplicitSurfaceFunctionBinding.h"
#include "IECore/bindings/CachedImplicitSurfaceFunctionBinding.h"
#include "IECore/bindings/MeshPrimitiveBuilderBinding.h"
#include "IECore/bindings/MarchingCubesBinding.h"
#include "IECore/bindings/PointMeshOpBinding.h"
#include "IECore/bindings/CSGImplicitSurfaceFunctionBinding.h"
#include "IECore/bindings/SphereImplicitSurfaceFunctionBinding.h"
#include "IECore/bindings/PlaneImplicitSurfaceFunctionBinding.h"
#include "IECore/bindings/BlobbyImplicitSurfaceFunctionBinding.h"
#include "IECore/bindings/ParticleMeshOpBinding.h"
#include "IECore/bindings/TypedPrimitiveOpBinding.h"
#include "IECore/bindings/PrimitiveEvaluatorBinding.h"
#include "IECore/bindings/MeshPrimitiveEvaluatorBinding.h"
#include "IECore/bindings/PrimitiveImplicitSurfaceFunctionBinding.h"
#include "IECore/bindings/MeshPrimitiveImplicitSurfaceFunctionBinding.h"
#include "IECore/bindings/MeshPrimitiveImplicitSurfaceOpBinding.h"
#include "IECore/bindings/TriangulateOpBinding.h"
#include "IECore/bindings/SpherePrimitiveBinding.h"
#include "IECore/bindings/SpherePrimitiveEvaluatorBinding.h"
#include "IECore/bindings/InverseDistanceWeightedInterpolationBinding.h"
#include "IECore/bindings/ImageCropOpBinding.h"
#include "IECore/bindings/MeshPrimitiveShrinkWrapOpBinding.h"
#include "IECore/bindings/ImagePrimitiveEvaluatorBinding.h"
#include "IECore/IECore.h"

using namespace IECore;
using namespace boost::python;

// Module declaration

BOOST_PYTHON_MODULE(_IECore)
{
	bindRefCounted();
	bindRunTimeTyped();
	bindException();
	bindImath();
	bindKDTree();
	bindObject();
	bindTypeId();
	bindData();
	bindAllSimpleTypedData();
	bindAllVectorTypedData();
	bindCompoundData();
	bindIndexedIO();
	bindMessageHandler();	
	bindAttributeCache();
	bindParameterised();
	bindOp();
	bindReader();	
	bindParticleReader();	
	bindPDCParticleReader();	
	bindBlindDataHolder();	
	bindRenderable();	
	bindStateRenderable();	
	bindVisibleRenderable();	
	bindRenderer();
	bindWriter();	
	bindParticleWriter();	
	bindPDCParticleWriter();
	bindObjectReader();
	bindObjectWriter();	
	bindParameter();	
	bindNumericParameter();	
	bindTypedParameter();	
	bindCompoundParameter();	
	bindValidatedStringParameter();	
	bindPathParameter();	
	bindFileNameParameter();	
	bindDirNameParameter();	
	bindCompoundObject();	
	bindPrimitive();	
	bindPrimitiveVariable();	
	bindPointsPrimitive();	
	bindPerlinNoise();	
	bindHalf();
	bindTimer();
	bindTurbulence();
	bindShader();
	bindSearchPath();
	bindCachedReader();
	bindObjectParameter();
	bindModifyOp();
	bindPrimitiveOp();
	bindImagePrimitive();
	bindImageReader();
	bindImageWriter();
	bindEXRImageReader();
	bindEXRImageWriter();
	
#ifdef IECORE_WITH_TIFF	
	bindTIFFImageReader();
	bindTIFFImageWriter();
#endif
	
	bindCINImageReader();
	bindCINImageWriter();
	bindDPXImageReader();
	bindDPXImageWriter();

#ifdef IECORE_WITH_JPEG
	bindJPEGImageReader();
	bindJPEGImageWriter();
#endif

	bindMeshPrimitive();
	bindMotionPrimitive();
	bindTransform();
	bindMatrixTransform();
	bindMatrixMotionTransform();
	bindGroup();
	bindAttributeState();
	bindOBJReader();
	bindNullObject();
	bindObjectInterpolator();
	bindPointNormalsOp();
	bindPointDensitiesOp();
	bindInterpolatedCache();
	bindTransformationMatrix();
	bindTransformationMatrixData();
	bindHierarchicalCache();
	bindBoundedKDTree();
	bindVectorDataFilterOp();
	bindTypedObjectParameter();	
	bindHeaderGenerator();
	bindPreWorldRenderable();
	bindCamera();
	bindNURBSPrimitive();
	bindDataCastOp();
	bindDataPromoteOp();
	bindMatrixMultiplyOp();
	bindPointBoundsOp();
	bindImathRandom();
	bindRandomRotationOp();
	bindImplicitSurfaceFunction();	
	bindCachedImplicitSurfaceFunction();	
	bindMeshPrimitiveBuilder();
	bindMarchingCubes();
	bindPointMeshOp();
	bindCSGImplicitSurfaceFunction();
	bindSphereImplicitSurfaceFunction();
	bindPlaneImplicitSurfaceFunction();
	bindBlobbyImplicitSurfaceFunction();		
	bindParticleMeshOp();
	bindTypedPrimitiveOp();	
	bindPrimitiveEvaluator();
	bindMeshPrimitiveEvaluator();
	bindPrimitiveImplicitSurfaceFunction();	
	bindMeshPrimitiveImplicitSurfaceFunction();		
	bindMeshPrimitiveImplicitSurfaceOp();	
	bindTriangulateOp();
	bindSpherePrimitive();
	bindSpherePrimitiveEvaluator();
	bindInverseDistanceWeightedInterpolation();
	bindImageCropOp();
	bindMeshPrimitiveShrinkWrapOp();
	bindImagePrimitiveEvaluator();
		
	def( "majorVersion", &IECore::majorVersion );
	def( "minorVersion", &IECore::minorVersion );
	def( "patchVersion", &IECore::patchVersion );
	def( "versionString", &IECore::versionString, return_value_policy<copy_const_reference>() );
	def( "withTIFF", &IECore::withTIFF );
	def( "withJPEG", &IECore::withJPEG );
	def( "withSQLite", &IECore::withSQLite );
}
