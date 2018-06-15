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

#include "IECoreScene/FaceVaryingPromotionOp.h"

#include "IECoreScene/PolygonVertexIterator.h"
#include "IECoreScene/private/PrimitiveVariableAlgos.h"

#include "IECore/CompoundParameter.h"
#include "IECore/DespatchTypedData.h"

#include "boost/format.hpp"
#include "boost/regex.hpp"

using namespace IECore;
using namespace IECoreScene;

IE_CORE_DEFINERUNTIMETYPED( FaceVaryingPromotionOp );

FaceVaryingPromotionOp::FaceVaryingPromotionOp() : MeshPrimitiveOp( "Calculates vertex normals for a mesh." )
{
	parameters()->addParameter(
		new StringVectorParameter(
			"primVarNames",
			"A list of names of primitive variables to be promoted to FaceVarying interpolation. "
			"An empty list matches all primitive variables. You may use regular expressions to match "
			"multiple variables with a single entry.",
			new StringVectorData()
		)
	);

	parameters()->addParameter(
		new BoolParameter(
			"promoteUniform",
			"Whether or not to promote Uniform interpolated data to FaceVarying.",
			true
		)
	);

	parameters()->addParameter(
		new BoolParameter(
			"promoteVarying",
			"Whether or not to promote Varying interpolated data to FaceVarying.",
			true
		)
	);

	parameters()->addParameter(
		new BoolParameter(
			"promoteVertex",
			"Whether or not to promote Vertex interpolated data to FaceVarying.",
			true
		)
	);
}

FaceVaryingPromotionOp::~FaceVaryingPromotionOp()
{
}

StringVectorParameter *FaceVaryingPromotionOp::primVarNamesParameter()
{
	return parameters()->parameter<StringVectorParameter>( "primVarNames" );
}

const StringVectorParameter *FaceVaryingPromotionOp::primVarNamesParameter() const
{
	return parameters()->parameter<StringVectorParameter>( "primVarNames" );
}

BoolParameter *FaceVaryingPromotionOp::promoteUniformParameter()
{
	return parameters()->parameter<BoolParameter>( "promoteUniform" );
}

const BoolParameter *FaceVaryingPromotionOp::promoteUniformParameter() const
{
	return parameters()->parameter<BoolParameter>( "promoteUniform" );
}

BoolParameter *FaceVaryingPromotionOp::promoteVaryingParameter()
{
	return parameters()->parameter<BoolParameter>( "promoteVarying" );
}

const BoolParameter *FaceVaryingPromotionOp::promoteVaryingParameter() const
{
	return parameters()->parameter<BoolParameter>( "promoteVarying" );
}

BoolParameter *FaceVaryingPromotionOp::promoteVertexParameter()
{
	return parameters()->parameter<BoolParameter>( "promoteVertex" );
}

const BoolParameter *FaceVaryingPromotionOp::promoteVertexParameter() const
{
	return parameters()->parameter<BoolParameter>( "promoteVertex" );
}

struct FaceVaryingPromotionOp::Promoter
{
	typedef DataPtr ReturnType;

	Promoter( const std::vector<int> &vertsPerFace,  const std::vector<int> &vertIds )
		:	m_interpolation( PrimitiveVariable::Invalid ), m_vertsPerFace( vertsPerFace ), m_vertIds( vertIds )
	{
	}

	void setInterpolation( PrimitiveVariable::Interpolation interpolation )
	{
		m_interpolation = interpolation;
	}

	template<typename T>
	ReturnType operator()( T *data )
	{
		typedef typename T::ValueType Container;
		typedef typename Container::const_iterator ConstIterator;

		typename T::Ptr result = new T;
		result->writable().reserve( m_vertIds.size() );
		std::back_insert_iterator<Container> inserter( result->writable() );
		switch( m_interpolation )
		{
			case PrimitiveVariable::Uniform :
			{
				ConstIterator dataIt = data->readable().begin();
				for( std::vector<int>::const_iterator it=m_vertsPerFace.begin(); it!=m_vertsPerFace.end(); ++it, ++dataIt )
				{
					std::fill_n( inserter, *it, *dataIt );
				}
				break;
			}
			case PrimitiveVariable::Vertex :
			case PrimitiveVariable::Varying :
			{
				std::copy(
					PolygonVertexIterator<ConstIterator>( m_vertIds.begin(), data->readable().begin() ),
					PolygonVertexIterator<ConstIterator>( m_vertIds.end(), data->readable().begin() ),
					inserter
				);
				break;
			}
			default :
				assert( 0 ); // shouldn't get here
		}

		assert( result->readable().size() == m_vertIds.size() );

		IECoreScene::PrimitiveVariableAlgos::GeometricInterpretationCopier<T> copier;
		copier( data, result.get() );

		return result;
	}

	private :

		PrimitiveVariable::Interpolation m_interpolation;
		const std::vector<int> &m_vertsPerFace;
		const std::vector<int> &m_vertIds;

};

void FaceVaryingPromotionOp::modifyTypedPrimitive( MeshPrimitive *mesh, const CompoundObject *operands )
{
	const std::vector<std::string> &names = operands->member<StringVectorData>( "primVarNames" )->readable();
	std::vector<boost::regex> regexes;
	for( std::vector<std::string>::const_iterator it=names.begin(); it!=names.end(); ++it )
	{
		regexes.push_back( boost::regex( *it ) );
	}

	bool promoteUniform = operands->member<BoolData>( "promoteUniform" )->readable();
	bool promoteVarying = operands->member<BoolData>( "promoteVarying" )->readable();
	bool promoteVertex = operands->member<BoolData>( "promoteVertex" )->readable();

	Promoter promoter( mesh->verticesPerFace()->readable(), mesh->vertexIds()->readable() );
	for( PrimitiveVariableMap::iterator it=mesh->variables.begin(); it!=mesh->variables.end(); ++it )
	{
		switch( it->second.interpolation )
		{
			case PrimitiveVariable::Invalid :
			case PrimitiveVariable::Constant :
			case PrimitiveVariable::FaceVarying :
				continue;
			case PrimitiveVariable::Uniform :
				if( !promoteUniform )
				{
					continue;
				}
				break;
			case PrimitiveVariable::Varying :
				if( !promoteVarying )
				{
					continue;
				}
				break;
			case PrimitiveVariable::Vertex :
				if( !promoteVertex )
				{
					continue;
				}
				break;
		}

		if( regexes.size() )
		{
			bool matched = false;
			for( std::vector<boost::regex>::const_iterator rIt=regexes.begin(); rIt!=regexes.end(); ++rIt )
			{
				if( boost::regex_match( it->first, *rIt ) )
				{
					matched = true;
					break;
				}
			}

			if( !matched )
			{
				continue;
			}
		}

		if( !mesh->isPrimitiveVariableValid( it->second ) )
		{
			throw Exception( boost::str( boost::format( "Primitive variable \"%s\" is not valid." ) % it->first ) );
		}

		promoter.setInterpolation( it->second.interpolation );

		if( it->second.indices )
		{
			it->second.indices = runTimeCast<IntVectorData>( despatchTypedData<Promoter, TypeTraits::IsVectorTypedData>( it->second.indices.get(), promoter ) );
		}
		else
		{
			it->second.data = despatchTypedData<Promoter, TypeTraits::IsVectorTypedData>( it->second.data.get(), promoter );
		}

		it->second.interpolation = PrimitiveVariable::FaceVarying;

		assert( mesh->isPrimitiveVariableValid( it->second ) );
	}
}
