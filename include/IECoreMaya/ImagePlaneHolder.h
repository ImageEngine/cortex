//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREMAYA_IMAGEPLANEHOLDER_H
#define IECOREMAYA_IMAGEPLANEHOLDER_H

#include "IECore/Op.h"

#include "IECoreMaya/ParameterisedHolder.h"

#include "maya/MPxImagePlane.h"

namespace IECoreMaya
{

/// A class which holds an Op, expected to return an ImagePrimitive, which is then placed onto an ImagePlane.
class ImagePlaneHolder : public ParameterisedHolderImagePlane
{
	public :

		ImagePlaneHolder();
		virtual ~ImagePlaneHolder();

		virtual void postConstructor();

		static void *creator();
		static MStatus initialize();
		static MTypeId id;

		virtual MStatus setDependentsDirty( const MPlug &plug, MPlugArray &plugArray );
		virtual MStatus loadImageMap ( const MString &fileName, int frame, MImage &image );

		/// Calls setParameterised( className, classVersion, "IECORE_OP_PATHS" ).
		MStatus setOp( const std::string &className, int classVersion );
		/// Returns runTimeCast<Op>( getParameterised( className, classVersion ) ).
		IECore::OpPtr getOp( std::string *className = 0, int *classVersion = 0 );

};

} // namespace IECoreMaya

#endif // IECOREMAYA_IMAGEPLANEHOLDER_H
