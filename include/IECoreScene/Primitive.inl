//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORESCENE_PRIMITIVE_INL
#define IECORESCENE_PRIMITIVE_INL

namespace IECoreScene
{

template<typename T> boost::optional<PrimitiveVariable::IndexedView < typename T::ValueType::value_type> >
Primitive::variableIndexedView( const std::string &name, PrimitiveVariable::Interpolation requiredInterpolation, bool throwIfInvalid ) const
{
	PrimitiveVariableMap::const_iterator it = variables.find( name );
	if( it == variables.end() )
	{
		if( throwIfInvalid )
		{
			throw IECore::InvalidArgumentException( boost::str( boost::format( "Primitive::variableIndexedView - No primvar named '%1%' found" ) % name ) );
		}
		else
		{
			return boost::none;
		}
	}

	if( requiredInterpolation != PrimitiveVariable::Invalid && it->second.interpolation != requiredInterpolation )
	{
		if( throwIfInvalid )
		{
			throw IECore::InvalidArgumentException(
				boost::str(
					boost::format(
						"Primitive::variableIndexedView - PrimVar '%1%' interpolation (%2%) doesn't match requiredInterpolation (%3%)"
					) % name % it->second.interpolation % requiredInterpolation
				)
			);
		}
		else
		{
			return boost::none;
		}
	}

	const T *data = IECore::runTimeCast<const T>( it->second.data.get() );
	if( data )
	{
		return PrimitiveVariable::IndexedView<typename T::ValueType::value_type>(
			data->readable(),
			it->second.indices ? &it->second.indices->readable() : nullptr
		);
	}
	else if( throwIfInvalid )
	{
		throw IECore::InvalidArgumentException(
			boost::str(
				boost::format( "Primitive::variableIndexedView - Unable to created indexed view for '%1%' PrimVar, requested type: '%2%', actual type: '%3%'" ) %
					name %
					T::baseTypeName() %
					it->second.data->typeName()
			)
		);
	}
	else
	{
		return boost::none;
	}

	return boost::none; // gcc 4.8.1 incorrectly warns about a missing return here.
}

template<typename T>
T *Primitive::variableData( const std::string &name, PrimitiveVariable::Interpolation requiredInterpolation )
{
	PrimitiveVariableMap::const_iterator it = variables.find( name );
	if( it==variables.end() )
	{
		return nullptr;
	}
	if( requiredInterpolation!=PrimitiveVariable::Invalid && it->second.interpolation!=requiredInterpolation )
	{
		return nullptr;
	}
	if( it->second.indices )
	{
		throw IECore::Exception(
			boost::str(
				boost::format(
					"Primitive::variableData() can only be used for non-indexed variables. Use Primitive::expandedVariableData() or access Primitive::variables directly. Primitive variable name: '%1%'"
				) % name
			)
		);
	}

	return IECore::runTimeCast<T>( it->second.data.get() );
}

template<typename T>
const T *Primitive::variableData( const std::string &name, PrimitiveVariable::Interpolation requiredInterpolation ) const
{
	PrimitiveVariableMap::const_iterator it = variables.find( name );
	if( it==variables.end() )
	{
		return nullptr;
	}
	if( requiredInterpolation!=PrimitiveVariable::Invalid && it->second.interpolation!=requiredInterpolation )
	{
		return nullptr;
	}
	if( it->second.indices )
	{
		throw IECore::Exception(
			boost::str(
				boost::format(
					"Primitive::variableData() can only be used for non-indexed variables. Use Primitive::expandedVariableData() or access Primitive::variables directly. Primitive variable name: '%1%'"
				) % name
			)
		);
	}

	return IECore::runTimeCast<const T>( it->second.data.get() );
}

template<typename T>
typename T::Ptr Primitive::expandedVariableData( const std::string &name, PrimitiveVariable::Interpolation requiredInterpolation, bool throwIfInvalid /* = false */) const
{
	PrimitiveVariableMap::const_iterator it = variables.find( name );
	if( it==variables.end() )
	{
		if ( throwIfInvalid )
		{
			throw IECore::Exception( boost::str( boost::format( "Primitive::expandedVariableData() - Primitive Variable '%1%' not found." ) % name ) );
		}

		return nullptr;
	}
	if( requiredInterpolation!=PrimitiveVariable::Invalid && it->second.interpolation!=requiredInterpolation )
	{
		if( throwIfInvalid )
		{
			throw IECore::Exception(
				boost::str(
					boost::format( "Primitive::expandedVariableData() - Primitive Variable '%1%' has interpolation: %2%, required :%3%." ) %
						name %
						it->second.interpolation %
						requiredInterpolation
				)
			);
		}
		return nullptr;
	}

	T *data = IECore::runTimeCast<T>( it->second.data.get() );
	if( !data )
	{
		if( throwIfInvalid )
		{
			throw IECore::Exception(
				boost::str(
					boost::format( "Primitive::expandedVariableData() - Primitive Variable '%1%' has invalid data type: %2%, required :%3%." ) %
						name %
						it->second.data->typeName() %
						T::staticTypeName()
				)
			);
		}

		return nullptr;
	}

	return IECore::runTimeCast<T>( it->second.expandedData() );
}

} // namespace IECoreScene

#endif // IECORESCENE_PRIMITIVE_INL
