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
#include "pxr/usd/usdGeom/nurbsCurves.h"
#include "pxr/usd/usdGeom/bboxCache.h"
#include "pxr/usd/usdGeom/tokens.h"

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

float convert( float v )
{
	return v;
}

double convert( double v )
{
	return v;
}

bool convert (bool v )
{
	return v;
}

int convert (int v )
{
	return v;
}

unsigned int convert (unsigned int v )
{
	return v;
}

char convert (char v )
{
	return v;
}

pxr::GfHalf convert (half v )
{
	return pxr::GfHalf(v); // conversion half -> float -> half?
}

Imath::V3d convert( const pxr::GfVec3d &src )
{
	return Imath::V3d( src[0], src[1], src[2] );
}

Imath::V3f convert( const pxr::GfVec3f &src )
{
	return Imath::V3f( src[0], src[1], src[2] );
}

Imath::V4f convert( const pxr::GfVec4f &src )
{
	return Imath::V4f( src[0], src[1], src[2], src[3] );
}

Imath::V2d convert( const pxr::GfVec2d &src )
{
	return Imath::V2d( src[0], src[1] );
}

Imath::V2f convert( const pxr::GfVec2f &src )
{
	return Imath::V2f( src[0], src[1] );
}

Imath::Box3d convert( const pxr::GfBBox3d &src )
{
	const auto &srcBox = src.GetBox();

	return Imath::Box3d( convert( srcBox.GetMin() ), convert( srcBox.GetMax() ) );
}

Imath::M44d convert( const pxr::GfMatrix4d &src )
{
	Imath::M44d r;
	for( int i = 0; i < 4; ++i )
	{
		for( int j = 0; j < 4; ++j )
		{
			r[i][j] = src[i][j];
		}
	}

	return r;
}

IECore::IntVectorDataPtr convert( const pxr::VtIntArray &data )
{
	IECore::IntVectorDataPtr newData = new IECore::IntVectorData();

	std::vector<int> &writable = newData->writable();
	writable.resize( data.size() );
	for( size_t i = 0; i < data.size(); ++i )
	{
		writable[i] = data[i];
	}
	return newData;
}

IECore::V3fVectorDataPtr convert( const pxr::VtVec3fArray &data )
{
	IECore::V3fVectorDataPtr newData = new IECore::V3fVectorData();

	std::vector<Imath::V3f> &writable = newData->writable();
	writable.resize( data.size() );
	for( size_t i = 0; i < data.size(); ++i )
	{
		writable[i] = Imath::V3f( data[i][0], data[i][1], data[i][2] );
	}
	return newData;
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

template<typename DestElementType, typename SourceElementType, template <typename P> class StorageType = IECore::GeometricTypedData>
struct TypedConverter
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
				t[i] = convert( r[i] );
			}

			return d;
		}

		return nullptr;
	}
};


std::string cleanPrimVarName( const std::string &primVarName )
{
	return boost::algorithm::replace_first_copy( primVarName, "primvars:", "" );
}

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
		m_primVar.Get( &value, m_time );

		TypedConverter converter;
		auto p = converter.doConversion( value );

		if( !p )
		{
			IECore::msg(IECore::MessageHandler::Level::Warning, "USDScene", boost::format("Typed conversion failed for PrimVar: %1% type: %2%") % m_primVar.GetName().GetString() %
				m_primVar.GetTypeName().GetAsToken().GetString());
			return;
		}

		pxr::VtIntArray srcIndices;
		m_primVar.GetIndices( &srcIndices, m_time );

		auto indices = !srcIndices.empty() ? convert( srcIndices ) : nullptr;

		std::string cleanedPrimvarName = cleanPrimVarName( m_primVar.GetName() );
		m_primitive->variables[cleanedPrimvarName] = IECoreScene::PrimitiveVariable( interpolation, p, indices );
	}

	IECoreScene::PrimitivePtr m_primitive;
	const pxr::UsdGeomPrimvar &m_primVar;
	pxr::UsdTimeCode m_time;
};

