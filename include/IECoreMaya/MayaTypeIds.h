//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREMAYA_MAYATYPEIDS_H
#define IECOREMAYA_MAYATYPEIDS_H

namespace IECoreMaya
{

/// An enum for all the MTypeId values used by
/// the nodes and datatypes of IECoreMaya. Note that these
/// are maya type ids and are distinct from the IECore::TypeId
/// enumeration. The range here was obtained by Andrew Chapman
/// and is set aside specifically for the Cortex project.
enum MayaTypeId
{

	CacheSetId = 0x00110DC0,
	ObjectDataId = 0x00110DC1,
	ParameterisedHolderLocatorId = 0x00110DC2,
	ParameterisedHolderDeformerId = 0x00110DC3,
	ParameterisedHolderFieldId = 0x00110DC4,
	ParameterisedHolderSetId = 0x00110DC5,
	OpHolderNodeId = 0x00110DC6,
	ConverterHolderId = 0x00110DC7,
	ParameterisedHolderSurfaceShapeId = 0x00110DC8,
	ParameterisedHolderComponentShapeId = 0x00110DC9,
	ParameterisedHolderNodeId = 0x00110DCA,
	ProceduralHolderId = 0x00110DCB,
	TransientParameterisedHolderNodeId = 0x00110DCC,
	ParameterisedHolderImagePlaneId = 0x00110DCD,
	ImagePlaneHolderId = 0x00110DCE,

	/// Don't forget to update MayaTypeIdsBinding.cpp

	LastId = 0x00110E3F,

};

} // namespace IECoreMaya

#endif // IECOREMAYA_MAYATYPEIDS_H
