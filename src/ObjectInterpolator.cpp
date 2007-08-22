
#include "IECore/Object.h"
#include "IECore/ObjectInterpolator.h"

using namespace IECore;


ObjectPtr IECore::linearObjectInterpolation( const ObjectPtr &y0, const ObjectPtr &y1, double x )
{
	ObjectPtr result = Object::create( y0->typeId() );
	if( !ObjectInterpolator< LinearInterpolator >(y0, y1, x, result) )
	{
		result = 0;
	}
	return result;
}


ObjectPtr IECore::cosineObjectInterpolation( const ObjectPtr &y0, const ObjectPtr &y1, double x )
{
	ObjectPtr result = Object::create( y0->typeId() );
	if( !ObjectInterpolator< CosineInterpolator >(y0, y1, x, result) )
	{
		result = 0;
	}
	return result;
}


ObjectPtr IECore::cubicObjectInterpolation( const ObjectPtr &y0, const ObjectPtr &y1, const ObjectPtr &y2, const ObjectPtr &y3, double x )
{
	ObjectPtr result = Object::create( y0->typeId() );
	if( !ObjectInterpolator< CubicInterpolator >(y0, y1, y2, y3, x, result) )
	{
		result = 0;
	}
	return result;
}
