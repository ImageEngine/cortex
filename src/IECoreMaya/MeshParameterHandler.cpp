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

#include "IECoreMaya/Parameter.h"

#include "IECoreMaya/ToMayaObjectConverter.h"
#include "IECoreMaya/FromMayaObjectConverter.h"
#include "IECoreMaya/FromMayaMeshConverter.h"
#include "IECoreMaya/MeshParameterHandler.h"

#include "IECore/TypedPrimitiveParameter.h"
#include "IECore/MeshPrimitive.h"

#include "maya/MFnGenericAttribute.h"
#include "maya/MFnMeshData.h"

using namespace IECoreMaya;
using namespace Imath;
using namespace boost;

static ParameterHandler::Description< MeshParameterHandler > registrar( IECore::MeshPrimitiveParameter::staticTypeId(), IECore::MeshPrimitive::staticTypeId() );

MStatus MeshParameterHandler::update( IECore::ConstParameterPtr parameter, MObject &attribute ) const
{
	IECore::ConstObjectParameterPtr p = IECore::runTimeCast<const IECore::ObjectParameter>( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	MFnGenericAttribute fnGAttr( attribute );
	if( !fnGAttr.hasObj( attribute ) )
	{
		return MS::kFailure;
	}

	fnGAttr.addAccept( MFnData::kMesh );

	return MS::kSuccess;
}

MObject MeshParameterHandler::create( IECore::ConstParameterPtr parameter, const MString &attributeName ) const
{
	IECore::ConstObjectParameterPtr p = IECore::runTimeCast<const IECore::ObjectParameter>( parameter );
	if( !p )
	{
		return MObject::kNullObj;
	}

	/// Use a generic attribute, so we could eventually accept other ObjectParamter types, too.
	MFnGenericAttribute fnGAttr;
	MObject result = fnGAttr.create( attributeName, attributeName );

	if ( !update( parameter, result ) )
	{
		return MObject::kNullObj;
	}

	return result;
}

MStatus MeshParameterHandler::setValue( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	IECore::ConstObjectParameterPtr p = IECore::runTimeCast<const IECore::ObjectParameter>( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	MFnMeshData fnData;
	MObject data = fnData.create();

	/// \todo Pull in userData from parameter to set up conversion parameters
	ToMayaObjectConverterPtr converter = ToMayaObjectConverter::create( p->getValue(), MFn::kMeshData );
	assert(converter);
	bool conversionSuccess = converter->convert( data );

	if ( !conversionSuccess )
	{
		return MS::kFailure;
	}

	/// \todo It seems like this can occassionally fail, usually with an empty mesh, but sometimes not. Try to establish exactly why.
	plug.setValue( data );

	return MS::kSuccess;
}

MStatus MeshParameterHandler::setValue( const MPlug &plug, IECore::ParameterPtr parameter ) const
{
	IECore::ObjectParameterPtr p = IECore::runTimeCast<IECore::ObjectParameter>( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	MObject v;
	MStatus result = plug.getValue( v );
	if( result )
	{
		/// \todo Pull in userData from parameter to set up conversion parameters
		FromMayaMeshConverterPtr converter = boost::dynamic_pointer_cast< FromMayaMeshConverter > ( FromMayaObjectConverter::create( v, IECore::MeshPrimitive::staticTypeId() ) );
		assert(converter);

		converter->spaceParameter()->setNumericValue( (int)FromMayaMeshConverter::World );
		p->setValue( converter->convert() );
	}
	return result;
}
