//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2011 Image Engine Design Inc. All rights reserved.
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

#include "IECoreMaya/ToMayaObjectConverter.h"
#include "IECoreMaya/FromMayaObjectConverter.h"
#include "IECoreMaya/FromMayaMeshConverter.h"
#include "IECoreMaya/MeshParameterHandler.h"
#include "IECoreMaya/MayaTypeIds.h"
#include "IECoreMaya/ObjectData.h"

#include "IECoreScene/TypedPrimitiveParameter.h"
#include "IECoreScene/MeshPrimitive.h"

#include "maya/MFnGenericAttribute.h"
#include "maya/MFnMeshData.h"

using namespace IECoreMaya;
using namespace Imath;
using namespace boost;

static ParameterHandler::Description< MeshParameterHandler > registrar( IECoreScene::MeshPrimitiveParameter::staticTypeId(), IECoreScene::MeshPrimitive::staticTypeId() );

MStatus MeshParameterHandler::doUpdate( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	IECore::ConstObjectParameterPtr p = IECore::runTimeCast<const IECore::ObjectParameter>( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	MObject attribute = plug.attribute();
	MFnGenericAttribute fnGAttr( attribute );
	if( !fnGAttr.hasObj( attribute ) )
	{
		return MS::kFailure;
	}

	fnGAttr.addDataAccept( MFnData::kMesh );
	// maya has an odd behaviour whereby a generic attribute with only one accepted datatype will
	// transform itself into a typed attribute after file save and load. here we add an accept
	// for a second dummy datatype to ensure that the attribute will still be a generic attribute
	// when saved and loaded.
	fnGAttr.addAccept( DummyDataId );

	return finishUpdating( parameter, plug );
}

MPlug MeshParameterHandler::doCreate( IECore::ConstParameterPtr parameter, const MString &plugName, MObject &node ) const
{
	IECore::ConstObjectParameterPtr p = IECore::runTimeCast<const IECore::ObjectParameter>( parameter );
	if( !p )
	{
		return MPlug();
	}

	/// Use a generic attribute, so we could eventually accept other ObjectParamter types, too.
	MFnGenericAttribute fnGAttr;
	MObject attribute = fnGAttr.create( plugName, plugName );

	MPlug result = finishCreating( parameter, attribute, node );
	doUpdate( parameter, result );

	return result;
}

MStatus MeshParameterHandler::doSetValue( IECore::ConstParameterPtr parameter, MPlug &plug ) const
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

MStatus MeshParameterHandler::doSetValue( const MPlug &plug, IECore::ParameterPtr parameter ) const
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
		FromMayaMeshConverterPtr converter = boost::dynamic_pointer_cast< FromMayaMeshConverter > ( FromMayaObjectConverter::create( v, IECoreScene::MeshPrimitive::staticTypeId() ) );
		assert(converter);

		converter->spaceParameter()->setNumericValue( (int)FromMayaMeshConverter::World );
		p->setValue( converter->convert() );
		return MS::kSuccess;
	}
	else
	{
		// technically we should be returning the error status here, but we don't
		// as this case appears to be pretty common, and the resulting errors tend to be
		// annoying rather than helpful.
		//
		// the failure to get the plug value appears to be because we don't save
		// empty mesh values to file (see ParameterisedHolder::shouldSave).
		// this means that when the file is loaded again the plug.getValue()
		// method will fail and we end up here. the best thing we can do in this
		// case seems to be to assume that the value should be an empty mesh.
		p->setValue( new IECoreScene::MeshPrimitive() );
		return MS::kSuccess;
	}
}
