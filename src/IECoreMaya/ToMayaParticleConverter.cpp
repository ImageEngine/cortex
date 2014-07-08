//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#include "maya/MSelectionList.h"
#include "maya/MDGModifier.h"
#include "maya/MGlobal.h"
#include "maya/MFnTypedAttribute.h"
#include "maya/MPlug.h"
#include "maya/MPointArray.h"

#include "IECore/PointsPrimitive.h"

#include "IECoreMaya/ToMayaParticleConverter.h"
#include "IECoreMaya/Convert.h"
#include "IECoreMaya/ToMayaObjectConverter.h"
#include "IECoreMaya/MArrayIter.h"

using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreMaya;

ToMayaParticleConverter::Description ToMayaParticleConverter::g_description( IECore::PointsPrimitive::staticTypeId(), MFn::kParticle );

ToMayaParticleConverter::ToMayaParticleConverter( IECore::ConstObjectPtr object )
: ToMayaObjectConverter( "Converts IECore::Primitive objects to Maya particle shapes.", object )
{
}

bool ToMayaParticleConverter::doConversion( IECore::ConstObjectPtr from, MObject &to, IECore::ConstCompoundObjectPtr operands ) const
{
	MStatus s;

	IECore::ConstPrimitivePtr primitive = IECore::runTimeCast<const IECore::Primitive>( from );
	if( !primitive )
	{
		return false;
	}

	if( !primitive->arePrimitiveVariablesValid() )
	{
		return false;
	}

	// get the position primitive variable and abort if it's not there
	ConstDataPtr p = primitive->variableData<Data>( "P", PrimitiveVariable::Vertex );
	if( !p )
	{
		// accept "position" so we can convert the results of the PDCParticleReader without having
		// to rename things
		p = primitive->variableData<Data>( "position", PrimitiveVariable::Vertex );
		if( !p )
		{
			return false;
		}
	}
	if( !p->isInstanceOf( V3fVectorDataTypeId ) && !p->isInstanceOf( V3dVectorDataTypeId ) )
	{
		return false;
	}
	
	// get/make a particle shape.
	// if we've been passed a particle system then we'll update it.
	// if we're passed a parent then we'll create one under it.
	// if we're passed nothing then we'll create our own parent.
	MFnParticleSystem fnPS;
	if( !fnPS.hasObj( to ) )
	{
		to = fnPS.create( to );
	}
	
	// emit a particle for each position. it is much faster to emit
	// them all at once from an array than to call the emit( MPoint ) method
	// many times.
		
	MPointArray mp( primitive->variableSize( PrimitiveVariable::Vertex ) );
	if( p->isInstanceOf( V3fVectorDataTypeId ) )
	{
		const vector<V3f> &pReadable = boost::static_pointer_cast<const V3fVectorData>( p )->readable();
		std::transform( pReadable.begin(), pReadable.end(), MArrayIter<MPointArray>::begin( mp ), IECore::convert<MPoint, V3f> );
	}
	else
	{
		const vector<V3d> &pReadable = boost::static_pointer_cast<const V3dVectorData>( p )->readable();
		std::transform( pReadable.begin(), pReadable.end(), MArrayIter<MPointArray>::begin( mp ), IECore::convert<MPoint, V3d> );
	}

	fnPS.emit( mp );
	
	// add all other vertex primvars as per-particle attributes
	
	for( PrimitiveVariableMap::const_iterator it=primitive->variables.begin(); it!=primitive->variables.end(); it++ )
	{
		if( it->second.interpolation!=PrimitiveVariable::Vertex )
		{
			continue;
		}
		if( it->first=="P" )
		{
			continue;
		}
		
		if( it->first=="Cs" )
		{
			addAttribute( it->second.data.get(), fnPS, "rgbPP" );
		}
		else
		{
			addAttribute( it->second.data.get(), fnPS, it->first.c_str() );
		}
	}
	
	// make sure it can be rendered.
	// it would perhaps be preferable to use MFnSet::addMember() instead but at the time of
	// writing (maya 2010) that seems to print out "Result : initialParticleSE" totally unnecessarily.
	MGlobal::executeCommand( "sets -addElement initialParticleSE " + fnPS.fullPathName() );
	
	// freeze everything
	fnPS.saveInitialState();
	
	// connect it to time so it can be used for simulation
	MSelectionList selection;
	selection.add( "time1" );
	MObject time1;
	selection.getDependNode( 0, time1 );
	MFnDependencyNode fnTime1( time1 );
	
	MDGModifier dgMod;
	dgMod.connect( fnTime1.findPlug( "outTime" ), fnPS.findPlug( "currentTime" ) );
	dgMod.doIt();

	return true;
}

void ToMayaParticleConverter::addAttribute( const IECore::Data *data, MFnParticleSystem &fnPS, const MString &attrName ) const
{
	MFnData::Type attrType = MFnData::kInvalid;
	MFn::Type dataType = MFn::kInvalid;
	switch( data->typeId() )
	{
		case V3fVectorDataTypeId :
		case V3dVectorDataTypeId :
		case Color3fVectorDataTypeId :
			attrType = MFnData::kVectorArray;
			dataType = MFn::kVectorArrayData;
			break;
		case FloatVectorDataTypeId :
			attrType = MFnData::kDoubleArray;
			dataType = MFn::kDoubleArrayData;
			break;
		default :
			return;
	}
		
	MPlug plug = fnPS.findPlug( attrName );
	if( plug.isNull() )
	{
		MFnTypedAttribute fnTA;
		fnPS.addAttribute( fnTA.create( attrName, attrName, attrType ) );
		fnPS.addAttribute( fnTA.create( attrName + "0", attrName + "0", attrType ) );
		plug = fnPS.findPlug( attrName );
	}

	MObject value;
	ToMayaObjectConverterPtr dataConverter = ToMayaObjectConverter::create( data, dataType );
	dataConverter->convert( value );
	plug.setValue( value );
}
