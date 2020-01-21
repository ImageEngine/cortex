//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2017, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREALEMBIC_PRIMITIVEREADER_INL
#define IECOREALEMBIC_PRIMITIVEREADER_INL

#include "IECoreAlembic/IGeomParamTraits.h"

namespace IECoreAlembic
{

namespace Private
{

// Functor for setting the geometric interpretation of
// DataType based on the GeomParam type. The base version
// does nothing because not all Data classes have a
// `setInterpretation()` method. We then specialise this
// below for the GeometricTypedData classes where the
// method is available.
//
// Note that we could use `DataAlgo::setGeometricInterpretation()`
// to do this for us, but by doing it ourselves we avoid the
// overhead of the internal TypedDataDespatch that it would perform.
template<typename DataType, typename GeomParam>
struct ApplyGeometricInterpretation
{

	static void apply( DataType *data )
	{
	};

};

template<typename T, typename GeomParam>
struct ApplyGeometricInterpretation<IECore::GeometricTypedData<T>, GeomParam>
{

	static void apply( IECore::GeometricTypedData<T> *data )
	{
		data->setInterpretation( IGeomParamTraits<GeomParam>::geometricInterpretation() );
	};

};

} // namespace Private

template<typename T>
void PrimitiveReader::readGeomParam( const T &param, const Alembic::Abc::ISampleSelector &sampleSelector, IECoreScene::Primitive *primitive ) const
{

	typedef typename T::prop_type::sample_ptr_type SamplePtr;
	typedef typename IGeomParamTraits<T>::DataType DataType;
	typedef typename T::sample_type GeomParamSample;

	if( param.getArrayExtent() > 1 )
	{
		IECore::msg( IECore::Msg::Warning, "FromAlembicGeomBaseConverter::convertArbGeomParam", boost::format( "Param \"%s\" has unsupported array extent" ) % param.getHeader().getName() );
		return;
	}

	SamplePtr sample;
	Alembic::Abc::UInt32ArraySamplePtr indices;

	if( param.isIndexed() )
	{
		GeomParamSample geomParamSample = param.getIndexedValue( sampleSelector );
		sample = geomParamSample.getVals();
		indices = geomParamSample.getIndices();
	}
	else
	{
		sample = param.getExpandedValue( sampleSelector ).getVals();
	}

	typename DataType::Ptr data = new DataType();
	data->writable().resize( sample->size() );
	std::copy( sample->get(), sample->get() + sample->size(), data->writable().begin() );

	Private::ApplyGeometricInterpretation<DataType, T>::apply( data.get() );

	IECoreScene::PrimitiveVariable pv;
	pv.interpolation = interpolation( param.getScope() );
	pv.data = data;

	if( param.isIndexed() )
	{
		IECore::IntVectorDataPtr indexData = new IECore::IntVectorData();
		indexData->writable().resize( indices->size() );
		std::copy( indices->get(), indices->get() + indices->size(), indexData->writable().begin() );
		pv.indices = indexData;
	}

	primitive->variables[param.getHeader().getName()] = pv;
}

} // namespace IECoreAlembic

#endif // IECOREALEMBIC_PRIMITIVEREADER_INL
