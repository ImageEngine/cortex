//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_COREMAYA_MAYAMESHSIGNEDDISTANCEFUNCTION_H
#define IE_COREMAYA_MAYAMESHSIGNEDDISTANCEFUNCTION_H

#include "maya/MFnMesh.h"
#include "maya/MPoint.h"
#include "maya/MVector.h"

#include "OpenEXR/ImathVec.h"

#include "IECore/ImplicitSurfaceFunction.h"
#include "IECoreMaya/VectorTraits.h"


namespace IECoreMaya
{

/// A model of IEcore::ImplicitSurfaceFunction for creating a signed distance field with respect to a Maya mesh.
class MayaMeshSignedDistanceFunction : public IECore::ImplicitSurfaceFunction< Imath::V3d, double >
{
        public:
                typedef boost::intrusive_ptr<MayaMeshSignedDistanceFunction> Ptr;
                typedef boost::intrusive_ptr<const MayaMeshSignedDistanceFunction> ConstPtr;
		
		//// Create the function. The passed MObject must be compatible with the MFnMesh function set
		MayaMeshSignedDistanceFunction( const MObject &obj, MSpace::Space space = MSpace::kWorld );
		
		virtual ~MayaMeshSignedDistanceFunction();
                
		// Retrieve the signed distance from the mesh at the given point              
                Value operator()( const Point &p );
		
		// Retrieve the signed distance from the mesh at the given point		
		virtual Value getValue( const Point &p );

		
	protected:	
	
		MFnMesh *m_fnMesh;
		MSpace::Space m_space;
};

}

#endif // IE_COREMAYA_MAYAMESHSIGNEDDISTANCEFUNCTION_H
