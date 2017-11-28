//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_TYPEIDS_H
#define IE_CORE_TYPEIDS_H

namespace IECore
{

enum TypeId
{
	InvalidTypeId = 0,
	ObjectTypeId = 1,
	DataTypeId = 2,
	FloatVectorDataTypeId = 3,
	DoubleVectorDataTypeId = 4,
	IntVectorDataTypeId = 5,
	UIntVectorDataTypeId = 6,
	CharVectorDataTypeId = 7,
	UCharVectorDataTypeId = 8,
	V2fVectorDataTypeId = 9,
	V2dVectorDataTypeId = 10,
	V3fVectorDataTypeId = 11,
	V3dVectorDataTypeId = 12,
	Box3fVectorDataTypeId = 13,
	Box3dVectorDataTypeId = 14,
	M33fVectorDataTypeId = 15,
	M33dVectorDataTypeId = 16,
	M44fVectorDataTypeId = 17,
	M44dVectorDataTypeId = 18,
	QuatfVectorDataTypeId = 19,
	QuatdVectorDataTypeId = 20,
	StringVectorDataTypeId = 21,
	FloatDataTypeId = 22,
	DoubleDataTypeId = 23,
	IntDataTypeId = 24,
	LongDataTypeId = 25, /// Obsolete: LongData has been removed. The typeId remains for compatibility with old files, which now load as IntData
	UIntDataTypeId = 26,
	CharDataTypeId = 27,
	UCharDataTypeId = 28,
	StringDataTypeId = 29,
	LongVectorDataTypeId = 30, /// Obsolete: LongVectorData has been removed. The typeId remains for compatibility with old files, which now load as IntVectorData
	CompoundDataTypeId = 31,
	V2fDataTypeId = 32,
	V3fDataTypeId = 33,
	V2dDataTypeId = 34,
	V3dDataTypeId = 35,
	Box2fDataTypeId = 36,
	Box3fDataTypeId = 37,
	Box2dDataTypeId = 38,
	Box3dDataTypeId = 39,
	M44fDataTypeId = 40,
	M44dDataTypeId = 41,
	QuatfDataTypeId = 42,
	QuatdDataTypeId = 43,
	Color3fDataTypeId = 44,
	Color4fDataTypeId = 45,
	Color3dDataTypeId = 46,
	Color4dDataTypeId = 47,
	Color3fVectorDataTypeId = 48,
	Color4fVectorDataTypeId = 49,
	Color3dVectorDataTypeId = 50,
	Color4dVectorDataTypeId = 51,
	BlindDataHolderTypeId = 52,
	RenderableTypeId = 53, // obsolete - available for reuse
	ParameterListTypeId = 54, // Obsolete
	CompoundObjectTypeId = 55,
	M33fDataTypeId = 56,
	M33dDataTypeId = 57,
	Box2fVectorDataTypeId = 58,
	Box2dVectorDataTypeId = 59,
	BoolDataTypeId = 60,
	PrimitiveTypeId = 61, // obsolete - available for reuse
	PointsPrimitiveTypeId = 62, // obsolete - available for reuse
	ImagePrimitiveTypeId = 63, // obsolete - available for reuse
	Box2iDataTypeId = 64,
	HalfVectorDataTypeId = 65,
	V2iDataTypeId = 66,
	MeshPrimitiveTypeId = 67, // obsolete - available for reuse
	ShaderTypeId = 68, // obsolete - available for reuse
	RunTimeTypedTypeId = 69,
	ParameterTypeId = 70,
	CompoundParameterTypeId = 71,
	StringParameterTypeId = 72,
	ValidatedStringParameterTypeId = 73,
	FileNameParameterTypeId = 74,
	IntParameterTypeId = 75,
	FloatParameterTypeId = 76,
	DoubleParameterTypeId = 77,
	BoolParameterTypeId = 78,
	V2fParameterTypeId = 79,
	V3fParameterTypeId = 80,
	V2dParameterTypeId = 81,
	V3dParameterTypeId = 82,
	Color3fParameterTypeId = 83,
	Color4fParameterTypeId = 84,
	Box2iParameterTypeId = 85,
	Box2fParameterTypeId = 86,
	Box3fParameterTypeId = 87,
	Box2dParameterTypeId = 88,
	Box3dParameterTypeId = 89,
	M44fParameterTypeId = 90,
	M44dParameterTypeId = 91,
	IntVectorParameterTypeId = 92,
	FloatVectorParameterTypeId = 93,
	DoubleVectorParameterTypeId = 94,
	StringVectorParameterTypeId = 95,
	V2fVectorParameterTypeId = 96,
	V3fVectorParameterTypeId = 97,
	V2dVectorParameterTypeId = 98,
	V3dVectorParameterTypeId = 99,
	Box3fVectorParameterTypeId = 100,
	Box3dVectorParameterTypeId = 101,
	M33fVectorParameterTypeId = 102,
	M44fVectorParameterTypeId = 103,
	M33dVectorParameterTypeId = 104,
	M44dVectorParameterTypeId = 105,
	QuatfVectorParameterTypeId = 106,
	QuatdVectorParameterTypeId = 107,
	Color3fVectorParameterTypeId = 108,
	Color4fVectorParameterTypeId = 109,
	NullObjectTypeId = 110,
	ParameterisedTypeId = 111,
	OpTypeId = 112,
	ReaderTypeId = 113,
	WriterTypeId = 114,
	ImageReaderTypeId = 115, // obsolete - available for reuse
	ImageWriterTypeId = 116, // obsolete - available for reuse
	CINImageReaderTypeId = 117, // obsolete - available for reuse
	CINImageWriterTypeId = 118, // obsolete - available for reuse
	EXRImageReaderTypeId = 119, // obsolete - available for reuse
	EXRImageWriterTypeId = 120, // obsolete - available for reuse
	JPEGImageReaderTypeId = 121, // obsolete - available for reuse
	JPEGImageWriterTypeId = 122, // obsolete - available for reuse
	TIFFImageReaderTypeId = 123, // obsolete - available for reuse
	TIFFImageWriterTypeId = 124, // obsolete - available for reuse
	ObjectReaderTypeId = 125,
	ObjectWriterTypeId = 126,
	PDCParticleReaderTypeId = 127, // obsolete - available for reuse
	PDCParticleWriterTypeId = 128, // obsolete - available for reuse
	PathParameterTypeId = 129,
	DirNameParameterTypeId = 130,
	V3iDataTypeId = 131,
	RendererTypeId = 132, // obsolete - available for reuse
	Box3iDataTypeId = 133,
	ObjectParameterTypeId = 134,
	ModifyOpTypeId = 135,
	ImageOpTypeId = 136,
	PrimitiveOpTypeId = 137, // obsolete - available for reuse
	ProceduralTypeId = 138, // Obsolete
	Box3iParameterTypeId = 139,
	V2iParameterTypeId = 140,
	V3iParameterTypeId = 141,
	ParticleReaderTypeId = 142, // obsolete - available for reuse
	ParticleWriterTypeId = 143, // obsolete - available for reuse
	MotionPrimitiveTypeId = 144, // obsolete - available for reuse
	DPXImageReaderTypeId = 145, // obsolete - available for reuse
	TransformTypeId = 146, // obsolete - available for reuse
	MatrixTransformTypeId = 147, // obsolete - available for reuse
	MotionTransformTypeId = 148, // obsolete - available for reuse
	MatrixMotionTransformTypeId = 149, // obsolete - available for reuse
	GroupTypeId = 150, // obsolete - available for reuse
	AttributeStateTypeId = 151, // obsolete - available for reuse
	VisibleRenderableTypeId = 152, // obsolete - available for reuse
	StateRenderableTypeId = 153, // obsolete - available for reuse
	OBJReaderTypeId = 154, // obsolete - available for reuse
	TransformationMatrixfDataTypeId = 155,
	TransformationMatrixdDataTypeId = 156,
	PointNormalsOpTypeId = 157, // obsolete - available for reuse
	PointDensitiesOpTypeId = 158, // obsolete - available for reuse
	DPXImageWriterTypeId = 159, // obsolete - available for reuse
	BoolVectorDataTypeId = 160,
	VectorDataFilterOpTypeId = 161,
	RenderableParameterTypeId = 162, // obsolete - available for reuse
	StateRenderableParameterTypeId = 163, // obsolete - available for reuse
	AttributeStateParameterTypeId = 164, // obsolete - available for reuse
	ShaderParameterTypeId = 165, // obsolete - available for reuse
	TransformParameterTypeId = 166, // obsolete - available for reuse
	MatrixMotionTransformParameterTypeId = 167, // obsolete - available for reuse
	MatrixTransformParameterTypeId = 168, // obsolete - available for reuse
	VisibleRenderableParameterTypeId = 169, // obsolete - available for reuse
	GroupParameterTypeId = 170, // obsolete - available for reuse
	MotionPrimitiveParameterTypeId = 171, // obsolete - available for reuse
	PrimitiveParameterTypeId = 172, // obsolete - available for reuse
	ImagePrimitiveParameterTypeId = 173, // obsolete - available for reuse
	MeshPrimitiveParameterTypeId = 174, // obsolete - available for reuse
	PointsPrimitiveParameterTypeId = 175, // obsolete - available for reuse
	PreWorldRenderableTypeId = 176, // obsolete - available for reuse
	CameraTypeId = 177, // obsolete - available for reuse
	NURBSPrimitiveTypeId = 178, // obsolete - available for reuse
	DataCastOpTypeId = 179,
	DataPromoteOpTypeId = 180,
	MatrixMultiplyOpTypeId = 181,
	PointBoundsOpTypeId = 182, // obsolete - available for reuse
	RandomRotationOpTypeId = 183, // obsolete - available for reuse
	V2iVectorDataTypeId = 184,
	V3iVectorDataTypeId = 185,
	ClippingPlaneTypeId = 186, // obsolete - available for reuse
	ParticleMeshOpTypeId = 187, // obsolete - available for reuse
	HalfDataTypeId = 188,
	MeshPrimitiveOpTypeId = 189, // obsolete - available for reuse
	PrimitiveEvaluatorTypeId = 190, // obsolete - available for reuse
	MeshPrimitiveEvaluatorTypeId = 191, // obsolete - available for reuse
	MeshPrimitiveImplicitSurfaceOpTypeId = 192, // obsolete - available for reuse
	TriangulateOpTypeId = 193, // obsolete - available for reuse
	SpherePrimitiveEvaluatorTypeId = 194, // obsolete - available for reuse
	SpherePrimitiveTypeId = 195, // obsolete - available for reuse
	ConverterTypeId = 196,
	ToCoreConverterTypeId = 197,
	ImageCropOpTypeId = 198, // obsolete - available for reuse
	MeshPrimitiveShrinkWrapOpTypeId = 199, // obsolete - available for reuse
	ImagePrimitiveEvaluatorTypeId = 200, // obsolete - available for reuse
	FromCoreConverterTypeId = 201,
	ShortDataTypeId = 202,
	UShortDataTypeId = 203,
	ShortVectorDataTypeId = 204,
	UShortVectorDataTypeId = 205,
	PathVectorParameterTypeId = 206,
	ColorTransformOpTypeId = 207, // obsolete - available for reuse
	TransformOpTypeId = 208, // obsolete - available for reuse
	ImageDiffOpTypeId = 209, // obsolete - available for reuse
	CurvesPrimitiveTypeId = 210, // obsolete - available for reuse
	CoordinateSystemTypeId = 211, // obsolete - available for reuse
	MeshNormalsOpTypeId = 212, // obsolete - available for reuse
	MeshMergeOpTypeId = 213, // obsolete - available for reuse
	FontTypeId = 214, // obsolete - available for reuse
	UniformRandomPointDistributionOpTypeId = 215, // obsolete - available for reuse
	Int64DataTypeId = 216,
	UInt64DataTypeId = 217,
	Int64VectorDataTypeId = 218,
	UInt64VectorDataTypeId = 219,
	MappedRandomPointDistributionOpTypeId = 220, // obsolete - available for reuse
	PointRepulsionOpTypeId = 221, // obsolete - available for reuse
	LuminanceOpTypeId = 222, // obsolete - available for reuse
	ImagePrimitiveOpTypeId = 223, // obsolete - available for reuse
	ChannelOpTypeId = 224, // obsolete - available for reuse
	SummedAreaOpTypeId = 225, // obsolete - available for reuse
	GradeTypeId = 226, // obsolete - available for reuse
	Box2iVectorDataTypeId = 227,
	Box3iVectorDataTypeId = 228,
	MedianCutSamplerTypeId = 229, // obsolete - available for reuse
	EnvMapSamplerTypeId = 230, // obsolete - available for reuse
	MeshVertexReorderOpTypeId = 231, // obsolete - available for reuse
	SplineffDataTypeId = 232,
	SplineddDataTypeId = 233,
	SplinefColor3fDataTypeId = 234,
	SplinefColor4fDataTypeId = 235,
	SplineffParameterTypeId = 236,
	SplineddParameterTypeId = 237,
	SplinefColor3fParameterTypeId = 238,
	SplinefColor4fParameterTypeId = 239,
	CompoundObjectParameterTypeId = 240,
	DisplayDriverTypeId = 241, // obsolete - available for reuse
	DisplayDriverCreatorTypeId = 242, // obsolete - available for reuse
	ImageDisplayDriverTypeId = 243, // obsolete - available for reuse
	DisplayDriverServerTypeId = 244, // obsolete - available for reuse
	ClientDisplayDriverTypeId = 245, // obsolete - available for reuse
	SplineToImageTypeId = 246, // obsolete - available for reuse
	DisplayTypeId = 247, // obsolete - available for reuse
	MeshTangentsOpTypeId = 248, // obsolete - available for reuse
	WarpOpTypeId = 249, // obsolete - available for reuse
	UVDistortOpTypeId = 250, // obsolete - available for reuse
	LinearToSRGBOpTypeId = 251, // obsolete - available for reuse
	SRGBToLinearOpTypeId = 252, // obsolete - available for reuse
	LinearToCineonOpTypeId = 253, // obsolete - available for reuse
	CineonToLinearOpTypeId = 254, // obsolete - available for reuse
	CubeColorTransformOpTypeId = 255, // obsolete - available for reuse
	CubeColorLookupfDataTypeId = 256, // obsolete - available for reuse
	CubeColorLookupdDataTypeId = 257, // obsolete - available for reuse
	CubeColorLookupfParameterTypeId = 258, // obsolete - available for reuse
	CubeColorLookupdParameterTypeId = 259, // obsolete - available for reuse
	BoolVectorParameterTypeId = 260,
	LinearToRec709OpTypeId = 261, // obsolete - available for reuse
	Rec709ToLinearOpTypeId = 262, // obsolete - available for reuse
	ObjectVectorTypeId = 263,
	ObjectVectorParameterTypeId = 264,
	YUVImageWriterTypeId = 265,
	ImageCompositeOpTypeId = 266, // obsolete - available for reuse
	ImagePremultiplyOpTypeId = 267, // obsolete - available for reuse
	ImageUnpremultiplyOpTypeId = 268, // obsolete - available for reuse
	DateTimeDataTypeId = 269,
	DateTimeParameterTypeId = 270,
	SGIImageReaderTypeId = 271, // obsolete - available for reuse
	TimeDurationDataTypeId = 272,
	TimeDurationParameterTypeId = 273,
	TimePeriodDataTypeId = 274,
	TimePeriodParameterTypeId = 275,
	PatchMeshPrimitiveTypeId = 276, // obsolete - available for reuse
	CurvesPrimitiveParameterTypeId = 277, // obsolete - available for reuse
	CurveExtrudeOpTypeId = 278, // obsolete - available for reuse
	FrameListTypeId = 279,
	EmptyFrameListTypeId = 280,
	FrameRangeTypeId = 281,
	CompoundFrameListTypeId = 282,
	ReorderedFrameListTypeId = 283,
	BinaryFrameListTypeId = 284,
	ReversedFrameListTypeId = 285,
	ExclusionFrameListTypeId = 286,
	FrameListParameterTypeId = 287,
	FileSequenceTypeId = 288,
	FileSequenceParameterTypeId = 289,
	FileSequenceVectorParameterTypeId = 290,
	ParameterisedProceduralTypeId = 291, // obsolete - available for reuse
	ColorSpaceTransformOpTypeId = 292, // obsolete - available for reuse
	TGAImageReaderTypeId = 293, // obsolete - available for reuse
	TGAImageWriterTypeId = 294, // obsolete - available for reuse
	BINParticleReaderTypeId = 295, // obsolete - available for reuse
	BINParticleWriterTypeId = 296, // obsolete - available for reuse
	BINMeshReaderTypeId = 297, // obsolete - available for reuse
	BGEOParticleReaderTypeId = 298, // obsolete - available for reuse
	NParticleReaderTypeId = 299, // obsolete - available for reuse
	IFFImageReaderTypeId = 300, // obsolete - available for reuse
	IFFHairReaderTypeId = 301, // obsolete - available for reuse
	FaceAreaOpTypeId = 302, // obsolete - available for reuse
	CurvesMergeOpTypeId = 303, // obsolete - available for reuse
	CurvesPrimitiveOpTypeId = 304, // obsolete - available for reuse
	CurvesPrimitiveEvaluatorTypeId = 305, // obsolete - available for reuse
	HdrMergeOpTypeId = 306, // obsolete - available for reuse
	HitMissTransformTypeId = 307, // obsolete - available for reuse
	CurveTracerTypeId = 308, // obsolete - available for reuse
	ImageThinnerTypeId = 309, // obsolete - available for reuse
	CurveLineariserTypeId = 310, // obsolete - available for reuse
	CompoundDataBaseTypeId = 311,
	ImageConvolveOpTypeId = 312, // obsolete - available for reuse
	ClassParameterTypeId = 313,
	ClassVectorParameterTypeId = 314,
	CurveTangentsOpTypeId = 315, // obsolete - available for reuse
	MarschnerParameterTypeId = 316, // obsolete - available for reuse
	MarschnerLookupTableOpTypeId = 317, // obsolete - available for reuse
	SmoothSkinningDataTypeId = 318, // obsolete - available for reuse
	FaceVaryingPromotionOpTypeId = 319, // obsolete - available for reuse
	MeshDistortionsOpTypeId = 320, // obsolete - available for reuse
	PointVelocityDisplaceOpTypeId = 321, // obsolete - available for reuse
	SmoothSkinningDataParameterTypeId = 322, // obsolete - available for reuse
	CompressSmoothSkinningDataOpTypeId = 323, // obsolete - available for reuse
	DecompressSmoothSkinningDataOpTypeId = 324, // obsolete - available for reuse
	NormalizeSmoothSkinningWeightsOpTypeId = 325, // obsolete - available for reuse
	ReorderSmoothSkinningInfluencesOpTypeId = 326, // obsolete - available for reuse
	RemoveSmoothSkinningInfluencesOpTypeId = 327, // obsolete - available for reuse
	SmoothSmoothSkinningWeightsOpTypeId = 328, // obsolete - available for reuse
	MixSmoothSkinningWeightsOpTypeId = 329, // obsolete - available for reuse
	PointSmoothSkinningOpTypeId = 330, // obsolete - available for reuse
	AddSmoothSkinningInfluencesOpTypeId = 331, // obsolete - available for reuse
	LimitSmoothSkinningInfluencesOpTypeId = 332, // obsolete - available for reuse
	PointsPrimitiveEvaluatorTypeId = 333, // obsolete - available for reuse
	TransformationMatrixfParameterTypeId = 334,
	TransformationMatrixdParameterTypeId = 335,
	PointsMotionOpTypeId = 336, // obsolete - available for reuse
	CapturingRendererTypeId = 337, // obsolete - available for reuse
	LinearToPanalogOpTypeId = 338, // obsolete - available for reuse
	PanalogToLinearOpTypeId = 339, // obsolete - available for reuse
	EnvMapSHProjectorTypeId = 340, // obsolete - available for reuse
	LightTypeId = 341, // obsolete - available for reuse
	ContrastSmoothSkinningWeightsOpTypeId = 342, // obsolete - available for reuse
	PointDistributionOpTypeId = 343, // obsolete - available for reuse
	LineSegment3fDataTypeId = 344,
	LineSegment3dDataTypeId = 345,
	LineSegment3fParameterTypeId = 346,
	LineSegment3dParameterTypeId = 347,
	DataInterleaveOpTypeId = 348,
	DataConvertOpTypeId = 349,
	PNGImageReaderTypeId = 350, // obsolete - available for reuse
	DeepImageReaderTypeId = 351, // obsolete - available for reuse
	DeepImageWriterTypeId = 352, // obsolete - available for reuse
	DeepImageConverterTypeId = 353, // obsolete - available for reuse
	V2iVectorParameterTypeId = 354,
	V3iVectorParameterTypeId = 355,
	DiskPrimitiveTypeId = 356, // obsolete - available for reuse
	LinearToAlexaLogcOpTypeId = 357, // obsolete - available for reuse
	AlexaLogcToLinearOpTypeId = 358, // obsolete - available for reuse
	ClampOpTypeId = 359, // obsolete - available for reuse
	MeshFaceFilterOpTypeId = 360, // obsolete - available for reuse
	TimeCodeDataTypeId = 361,
	TimeCodeParameterTypeId = 362,
	OptionsTypeId = 363, // obsolete - available for reuse
	MPlayDisplayDriverTypeId = 364, // obsolete - available for reuse
	SceneInterfaceTypeId = 365, // obsolete - available for reuse
	SampledSceneInterfaceTypeId = 366, // obsolete - available for reuse
	SceneCacheTypeId = 367, // obsolete - available for reuse
	IndexedIOTypeId = 368,
	StreamIndexedIOTypeId = 369,
	FileIndexedIOTypeId = 370,
	MemoryIndexedIOTypeId = 371,
	InternedStringVectorDataTypeId = 372,
	InternedStringDataTypeId = 373,
	LinkedSceneTypeId = 374, // obsolete - available for reuse
	V2fDataBaseTypeId = 375,
	V2dDataBaseTypeId = 376,
	V2iDataBaseTypeId = 377,
	V3fDataBaseTypeId = 378,
	V3dDataBaseTypeId = 379,
	V3iDataBaseTypeId = 380,
	V2fVectorDataBaseTypeId = 381,
	V2dVectorDataBaseTypeId = 382,
	V2iVectorDataBaseTypeId = 383,
	V3fVectorDataBaseTypeId = 384,
	V3dVectorDataBaseTypeId = 385,
	V3iVectorDataBaseTypeId = 386,
	LensModelTypeId = 387,
	StandardRadialLensModelTypeId = 388,
	LensDistortOpTypeId = 389, // obsolete - available for reuse
	TransferSmoothSkinningWeightsOpTypeId = 390, // obsolete - available for reuse
	EXRDeepImageReaderTypeId = 391, // obsolete - available for reuse
	EXRDeepImageWriterTypeId = 392, // obsolete - available for reuse
	ExternalProceduralTypeId = 393, // obsolete - available for reuse

