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

#ifndef IECOREIMAGE_TYPEIDS_H
#define IECOREIMAGE_TYPEIDS_H

namespace IECoreImage
{

enum TypeId
{
	FirstCoreImageTypeId = 104000,
	ImageReaderTypeId = 104001,
	ImageWriterTypeId = 104002,
	ChannelOpTypeId = 104003,
	ClampOpTypeId = 104004,
	CurveTracerTypeId = 104005,
	EnvMapSamplerTypeId = 104006,
	HdrMergeOpTypeId = 104007,
	ImageCropOpTypeId = 104008,
	ImageDiffOpTypeId = 104009,
	ImageThinnerTypeId = 104010,
	LensDistortOpTypeId = 104011,
	LuminanceOpTypeId = 104012,
	MedianCutSamplerTypeId = 104013,
	SplineToImageTypeId = 104014,
	SummedAreaOpTypeId = 104015,
	WarpOpTypeId = 104016,
	ImageDisplayDriverTypeId = 104017,
	ImagePrimitiveParameterTypeId = 104018,
	FontTypeId = 104019,
	LastCoreImageTypeId = 104999,
};

} // namespace IECoreImage

#endif // IECOREIMAGE_TYPEIDS_H
