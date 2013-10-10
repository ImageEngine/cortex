//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2012, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREMAYA_GEOMETRYCOMBINER_H
#define IECOREMAYA_GEOMETRYCOMBINER_H

#include "maya/MPxNode.h"

namespace IECoreMaya
{

/// The GeometryCombiner node takes multiple maya geometry inputs and
/// combines them into a single IECore::Group output.
class GeometryCombiner : public MPxNode
{

	public :

		GeometryCombiner();
		virtual ~GeometryCombiner();

		static void *creator();
		static MStatus initialize();
		static const MTypeId id;
		static const MString typeName;
		
		virtual MStatus compute( const MPlug &plug, MDataBlock &dataBlock );

		static MObject aConvertPrimVars;
		static MObject aConvertBlindData;
		static MObject aBlindDataAttrPrefix;
		static MObject aConversionSpace;
		static MObject aInputGeometry;
		static MObject aOutputGroup;

};

} // namespace IECoreMaya

#endif // IECOREMAYA_GEOMETRYCOMBINER_H