	// Remember to update TypeIdBinding.cpp !!!

	// If we ever get this far then the core library is too big.
	LastCoreTypeId = 99999,
	// All RunTimeTyped derived classes in extension
	// libraries should use a TypeId in the following range.
	// Don't put the TypeId in here. For python derived classes use
	// the registerTypeId function in RunTimeTypedUtil.py to register the
	// TypeId into the python TypeId enum and check for conflicts.
	FirstExtensionTypeId = 100000,

	FirstCoreImageTypeId = 104000,
	LastCoreImageTypeId = 104999,

	FirstCoreGLTypeId = 105000,
	LastCoreGLTypeId = 105999,

	FirstCoreRITypeId = 106000,
	LastCoreRITypeId = 106999,

	FirstCoreNukeTypeId = 107000,
	LastCoreNukeTypeId = 107999,

	FirstCoreSceneTypeId = 108000,
	LastCoreSceneTypeId = 108999,

	FirstCoreMayaTypeId = 109000,
	LastCoreMayaTypeId = 109999,

	FirstGafferTypeId = 110000,
	LastGafferTypeId = 110999,

	FirstCoreHoudiniTypeId = 111000,
	LastCoreHoudiniTypeId = 111999,

	FirstCoreAlembicTypeId = 112000,
	LastCoreAlembicTypeId = 112999,

	FirstCoreMantraTypeId = 113000,
	LastCoreMantraTypeId = 113999,

	FirstCoreArnoldTypeId = 114000,
	LastCoreArnoldTypeId = 114999,

	FirstCoreAppleseedTypeId = 115000,
	LastCoreAppleseedTypeId = 115999,

	FirstCoreUSDTypeId = 116000,
	LastCoreUSDTypeId = 116999,

	// TypeIds dynamically allocated by registerRunTimeTyped (IECore Python)
	FirstDynamicTypeId = 300000,
	LastDynamicTypeId = 399999,

	LastExtensionTypeId = 399999,
	// Any TypeIds beyond this point can be considered safe for private internal use.

};

} // namespace IECore

#endif // IE_CORE_TYPEIDS_H
