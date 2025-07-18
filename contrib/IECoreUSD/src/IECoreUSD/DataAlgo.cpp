//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2020, Cinesite VFX Ltd. All rights reserved.
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

#include "IECoreUSD/DataAlgo.h"

#include "IECore/CompoundData.h"
#include "IECore/DataAlgo.h"
#include "IECore/MessageHandler.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "pxr/base/gf/matrix3f.h"
#include "pxr/base/gf/matrix3d.h"
#include "pxr/base/gf/matrix4f.h"
#include "pxr/base/gf/matrix4d.h"

#include "pxr/usd/sdf/layerUtils.h"
#include "pxr/usd/sdf/schema.h"
IECORE_POP_DEFAULT_VISIBILITY

#include "boost/unordered_map.hpp"

#ifdef _MSC_VER
#include <filesystem>
#endif

using namespace std;
using namespace pxr;
using namespace IECore;
using namespace IECoreUSD;

//////////////////////////////////////////////////////////////////////////
// Conversion between Sdf Role and GeometricData::Interpretation
//////////////////////////////////////////////////////////////////////////

namespace
{

static const TfToken g_point( "Point" );
static const TfToken g_vector( "Vector" );
static const TfToken g_normal( "Normal" );
static const TfToken g_textureCoordinate( "TextureCoordinate" );
static const TfToken g_color( "Color" );

GeometricData::Interpretation interpretation( TfToken role )
{
	if( role == g_point )
	{
		return GeometricData::Point;
	}
	else if( role == g_vector )
	{
		return GeometricData::Vector;
	}
	else if( role == g_normal )
	{
		return GeometricData::Normal;
	}
	else if( role == g_textureCoordinate )
	{
		return GeometricData::UV;
	}
	else if( role == g_color )
	{
		return GeometricData::Color;
	}
	return GeometricData::None;
}

#ifdef _MSC_VER

static const bool g_forceAssetPathForwardSlash = []() -> bool {
	const char *c = getenv( "IECOREUSD_FORCE_ASSET_PATH_FORWARD_SLASH" );
	if( !c )
	{
		return true;
	}
	return strcmp( c, "0" );
}();

#endif

} // namespace


//////////////////////////////////////////////////////////////////////////
// Implementation of `fromUSD()`
//////////////////////////////////////////////////////////////////////////