namespace
{
static std::map<pxr::TfToken, std::function<void( PrimVarConverter& converter )> > primvarConversions =
{
	{ pxr::TfToken( "bool[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedConverter<bool, bool, IECore::TypedData> >(); } },
	{ pxr::TfToken( "half[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedConverter<half, pxr::GfHalf, IECore::TypedData > >(); } },
	{ pxr::TfToken( "double[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedConverter<double, double, IECore::TypedData > >();  } },
	{ pxr::TfToken( "int[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedConverter<int, int, IECore::TypedData> >(); } },
	{ pxr::TfToken( "uint[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedConverter<unsigned int, unsigned int, IECore::TypedData> >(); } },
	{ pxr::TfToken( "char[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedConverter<char, char, IECore::TypedData> >(); } },
	{ pxr::TfToken( "float[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedConverter<float, float, IECore::TypedData> >(); } },
	{ pxr::TfToken( "color3f[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedConverter<Imath::V3f, pxr::GfVec3f, IECore::GeometricTypedData> >(); } },
	{ pxr::TfToken( "float3[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedConverter<Imath::V3f, pxr::GfVec3f, IECore::GeometricTypedData> >(); } },
	{ pxr::TfToken( "float2[]" ), []( PrimVarConverter& converter ) { converter.doConversion<TypedConverter<Imath::V2f, pxr::GfVec2f, IECore::GeometricTypedData> >(); } }
};

}

void convertPrimVar( IECoreScene::PrimitivePtr primitive, const pxr::UsdGeomPrimvar &primVar, pxr::UsdTimeCode time )
{
	pxr::TfToken typeToken = primVar.GetTypeName().GetAsToken();

	auto it = primvarConversions.find( typeToken );
	if ( it != primvarConversions.end() )
	{
		PrimVarConverter converter( primitive, primVar, time );
		it->second( converter );
	}
	else
	{
		IECore::msg(IECore::MessageHandler::Level::Warning, "USDScene", boost::format("Unknown type %1% on PrimVar %2% ") % typeToken.GetString() % primVar.GetName().GetString());
	}

}

void convertPrimVars( pxr::UsdGeomImageable imagable, IECoreScene::PrimitivePtr primitive, pxr::UsdTimeCode time )
{
	for( const auto &primVar : imagable.GetPrimvars() )
	{
		convertPrimVar( primitive, primVar, time );
	}
}

IECoreScene::PointsPrimitivePtr convert( pxr::UsdGeomPoints points, pxr::UsdTimeCode time )
{
	pxr::UsdAttribute attr = points.GetPointsAttr();

	pxr::VtVec3fArray pointsData;

	attr.Get( &pointsData, time );

	IECore::V3fVectorDataPtr positionData = convert( pointsData );

	IECoreScene::PointsPrimitivePtr newPoints = new IECoreScene::PointsPrimitive( positionData );

	convertPrimVars( points, newPoints, time );
	return newPoints;
}

IECoreScene::CurvesPrimitivePtr convert( pxr::UsdGeomCurves curves, pxr::UsdTimeCode time )
{
	pxr::UsdAttribute vertexCountsAttr = curves.GetCurveVertexCountsAttr();
	pxr::VtIntArray vertexCountsData;
	vertexCountsAttr.Get( &vertexCountsData, time );
	IECore::IntVectorDataPtr countData = convert( vertexCountsData );

	pxr::UsdAttribute attr = curves.GetPointsAttr();
	pxr::VtVec3fArray pointsData;
	attr.Get( &pointsData, time );
	IECore::V3fVectorDataPtr positionData = convert( pointsData );

	IECoreScene::CurvesPrimitivePtr newCurves = new IECoreScene::CurvesPrimitive( countData, IECore::CubicBasisf::linear(), false, positionData );

	convertPrimVars( curves, newCurves, time );
	return newCurves;
}

IECoreScene::MeshPrimitivePtr convert( pxr::UsdGeomMesh mesh, pxr::UsdTimeCode time )
{
	pxr::UsdAttribute subdivSchemeAttr = mesh.GetSubdivisionSchemeAttr();

	pxr::TfToken subdivScheme;
	subdivSchemeAttr.Get( &subdivScheme );

	pxr::UsdAttribute faceVertexCountsAttr = mesh.GetFaceVertexCountsAttr();

	pxr::VtIntArray faceVertexCounts;
	faceVertexCountsAttr.Get( &faceVertexCounts );
	IECore::IntVectorDataPtr vertexCountData = convert( faceVertexCounts );

	pxr::UsdAttribute faceVertexIndexAttr = mesh.GetFaceVertexIndicesAttr();
	pxr::VtIntArray faceVertexIndices;
	faceVertexIndexAttr.Get( &faceVertexIndices );

	IECore::IntVectorDataPtr vertexIndicesData = convert( faceVertexIndices );

	IECoreScene::MeshPrimitivePtr newMesh = new IECoreScene::MeshPrimitive( vertexCountData, vertexIndicesData );

	pxr::UsdAttribute attr = mesh.GetPointsAttr();
	pxr::VtVec3fArray pointsData;

	attr.Get( &pointsData, time );

	IECore::V3fVectorDataPtr positionData = convert( pointsData );

	convertPrimVars( mesh, newMesh, time );
	newMesh->variables["P"] = IECoreScene::PrimitiveVariable( IECoreScene::PrimitiveVariable::Vertex, positionData );

	if( subdivScheme == pxr::UsdGeomTokens->catmullClark )
	{
		newMesh->setInterpolation( "catmullClark" );
	}

	return newMesh;
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

IECore::ConstObjectPtr convert( pxr::UsdPrim prim, pxr::UsdTimeCode time )
{
	if( pxr::UsdGeomMesh mesh = pxr::UsdGeomMesh( prim ) )
	{
		return convert( mesh, time );
	}

	if( pxr::UsdGeomPoints points = pxr::UsdGeomPoints( prim ) )
	{
		return convert( points, time );
	}

	if( pxr::UsdGeomCurves curves = pxr::UsdGeomCurves( prim ) )
	{
		return convert( curves, time );
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

		const std::string &fileName() const
		{
			return m_fileName;
		}

		virtual pxr::UsdPrim root() const = 0;
		virtual pxr::UsdTimeCode getTime( double timeSeconds ) const = 0;
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

	private:
		pxr::UsdStageRefPtr m_usdStage;
		pxr::UsdPrim m_rootPrim;

		double m_timeCodesPerSecond;
};

class USDScene::Writer : public USDScene::IO
{
	public:
		Writer( const std::string &fileName ) : IO( fileName )
		{
			m_usdStage = pxr::UsdStage::CreateNew( "gaffer" );
			m_rootPrim = m_usdStage->GetPseudoRoot();
		}

		pxr::UsdPrim root() const override
		{
			return m_rootPrim;
		}

		pxr::UsdTimeCode getTime( double timeSeconds ) const override
		{
			return 0.0;
		}

	private:
		pxr::UsdStageRefPtr m_usdStage;
		pxr::UsdPrim m_rootPrim;
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
	if( pxr::UsdGeomBoundable boundable = pxr::UsdGeomBoundable( m_location->prim ) )
	{
		pxr::UsdAttribute attr = boundable.GetExtentAttr();

		pxr::VtArray<pxr::GfVec3f> extents;
		attr.Get<pxr::VtArray<pxr::GfVec3f> >( &extents, m_root->getTime( time ) );

		return Imath::Box3d( convert( extents[0] ), convert( extents[1] ) );
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
	return convert( transform );
}

ConstObjectPtr USDScene::readAttribute( const SceneInterface::Name &name, double time ) const
{
	return nullptr;
}

ConstObjectPtr USDScene::readObject( double time ) const
{
	return convert( m_location->prim, m_root->getTime( time ) );
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
	return true;
}

void USDScene::writeBound( const Imath::Box3d &bound, double time )
{
	throw IECore::NotImplementedException( "USDScene::writeBound not supported" );
}

void USDScene::writeTransform( const Data *transform, double time )
{
	throw IECore::NotImplementedException( "USDScene::writeTransform not supported" );
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
	throw IECore::NotImplementedException( "USDScene::writeAttribute not supported" );
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
	throw IECore::NotImplementedException( "USDScene::writeTags not supported" );
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
	throw IECore::NotImplementedException( "USDScene::writeObject not supported" );
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
		if( i.GetTypeName() == "Xform" || isConvertible( i ) )
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
			throw InvalidArgumentException( "Child creation not supported" );
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
	throw IECore::NotImplementedException( "USDScene::createChild not supported" );
	return nullptr;
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
