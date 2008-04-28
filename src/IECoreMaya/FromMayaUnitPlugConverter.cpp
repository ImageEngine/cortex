//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#include "IECoreMaya/FromMayaUnitPlugConverter.h"

#include "IECore/CompoundParameter.h"

#include "maya/MAngle.h"
#include "maya/MDistance.h"
#include "maya/MTime.h"

#include "boost/type_traits/is_same.hpp"

namespace IECoreMaya
{

template<typename T> 
FromMayaPlugConverter::Description<FromMayaUnitPlugConverter<T> > FromMayaUnitPlugConverter<T>::m_angleDescription( MFnUnitAttribute::kAngle, IECore::TypedData<T>::staticTypeId(), boost::is_same<T, double>::value );
template<typename T> 
FromMayaPlugConverter::Description<FromMayaUnitPlugConverter<T> > FromMayaUnitPlugConverter<T>::m_distanceDescription( MFnUnitAttribute::kDistance, IECore::TypedData<T>::staticTypeId(), boost::is_same<T, double>::value );
template<typename T> 
FromMayaPlugConverter::Description<FromMayaUnitPlugConverter<T> > FromMayaUnitPlugConverter<T>::m_timeDescription( MFnUnitAttribute::kTime, IECore::TypedData<T>::staticTypeId(), boost::is_same<T, double>::value );

template<typename T> 
FromMayaUnitPlugConverter<T>::FromMayaUnitPlugConverter( const MPlug &plug )
	:	FromMayaPlugConverter( plug )
{
	IECore::IntParameter::PresetsMap anglePresets;
	anglePresets["Radians"] = MAngle::kRadians;	
	anglePresets["Degrees"] = MAngle::kDegrees;			
	m_angleUnitParameter = new IECore::IntParameter(
		"angleUnit",
		"The unit in which angular values are returned.",
		MAngle::kRadians,
		MAngle::kRadians,
		MAngle::kDegrees,
		anglePresets,
		true
	);
	
	IECore::IntParameter::PresetsMap distancePresets;
	distancePresets["Inches"] = MDistance::kInches;
	distancePresets["Feet"] = MDistance::kFeet;
	distancePresets["Yards"] = MDistance::kYards;
	distancePresets["Miles"] = MDistance::kMiles;
	distancePresets["Millimeters"] = MDistance::kMillimeters;
	distancePresets["Centimeters"] = MDistance::kCentimeters;
	distancePresets["Meters"] = MDistance::kMeters;
	distancePresets["Kilometers"] = MDistance::kKilometers;
	m_distanceUnitParameter = new IECore::IntParameter(
		"distanceUnit",
		"The unit in which distance values are returned.",
		MDistance::kCentimeters,
		MDistance::kInches,
		MDistance::kMeters,
		distancePresets,
		true
	);
	
	IECore::IntParameter::PresetsMap timePresets;
	timePresets["Hours"] = MTime::kHours;
	timePresets["Minutes"] = MTime::kMinutes;
	timePresets["Seconds"] = MTime::kSeconds;
	timePresets["Milliseconds"] = MTime::kMilliseconds;
	
	m_timeUnitParameter = new IECore::IntParameter(
		"timeUnit",
		"The unit in which time values are returned.",
		MTime::kSeconds,
		MTime::kHours,
		MTime::kMilliseconds,
		timePresets,
		true
	);
	
	parameters()->addParameter( m_angleUnitParameter );
	parameters()->addParameter( m_distanceUnitParameter );
	parameters()->addParameter( m_timeUnitParameter );
	
}
		
template<typename T> 
IECore::IntParameterPtr FromMayaUnitPlugConverter<T>::angleUnitParameter()
{
	return m_angleUnitParameter;
}

template<typename T> 
IECore::ConstIntParameterPtr FromMayaUnitPlugConverter<T>::angleUnitParameter() const
{
	return m_angleUnitParameter;
}

template<typename T> 
IECore::IntParameterPtr FromMayaUnitPlugConverter<T>::distanceUnitParameter()
{
	return m_distanceUnitParameter;
}

template<typename T> 
IECore::ConstIntParameterPtr FromMayaUnitPlugConverter<T>::distanceUnitParameter() const
{
	return m_distanceUnitParameter;
}

template<typename T> 
IECore::IntParameterPtr FromMayaUnitPlugConverter<T>::timeUnitParameter()
{
	return m_timeUnitParameter;
}

template<typename T> 
IECore::ConstIntParameterPtr FromMayaUnitPlugConverter<T>::timeUnitParameter() const
{
	return m_timeUnitParameter;
}

template<typename T> 
IECore::ObjectPtr FromMayaUnitPlugConverter<T>::doConversion( IECore::ConstCompoundObjectPtr operands ) const
{
	typedef IECore::TypedData<T> ResultType;
	typedef typename IECore::TypedData<T>::Ptr ResultTypePtr;
	
	MObject attr = plug().attribute();
	MFnUnitAttribute fnUAttr( attr );
	switch( fnUAttr.unitType() )
	{
		case MFnUnitAttribute::kTime :
		{
			MTime t;
			plug().getValue( t );
			return new ResultType( t.as( (MTime::Unit)m_timeUnitParameter->getNumericValue() ) );
		}
		case MFnUnitAttribute::kDistance :
		{
			MDistance d;
			plug().getValue( d );
			return new ResultType( d.as( (MDistance::Unit)m_distanceUnitParameter->getNumericValue() ) );
		}
		case MFnUnitAttribute::kAngle :
		{
			MAngle a;
			plug().getValue( a );
			return new ResultType( a.as( (MAngle::Unit)m_angleUnitParameter->getNumericValue() ) );
		}
		default :
			return 0;
	}
}

// explicit instantiation
template class FromMayaUnitPlugConverter<float>;
template class FromMayaUnitPlugConverter<double>;

} // namespace IECoreMaya
