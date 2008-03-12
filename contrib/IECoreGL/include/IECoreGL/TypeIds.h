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

#ifndef IE_COREGL_TYPEIDS_H
#define IE_COREGL_TYPEIDS_H

namespace IECoreGL
{

enum TypeId
{
	StateTypeId = 105000,
	StateComponentTypeId = 105001,
	PrimitiveBoundTypeId = 105002,
	PrimitiveWireframeTypeId = 105003,
	PrimitiveSolidTypeId = 105004,
	PrimitiveOutlineTypeId = 105005,
	RenderableTypeId = 105006,
	BindableTypeId = 105007,
	TextureTypeId = 105008,
	RendererTypeId = 105009,
	ShaderTypeId = 105010,
	PrimitiveTypeId = 105011,
	PointsPrimitiveTypeId = 105012,
	DiskPrimitiveTypeId = 105013,
	MeshPrimitiveTypeId = 105014,
	QuadPrimitiveTypeId = 105015,
	SpherePrimitiveTypeId = 105016,
	BoxPrimitiveTypeId = 105017,
	RendererImplementationTypeId = 105018,
	DeferredRendererImplementationTypeId = 105019,
	PrimitiveWireframeWidthTypeId = 105020,
	PrimitiveOutlineWidthTypeId = 105021,
	PrimitivePointsTypeId = 105022,
	PrimitivePointWidthTypeId = 105023,
	ColorTypeId = 105024,
	ShaderStateComponentTypeId = 105025,
	FrameBufferTypeId = 105026,
	DepthTextureTypeId = 105027,
	ColorTextureTypeId = 105028,
	ImmediateRendererImplementationTypeId = 105029,
	BlendFuncStateComponentTypeId = 105030,
	BlendColorStateComponentTypeId = 105031,
	BlendEquationStateComponentTypeId = 105032,
	TransparentShadingStateComponentTypeId = 105033,
	PrimitiveTransparencySortStateComponentTypeId = 105034,
	BoundColorStateComponentTypeId = 105035,
	WireframeColorStateComponentTypeId = 105036,
	OutlineColorStateComponentTypeId = 105037,
	PointColorStateComponentTypeId = 105038,
	CameraTypeId = 105039,
	OrthographicCameraTypeId = 105040,
	PerspectiveCameraTypeId = 105041,
	PointsPrimitiveUseGLPointsTypeId = 105042,
	PointsPrimitiveGLPointWidthTypeId = 105043,
	NameStateComponentTypeId = 105044,
	ToGLConverterTypeId = 105045,
	ToGLCameraConverterTypeId = 105046,
	DoubleSidedStateComponentTypeId = 105047,
	/// If we ever get here we should start over again
	LastCoreGLTypeId = 105999,	
};	

} // namespace IECoreGL

#endif // IE_COREGL_TYPEIDS_H