namespace
{

template<typename T>
void setInterpretation( GeometricTypedData<T> *data, GeometricData::Interpretation interpretation )
{
	data->setInterpretation( interpretation );
}

void setInterpretation( Data *data, GeometricData::Interpretation interpretation )
{
}

template<typename T>
IECore::DataPtr dataFromValue( const pxr::VtValue &value, GeometricData::Interpretation interpretation, bool arrayAccepted )
{
	using CortexDataType = typename USDTypeTraits<T>::CortexDataType;
	typename CortexDataType::Ptr d = new CortexDataType( DataAlgo::fromUSD( value.Get<T>() ) );
	setInterpretation( d.get(), interpretation );
	return d;
}

template<typename T>
IECore::DataPtr dataFromArray( const pxr::VtValue &value, GeometricData::Interpretation interpretation, bool arrayAccepted )
{
	auto a = value.Get<VtArray<T>>();
	if( !arrayAccepted )
	{
		if( a.size() != 1 )
		{
			IECore::msg( IECore::Msg::Warning, "IECoreUSD::DataAlgo::fromUSD", "Array not accepted but array length is not 1" );
			return nullptr;
		}
		return dataFromValue<T>( VtValue( a[0] ), interpretation, arrayAccepted );
	}

	auto d = DataAlgo::fromUSD( a );
	setInterpretation( d.get(), interpretation );
	return d;
}

IECore::DataPtr dataFromSdfAssetPath( const SdfAssetPath &assetPath, const pxr::UsdAttribute *attribute = nullptr )
{

#ifdef _MSC_VER
	const std::string p = g_forceAssetPathForwardSlash ? std::filesystem::path( assetPath.GetResolvedPath() ).generic_string() : assetPath.GetResolvedPath();
#else
	const std::string p = assetPath.GetResolvedPath();
#endif

	if( p.size() || !assetPath.GetAssetPath().size() || !attribute )
	{
		return new StringData( p );
	}

	// Path resolution failed, for a couple of possible reasons :
	//
	// - The asset may not exist. In this case we still want to load the source
	//   asset path so that users can debug the problem.
	// - The source path may have contained a `<UDIM>` token, which is not
	//   understood by ArResolvers. USD defers all UDIM handling to UsdImaging,
	//   which is of no use to us. We still want to load the source path
	//   because the `<UDIM>` token will be resolved by the Cortex Renderer.
	//
	// In both cases, the source path may be relative to the layer in which
	// it was authored, which may be buried deep in a complex composition, so
	// we need to find that layer and make the path absolute.

	SdfLayerHandle layer;
	for( const auto &spec : attribute->GetPropertyStack() )
	{
		if(
			spec->HasDefaultValue() ||
			spec->GetLayer()->GetNumTimeSamplesForPath( spec->GetPath() )
		)
		{
#ifdef _MSC_VER
			const std::string result = SdfComputeAssetPathRelativeToLayer( spec->GetLayer(), assetPath.GetAssetPath() );
			return new StringData(
				g_forceAssetPathForwardSlash ? std::filesystem::path( result ).generic_string() : result
			);
#else
			return new StringData(
				SdfComputeAssetPathRelativeToLayer( spec->GetLayer(), assetPath.GetAssetPath() )
			);
#endif
		}
	}

	return new StringData();
}

IECore::DataPtr dataFromSdfAssetPath( const pxr::VtValue &value, GeometricData::Interpretation interpretation, bool arrayAccepted )
{
	return dataFromSdfAssetPath( value.UncheckedGet<SdfAssetPath>() );
}

IECore::DataPtr dataFromDictionary( const pxr::VtValue &value, GeometricData::Interpretation interpretation, bool arrayAccepted )
{
	CompoundDataPtr result = new CompoundData;
	for( const auto &[name, v] : value.Get<VtDictionary>() )
	{
		if( IECore::DataPtr d = IECoreUSD::DataAlgo::fromUSD( v, pxr::SdfValueTypeName() ) )
		{
			result->writable()[name] = d;
		}
	}
	return result;
}

static const std::map<pxr::TfType, IECore::DataPtr (*)( const pxr::VtValue &, GeometricData::Interpretation, bool )> g_fromVtValueConverters = {

	// Numeric types

	{ TfType::Find<bool>(), &dataFromValue<bool> },
	{ TfType::Find<VtArray<bool>>(), &dataFromArray<bool> },
	{ TfType::Find<unsigned char>(), &dataFromValue<unsigned char> },
	{ TfType::Find<VtArray<unsigned char>>(), &dataFromArray<unsigned char> },
	{ TfType::Find<GfHalf>(), &dataFromValue<GfHalf> },
	{ TfType::Find<VtArray<GfHalf>>(), &dataFromArray<GfHalf> },
	{ TfType::Find<float>(), &dataFromValue<float> },
	{ TfType::Find<VtArray<float>>(), &dataFromArray<float> },
	{ TfType::Find<double>(), &dataFromValue<double> },
	{ TfType::Find<VtArray<double>>(), &dataFromArray<double> },
	{ TfType::Find<int>(), &dataFromValue<int> },
	{ TfType::Find<VtArray<int>>(), &dataFromArray<int> },
	{ TfType::Find<unsigned int>(), &dataFromValue<unsigned int> },
	{ TfType::Find<VtArray<unsigned int>>(), &dataFromArray<unsigned int> },
	{ TfType::Find<int64_t>(), &dataFromValue<int64_t> },
	{ TfType::Find<VtArray<int64_t>>(), &dataFromArray<int64_t> },
	{ TfType::Find<uint64_t>(), &dataFromValue<uint64_t> },
	{ TfType::Find<VtArray<uint64_t>>(), &dataFromArray<uint64_t> },

	// Vectors

	{ TfType::Find<GfVec2i>(), &dataFromValue<GfVec2i> },
	{ TfType::Find<VtArray<GfVec2i>>(), &dataFromArray<GfVec2i> },
	{ TfType::Find<GfVec3i>(), &dataFromValue<GfVec3i> },
	{ TfType::Find<VtArray<GfVec3i>>(), &dataFromArray<GfVec3i> },
	{ TfType::Find<GfVec2f>(), &dataFromValue<GfVec2f> },
	{ TfType::Find<VtArray<GfVec2f>>(), &dataFromArray<GfVec2f> },
	{ TfType::Find<GfVec3f>(), &dataFromValue<GfVec3f> },
	{ TfType::Find<VtArray<GfVec3f>>(), &dataFromArray<GfVec3f> },
	{ TfType::Find<GfVec4f>(), &dataFromValue<GfVec4f> },
	{ TfType::Find<VtArray<GfVec4f>>(), &dataFromArray<GfVec4f> },
	{ TfType::Find<GfVec2d>(), &dataFromValue<GfVec2d> },
	{ TfType::Find<VtArray<GfVec2d>>(), &dataFromArray<GfVec2d> },
	{ TfType::Find<GfVec3d>(), &dataFromValue<GfVec3d> },
	{ TfType::Find<VtArray<GfVec3d>>(), &dataFromArray<GfVec3d> },

	// Matrices

	{ TfType::Find<GfMatrix3f>(), &dataFromValue<GfMatrix3f> },
	{ TfType::Find<VtArray<GfMatrix3f>>(), &dataFromArray<GfMatrix3f> },
	{ TfType::Find<GfMatrix4f>(), &dataFromValue<GfMatrix4f> },
	{ TfType::Find<VtArray<GfMatrix4f>>(), &dataFromArray<GfMatrix4f> },
	{ TfType::Find<GfMatrix3d>(), &dataFromValue<GfMatrix3d> },
	{ TfType::Find<VtArray<GfMatrix3d>>(), &dataFromArray<GfMatrix3d> },
	{ TfType::Find<GfMatrix4d>(), &dataFromValue<GfMatrix4d> },
	{ TfType::Find<VtArray<GfMatrix4d>>(), &dataFromArray<GfMatrix4d> },

	// Quaternions

	{ TfType::Find<GfQuath>(), &dataFromValue<GfQuath> },
	{ TfType::Find<VtArray<GfQuath>>(), &dataFromArray<GfQuath> },
	{ TfType::Find<GfQuatf>(), &dataFromValue<GfQuatf> },
	{ TfType::Find<VtArray<GfQuatf>>(), &dataFromArray<GfQuatf> },
	{ TfType::Find<GfQuatd>(), &dataFromValue<GfQuatd> },
	{ TfType::Find<VtArray<GfQuatd>>(), &dataFromArray<GfQuatd> },

	// Strings

	{ TfType::Find<string>(), &dataFromValue<string> },
	{ TfType::Find<VtArray<string>>(), &dataFromArray<string> },
	{ TfType::Find<TfToken>(), &dataFromValue<TfToken> },
	{ TfType::Find<VtArray<TfToken>>(), &dataFromArray<TfToken> },
	{ TfType::Find<SdfAssetPath>(), &dataFromSdfAssetPath },

	// Dictionary

	{ TfType::Find<VtDictionary>(), &dataFromDictionary }

};

template<typename USDType, typename CortexType>
IECore::DataPtr colorDataFromValue( const pxr::VtValue &value, bool arrayAccepted )
{
	return new TypedData<CortexType>( reinterpret_cast<const CortexType &>( value.Get<USDType>() ) );
}

template<typename USDType, typename CortexType>
IECore::DataPtr colorDataFromArray( const pxr::VtValue &value, bool arrayAccepted )
{
	const auto &array = value.Get<VtArray<USDType>>();
	if( !arrayAccepted )
	{
		if( array.size() != 1 )
		{
			IECore::msg( IECore::Msg::Warning, "IECoreUSD::DataAlgo::fromUSD", "Array not accepted but array length is not 1" );
			return nullptr;
		}
		return new TypedData<CortexType>( DataAlgo::fromUSD( array[0] ) );
	}

	return new TypedData<vector<CortexType>>(
		vector<CortexType>(
			reinterpret_cast<const CortexType *>( array.cdata() ),
			reinterpret_cast<const CortexType *>( array.cdata() ) + array.size()
		)
	);
}

static const std::map<pxr::TfType, std::function<IECore::DataPtr ( const pxr::VtValue &, bool )>> g_fromVtValueColorConverters = {

	{ TfType::Find<GfVec3f>(), &colorDataFromValue<GfVec3f, Imath::Color3f> },
	{ TfType::Find<VtArray<GfVec3f>>(), &colorDataFromArray<GfVec3f, Imath::Color3f> },
	{ TfType::Find<GfVec4f>(), &colorDataFromValue<GfVec4f, Imath::Color4f> },
	{ TfType::Find<VtArray<GfVec4f>>(), &colorDataFromArray<GfVec4f, Imath::Color4f> }
};

} // namespace

