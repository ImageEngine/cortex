//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

#include "tbb/tbb.h"

#include "IECore/CompoundParameter.h"
#include "IECore/FaceAreaOp.h"
#include "IECore/PointDistribution.h"
#include "IECore/PointsPrimitive.h"
#include "IECore/MeshPrimitive.h"
#include "IECore/TriangulateOp.h"
#include "IECore/TriangleAlgo.h"

#include "IECore/PointDistributionOp.h"

using namespace IECore;

IE_CORE_DEFINERUNTIMETYPED( PointDistributionOp );

PointDistributionOp::PointDistributionOp()
	: Op(
		"The PointDistributionOp distributes points over a mesh using IECore::PointDistribution in UV space and mapping it to 3d space.",
		new PointsPrimitiveParameter(
			"result",
			"Resulting points distributed over mesh.",
			new PointsPrimitive()
		)
	)
{
	m_meshParameter = new MeshPrimitiveParameter(
		"mesh",
		"The mesh to distribute points over.",
		new MeshPrimitive()
	);
	
	m_densityParameter = new FloatParameter(
		"density",
		"The density of the distributed points.",
		100.0,
		0.0f
	);
	
	m_offsetParameter = new V2fParameter(
		"offset",
		"A UV offset for the PointDistribution",
		Imath::V2f( 0, 0 )
	);
	
	m_densityPrimVarNameParameter = new StringParameter(
		"densityPrimVarName",
		"The primitive variable to use as a density threshold.",
		"density"
	);
	
	m_pRefPrimVarNameParameter = new StringParameter(
		"pRefPrimVarName",	
		"The primitive variable that holds the reference positions.",
		"Pref"
	);

	m_uPrimVarNameParameter = new StringParameter(
		"uPrimVarName",
		"The primitive variable for u coordinates.",
		"s"
	);

	m_vPrimVarNameParameter = new StringParameter(
		"vPrimVarName",
		"The primitive variable for v coordinates.",
		"t"
	);
	
	parameters()->addParameter( m_meshParameter );
	parameters()->addParameter( m_densityParameter );
	parameters()->addParameter( m_offsetParameter );
	parameters()->addParameter( m_densityPrimVarNameParameter );
	parameters()->addParameter( m_pRefPrimVarNameParameter );
	parameters()->addParameter( m_uPrimVarNameParameter );
	parameters()->addParameter( m_vPrimVarNameParameter );
}

PointDistributionOp::~PointDistributionOp()
{
}

MeshPrimitiveParameter * PointDistributionOp::meshParameter()
{
	return m_meshParameter.get();
}

const MeshPrimitiveParameter * PointDistributionOp::meshParameter() const
{
	return m_meshParameter.get();
}

FloatParameter *PointDistributionOp::densityParameter()
{
	return m_densityParameter.get();
}

const FloatParameter *PointDistributionOp::densityParameter() const
{
	return m_densityParameter.get();
}

void PointDistributionOp::processMesh( const IECore::MeshPrimitive *mesh )
{
	if ( !mesh )
	{
		throw InvalidArgumentException( "PointDistributionOp: The input mesh is not valid" );
	}
	
	const V3fVectorData *positions = mesh->variableData<const V3fVectorData>( "P" );
	if ( !positions )
	{
		throw InvalidArgumentException( "PointDistributionOp: The input mesh has no 'P' data" );
	}
	
	TriangulateOpPtr op = new TriangulateOp();
	op->inputParameter()->setValue( const_cast<MeshPrimitive *>( mesh ) );
	op->throwExceptionsParameter()->setTypedValue( false );
	m_mesh = runTimeCast<MeshPrimitive>( op->operate() );
	if ( !m_mesh || !m_mesh->arePrimitiveVariablesValid() )
	{
		throw InvalidArgumentException( "PointDistributionOp: The input mesh could not be triangulated" );
	}
	
	FaceAreaOpPtr faceAreaOp = new FaceAreaOp();
	faceAreaOp->inputParameter()->setValue( m_mesh );
	faceAreaOp->copyParameter()->setTypedValue( false );
	
	// use reference positions if they exist
	std::string pRefPrimVarName = m_pRefPrimVarNameParameter->getTypedValue();
	if( m_mesh->variables.find( pRefPrimVarName ) != m_mesh->variables.end() )
	{
		faceAreaOp->parameters()->parameter<StringParameter>( "pointPrimVar" )->setTypedValue( pRefPrimVarName );
	}
	
	// use specified s and t parameters
	faceAreaOp->parameters()->parameter<StringParameter>( "sPrimVar" )->setTypedValue( m_uPrimVarNameParameter->getTypedValue() );
	faceAreaOp->parameters()->parameter<StringParameter>( "tPrimVar" )->setTypedValue( m_vPrimVarNameParameter->getTypedValue() );
	
	faceAreaOp->operate();
	
	// add a constant density if it doesn't exist already
	std::string densityPrimVarName = m_densityPrimVarNameParameter->getTypedValue();
	if ( m_mesh->variables.find( densityPrimVarName ) == m_mesh->variables.end() )
	{
		m_mesh->variables[densityPrimVarName] = PrimitiveVariable( PrimitiveVariable::Constant, new FloatData( 1.0 ) );
	}
 	
	m_meshEvaluator = new IECore::MeshPrimitiveEvaluator( m_mesh );
	
	assert( m_mesh );
	assert( m_meshEvaluator );

}

struct PointDistributionOp::Emitter
{
	public :
	
