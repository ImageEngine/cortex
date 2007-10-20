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

#include <boost/python.hpp>

#include "IECore/MatrixMotionTransform.h"
#include "IECore/bindings/MatrixMotionTransformBinding.h"
#include "IECore/bindings/IntrusivePtrPatch.h"
#include "IECore/bindings/RunTimeTypedBinding.h"

using namespace boost::python;
using namespace Imath;

namespace IECore
{

static unsigned int len( MatrixMotionTransform &p )
{
	return p.snapshots().size();
}

static M44f getItem( MatrixMotionTransform &p, float t )
{
	MatrixMotionTransform::SnapshotMap::const_iterator it = p.snapshots().find( t );
	if( it==p.snapshots().end() )
	{
		throw std::out_of_range( "Bad index" );
	}
	return it->second;
}

static void setItem( MatrixMotionTransform &p, float t, const Imath::M44f &v )
{
	p.snapshots()[t] = v;
}

static bool contains( MatrixMotionTransform &p, float t )
{
	return p.snapshots().find( t )!=p.snapshots().end();
}

static void delItem( MatrixMotionTransform &p, float t )
{
	MatrixMotionTransform::SnapshotMap::iterator it = p.snapshots().find( t );
	if( it==p.snapshots().end() )
	{
		throw std::out_of_range( "Bad index" );
	}
	p.snapshots().erase( it );
}

static boost::python::list keys( MatrixMotionTransform &p )
{
	boost::python::list result;
	const MatrixMotionTransform::SnapshotMap &s = p.snapshots();
	MatrixMotionTransform::SnapshotMap::const_iterator it;
	for( it = s.begin(); it!=s.end(); it++ )
	{
		result.append( it->first );
	}
	return result;
}

static boost::python::list values( MatrixMotionTransform &p )
{
	boost::python::list result;
	const MatrixMotionTransform::SnapshotMap &s = p.snapshots();
	MatrixMotionTransform::SnapshotMap::const_iterator it;
	for( it = s.begin(); it!=s.end(); it++ )
	{
		result.append( it->second );
	}
	return result;
}


void bindMatrixMotionTransform()
{	
	typedef class_< MatrixMotionTransform, MatrixMotionTransformPtr, bases<Transform>, boost::noncopyable > MatrixMotionTransformPyClass;
	MatrixMotionTransformPyClass( "MatrixMotionTransform" )
		.def( "__len__", &len )
		.def( "__getitem__", &getItem )
		.def( "__setitem__", &setItem )
		.def( "__delitem__", &delItem )
		.def( "__contains__", &contains )
		.def( "keys", &keys )
		.def( "values", &values )
		.IE_COREPYTHON_DEFRUNTIMETYPEDSTATICMETHODS(MatrixMotionTransform)
	;
	INTRUSIVE_PTR_PATCH( MatrixMotionTransform, MatrixMotionTransformPyClass );
	implicitly_convertible<MatrixMotionTransformPtr, TransformPtr>();
}

}
