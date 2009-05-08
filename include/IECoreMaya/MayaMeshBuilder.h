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


#ifndef IE_COREMAYA_MAYAMESHBUILDER_H
#define IE_COREMAYA_MAYAMESHBUILDER_H

#include <cassert>

#include "maya/MObject.h"
#include "maya/MFnMesh.h"

#include "IECore/RefCounted.h"
#include "IECore/VectorTypedData.h"


namespace IECoreMaya
{

/// MayaMeshBuilder is a class which allows construction of Maya mesh data, templated
/// on the base type of the resulting point/normal data (e.g. float or double).
template<typename T>
class MayaMeshBuilder : public IECore::RefCounted
{
	public:

		typedef T BaseType;

		typedef boost::intrusive_ptr <MayaMeshBuilder<T> > Ptr;
		typedef boost::intrusive_ptr< const MayaMeshBuilder<T> > ConstPtr;

		MayaMeshBuilder( MObject parentOrOwner );
		virtual ~MayaMeshBuilder();

		/// Add a vertex position and normal
		void addVertex( const Imath::Vec3<T> &p, const Imath::Vec3<T> &n );

		/// Construct a triangle from the 3 specified vertex indices
		void addTriangle( int v0, int v1, int v2 );

		/// Retrieve the resultant mesh as MFnMeshData
		MObject mesh() const;

	protected:
		MObject m_parentOrOwner;

		struct Data;

		Data *m_data;

};

}

#include "IECoreMaya/MayaMeshBuilder.inl"

#endif // IE_COREMAYA_MAYAMESHBUILDER_H