		Emitter( MeshPrimitiveEvaluator *evaluator, const PrimitiveVariable &densityVar, std::vector<Imath::V3f> &positions, size_t triangleIndex, const Imath::V2f &v0, const Imath::V2f &v1, const Imath::V2f &v2 )
			: m_meshEvaluator( evaluator ), m_densityVar( densityVar ), m_p( positions ), m_triangleIndex( triangleIndex ), m_v0( v0 ), m_v1( v1 ), m_v2( v2 )
		{
			m_evaluatorResult = staticPointerCast<MeshPrimitiveEvaluator::Result>( m_meshEvaluator->createResult() );
		}

		void operator() ( const Imath::V2f pos, float densityThreshold )
		{
			Imath::V3f bary;
			if( triangleContainsPoint( m_v0, m_v1, m_v2, pos, bary ) )
			{
				m_meshEvaluator->barycentricPosition( m_triangleIndex, bary, m_evaluatorResult.get() );
				if ( m_evaluatorResult->floatPrimVar( m_densityVar ) >= densityThreshold )
				{
					m_p.push_back( m_evaluatorResult->point() );
				}
			}
		}

	private :
	
		MeshPrimitiveEvaluator *m_meshEvaluator;
		const PrimitiveVariable &m_densityVar;
		std::vector<Imath::V3f> &m_p;
		size_t m_triangleIndex;
		Imath::V2f m_v0;
		Imath::V2f m_v1;
		Imath::V2f m_v2;
		
		MeshPrimitiveEvaluator::ResultPtr m_evaluatorResult;

};

struct PointDistributionOp::Generator
{
	public :
		
		Generator( MeshPrimitiveEvaluator *evaluator, const std::vector<float> &s, const std::vector<float> &t, const std::vector<float> &faceArea, const std::vector<float> &textureArea, float density, const PrimitiveVariable &densityVar, Imath::V2f offset )
			: m_meshEvaluator( evaluator ), m_s( s ), m_t( t ), m_faceArea( faceArea ), m_textureArea( textureArea ), m_density( density ), m_densityVar( densityVar ), m_offset( offset ), m_positions()
		{
		}
		
		Generator( Generator &that, tbb::split )
			: m_meshEvaluator( that.m_meshEvaluator ), m_s( that.m_s ), m_t( that.m_t ), m_faceArea( that.m_faceArea ), m_textureArea( that.m_textureArea ), m_density( that.m_density ), m_densityVar( that.m_densityVar ), m_offset( that.m_offset ), m_positions()
		{
		}
		
		void operator()( const tbb::blocked_range<size_t> &r )
		{
			for ( size_t i=r.begin(); i!=r.end(); ++i )
			{
				float textureDensity = m_density * m_faceArea[i] / m_textureArea[i];
				
				size_t v0I = i * 3;
				size_t v1I = v0I + 1;
				size_t v2I = v1I + 1;

				Imath::V2f st0( m_s[v0I], m_t[v0I] );
				Imath::V2f st1( m_s[v1I], m_t[v1I] );
				Imath::V2f st2( m_s[v2I], m_t[v2I] );

				st0 += m_offset;
				st1 += m_offset;
				st2 += m_offset;

				Imath::Box2f stBounds;
				stBounds.extendBy( st0 );
				stBounds.extendBy( st1 );
				stBounds.extendBy( st2 );
				
				Emitter emitter( m_meshEvaluator, m_densityVar, m_positions, i, st0, st1, st2 );
				PointDistribution::defaultInstance()( stBounds, textureDensity, emitter );
			}
		}
	
		void join( Generator &that )
		{
			size_t numThisPoints = this->m_positions.size();
			this->m_positions.resize( numThisPoints + that.m_positions.size() );
			std::copy( that.m_positions.begin(), that.m_positions.end(), this->m_positions.begin() + numThisPoints );
		}
		
		std::vector<Imath::V3f> &positions()
		{
			return m_positions;
		}

	private :
		
		MeshPrimitiveEvaluator *m_meshEvaluator;
		const std::vector<float> &m_s;
		const std::vector<float> &m_t;
		const std::vector<float> &m_faceArea;
		const std::vector<float> &m_textureArea;
		float m_density;
		const PrimitiveVariable &m_densityVar;
		Imath::V2f m_offset;
		
		std::vector<Imath::V3f> m_positions;

};

ObjectPtr PointDistributionOp::doOperation( const CompoundObject * operands )
{
	processMesh( m_meshParameter->getTypedValue<MeshPrimitive>() );
	
	float density = m_densityParameter->getNumericValue();
	/// \todo: densityMask could optionally come from an image primitive
	const PrimitiveVariable &densityVar = m_mesh->variables.find( m_densityPrimVarNameParameter->getTypedValue() )->second;
	
	Imath::V2f offset = m_offsetParameter->getTypedValue();
	
	const std::vector<float> &s = m_mesh->variableData<FloatVectorData>( m_uPrimVarNameParameter->getTypedValue(), PrimitiveVariable::FaceVarying )->readable();
	const std::vector<float> &t = m_mesh->variableData<FloatVectorData>( m_vPrimVarNameParameter->getTypedValue(), PrimitiveVariable::FaceVarying )->readable();
	const std::vector<float> &faceArea = m_mesh->variableData<FloatVectorData>( "faceArea", PrimitiveVariable::Uniform )->readable();
	const std::vector<float> &textureArea = m_mesh->variableData<FloatVectorData>( "textureArea", PrimitiveVariable::Uniform )->readable();
	
	size_t numFaces = m_mesh->verticesPerFace()->readable().size();	
	Generator gen( m_meshEvaluator.get(), s, t, faceArea, textureArea, density, densityVar, offset );
	tbb::parallel_reduce( tbb::blocked_range<size_t>( 0, numFaces ), gen );
	
	V3fVectorDataPtr pData = new V3fVectorData();
	pData->writable().swap( gen.positions() );
	
	return new PointsPrimitive( pData );
}