IECore::DataPtr IECoreUSD::DataAlgo::fromUSD( const pxr::VtValue &value, const pxr::SdfValueTypeName &valueTypeName, bool arrayAccepted )
{
	GeometricData::Interpretation i;
	TfType type;
	if( valueTypeName )
	{
		i = interpretation( valueTypeName.GetRole() );
		type = valueTypeName.GetType();
	}
	else
	{
		i = GeometricData::Interpretation::None;
		type = value.GetType();
	}

	if( i == GeometricData::Color )
	{
		// Colors can not be identified by TfType because they borrow GfVec3,
		// so they require their own dispatch table.
		const auto it = g_fromVtValueColorConverters.find( type );
		if( it == g_fromVtValueColorConverters.end() )
		{
			return nullptr;
		}
		return it->second( value, arrayAccepted );
	}

	const auto it = g_fromVtValueConverters.find( type );
	if( it == g_fromVtValueConverters.end() )
	{
		return nullptr;
	}
	return it->second( value, i, arrayAccepted );
}

IECore::DataPtr IECoreUSD::DataAlgo::fromUSD( const pxr::UsdAttribute &attribute, pxr::UsdTimeCode time, bool arrayAccepted )
{
	pxr::VtValue value;
	if( !attribute.Get( &value, time ) )
	{
		return nullptr;
	}

	if( value.IsHolding<SdfAssetPath>() )
	{
		// Special case to deal with resolution of UDIM textures.
		return dataFromSdfAssetPath( value.UncheckedGet<SdfAssetPath>(), &attribute );
	}
	else
	{
		return DataAlgo::fromUSD( value, attribute.GetTypeName(), arrayAccepted );
	}
}

