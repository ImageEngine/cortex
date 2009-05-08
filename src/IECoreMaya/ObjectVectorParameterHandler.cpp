#include "IECoreMaya/ObjectVectorParameterHandler.h"
#include "IECoreMaya/FromMayaPlugConverter.h"
#include "IECoreMaya/ObjectData.h"

#include "IECore/MessageHandler.h"
#include "IECore/TypedObjectParameter.h"
#include "IECore/ObjectVector.h"

#include "maya/MFnTypedAttribute.h"

using namespace std;
using namespace IECore;
using namespace IECoreMaya;

static ParameterHandler::Description< ObjectVectorParameterHandler > registrar( ObjectVectorParameter::staticTypeId() );

MObject ObjectVectorParameterHandler::create( IECore::ConstParameterPtr parameter, const MString &attributeName ) const
{
	ConstObjectVectorParameterPtr p = IECore::runTimeCast< const ObjectVectorParameter >( parameter );
	if( !p )
	{
		return MObject::kNullObj;
	}

	MFnTypedAttribute fnTAttr;
	MObject result = fnTAttr.create( attributeName, attributeName, ObjectData::id );
	fnTAttr.setArray( true );
	fnTAttr.setDisconnectBehavior( MFnAttribute::kDelete );

	return result;
}

MStatus ObjectVectorParameterHandler::update( IECore::ConstParameterPtr parameter, MObject &attribute ) const
{
	ConstObjectVectorParameterPtr p = IECore::runTimeCast< const ObjectVectorParameter >( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	MFnTypedAttribute fnTAttr( attribute );
	if( !fnTAttr.hasObj( attribute ) )
	{
		return MS::kFailure;
	}

	return MS::kSuccess;
}

MStatus ObjectVectorParameterHandler::setValue( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	ConstObjectVectorParameterPtr p = IECore::runTimeCast<const ObjectVectorParameter>( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	/// \todo Can we implement this?

	return MS::kSuccess;
}

MStatus ObjectVectorParameterHandler::setValue( const MPlug &plug, IECore::ParameterPtr parameter ) const
{
	ObjectVectorParameterPtr p = IECore::runTimeCast<ObjectVectorParameter>( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	MStatus s;

	ObjectVectorPtr v = new ObjectVector;

	unsigned numElements = const_cast<MPlug &>( plug ).evaluateNumElements();
	for( unsigned i=0; i<numElements; i++ )
	{
		MPlug elementPlug = plug.elementByPhysicalIndex( i, &s );
		assert( s );

		ObjectPtr obj = 0;
		FromMayaConverterPtr c = FromMayaPlugConverter::create( elementPlug, Object::staticTypeId() );
		if( c )
		{
			obj = c->convert();
		}

		v->members().resize( std::max<ObjectVector::MemberContainer::size_type>( elementPlug.logicalIndex() + 1, v->members().size() ), 0 );
		v->members()[elementPlug.logicalIndex()] = obj;
	}

	p->setValue( v );

	return MS::kSuccess;
}
