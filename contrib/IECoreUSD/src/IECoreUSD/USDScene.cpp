//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, Image Engine Design. All rights reserved.
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

#include <iostream>

#include "boost/functional/hash.hpp"
#include "boost/algorithm/string/replace.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/classification.hpp"
#include "boost/format.hpp"

#include "pxr/usd/usd/stage.h"
#include "pxr/usd/usdGeom/mesh.h"
#include "pxr/usd/usdGeom/points.h"
#include "pxr/usd/usdGeom/basisCurves.h"
#include "pxr/usd/usdGeom/bboxCache.h"
#include "pxr/usd/usdGeom/tokens.h"
#include "pxr/usd/usdGeom/xform.h"
#include "pxr/base/gf/matrix3d.h"
#include "pxr/base/gf/matrix3f.h"
#include "pxr/base/gf/matrix4d.h"
#include "pxr/base/gf/matrix4f.h"

#include <IECore/MessageHandler.h>
#include "IECore/VectorTypedData.h"
#include "IECore/SimpleTypedData.h"
#include "IECoreScene/MeshPrimitive.h"
#include "IECoreScene/PointsPrimitive.h"
#include "IECoreScene/CurvesPrimitive.h"

#include "IECoreUSD/USDScene.h"

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreUSD;

namespace
{

template<typename SrcType, typename DstType>
void convert( DstType &dst, const SrcType &src )
{
	dst = src;
}

template<>
void convert( pxr::GfHalf &dst, const half &src )
{
	dst = pxr::GfHalf( src );
}

template<>
void convert( Imath::V3d &dst, const pxr::GfVec3d &src )
{
	dst = Imath::V3d( src[0], src[1], src[2] );
}

template<>
void convert( Imath::V3f &dst, const pxr::GfVec3f &src )
{
	dst = Imath::V3f( src[0], src[1], src[2] );
}

template<>
void convert( pxr::GfVec3f &dst, const Imath::V3f &src )
{
	dst = pxr::GfVec3f( src[0], src[1], src[2] );
}

template<>
void convert( Imath::V4f &dst, const pxr::GfVec4f &src )
{
	dst = Imath::V4f( src[0], src[1], src[2], src[3] );
}

template<>
void convert( Imath::V2d &dst, const pxr::GfVec2d &src )
{
	dst = Imath::V2d( src[0], src[1] );
}

template<>
void convert( pxr::GfVec2d &dst, const Imath::V2d &src )
{
	dst = pxr::GfVec2d( src[0], src[1] );
}

template<>
void convert( Imath::V2f &dst, const pxr::GfVec2f &src )
{
	dst = Imath::V2f( src[0], src[1] );
}

template<>
void convert( pxr::GfVec2f &dst, const Imath::V2f &src )
{
	dst = pxr::GfVec2f( src[0], src[1] );
}

template<>
void convert( Imath::V2i &dst, const pxr::GfVec2i &src )
{
	dst = Imath::V2i( src[0], src[1] );
}

template<>
void convert( pxr::GfVec2i &dst, const Imath::V2i &src)
{
	dst = pxr::GfVec2i( src[0], src[1] );
}

template<>
void convert( Imath::V3i &dst, const pxr::GfVec3i &src )
{
	dst = Imath::V3i( src[0], src[1], src[2] );
}

template<>
void convert( pxr::GfVec3i &dst, const Imath::V3i &src)
{
	dst = pxr::GfVec3i( src[0], src[1], src[2] );
}

template<>
void convert( Imath::V4i &dst, const pxr::GfVec4i &src )
{
	dst = Imath::V4i( src[0], src[1], src[2], src[3] );
}

template<>
void convert( Imath::V4d &dst, const pxr::GfVec4d &src )
{
	dst = Imath::V4f( src[0], src[1], src[2], src[3] );
}

template<>
void convert( Imath::Color3f& dst, const pxr::GfVec3f& src)
{
	dst = Imath::Color3f(src[0], src[1], src[2]);
}

template<>
void convert( pxr::GfVec3f &dst, const Imath::Color3f& src )
{
	dst = pxr::GfVec3f(src[0], src[1], src[2]);
}

template<>
void convert( pxr::GfVec3d &dst, const Imath::V3d &src)
{
	dst = pxr::GfVec3d(src[0], src[1], src[2]);
}

template<>
void convert( Imath::Color3<double>& dst, const pxr::GfVec3d& src)
{
	dst = Imath::Color3<double>(src[0], src[1], src[2]);
}

template<>
void convert( pxr::GfVec3d &dst, const Imath::Color3<double> &src)
{
	dst = pxr::GfVec3d(src[0], src[1], src[2]);
}

template<>
void convert( Imath::Color4f& dst, const pxr::GfVec4f& src)
{
	dst = Imath::Color4f(src[0], src[1], src[2], src[3]);
}

template<>
void convert( pxr::GfVec4f &dst, const Imath::Color4f &src )
{
	dst = pxr::GfVec4f(src[0], src[1], src[2], src[3]);
}


template<>
void convert( Imath::Color4<double>& dst, const pxr::GfVec4d& src)
{
	dst = Imath::Color4<double>(src[0], src[1], src[2], src[3]);
}

template<>
void convert( pxr::GfVec4d &dst, const Imath::Color4<double> &src )
{
	dst = pxr::GfVec4d(src[0], src[1], src[2], src[3]);
}

template<>
void convert( IECore::InternedString& dst, const pxr::TfToken &src)
{
	dst = IECore::InternedString( src.GetString() );
}

template<>
void convert( pxr::TfToken &dst, const IECore::InternedString& src )
{
	dst = pxr::TfToken( src.string() );
}



template<>
void convert( Imath::M33f& dst, const pxr::GfMatrix3f& src)
{
	for( int i = 0; i < 3; ++i )
	{
		for( int j = 0; j < 3; ++j )
		{
			dst[i][j] = src[i][j];
		}
	}
}


template<>
void convert( pxr::GfMatrix3f& dst, const Imath::M33f& src)
{
	for( int i = 0; i < 3; ++i )
	{
		for( int j = 0; j < 3; ++j )
		{
			dst[i][j] = src[i][j];
		}
	}
}

template<>
void convert( pxr::GfMatrix3d& dst, const Imath::M33f& src)
{
	for( int i = 0; i < 3; ++i )
	{
		for( int j = 0; j < 3; ++j )
		{
			dst[i][j] = src[i][j];
		}
	}
}

template<>
void convert( Imath::M33d& dst, const pxr::GfMatrix3d& src)
{
	for( int i = 0; i < 3; ++i )
	{
		for( int j = 0; j < 3; ++j )
		{
			dst[i][j] = src[i][j];
		}
	}
}

template<>
void convert( pxr::GfMatrix3d& dst, const Imath::M33d& src)
{
	for( int i = 0; i < 3; ++i )
	{
		for( int j = 0; j < 3; ++j )
		{
			dst[i][j] = src[i][j];
		}
	}
}

template<>
void convert( Imath::M44f& dst, const pxr::GfMatrix4f& src)
{
	for( int i = 0; i < 4; ++i )
	{
		for( int j = 0; j < 4; ++j )
		{
			dst[i][j] = src[i][j];
		}
	}
}

template<>
void convert( pxr::GfMatrix4f& dst, const Imath::M44f& src )
{
	for( int i = 0; i < 4; ++i )
	{
		for( int j = 0; j < 4; ++j )
		{
			dst[i][j] = src[i][j];
		}
	}
}



template<>
void convert( Imath::M44d& dst, const pxr::GfMatrix4d& src)
{

	for( int i = 0; i < 4; ++i )
	{
		for( int j = 0; j < 4; ++j )
		{
			dst[i][j] = src[i][j];
		}
	}
}

template<>
void convert( pxr::GfMatrix4d& dst, const Imath::M44f& src )
{
	for( int i = 0; i < 4; ++i )
	{
		for( int j = 0; j < 4; ++j )
		{
			dst[i][j] = src[i][j];
		}
	}
}


template<>
void convert( pxr::GfMatrix4d& dst, const Imath::M44d& src)
{

	for( int i = 0; i < 4; ++i )
	{
		for( int j = 0; j < 4; ++j )
		{
			dst[i][j] = src[i][j];
		}
	}

}


template<>
void convert( Imath::Box3d &dst, const pxr::GfBBox3d &src )
{
	const auto &srcBox = src.GetBox();

	Imath::V3d min;
	convert( min, srcBox.GetMin() );

	Imath::V3d max;
	convert( max, srcBox.GetMax() );

	dst = Imath::Box3d( min, max );
}

template<>
void convert( IECore::IntVectorDataPtr &dst, const pxr::VtIntArray &data )
{
	IECore::IntVectorDataPtr newData = new IECore::IntVectorData();

	std::vector<int> &writable = newData->writable();
	writable.resize( data.size() );
	for( size_t i = 0; i < data.size(); ++i )
	{
		writable[i] = data[i];
	}
	dst = newData;
}

template<>
void convert( IECore::V3fVectorDataPtr &dst, const pxr::VtVec3fArray &data )
{
	IECore::V3fVectorDataPtr newData = new IECore::V3fVectorData();

	std::vector<Imath::V3f> &writable = newData->writable();
	writable.resize( data.size() );
	for( size_t i = 0; i < data.size(); ++i )
	{
		writable[i] = Imath::V3f( data[i][0], data[i][1], data[i][2] );
	}
	dst = newData;
}

template<>
void convert(Imath::Quatf& dst, const pxr::GfQuatf &src)
{
	Imath::V3f img;
	convert(img, src.GetImaginary());
	dst = Imath::Quatf(src.GetReal(), img);
}

template<>
void convert(pxr::GfQuatf &dst, const Imath::Quatf& src )
{
	dst = pxr::GfQuatf(src.r, src.v[0], src.v[1], src.v[2]);
}

template<>
void convert(Imath::Quatd& dst, const pxr::GfQuatd &src)
{
	Imath::V3d img;
	convert(img, src.GetImaginary());
	dst = Imath::Quatf(src.GetReal(), img);
}

template<>
void convert(pxr::GfQuatd &dst, const Imath::Quatd& src )
{
	dst = pxr::GfQuatf(src.r, src.v[0], src.v[1], src.v[2]);
}

IECoreScene::PrimitiveVariable::Interpolation convertInterpolation( pxr::TfToken interpolationToken )
{
	if( interpolationToken == pxr::UsdGeomTokens->varying )
	{
		return IECoreScene::PrimitiveVariable::Varying;
	}
	if( interpolationToken == pxr::UsdGeomTokens->vertex )
	{
		return IECoreScene::PrimitiveVariable::Vertex;
	}
	if( interpolationToken == pxr::UsdGeomTokens->uniform )
	{
		return IECoreScene::PrimitiveVariable::Uniform;
	}
	if( interpolationToken == pxr::UsdGeomTokens->faceVarying )
	{
		return IECoreScene::PrimitiveVariable::FaceVarying;
	}
	if( interpolationToken == pxr::UsdGeomTokens->constant )
	{
		return IECoreScene::PrimitiveVariable::Constant;
	}

	return IECoreScene::PrimitiveVariable::Invalid;
}

pxr::TfToken convertInterpolation( IECoreScene::PrimitiveVariable::Interpolation interpolation)
{
	if( interpolation == IECoreScene::PrimitiveVariable::Constant )
	{
		return pxr::UsdGeomTokens->constant;
	}
	if ( interpolation == IECoreScene::PrimitiveVariable::Uniform )
	{
		return pxr::UsdGeomTokens->uniform;
	}
	if ( interpolation == IECoreScene::PrimitiveVariable::Vertex)
	{
		return pxr::UsdGeomTokens->vertex;
	}
	if ( interpolation == IECoreScene::PrimitiveVariable::Varying)
	{
		return pxr::UsdGeomTokens->varying;
	}
	if ( interpolation == IECoreScene::PrimitiveVariable::FaceVarying)
	{
		return pxr::UsdGeomTokens->faceVarying;
	}

	return pxr::TfToken();
}



template<typename DestElementType, typename SourceElementType, template <typename P> class StorageType = IECore::GeometricTypedData>
struct TypedArrayConverter
{
	typename StorageType<std::vector<DestElementType> >::Ptr
	doConversion( const pxr::VtValue &value )
	{
		typedef pxr::VtArray<SourceElementType> SourceArrayType;
		typedef StorageType<std::vector<DestElementType> > DestArrayType;
		typedef std::vector<DestElementType> DestStorageType;

		if( value.IsHolding<SourceArrayType>() )
		{
			const auto &r = value.Get<SourceArrayType>();

			typename DestArrayType::Ptr d = new DestArrayType();
			DestStorageType &t = d->writable();

			t.resize( r.size() );

			for( size_t i = 0; i < r.size(); ++i )
			{
				convert( t[i], r[i] );
			}

			return d;
		}

		return nullptr;
	}
};

template<typename DestType, typename SourceType, template <typename P> class StorageType = IECore::GeometricTypedData>
struct TypedScalarConverter
{
	typename StorageType<DestType >::Ptr
	doConversion( const pxr::VtValue &value )
	{
		typedef StorageType<DestType> DestDataType;

		if( value.IsHolding<SourceType>() )
		{
			const auto &r = value.Get<SourceType>();

			typename DestDataType::Ptr d = new DestDataType();
			auto &t = d->writable();

			convert( t, r );

			return d;
		}

		return nullptr;
	}
};

template<typename DestType, typename SourceType, template <typename P> class StorageType = IECore::GeometricTypedData >
struct ToUSDArray
{
	typedef std::vector<SourceType> ArrayType;
	typedef StorageType<ArrayType > SourceDataType;
	typedef typename SourceDataType::ConstPtr SourceDataTypeConstPtr;