//////////////////////////////////////////////////////////////////////////
// Implementation of `toUSD()`
//////////////////////////////////////////////////////////////////////////

namespace
{

struct VtValueFromData
{

	template<typename T>
	VtValue operator()( const IECore::TypedData<vector<T>> *data, bool arrayRequired, typename std::enable_if<CortexTypeTraits<T>::BitwiseEquivalent>::type *enabler = nullptr ) const
	{
		using USDType = typename CortexTypeTraits<T>::USDType;
		using ArrayType = VtArray<USDType>;
		ArrayType array;
		array.assign(
			reinterpret_cast<const USDType *>( data->readable().data() ),
			reinterpret_cast<const USDType *>( data->readable().data() + data->readable().size() )
		);
		return VtValue( array );
	}

	template<typename T>
	VtValue operator()( const IECore::TypedData<vector<T>> *data, bool arrayRequired, typename std::enable_if<!std::is_void<typename CortexTypeTraits<T>::USDType>::value && !CortexTypeTraits<T>::BitwiseEquivalent>::type *enabler = nullptr ) const
	{
		using USDType = typename CortexTypeTraits<T>::USDType;
		using ArrayType = VtArray<USDType>;
		ArrayType array;
		array.reserve( data->readable().size() );
		for( const auto &e : data->readable() )
		{
			array.push_back( DataAlgo::toUSD( e ) );
		}
		return VtValue( array );
	}

	// Specialisation because `vector<bool>` is not laid out as an array of bools.
	VtValue operator()( const IECore::BoolVectorData *data, bool arrayRequired ) const
	{
		VtBoolArray array;
		array.assign( data->readable().begin(), data->readable().end() );
		return VtValue( array );
	}

	template<typename T>
	VtValue operator()( const IECore::TypedData<T> *data, bool arrayRequired, typename std::enable_if<!std::is_void<typename CortexTypeTraits<T>::USDType>::value>::type *enabler = nullptr ) const
	{
		if( arrayRequired )
		{
			using USDType = typename CortexTypeTraits<T>::USDType;
			using ArrayType = VtArray<USDType>;
			return VtValue( ArrayType( 1, DataAlgo::toUSD( data->readable() ) ) );
		}
		return VtValue( DataAlgo::toUSD( data->readable() ) );
	}

	VtValue operator()( const IECore::CompoundData *data, bool arrayRequired )
	{
		if( arrayRequired )
		{
			return VtValue();
		}

		VtDictionary result;
		for( const auto &[name, value] : data->readable() )
		{
			VtValue v = DataAlgo::toUSD( value.get() );
			if( !v.IsEmpty() )
			{
				result[name] = v;
			}
		}

		return VtValue( result );
	}

	VtValue operator()( const IECore::Data *data, bool arrayRequired ) const
	{
		return VtValue();
	}

};

} // namespace

