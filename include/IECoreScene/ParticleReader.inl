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

#ifndef IE_CORE_PARTICLEREADER_INL
#define IE_CORE_PARTICLEREADER_INL

#include "OpenEXR/ImathRandom.h"

#include "IECore/MessageHandler.h"
#include "IECore/Convert.h"

namespace IECoreScene
{

template<typename T, typename F >
typename T::Ptr ParticleReader::filterAttr( const F *attr, float percentage, const IECore::Data *idAttr ) const
{
	if( percentage < 100.0f )
	{
		if( idAttr )
		{
			if ( idAttr->typeId() == IECore::DoubleVectorDataTypeId )
			{
				return filterAttr< T, F, double >( attr, percentage, static_cast< const IECore::DoubleVectorData * >( idAttr )->readable() );
			}
			else if ( idAttr->typeId() == IECore::IntVectorDataTypeId )
			{
				return filterAttr< T, F, int >( attr, percentage, static_cast< const IECore::IntVectorData * >( idAttr )->readable() );
			}
			else
			{
				msg( IECore::Msg::Warning, "ParticleReader::filterAttr", boost::format( "Unrecognized id data type in file \"%s\"! Disabling filtering." ) % fileName() );
			}
		}
		else
		{
			// apply filtering only based on order
			// percentage filtering (and type conversion if necessary)
			typename T::Ptr result( new T );
			const typename F::ValueType &in = attr->readable();
			typename T::ValueType &out = result->writable();
			float fraction = percentage / 100.0f;
			out.reserve( std::min((size_t)1, (size_t)(in.size() * fraction) ) );
			int seed = particlePercentageSeed();
			Imath::Rand48 r;
			r.init( seed );
			for( typename F::ValueType::size_type i=0; i<in.size(); i++ )
			{
				if( r.nextf() <= fraction )
				{
					out.push_back( IECore::convert< typename T::ValueType::value_type, typename F::ValueType::value_type >( in[i] ) );
				}
			}
			return result;
		}
	}

	if( T::staticTypeId()!=F::staticTypeId() )
	{
		// type conversion only
		typename T::Ptr result( new T );
		const typename F::ValueType &in = attr->readable();
		typename T::ValueType &out = result->writable();
		out.resize( in.size() );
		transform( in.begin(), in.end(), out.begin(), IECore::convert< typename T::ValueType::value_type, typename F::ValueType::value_type > );
		return result;
	}

	// no filtering of any sort needed
	return typename T::Ptr( (T *)attr );
}

template<typename T, typename F, typename U >
typename T::Ptr ParticleReader::filterAttr( const F *attr, float percentage, const std::vector< U > &ids ) const
{
	// percentage filtering based on id (and type conversion if necessary)
	typename T::Ptr result( new T );
	const typename F::ValueType &in = attr->readable();
	typename T::ValueType &out = result->writable();
	int seed = particlePercentageSeed();
	float fraction = percentage / 100.0f;
	Imath::Rand48 r;
	for( typename F::ValueType::size_type i=0; i<in.size(); i++ )
	{
		r.init( seed + (int)ids[i] );
		if( r.nextf() <= fraction )
		{
			out.push_back( IECore::convert< typename T::ValueType::value_type, typename F::ValueType::value_type >( in[i] ) );
		}
	}
	return result;
}

} // namespace IECoreScene

#endif // IE_CORE_PARTICLEREADER_INL