	pxr::VtValue doConversion( IECore::ConstDataPtr srcData)
	{
		SourceDataTypeConstPtr ptr = IECore::runTimeCast<const SourceDataType>( srcData );

		const ArrayType& arr = ptr->readable();

		pxr::VtArray<DestType> destArray( arr.size() );

		for (size_t i = 0; i < arr.size(); ++i)
		{
			convert( destArray[i], arr[i] );
		}

		return pxr::VtValue( destArray );
	}
};

template<typename DestType, typename SourceType, template <typename P> class StorageType = IECore::GeometricTypedData >
struct ToUSD
{
	typedef StorageType<SourceType> SourceDataType;
	typedef typename SourceDataType::ConstPtr SourceDataTypeConstPtr;

	pxr::VtValue doConversion( IECore::ConstDataPtr srcData)
	{
		SourceDataTypeConstPtr ptr = IECore::runTimeCast<const SourceDataType>( srcData );

		const SourceType& readableData = ptr->readable();

		DestType dest;
		convert(dest, readableData);

		return pxr::VtValue( dest );
	}
};

std::string cleanPrimVarName( const std::string &primVarName )
{
	return boost::algorithm::replace_first_copy( primVarName, "primvars:", "" );
}


struct ToUSDConverter
{
	ToUSDConverter( pxr::UsdGeomImageable &imageable, const std::string &name, const IECoreScene::PrimitiveVariable &primitiveVariable, pxr::UsdTimeCode time )
		: m_imageable( imageable ), m_name( name ), m_primitiveVariable( primitiveVariable ), m_time( time )
	{
	}

	template<typename ToUSDTypedConverter>
	void doConversion( const pxr::SdfValueTypeName & valueTypeName)
	{
		pxr::TfToken usdInterpolation = convertInterpolation( m_primitiveVariable.interpolation );

		if ( usdInterpolation.IsEmpty() )
		{
			IECore::msg(IECore::MessageHandler::Level::Warning, "USDScene", boost::format("Invalid Interpolation on %1%") % m_name );
			return;
		}

		ToUSDTypedConverter typedConverter;

		pxr::VtValue primVarValue = typedConverter.doConversion( m_primitiveVariable.data );

		pxr::UsdGeomPrimvar usdPrimVar = m_imageable.CreatePrimvar( pxr::TfToken( m_name ), valueTypeName, usdInterpolation );

		usdPrimVar.Set( primVarValue, m_time);

		if ( m_primitiveVariable.indices )
		{
			pxr::VtIntArray usdIndices(m_primitiveVariable.indices->readable().size());
			for (size_t i = 0; i < m_primitiveVariable.indices->readable().size(); ++i)
			{
				usdIndices[i] = m_primitiveVariable.indices->readable()[i];
			}
			usdPrimVar.SetIndices( usdIndices );
		}
	}

	pxr::UsdGeomImageable m_imageable;
	const std::string &m_name;
	const IECoreScene::PrimitiveVariable &m_primitiveVariable;
	pxr::UsdTimeCode m_time;

};

struct PrimVarConverter
{
	PrimVarConverter( IECoreScene::PrimitivePtr primitive, const pxr::UsdGeomPrimvar &primVar, pxr::UsdTimeCode time ) : m_primitive( primitive ), m_primVar( primVar ), m_time( time )
	{
	}