pxr::VtValue IECoreUSD::DataAlgo::toUSD( const IECore::Data *data, bool arrayRequired )
{
	try
	{
		VtValueFromData valueFromData;
		if( auto cd = runTimeCast<const CompoundData>( data ) )
		{
			// Manual dispatch since CompoundData not handled by `dispatch()`.
			return valueFromData( cd, arrayRequired );
		}
		return IECore::dispatch( data, valueFromData, arrayRequired );
	}
	catch( const IECore::Exception & )
	{
		// Type not supported by `dispatch()`.
		return VtValue();
	}
}

pxr::TfToken IECoreUSD::DataAlgo::role( GeometricData::Interpretation interpretation )
{
	switch( interpretation )
	{
		case GeometricData::Point : return g_point;
		case GeometricData::Vector : return g_vector;
		case GeometricData::Normal : return g_normal;
		case GeometricData::UV : return g_textureCoordinate;
		case GeometricData::Color : return g_color;
		default : return TfToken();
	}
}

//////////////////////////////////////////////////////////////////////////
// Implementation of `valueTypeName()`
//////////////////////////////////////////////////////////////////////////

namespace
{

struct VtValueTypeNameFromData
{

	// Geometric data

	template<typename T>
	SdfValueTypeName operator()( const IECore::GeometricTypedData<vector<T>> *data, typename std::enable_if<CortexTypeTraits<T>::BitwiseEquivalent>::type *enabler = nullptr ) const
	{
		using ArrayType = VtArray<typename CortexTypeTraits<T>::USDType>;
		const auto &s = SdfSchema::GetInstance();
		return s.FindType( TfType::Find<ArrayType>(), DataAlgo::role( data->getInterpretation() ) );
	}

	template<typename T>
	SdfValueTypeName operator()( const IECore::GeometricTypedData<T> *data ) const
	{
		const auto &s = SdfSchema::GetInstance();
		return s.FindType( TfType::Find<typename CortexTypeTraits<T>::USDType>(), DataAlgo::role( data->getInterpretation() ) );
	}

	// Colors

	template<typename T>
	SdfValueTypeName operator()( const IECore::TypedData<vector<Imath::Color3<T>>> *data ) const
	{
		using ArrayType = VtArray<typename CortexTypeTraits<Imath::Color3<T>>::USDType>;
		const auto &s = SdfSchema::GetInstance();
		return s.FindType( TfType::Find<ArrayType>(), g_color );
	}

	template<typename T>
	SdfValueTypeName operator()( const IECore::TypedData<Imath::Color3<T>> *data ) const
	{
		const auto &s = SdfSchema::GetInstance();
		return s.FindType( TfType::Find<typename CortexTypeTraits<Imath::Color3<T>>::USDType>(), g_color );
	}

	template<typename T>
	SdfValueTypeName operator()( const IECore::TypedData<vector<Imath::Color4<T>>> *data ) const
	{
		using ArrayType = VtArray<typename CortexTypeTraits<Imath::Color4<T>>::USDType>;
		const auto &s = SdfSchema::GetInstance();
		return s.FindType( TfType::Find<ArrayType>(), g_color );
	}

	template<typename T>
	SdfValueTypeName operator()( const IECore::TypedData<Imath::Color4<T>> *data ) const
	{
		const auto &s = SdfSchema::GetInstance();
		return s.FindType( TfType::Find<typename CortexTypeTraits<Imath::Color4<T>>::USDType>(), g_color );
	}

	// Generic

	template<typename T>
	SdfValueTypeName operator()( const IECore::TypedData<vector<T>> *data, typename std::enable_if<!std::is_void<typename CortexTypeTraits<T>::USDType>::value>::type *enabler = nullptr ) const
	{
		using ArrayType = VtArray<typename CortexTypeTraits<T>::USDType>;
		const auto &s = SdfSchema::GetInstance();
		return s.FindType( TfType::Find<ArrayType>() );
	}

	template<typename T>
	SdfValueTypeName operator()( const IECore::TypedData<T> *data ) const
	{
		const auto &s = SdfSchema::GetInstance();
		return s.FindType( TfType::Find<typename CortexTypeTraits<T>::USDType>() );
	}

	SdfValueTypeName operator()( const IECore::Data *data ) const
	{
		return SdfValueTypeName();
	}

};

} // namespace

pxr::SdfValueTypeName IECoreUSD::DataAlgo::valueTypeName( const IECore::Data *data )
{
	try
	{
		return IECore::dispatch( data, VtValueTypeNameFromData() );
	}
	catch( const IECore::Exception & )
	{
		// Type not supported by `dispatch()`.
		return SdfValueTypeName();
	}
}
