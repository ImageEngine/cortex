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
	UIntDataTypeId = 26,
	CharDataTypeId = 27,
	UCharDataTypeId = 28,
	StringDataTypeId = 29,
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
	Color3dDataTypeId = 46, // Obsolete
	Color4dDataTypeId = 47, // Obsolete
	Color3fVectorDataTypeId = 48,
	Color4fVectorDataTypeId = 49,
	Color3dVectorDataTypeId = 50, // Obsolete
	PathMatcherDataTypeId = 51,
	BlindDataHolderTypeId = 52,
	CompoundObjectTypeId = 55,
	M33fDataTypeId = 56,
	M33dDataTypeId = 57,
	Box2fVectorDataTypeId = 58,
	Box2dVectorDataTypeId = 59,
	BoolDataTypeId = 60,
	Box2iDataTypeId = 64,
	HalfVectorDataTypeId = 65,
	V2iDataTypeId = 66,
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
	ObjectReaderTypeId = 125,
	ObjectWriterTypeId = 126,
	PathParameterTypeId = 129,
	DirNameParameterTypeId = 130,
	V3iDataTypeId = 131,
	Box3iDataTypeId = 133,
	ObjectParameterTypeId = 134,
	ModifyOpTypeId = 135,
	ImageOpTypeId = 136,
	Box3iParameterTypeId = 139,
	V2iParameterTypeId = 140,
	V3iParameterTypeId = 141,
	TransformationMatrixfDataTypeId = 155,
	TransformationMatrixdDataTypeId = 156,
	BoolVectorDataTypeId = 160,
	VectorDataFilterOpTypeId = 161,
	DataCastOpTypeId = 179,
	DataPromoteOpTypeId = 180,
	MatrixMultiplyOpTypeId = 181,
	RandomRotationOpTypeId = 183,
	V2iVectorDataTypeId = 184,
	V3iVectorDataTypeId = 185,
	HalfDataTypeId = 188,
	ConverterTypeId = 196,
	ToCoreConverterTypeId = 197,
	FromCoreConverterTypeId = 201,
	ShortDataTypeId = 202,
	UShortDataTypeId = 203,
	ShortVectorDataTypeId = 204,
	UShortVectorDataTypeId = 205,
	PathVectorParameterTypeId = 206,
	Int64DataTypeId = 216,
	UInt64DataTypeId = 217,
	Int64VectorDataTypeId = 218,
	UInt64VectorDataTypeId = 219,
	Box2iVectorDataTypeId = 227,
	Box3iVectorDataTypeId = 228,
	SplineffDataTypeId = 232,
	SplineddDataTypeId = 233,
	SplinefColor3fDataTypeId = 234,
	SplinefColor4fDataTypeId = 235,
	SplineffParameterTypeId = 236,
	SplineddParameterTypeId = 237,
	SplinefColor3fParameterTypeId = 238,
	SplinefColor4fParameterTypeId = 239,
	CompoundObjectParameterTypeId = 240,
	BoolVectorParameterTypeId = 260,
	ObjectVectorTypeId = 263,
	ObjectVectorParameterTypeId = 264,
	YUVImageWriterTypeId = 265,
	DateTimeDataTypeId = 269,
	DateTimeParameterTypeId = 270,
	TimeDurationDataTypeId = 272,
	TimeDurationParameterTypeId = 273,
	TimePeriodDataTypeId = 274,
	TimePeriodParameterTypeId = 275,
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
	CompoundDataBaseTypeId = 311,
	ClassParameterTypeId = 313,
	ClassVectorParameterTypeId = 314,
	TransformationMatrixfParameterTypeId = 334,
	TransformationMatrixdParameterTypeId = 335,
	LineSegment3fDataTypeId = 344,
	LineSegment3dDataTypeId = 345,
	LineSegment3fParameterTypeId = 346,
	LineSegment3dParameterTypeId = 347,
	DataInterleaveOpTypeId = 348,
	DataConvertOpTypeId = 349,
	V2iVectorParameterTypeId = 354,
	V3iVectorParameterTypeId = 355,
	TimeCodeDataTypeId = 361,
	TimeCodeParameterTypeId = 362,
	IndexedIOTypeId = 368,
	StreamIndexedIOTypeId = 369,
	FileIndexedIOTypeId = 370,
	MemoryIndexedIOTypeId = 371,
	InternedStringVectorDataTypeId = 372,
	InternedStringDataTypeId = 373,
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