	template<typename TypedConverter>
	void doConversion()
	{
		IECoreScene::PrimitiveVariable::Interpolation interpolation = convertInterpolation( m_primVar.GetInterpolation() );
		if( interpolation == IECoreScene::PrimitiveVariable::Invalid )
		{
			IECore::msg(IECore::MessageHandler::Level::Warning, "USDScene", boost::format("Invalid Interpolation on %1%") % m_primVar.GetName().GetString() );
			return;
		}

		pxr::VtValue value;

		if ( !m_primVar.Get( &value, m_time ) )
		{
			IECore::msg(
				IECore::MessageHandler::Level::Warning,
				"USDScene",
				boost::format( "Unable to get value for PrimVar: '%1%' type: %2%" ) % m_primVar.GetName().GetString() % m_primVar.GetTypeName().GetAsToken().GetString()
			);

			return;
		}

		TypedConverter converter;
		auto p = converter.doConversion( value );

		if( !p )
		{
			IECore::msg(IECore::MessageHandler::Level::Warning, "USDScene", boost::format("Typed conversion failed for PrimVar: '%1%' type: %2%") % m_primVar.GetName().GetString() %
				m_primVar.GetTypeName().GetAsToken().GetString());
			return;
		}

		pxr::VtIntArray srcIndices;
		m_primVar.GetIndices( &srcIndices, m_time );

		IECore::IntVectorDataPtr indices;
		if( !srcIndices.empty() )
		{
			convert( indices, srcIndices );
		}

		std::string cleanedPrimvarName = cleanPrimVarName( m_primVar.GetName() );
		m_primitive->variables[cleanedPrimvarName] = IECoreScene::PrimitiveVariable( interpolation, p, indices );
	}

	IECoreScene::PrimitivePtr m_primitive;
	const pxr::UsdGeomPrimvar &m_primVar;
	pxr::UsdTimeCode m_time;
};


static std::map<IECore::TypeId, std::function<void( ToUSDConverter&)> > ToUSDConverters =
{
	{ IECore::BoolVectorDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSDArray<bool, bool, IECore::TypedData> >( pxr::SdfValueTypeNames->BoolArray ); } },
	{ IECore::BoolDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSD<bool, bool, IECore::TypedData> >( pxr::SdfValueTypeNames->Bool ); } },

	{ IECore::HalfVectorDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSDArray<pxr::GfHalf, half, IECore::TypedData> >( pxr::SdfValueTypeNames->HalfArray ); } },
	{ IECore::HalfDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSD<pxr::GfHalf, half, IECore::TypedData> >( pxr::SdfValueTypeNames->Half); } },

	{ IECore::FloatVectorDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSDArray<float, float, IECore::TypedData> >( pxr::SdfValueTypeNames->FloatArray ); } },
	{ IECore::FloatDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSD<float, float, IECore::TypedData> >( pxr::SdfValueTypeNames->Float); } },

	{ IECore::DoubleVectorDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSDArray<double, double, IECore::TypedData> >( pxr::SdfValueTypeNames->DoubleArray ); } },
	{ IECore::DoubleDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSD<double, double, IECore::TypedData> >( pxr::SdfValueTypeNames->Double); } },

	{ IECore::IntVectorDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSDArray<int, int, IECore::TypedData> >( pxr::SdfValueTypeNames->IntArray ); } },
	{ IECore::IntDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSD<int, int, IECore::TypedData> >( pxr::SdfValueTypeNames->Int ); } },

	{ IECore::UIntVectorDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSDArray<unsigned int, unsigned int, IECore::TypedData> >( pxr::SdfValueTypeNames->UIntArray ); } },
	{ IECore::UIntDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSD<unsigned int, unsigned int, IECore::TypedData> >( pxr::SdfValueTypeNames->UInt ); } },

	{ IECore::CharVectorDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSDArray<unsigned char, char, IECore::TypedData> >( pxr::SdfValueTypeNames->UCharArray ); } },
	{ IECore::CharDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSD<unsigned char, char, IECore::TypedData> >( pxr::SdfValueTypeNames->UChar ); } },

	{ IECore::UCharVectorDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSDArray<unsigned char, unsigned char, IECore::TypedData> >( pxr::SdfValueTypeNames->UCharArray ); } },
	{ IECore::UCharDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSD<unsigned char, unsigned char, IECore::TypedData> >( pxr::SdfValueTypeNames->UChar ); } },

	{ IECore::ShortVectorDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSDArray<int, short, IECore::TypedData> >( pxr::SdfValueTypeNames->IntArray ); } },
	{ IECore::ShortDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSD<int, short, IECore::TypedData> >( pxr::SdfValueTypeNames->Int ); } },

	{ IECore::UShortVectorDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSDArray<int, unsigned short, IECore::TypedData> >( pxr::SdfValueTypeNames->IntArray ); } },
	{ IECore::UShortDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSDArray<int, unsigned short, IECore::TypedData> >( pxr::SdfValueTypeNames->Int ); } },

	{ IECore::Int64VectorDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSDArray<int64_t, int64_t, IECore::TypedData> >( pxr::SdfValueTypeNames->Int64Array ); } },
	{ IECore::Int64DataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSD<int64_t, int64_t, IECore::TypedData> >( pxr::SdfValueTypeNames->Int64 ); } },

	{ IECore::UInt64VectorDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSDArray<uint64_t, uint64_t, IECore::TypedData> >( pxr::SdfValueTypeNames->UInt64Array ); } },
	{ IECore::UInt64DataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSD<uint64_t, uint64_t, IECore::TypedData> >( pxr::SdfValueTypeNames->UInt64 ); } },

	{ IECore::StringVectorDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSDArray<std::string, std::string, IECore::TypedData> >( pxr::SdfValueTypeNames->StringArray ); } },
	{ IECore::StringDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSD<std::string, std::string, IECore::TypedData> >( pxr::SdfValueTypeNames->String ); } },

	{ IECore::InternedStringVectorDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSDArray<pxr::TfToken, InternedString, IECore::TypedData> >( pxr::SdfValueTypeNames->TokenArray ); } },
	{ IECore::InternedStringDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSD<pxr::TfToken, InternedString, IECore::TypedData> >( pxr::SdfValueTypeNames->Token ); } },

	{ IECore::V2fVectorDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSDArray<pxr::GfVec2f, Imath::V2f> >( pxr::SdfValueTypeNames->Float2Array ); } },
	{ IECore::V2fDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSD<pxr::GfVec2f, Imath::V2f, IECore::GeometricTypedData> >( pxr::SdfValueTypeNames->Float2 ); } },

	{ IECore::V3fVectorDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSDArray<pxr::GfVec3f, Imath::V3f> >( pxr::SdfValueTypeNames->Float3Array ); } },
	{ IECore::V3fDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSD<pxr::GfVec3f, Imath::V3f, IECore::GeometricTypedData> >( pxr::SdfValueTypeNames->Float3 ); } },

	{ IECore::V2iVectorDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSDArray<pxr::GfVec2i, Imath::V2i, IECore::TypedData> >( pxr::SdfValueTypeNames->Int2Array ); } },
	{ IECore::V2iDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSD<pxr::GfVec2i, Imath::V2i, IECore::TypedData> >( pxr::SdfValueTypeNames->Int2 ); } },

	{ IECore::V3iVectorDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSDArray<pxr::GfVec3i, Imath::V3i, IECore::TypedData> >( pxr::SdfValueTypeNames->Int3Array ); } },
	{ IECore::V3iDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSD<pxr::GfVec3i, Imath::V3i, IECore::TypedData> >( pxr::SdfValueTypeNames->Int3 ); } },

	{ IECore::V2dVectorDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSDArray<pxr::GfVec2d, Imath::V2d> >( pxr::SdfValueTypeNames->Double2Array ); } },
	{ IECore::V2dDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSD<pxr::GfVec2d, Imath::V2d> >( pxr::SdfValueTypeNames->Double2 ); } },

	{ IECore::V3dVectorDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSDArray<pxr::GfVec3d, Imath::V3d> >( pxr::SdfValueTypeNames->Double3Array ); } },
	{ IECore::V3dDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSD<pxr::GfVec3d, Imath::V3d> >( pxr::SdfValueTypeNames->Double3 ); } },

	{ IECore::Color3fVectorDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSDArray<pxr::GfVec3f, Imath::Color3<float>, IECore::TypedData> >( pxr::SdfValueTypeNames->Color3fArray ); } },
	{ IECore::Color3fDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSD<pxr::GfVec3f, Imath::Color3<float>, IECore::TypedData> >( pxr::SdfValueTypeNames->Color3f ); } },

	{ IECore::Color3dVectorDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSDArray<pxr::GfVec3d, Imath::Color3<double>, IECore::TypedData> >( pxr::SdfValueTypeNames->Color3dArray ); } },
	{ IECore::Color3dDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSD<pxr::GfVec3d, Imath::Color3<double>, IECore::TypedData> >( pxr::SdfValueTypeNames->Color3d ); } },

	{ IECore::Color4fVectorDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSDArray<pxr::GfVec4f, Imath::Color4<float>, IECore::TypedData> >( pxr::SdfValueTypeNames->Color4fArray ); } },
	{ IECore::Color4fDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSD<pxr::GfVec4f, Imath::Color4<float>, IECore::TypedData> >( pxr::SdfValueTypeNames->Color4f ); } },

	{ IECore::Color4dVectorDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSDArray<pxr::GfVec4d, Imath::Color4<double>, IECore::TypedData> >( pxr::SdfValueTypeNames->Color4dArray ); } },
	{ IECore::Color4dDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSD<pxr::GfVec4d, Imath::Color4<double>, IECore::TypedData> >( pxr::SdfValueTypeNames->Color4d ); } },

	{ IECore::QuatfVectorDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSDArray<pxr::GfQuatf, Imath::Quatf, IECore::TypedData> >( pxr::SdfValueTypeNames->QuatfArray ); } },
	{ IECore::QuatfDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSD<pxr::GfQuatf, Imath::Quatf, IECore::TypedData> >( pxr::SdfValueTypeNames->Quatf ); } },

	{ IECore::QuatdVectorDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSDArray<pxr::GfQuatd, Imath::Quatd, IECore::TypedData> >( pxr::SdfValueTypeNames->QuatdArray ); } },
	{ IECore::QuatdDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSD<pxr::GfQuatd, Imath::Quatd, IECore::TypedData> >( pxr::SdfValueTypeNames->Quatd ); } },

	{ IECore::M33fVectorDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSDArray<pxr::GfMatrix3d, Imath::M33f, IECore::TypedData> >( pxr::SdfValueTypeNames->Matrix3dArray ); } },
	{ IECore::M33fDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSD<pxr::GfMatrix3d, Imath::M33f, IECore::TypedData> >( pxr::SdfValueTypeNames->Matrix3d ); } },

	{ IECore::M33dVectorDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSDArray<pxr::GfMatrix3d, Imath::M33d, IECore::TypedData> >( pxr::SdfValueTypeNames->Matrix3dArray ); } },
	{ IECore::M33dDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSD<pxr::GfMatrix3d, Imath::M33d, IECore::TypedData> >( pxr::SdfValueTypeNames->Matrix3d ); } },

	{ IECore::M44fVectorDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSDArray<pxr::GfMatrix4d, Imath::M44f, IECore::TypedData> >( pxr::SdfValueTypeNames->Matrix4dArray ); } },
	{ IECore::M44fDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSD<pxr::GfMatrix4d, Imath::M44f, IECore::TypedData> >( pxr::SdfValueTypeNames->Matrix4d ); } },

	{ IECore::M44dVectorDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSDArray<pxr::GfMatrix4d, Imath::M44d, IECore::TypedData> >( pxr::SdfValueTypeNames->Matrix4dArray ); } },
	{ IECore::M44dDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSD<pxr::GfMatrix4d, Imath::M44d, IECore::TypedData> >( pxr::SdfValueTypeNames->Matrix4d ); } },

	{ IECore::QuatfVectorDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSDArray<pxr::GfQuatf, Imath::Quatf, IECore::TypedData> >( pxr::SdfValueTypeNames->QuatfArray ); } },
	{ IECore::QuatfDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSD<pxr::GfQuatf, Imath::Quatf, IECore::TypedData> >( pxr::SdfValueTypeNames->QuatfArray ); } },

	{ IECore::QuatdVectorDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSDArray<pxr::GfQuatd, Imath::Quatd, IECore::TypedData> >( pxr::SdfValueTypeNames->QuatdArray ); } },
	{ IECore::QuatdDataTypeId, [] (ToUSDConverter& converter) { converter.doConversion< ToUSD<pxr::GfQuatd, Imath::Quatd, IECore::TypedData> >( pxr::SdfValueTypeNames->Quatd ); } },

};

static std::map<pxr::TfToken, std::function<void( PrimVarConverter& converter )> > primvarConversions =
{
	{ pxr::TfToken( "bool" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedScalarConverter<bool, bool, IECore::TypedData> >(); } },
	{ pxr::TfToken( "bool[]" ), []( PrimVarConverter& converter ) { throw IECore::NotImplementedException("bool[]"); } },

	{ pxr::TfToken( "uint" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedScalarConverter<unsigned int, unsigned int, IECore::TypedData> >(); } },
	{ pxr::TfToken( "uint[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedArrayConverter<unsigned int, unsigned int, IECore::TypedData> >(); } },
	{ pxr::TfToken( "char[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedArrayConverter<char, char, IECore::TypedData> >(); } },

	{ pxr::TfToken( "color3h" ), []( PrimVarConverter& converter ) { throw IECore::NotImplementedException("color3h"); } },
	{ pxr::TfToken( "color3f" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedScalarConverter<Imath::Color3f, pxr::GfVec3f, IECore::TypedData> >(); } },
	{ pxr::TfToken( "color3d" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedScalarConverter<Imath::Color3<double>, pxr::GfVec3d, IECore::TypedData> >(); } },

	{ pxr::TfToken( "color3h[]" ), []( PrimVarConverter& converter ) { throw IECore::NotImplementedException("color3h[]"); } },
	{ pxr::TfToken( "color3f[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedArrayConverter<Imath::Color3f, pxr::GfVec3f, IECore::TypedData> >(); } },
	{ pxr::TfToken( "color3d[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedArrayConverter<Imath::Color3<double>, pxr::GfVec3d, IECore::TypedData> >(); } },

	{ pxr::TfToken( "color4h" ), []( PrimVarConverter& converter ) {  throw IECore::NotImplementedException("color4h");} } ,
	{ pxr::TfToken( "color4f" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedScalarConverter<Imath::Color4f, pxr::GfVec4f, IECore::TypedData> >(); } },
	{ pxr::TfToken( "color4d" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedScalarConverter<Imath::Color4<double>, pxr::GfVec4d, IECore::TypedData> >(); } },

	{ pxr::TfToken( "color4h[]" ), []( PrimVarConverter& converter ) { throw IECore::NotImplementedException("color4h[]"); } },
	{ pxr::TfToken( "color4f[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedArrayConverter<Imath::Color4f, pxr::GfVec4f, IECore::TypedData> >(); } },
	{ pxr::TfToken( "color4d[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedArrayConverter<Imath::Color4<double>, pxr::GfVec4d, IECore::TypedData> >(); } },

	{ pxr::TfToken( "half" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedScalarConverter<half, pxr::GfHalf, IECore::TypedData > >(); } },
	{ pxr::TfToken( "half2" ), []( PrimVarConverter& converter ) { throw IECore::NotImplementedException("half2"); } },
	{ pxr::TfToken( "half3" ), []( PrimVarConverter& converter ) { throw IECore::NotImplementedException("half3"); } },
	{ pxr::TfToken( "half4" ), []( PrimVarConverter& converter ) { throw IECore::NotImplementedException("half4"); } },

	{ pxr::TfToken( "half[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedArrayConverter<half, pxr::GfHalf, IECore::TypedData > >(); } },
	{ pxr::TfToken( "half2[]" ), []( PrimVarConverter& converter ) { throw IECore::NotImplementedException("half2[]"); } },
	{ pxr::TfToken( "half3[]" ), []( PrimVarConverter& converter ) { throw IECore::NotImplementedException("half3[]"); } },
	{ pxr::TfToken( "half4[]" ), []( PrimVarConverter& converter ) { throw IECore::NotImplementedException("half4[]"); } },

	{ pxr::TfToken( "double" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedScalarConverter<double, double, IECore::TypedData > >();  } },
	{ pxr::TfToken( "double2" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedScalarConverter<Imath::V2d, pxr::GfVec2d, IECore::GeometricTypedData > >();  } },
	{ pxr::TfToken( "double3" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedScalarConverter<Imath::V3d, pxr::GfVec3d, IECore::GeometricTypedData > >();  } },
	{ pxr::TfToken( "double4" ), []( PrimVarConverter& converter ) { throw IECore::NotImplementedException("double4"); } },

	{ pxr::TfToken( "double[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedArrayConverter<double, double, IECore::TypedData > >();  } },
	{ pxr::TfToken( "double2[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedArrayConverter<Imath::V2d, pxr::GfVec2d, IECore::GeometricTypedData > >();  } },
	{ pxr::TfToken( "double3[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedArrayConverter<Imath::V3d, pxr::GfVec3d, IECore::GeometricTypedData > >();  } },
	{ pxr::TfToken( "double4[]" ), []( PrimVarConverter& converter ) { throw IECore::NotImplementedException("double4[]"); } },

	{ pxr::TfToken( "float" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedScalarConverter<float, float, IECore::TypedData> >(); } },
	{ pxr::TfToken( "float2" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedScalarConverter<Imath::V2f, pxr::GfVec2f, IECore::GeometricTypedData> >(); } },
	{ pxr::TfToken( "float3" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedScalarConverter<Imath::V3f, pxr::GfVec3f, IECore::GeometricTypedData> >(); } },
	{ pxr::TfToken( "float4" ), []( PrimVarConverter& converter ) { throw IECore::NotImplementedException("float4"); } },

	{ pxr::TfToken( "float[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedArrayConverter<float, float, IECore::TypedData> >(); } },
	{ pxr::TfToken( "float2[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedArrayConverter<Imath::V2f, pxr::GfVec2f, IECore::GeometricTypedData> >(); } },
	{ pxr::TfToken( "float3[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedArrayConverter<Imath::V3f, pxr::GfVec3f, IECore::GeometricTypedData> >(); } },
	{ pxr::TfToken( "float4[]" ), []( PrimVarConverter& converter ) { throw IECore::NotImplementedException("float4[]"); } },

	{ pxr::TfToken( "int" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedScalarConverter<int, int, IECore::TypedData> >(); } },
	{ pxr::TfToken( "int2" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedScalarConverter<Imath::V2i, pxr::GfVec2i, IECore::GeometricTypedData> >(); } },
	{ pxr::TfToken( "int3" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedScalarConverter<Imath::V3i, pxr::GfVec3i, IECore::GeometricTypedData> >(); } },
	{ pxr::TfToken( "int4" ), []( PrimVarConverter& converter ) { throw IECore::NotImplementedException("int4"); } },

	{ pxr::TfToken( "int[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedArrayConverter<int, int, IECore::TypedData> >(); } },
	{ pxr::TfToken( "int2[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedArrayConverter<Imath::V2i, pxr::GfVec2i, IECore::GeometricTypedData> >(); } },
	{ pxr::TfToken( "int3[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedArrayConverter<Imath::V3i, pxr::GfVec3i, IECore::GeometricTypedData> >(); } },
	{ pxr::TfToken( "int4[]" ), []( PrimVarConverter& converter ) { throw IECore::NotImplementedException("int4[]"); } },

	{ pxr::TfToken( "int64" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedScalarConverter<long int, long int, IECore::TypedData> >(); } },
	{ pxr::TfToken( "int64[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedArrayConverter<long int, long int, IECore::TypedData> >(); } },

	{ pxr::TfToken( "uint64" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedScalarConverter<unsigned long int, unsigned long int, IECore::TypedData> >(); } },
	{ pxr::TfToken( "uint64[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedArrayConverter<unsigned long int, unsigned long int, IECore::TypedData> >(); } },

	{ pxr::TfToken( "point3h" ), []( PrimVarConverter& converter ) { throw IECore::NotImplementedException("point3h"); } },
	{ pxr::TfToken( "point3f" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedScalarConverter<Imath::V3f, pxr::GfVec3f, IECore::GeometricTypedData> >(); } },
	{ pxr::TfToken( "point3d" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedScalarConverter<Imath::V3d, pxr::GfVec3d, IECore::GeometricTypedData> >(); } },

	{ pxr::TfToken( "point3h[]" ), []( PrimVarConverter& converter ) { throw IECore::NotImplementedException("point3h[]"); } },
	{ pxr::TfToken( "point3f[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedArrayConverter<Imath::V3f, pxr::GfVec3f, IECore::GeometricTypedData> >(); } },
	{ pxr::TfToken( "point3d[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedArrayConverter<Imath::V3d, pxr::GfVec3d, IECore::GeometricTypedData> >(); } },

	{ pxr::TfToken( "normal3h" ), []( PrimVarConverter& converter ) {  throw IECore::NotImplementedException("normal3h"); } },
	{ pxr::TfToken( "normal3f" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedScalarConverter<Imath::V3f, pxr::GfVec3f, IECore::GeometricTypedData> >(); } },
	{ pxr::TfToken( "normal3d" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedScalarConverter<Imath::V3d, pxr::GfVec3d, IECore::GeometricTypedData> >(); } },

	{ pxr::TfToken( "normal3h[]" ), []( PrimVarConverter& converter ) {  throw IECore::NotImplementedException("normal3h[]"); } },
	{ pxr::TfToken( "normal3f[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedArrayConverter<Imath::V3f, pxr::GfVec3f, IECore::GeometricTypedData> >(); } },
	{ pxr::TfToken( "normal3d[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedArrayConverter<Imath::V3d, pxr::GfVec3d, IECore::GeometricTypedData> >(); } },

	{ pxr::TfToken( "vector3h" ), []( PrimVarConverter& converter ) { throw IECore::NotImplementedException("vector3h"); } },
	{ pxr::TfToken( "vector3f" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedScalarConverter<Imath::V3f, pxr::GfVec3f, IECore::GeometricTypedData> >(); } },
	{ pxr::TfToken( "vector3d" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedScalarConverter<Imath::V3d, pxr::GfVec3d, IECore::GeometricTypedData> >(); } },

	{ pxr::TfToken( "vector3h[]" ), []( PrimVarConverter& converter ) { throw IECore::NotImplementedException("vector3h[]"); } },
	{ pxr::TfToken( "vector3f[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedArrayConverter<Imath::V3f, pxr::GfVec3f, IECore::GeometricTypedData> >(); } },
	{ pxr::TfToken( "vector3d[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedArrayConverter<Imath::V3d, pxr::GfVec3d, IECore::GeometricTypedData> >(); } },

	{ pxr::TfToken( "quath" ), []( PrimVarConverter& converter ) { throw IECore::NotImplementedException("quath"); } },
	{ pxr::TfToken( "quatf" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedScalarConverter<Imath::Quatf, pxr::GfQuatf, IECore::TypedData> >(); } },
	{ pxr::TfToken( "quatd" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedScalarConverter<Imath::Quatd, pxr::GfQuatd, IECore::TypedData> >(); } },

	{ pxr::TfToken( "quath[]" ), []( PrimVarConverter& converter ) { throw IECore::NotImplementedException("quath[]"); } },
	{ pxr::TfToken( "quatf[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedArrayConverter<Imath::Quatf, pxr::GfQuatf, IECore::TypedData> >(); } },
	{ pxr::TfToken( "quatd[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedArrayConverter<Imath::Quatd, pxr::GfQuatd, IECore::TypedData> >(); } },

	{ pxr::TfToken( "string" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedScalarConverter<std::string, std::string, IECore::TypedData> >(); } },
	{ pxr::TfToken( "string[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedArrayConverter<std::string, std::string, IECore::TypedData> >(); } },

	{ pxr::TfToken( "token" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedScalarConverter<IECore::InternedString, pxr::TfToken, IECore::TypedData> >(); } },
	{ pxr::TfToken( "token[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedArrayConverter<IECore::InternedString, pxr::TfToken, IECore::TypedData> >(); } },

	{ pxr::TfToken( "uchar" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedScalarConverter<unsigned char, unsigned char, IECore::TypedData> >(); } },
	{ pxr::TfToken( "uchar[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedArrayConverter<unsigned char, unsigned char, IECore::TypedData> >(); } },

	{ pxr::TfToken( "matrix2d" ), []( PrimVarConverter& converter ) { throw IECore::NotImplementedException("matrix2d"); } },
	{ pxr::TfToken( "matrix2d[]" ), []( PrimVarConverter& converter ) { throw IECore::NotImplementedException("matrix2d[]"); } },

	{ pxr::TfToken( "matrix3f" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedScalarConverter<Imath::M33f, pxr::GfMatrix3f, IECore::TypedData> >(); } },
	{ pxr::TfToken( "matrix3f[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedArrayConverter<Imath::M33f, pxr::GfMatrix3f, IECore::TypedData> >(); } },

	{ pxr::TfToken( "matrix3d" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedScalarConverter<Imath::M33d, pxr::GfMatrix3d, IECore::TypedData> >(); } },
	{ pxr::TfToken( "matrix3d[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedArrayConverter<Imath::M33d, pxr::GfMatrix3d, IECore::TypedData> >(); } },

	{ pxr::TfToken( "matrix4f" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedScalarConverter<Imath::M44f, pxr::GfMatrix4f, IECore::TypedData> >(); } },
	{ pxr::TfToken( "matrix4f[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedArrayConverter<Imath::M44f, pxr::GfMatrix4f, IECore::TypedData> >(); } },

	{ pxr::TfToken( "matrix4d" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedScalarConverter<Imath::M44d, pxr::GfMatrix4d, IECore::TypedData> >(); } },
	{ pxr::TfToken( "matrix4d[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedArrayConverter<Imath::M44d, pxr::GfMatrix4d, IECore::TypedData> >(); } },
};

void convertPrimVar( IECoreScene::PrimitivePtr primitive, const pxr::UsdGeomPrimvar &primVar, pxr::UsdTimeCode time )
{

	pxr::TfToken typeToken = primVar.GetTypeName().GetAsToken(); // docs state 'GetAsToken' should not be used for comparision purposes.

	auto it = primvarConversions.find( typeToken );
	if ( it != primvarConversions.end() )
	{
		PrimVarConverter converter( primitive, primVar, time );
		try
		{
			it->second( converter );
		}
		catch(const IECore::NotImplementedException& notImplemented)
		{
			IECore::msg( IECore::MessageHandler::Level::Warning, "USDScene", boost::format("PrimVar: %1% type: %2% not supported - skipping") % primVar.GetName().GetString() % notImplemented.what() );
		}
	}
	else
	{
		IECore::msg(IECore::MessageHandler::Level::Warning, "USDScene", boost::format("Unknown type %1% on PrimVar %2% ") % typeToken.GetString() % primVar.GetName().GetString());
	}
}


void convertPrimVar( pxr::UsdGeomImageable &imageablePrim, const std::string &srcPrimVarName, const IECoreScene::PrimitiveVariable &srcPrimVar, pxr::UsdTimeCode timeCode )
{
	auto it = ToUSDConverters.find( srcPrimVar.data->typeId() );

	if (it != ToUSDConverters.end())
	{
		ToUSDConverter converter(imageablePrim, srcPrimVarName, srcPrimVar, timeCode);
		it->second( converter );
	}
}

void convertPrimVars( pxr::UsdGeomImageable imagable, IECoreScene::PrimitivePtr primitive, pxr::UsdTimeCode time )
{
	for( const auto &primVar : imagable.GetPrimvars() )
	{
		convertPrimVar( primitive, primVar, time );
	}
}

IECoreScene::PointsPrimitivePtr convertPrimitive( pxr::UsdGeomPoints points, pxr::UsdTimeCode time )
{
	pxr::UsdAttribute attr = points.GetPointsAttr();

	pxr::VtVec3fArray pointsData;

	attr.Get( &pointsData, time );

	IECore::V3fVectorDataPtr positionData;
	convert( positionData, pointsData );

	IECoreScene::PointsPrimitivePtr newPoints = new IECoreScene::PointsPrimitive( positionData );

	convertPrimVars( points, newPoints, time );
	return newPoints;
}

IECoreScene::CurvesPrimitivePtr convertPrimitive( pxr::UsdGeomCurves curves, pxr::UsdTimeCode time )
{
	pxr::UsdAttribute vertexCountsAttr = curves.GetCurveVertexCountsAttr();
	pxr::VtIntArray vertexCountsData;
	vertexCountsAttr.Get( &vertexCountsData, time );
	IECore::IntVectorDataPtr countData;
	convert( countData, vertexCountsData );

	pxr::UsdAttribute attr = curves.GetPointsAttr();
	pxr::VtVec3fArray pointsData;
	attr.Get( &pointsData, time );
	IECore::V3fVectorDataPtr positionData;
	convert( positionData, pointsData );

	IECoreScene::CurvesPrimitivePtr newCurves = new IECoreScene::CurvesPrimitive( countData, IECore::CubicBasisf::linear(), false, positionData );

	convertPrimVars( curves, newCurves, time );
	return newCurves;
}

IECoreScene::MeshPrimitivePtr convertPrimitive( pxr::UsdGeomMesh mesh, pxr::UsdTimeCode time )
{
	pxr::UsdAttribute subdivSchemeAttr = mesh.GetSubdivisionSchemeAttr();

	pxr::TfToken subdivScheme;
	subdivSchemeAttr.Get( &subdivScheme );

	pxr::UsdAttribute faceVertexCountsAttr = mesh.GetFaceVertexCountsAttr();

	pxr::VtIntArray faceVertexCounts;
	faceVertexCountsAttr.Get( &faceVertexCounts, time );
	IECore::IntVectorDataPtr vertexCountData;
	convert( vertexCountData, faceVertexCounts );

	pxr::UsdAttribute faceVertexIndexAttr = mesh.GetFaceVertexIndicesAttr();
	pxr::VtIntArray faceVertexIndices;
	faceVertexIndexAttr.Get( &faceVertexIndices, time  );

	IECore::IntVectorDataPtr vertexIndicesData;
	convert( vertexIndicesData, faceVertexIndices );

	IECoreScene::MeshPrimitivePtr newMesh = new IECoreScene::MeshPrimitive( vertexCountData, vertexIndicesData );

	pxr::UsdAttribute attr = mesh.GetPointsAttr();
	pxr::VtVec3fArray pointsData;

	attr.Get( &pointsData, time );

	IECore::V3fVectorDataPtr positionData;
	convert( positionData, pointsData );

	convertPrimVars( mesh, newMesh, time );
	newMesh->variables["P"] = IECoreScene::PrimitiveVariable( IECoreScene::PrimitiveVariable::Vertex, positionData );

	if( subdivScheme == pxr::UsdGeomTokens->catmullClark )
	{
		newMesh->setInterpolation( "catmullClark" );
	}

	return newMesh;
}

void convertPrimitiveVariables( pxr::UsdGeomImageable imageable, const IECoreScene::Primitive *primitive, pxr::UsdTimeCode timeCode)
{
	const static std::set<std::string> primVarsToIgnore = {"P"};

	for ( const auto &primitiveVariable : primitive->variables )
	{
		if (primVarsToIgnore.find( primitiveVariable.first ) == primVarsToIgnore.end() )
		{
			convertPrimVar( imageable, primitiveVariable.first, primitiveVariable.second, timeCode );
		}
	}
}

void convertPoints( pxr::UsdGeomPointBased pointsBased, const IECoreScene::Primitive *primitive, pxr::UsdTimeCode timeCode)
{
	ToUSDArray<pxr::GfVec3f, Imath::V3f> toUSDVec3Array;

	const auto it = primitive->variables.find("P");
	if (it != primitive->variables.end() )
	{
		pointsBased.CreatePointsAttr().Set(  toUSDVec3Array.doConversion ( it->second.data ), timeCode);
	}
	else
	{
		//todo raise an exception
	}

}

void convertPrimitive( pxr::UsdGeomMesh usdMesh, const IECoreScene::MeshPrimitive *mesh, pxr::UsdTimeCode timeCode )
{
	// convert topology
	ToUSDArray<int, int, IECore::TypedData> toUSDIntArray;
	usdMesh.CreateFaceVertexCountsAttr().Set(  toUSDIntArray.doConversion( mesh->verticesPerFace() ) , timeCode);
	usdMesh.CreateFaceVertexIndicesAttr().Set( toUSDIntArray.doConversion( mesh->vertexIds() ), timeCode );

	// positions
	convertPoints(usdMesh, mesh, timeCode );

	// set the interpolation

	if (mesh->interpolation() == std::string("catmullClark"))
	{
		usdMesh.CreateSubdivisionSchemeAttr().Set( pxr::UsdGeomTokens->catmullClark );
	}
	else
	{
		usdMesh.CreateSubdivisionSchemeAttr().Set( pxr::UsdGeomTokens->none );
	}



	// convert all primvars to USD
	convertPrimitiveVariables( usdMesh, mesh, timeCode );
}

void convertPrimitive( pxr::UsdGeomPoints usdPoints, const IECoreScene::PointsPrimitive *points, pxr::UsdTimeCode timeCode )
{
	// positions
	convertPoints(usdPoints, points, timeCode);

	// convert all primvars to USD
	convertPrimitiveVariables( usdPoints, points, timeCode );
}

void convertPrimitive( pxr::UsdGeomBasisCurves usdCurves, const IECoreScene::CurvesPrimitive *curves, pxr::UsdTimeCode timeCode )
{
	// convert topology
	ToUSDArray<int, int, IECore::TypedData> toUSDIntArray;
	usdCurves.CreateCurveVertexCountsAttr().Set( toUSDIntArray.doConversion( curves->verticesPerCurve() ), timeCode );

	// positions
	convertPoints( usdCurves, curves, timeCode );

	// convert all primvars to USD
	convertPrimitiveVariables( usdCurves, curves, timeCode );
}

bool isConvertible( pxr::UsdPrim prim )
{
	pxr::UsdGeomMesh mesh( prim );
	if( mesh )
	{
		return true;
	}
	pxr::UsdGeomPoints points( prim );
	if( points )
	{
		return true;
	}

	pxr::UsdGeomCurves curves( prim );
	if( curves )
	{
		return true;
	}
	return false;
}

IECore::ConstObjectPtr convertPrimitive( pxr::UsdPrim prim, pxr::UsdTimeCode time )
{
	if( pxr::UsdGeomMesh mesh = pxr::UsdGeomMesh( prim ) )
	{
		return convertPrimitive( mesh, time );
	}

	if( pxr::UsdGeomPoints points = pxr::UsdGeomPoints( prim ) )
	{
		return convertPrimitive( points, time );
	}

	if( pxr::UsdGeomCurves curves = pxr::UsdGeomCurves( prim ) )
	{
		return convertPrimitive( curves, time );
	}

	return nullptr;
}

bool hasTimeVaryingPrimVars( pxr::UsdGeomImageable imagable )
{
	for( const auto &primVar : imagable.GetPrimvars() )
	{
		if( primVar.ValueMightBeTimeVarying() )
		{
			return true;
		}
	}
	return false;
}

bool isTimeVarying( pxr::UsdGeomMesh mesh )
{
	return mesh.GetPointsAttr().ValueMightBeTimeVarying() ||
		mesh.GetNormalsAttr().ValueMightBeTimeVarying() ||
		mesh.GetVelocitiesAttr().ValueMightBeTimeVarying() ||
		hasTimeVaryingPrimVars( mesh );
}

bool isTimeVarying( pxr::UsdGeomCurves curves )
{
	return curves.GetPointsAttr().ValueMightBeTimeVarying() ||
		curves.GetNormalsAttr().ValueMightBeTimeVarying() ||
		curves.GetVelocitiesAttr().ValueMightBeTimeVarying() ||
		hasTimeVaryingPrimVars( curves );
}

bool isTimeVarying( pxr::UsdGeomPoints points )
{
	return points.GetPointsAttr().ValueMightBeTimeVarying() ||
		points.GetNormalsAttr().ValueMightBeTimeVarying() ||
		points.GetVelocitiesAttr().ValueMightBeTimeVarying() ||
		points.GetWidthsAttr().ValueMightBeTimeVarying() ||
		points.GetIdsAttr().ValueMightBeTimeVarying() ||
		hasTimeVaryingPrimVars( points );
}

bool isTimeVarying( pxr::UsdPrim prim )
{

	if( pxr::UsdGeomMesh mesh = pxr::UsdGeomMesh( prim ) )
	{
		return isTimeVarying( mesh );
	}

	if( pxr::UsdGeomPoints points = pxr::UsdGeomPoints ( prim ) )
	{
		return isTimeVarying( points );
	}

	if( pxr::UsdGeomCurves curves = pxr::UsdGeomCurves( prim ) )
	{
		return isTimeVarying( curves );
	}

	return false;
}

} // namespace


class USDScene::Location : public RefCounted
{
	public:
		Location(pxr::UsdPrim prim ) : prim(prim) {}
		pxr::UsdPrim prim;
};

class USDScene::IO : public RefCounted
{
	public:
		IO( const std::string &fileName ) : m_fileName( fileName )
		{
		}

		virtual ~IO()
		{
		}

		const std::string &fileName() const
		{
			return m_fileName;
		}

		virtual pxr::UsdPrim root() const = 0;
		virtual pxr::UsdTimeCode getTime( double timeSeconds ) const = 0;

		virtual bool isReader() const = 0;

		pxr::UsdStageRefPtr getStage() const { return m_usdStage; }
	protected:
		pxr::UsdStageRefPtr m_usdStage;

	private:
		std::string m_fileName;
};

class USDScene::Reader : public USDScene::IO
{
	public:
		Reader( const std::string &fileName ) : IO( fileName )
		{
			m_usdStage = pxr::UsdStage::Open( fileName );
			m_timeCodesPerSecond = m_usdStage->GetTimeCodesPerSecond();
			m_rootPrim = m_usdStage->GetPseudoRoot();
		}

		pxr::UsdPrim root() const override
		{
			return m_rootPrim;
		}

		pxr::UsdTimeCode getTime( double timeSeconds ) const override
		{
			return timeSeconds * m_timeCodesPerSecond;
		}

		bool isReader()  const override { return true; }

	private:

		pxr::UsdPrim m_rootPrim;

		double m_timeCodesPerSecond;
};

class USDScene::Writer : public USDScene::IO
{
	public:
		Writer( const std::string &fileName ) : IO( fileName )
		{
			m_usdStage = pxr::UsdStage::CreateNew( fileName );
			m_timeCodesPerSecond = m_usdStage->GetTimeCodesPerSecond();
			m_rootPrim = m_usdStage->GetPseudoRoot();
		}

		~Writer() override
		{
			m_usdStage->GetRootLayer()->Save();
		}

		pxr::UsdPrim root() const override
		{
			return m_rootPrim;
		}

		pxr::UsdTimeCode getTime( double timeSeconds ) const override
		{
			return timeSeconds * m_timeCodesPerSecond;
		}

		bool isReader()  const override { return false; }

	private:

		pxr::UsdPrim m_rootPrim;

		double m_timeCodesPerSecond;
};

USDScene::USDScene( const std::string &path, IndexedIO::OpenMode &mode )
{
	switch( mode )
	{
		case IndexedIO::Read :
			m_root = new Reader( path );
			m_location = new Location( m_root->root() );
			break;
		case IndexedIO::Write :
			m_root = new Writer( path );
			m_location = new Location( m_root->root() );
			break;
		default:
			throw Exception( " Unsupported OpenMode " );
	}
}

USDScene::USDScene( IOPtr root, LocationPtr location) : m_root( root ), m_location( location )
{

}

USDScene::~USDScene()
{
}

std::string USDScene::fileName() const
{
	return m_root->fileName();
}

Imath::Box3d USDScene::readBound( double time ) const
{
	pxr::UsdGeomBoundable boundable = pxr::UsdGeomBoundable( m_location->prim );
	pxr::UsdGeomMesh mesh = pxr::UsdGeomMesh ( m_location->prim );

	if( !boundable )
	{
		return Imath::Box3d();
	}

	pxr::UsdAttribute attr = boundable.GetExtentAttr();

	if( !attr.IsValid() )
	{
		return Imath::Box3d();
	}

	pxr::VtArray<pxr::GfVec3f> extents;
	attr.Get<pxr::VtArray<pxr::GfVec3f> >( &extents, m_root->getTime( time ) );

	if( extents.size() == 2 )
	{
		Imath::V3f min;
		convert( min, extents[0] );

		Imath::V3f max;
		convert( max, extents[1] );
		return Imath::Box3d( min, max );
	}

	return Imath::Box3d();
}

ConstDataPtr USDScene::readTransform( double time ) const
{
	return new IECore::M44dData( readTransformAsMatrix( time ) );
}

Imath::M44d USDScene::readTransformAsMatrix( double time ) const
{
	pxr::UsdGeomXformable transformable( m_location->prim );
	pxr::GfMatrix4d transform;
	bool reset = false;

	transformable.GetLocalTransformation( &transform, &reset, m_root->getTime( time ) );
	Imath::M44d returnValue;
	convert( returnValue, transform );
	return returnValue;
}

ConstObjectPtr USDScene::readAttribute( const SceneInterface::Name &name, double time ) const
{
	return nullptr;
}

ConstObjectPtr USDScene::readObject( double time ) const
{
	return convertPrimitive( m_location->prim, m_root->getTime( time ) );
}

SceneInterface::Name USDScene::name() const
{
	return SceneInterface::Name( m_location->prim.GetName() );
}

void USDScene::path( SceneInterface::Path &p ) const
{
	std::vector<std::string> parts;
	pxr::SdfPath path = m_location->prim.GetPath();
	boost::split( parts, path.GetString(), boost::is_any_of( "/" ) );

	p.reserve( parts.size() );

	for( const auto &part : parts )
	{
		if( part != "" )
		{
			p.push_back( IECore::InternedString( part ) );
		}
	}
}

bool USDScene::hasBound() const
{
	pxr::UsdGeomBoundable boundable = pxr::UsdGeomBoundable( m_location->prim );
	pxr::UsdGeomMesh mesh = pxr::UsdGeomMesh( m_location->prim );
	pxr::UsdAttribute attr;

	if( boundable )
	{
		attr = boundable.GetExtentAttr();
	}

	return attr.IsValid();
}

void USDScene::writeBound( const Imath::Box3d &bound, double time )
{
	// unable to write bounds on root scene graph location
	if( m_location->prim.GetPath().IsEmpty() )
	{
		return;
	}

	pxr::UsdGeomBoundable boundable( m_location->prim );

	if( !boundable )
	{
		return;
	}

	pxr::VtArray<pxr::GfVec3f> pxrBounds( 2 );

	convert( pxrBounds[0], Imath::V3f( bound.min ) );
	convert( pxrBounds[1], Imath::V3f( bound.max ) );

	pxr::UsdAttribute extentAttr = boundable.CreateExtentAttr();
	extentAttr.Set( pxr::VtValue( pxrBounds ) );
}

void USDScene::writeTransform( const Data *transform, double time )
{

	pxr::UsdTimeCode timeCode = m_root->getTime( time );

	const M44dData *m44 = IECore::runTimeCast<const M44dData>( transform );
	if( !m44 )
	{
		return;
	}

	Imath::M44d matrix = m44->readable();
	pxr::UsdGeomXformable xformable( m_location->prim );

	if( xformable )
	{
		pxr::UsdGeomXformOp transformOp = xformable.MakeMatrixXform();
		pxr::GfMatrix4d usdMat;

		convert(usdMat, matrix);

		transformOp.Set( usdMat, timeCode );

		return;
	}

}

bool USDScene::hasAttribute( const SceneInterface::Name &name ) const
{
	return false;
}

void USDScene::attributeNames( SceneInterface::NameList &attrs ) const
{
}

void USDScene::writeAttribute( const SceneInterface::Name &name, const Object *attribute, double time )
{
}

bool USDScene::hasTag( const SceneInterface::Name &name, int filter ) const
{
	return false;
}

void USDScene::readTags( SceneInterface::NameList &tags, int filter ) const
{

}

void USDScene::writeTags( const SceneInterface::NameList &tags )
{
}

bool USDScene::hasObject() const
{
	return isConvertible( m_location->prim );
}

PrimitiveVariableMap USDScene::readObjectPrimitiveVariables( const std::vector<InternedString> &primVarNames, double time ) const
{
	return PrimitiveVariableMap();
}

void USDScene::writeObject( const Object *object, double time )
{
	pxr::UsdTimeCode timeCode = m_root->getTime( time );

	const IECoreScene::MeshPrimitive* meshPrimitive = IECore::runTimeCast<const IECoreScene::MeshPrimitive>( object );
	if ( meshPrimitive )
	{
		pxr::SdfPath p = m_location->prim.GetPath();

		pxr::UsdGeomMesh usdMesh = pxr::UsdGeomMesh::Define( m_root->getStage(), p );
		convertPrimitive( usdMesh, meshPrimitive, timeCode );
	}

	const IECoreScene::PointsPrimitive* pointsPrimitive = IECore::runTimeCast<const IECoreScene::PointsPrimitive>( object );
	if ( pointsPrimitive )
	{
		pxr::SdfPath p = m_location->prim.GetPath();

		pxr::UsdGeomPoints usdPoints = pxr::UsdGeomPoints::Define( m_root->getStage(), p );
		convertPrimitive( usdPoints, pointsPrimitive, timeCode );
	}

	const IECoreScene::CurvesPrimitive* curvesPrimitive = IECore::runTimeCast<const IECoreScene::CurvesPrimitive>( object );
	if ( curvesPrimitive )
	{
		pxr::SdfPath p = m_location->prim.GetPath();

		pxr::UsdGeomBasisCurves usdCurves = pxr::UsdGeomBasisCurves::Define( m_root->getStage(), p );
		convertPrimitive( usdCurves, curvesPrimitive, timeCode );
	}
}

bool USDScene::hasChild( const SceneInterface::Name &name ) const
{
	pxr::UsdPrim childPrim = m_location->prim.GetChild( pxr::TfToken( name.string() ) );

	return childPrim;
}

void USDScene::childNames( SceneInterface::NameList &childNames ) const
{
	for( const auto &i : m_location->prim.GetAllChildren() )
	{
		pxr::UsdGeomXformable xformable ( i );

		if( xformable )
		{
			childNames.push_back( IECore::InternedString( i.GetName() ) );
		}
	}
}

SceneInterfacePtr USDScene::child( const SceneInterface::Name &name, SceneInterface::MissingBehaviour missingBehaviour )
{
	pxr::UsdPrim childPrim = m_location->prim.GetChild( pxr::TfToken( name.string() ) );

	if( childPrim )
	{
		if( ( childPrim.GetTypeName() == "Xform" || isConvertible( childPrim ) ) )
		{
			SceneInterfacePtr newScene = new USDScene( m_root, new Location(childPrim) );
			return newScene;
		}
	}

	switch( missingBehaviour )
	{
		case SceneInterface::NullIfMissing :
			return nullptr;
		case SceneInterface::ThrowIfMissing :
			throw IOException( "Child \"" + name.string() + "\" does not exist" );
		case SceneInterface::CreateIfMissing :
		{
			if( m_root->isReader() )
			{
				throw InvalidArgumentException( "Child creation not supported" );
			}
			else
			{
				pxr::UsdPrim prim = m_location->prim;
				pxr::SdfPath newPath = prim.GetPath().AppendChild( pxr::TfToken( name.string() ) );
				pxr::UsdGeomXform newXform = pxr::UsdGeomXform::Define( m_root->getStage(), newPath );

				return new USDScene( m_root, new Location( newXform.GetPrim() ) );
			}
		}
		default:
			return nullptr;
	}
}

ConstSceneInterfacePtr USDScene::child( const SceneInterface::Name &name, SceneInterface::MissingBehaviour missingBehaviour ) const
{
	return const_cast<USDScene *>( this )->child( name, missingBehaviour );
}

SceneInterfacePtr USDScene::createChild( const SceneInterface::Name &name )
{
	pxr::UsdPrim prim = m_location->prim;
	pxr::SdfPath newPath = prim.GetPath().AppendChild( pxr::TfToken( name.string() ) );
	pxr::UsdGeomXform newXform = pxr::UsdGeomXform::Define( m_root->getStage(), newPath );

	return new USDScene( m_root, new Location( newXform.GetPrim() ) );
}

SceneInterfacePtr USDScene::scene( const SceneInterface::Path &path, SceneInterface::MissingBehaviour missingBehaviour )
{
	pxr::UsdPrim prim = m_location->prim;

	for( const Name &name : path )
	{
		prim = prim.GetChild( pxr::TfToken( name ) );
	}
	return new USDScene( m_root, new Location( prim ) );
}

ConstSceneInterfacePtr USDScene::scene( const SceneInterface::Path &path, SceneInterface::MissingBehaviour missingBehaviour ) const
{
	return const_cast<USDScene *>( this )->scene( path, missingBehaviour );
}

void USDScene::hash( SceneInterface::HashType hashType, double time, MurmurHash &h ) const
{
	SceneInterface::hash( hashType, time, h );

	h.append( hashType );

	switch( hashType )
	{
		case SceneInterface::TransformHash:
			transformHash( time, h );
			break;
		case SceneInterface::AttributesHash:
			break;
		case SceneInterface::BoundHash:
			boundHash( time, h );
			break;
		case SceneInterface::ObjectHash:
			objectHash( time, h );
			break;
		case SceneInterface::ChildNamesHash:
			childNamesHash( time, h );
			break;
		case SceneInterface::HierarchyHash:
			hierarchyHash( time, h );
			break;
	}
}

void USDScene::boundHash( double time, IECore::MurmurHash &h ) const
{
	if( pxr::UsdGeomBoundable boundable = pxr::UsdGeomBoundable( m_location->prim ) )
	{
		h.append( m_location->prim.GetPath().GetString() );
		h.append( m_root->fileName() );

		if( boundable.GetExtentAttr().ValueMightBeTimeVarying() )
		{
			h.append( time );
		}
	}
}

void USDScene::transformHash( double time, IECore::MurmurHash &h ) const
{
	if( pxr::UsdGeomXformable xformable = pxr::UsdGeomXformable( m_location->prim ) )
	{
		h.append( m_location->prim.GetPath().GetString() );
		h.append( m_root->fileName() );

		if( xformable.TransformMightBeTimeVarying() )
		{
			h.append( time );
		}
	}
}

void USDScene::attributeHash ( double time, IECore::MurmurHash &h) const
{

}

void USDScene::objectHash( double time, IECore::MurmurHash &h ) const
{
	if( isConvertible( m_location->prim ) )
	{
		h.append( m_location->prim.GetPath().GetString() );
		h.append( m_root->fileName() );

		if( isTimeVarying( m_location->prim ) )
		{
			h.append( time );
		}
	}
}
void USDScene::childNamesHash( double time, IECore::MurmurHash &h ) const
{
	h.append( m_location->prim.GetPath().GetString() );
	h.append( m_root->fileName() );
}

void USDScene::hierarchyHash( double time, IECore::MurmurHash &h ) const
{
	h.append( m_location->prim.GetPath().GetString() );
	h.append( m_root->fileName() );
	h.append( time );
}

namespace
{

SceneInterface::FileFormatDescription<USDScene> g_descriptionUSD( ".usd", IndexedIO::Read | IndexedIO::Write );
SceneInterface::FileFormatDescription<USDScene> g_descriptionUSDA( ".usda", IndexedIO::Read | IndexedIO::Write );
SceneInterface::FileFormatDescription<USDScene> g_descriptionUSDC( ".usdc", IndexedIO::Read | IndexedIO::Write );

} // namespace
