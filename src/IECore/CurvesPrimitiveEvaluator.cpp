//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2012, Image Engine Design Inc. All rights reserved.
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

#include "OpenEXR/ImathFun.h"

#include "IECore/CurvesPrimitiveEvaluator.h"
#include "IECore/CurvesPrimitive.h"
#include "IECore/Exception.h"
#include "IECore/FastFloat.h"
#include "IECore/LineSegment.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"

using namespace IECore;
using namespace Imath;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( CurvesPrimitiveEvaluator );

PrimitiveEvaluator::Description<CurvesPrimitiveEvaluator> CurvesPrimitiveEvaluator::g_evaluatorDescription;

//////////////////////////////////////////////////////////////////////////
// Implementation of Result
//////////////////////////////////////////////////////////////////////////

CurvesPrimitiveEvaluator::Result::Result( PrimitiveVariable p, bool linear, bool periodic )
	:	m_p( p ), m_linear( linear )
{
	// this is slightly curious. what we're doing is choosing a specific instantiation of
	// the init() template function and storing it as a pointer to a member function. because
	// the periodic and linear arguments are template arguments rather than parameters, the
	// compiler makes a specifically optimised version of the function for each of our four
	// cases. this lets us write the init() function in a readable form and leaves the optimisation
	// to the compiler.
	if( periodic )
	{
		if( m_linear )
		{
			m_init = &Result::init<true, true>;
		}
		else
		{
			m_init = &Result::init<false, true>;
		}
	}
	else
	{
		if( linear )
		{
			m_init = &Result::init<true, false>;
		}
		else
		{
			m_init = &Result::init<false, false>;
		}
	}
}

template<class T>
T CurvesPrimitiveEvaluator::Result::primVar( const PrimitiveVariable &primVar, const float *coefficients ) const
{
	switch( primVar.interpolation )
	{
		case PrimitiveVariable::Constant :
			{
				const TypedData<T> *d = static_cast<TypedData<T> *>( primVar.data.get() );
				return d->readable();
			}
			break;
		case PrimitiveVariable::Uniform :
			{
				const vector<T> &d = static_cast<TypedData<vector<T> > *>( primVar.data.get() )->readable();
				return d[m_curveIndex];
			}
		case PrimitiveVariable::Vertex :
			{
				const vector<T> &d = static_cast<TypedData<vector<T> > *>( primVar.data.get() )->readable();
				
				if ( m_linear )
				{
					return	(T)( coefficients[0] * d[m_vertexDataIndices[0]] +
						coefficients[1] * d[m_vertexDataIndices[1]] );
				}
				else
				{
					return	(T)( coefficients[0] * d[m_vertexDataIndices[0]] +
						coefficients[1] * d[m_vertexDataIndices[1]] +
						coefficients[2] * d[m_vertexDataIndices[2]] +
						coefficients[3] * d[m_vertexDataIndices[3]] );
				}
			}
		case PrimitiveVariable::Varying :
		case PrimitiveVariable::FaceVarying :
			{
				const vector<T> &d = static_cast<TypedData<vector<T> > *>( primVar.data.get() )->readable();
				return lerp( d[m_varyingDataIndices[0]], d[m_varyingDataIndices[1]], m_segmentV );
			}
		default :
			throw InvalidArgumentException( "PrimitiveVariable has invalid interpolation" );
	}
}

Imath::V3f CurvesPrimitiveEvaluator::Result::point() const
{
	return primVar<V3f>( m_p, m_coefficients );
}

Imath::V3f CurvesPrimitiveEvaluator::Result::normal() const
{
	throw NotImplementedException( __PRETTY_FUNCTION__ );
}

Imath::V2f CurvesPrimitiveEvaluator::Result::uv() const
{
	return V2f( 0, m_v );
}

Imath::V3f CurvesPrimitiveEvaluator::Result::uTangent() const
{
	throw NotImplementedException( __PRETTY_FUNCTION__ );
}

Imath::V3f CurvesPrimitiveEvaluator::Result::vTangent() const
{
	return primVar<V3f>( m_p, m_derivativeCoefficients );
}

unsigned CurvesPrimitiveEvaluator::Result::curveIndex() const
{
	return m_curveIndex;
}

Imath::V3f CurvesPrimitiveEvaluator::Result::vectorPrimVar( const PrimitiveVariable &pv ) const
{
	return primVar<V3f>( pv, m_coefficients );
}

V2f CurvesPrimitiveEvaluator::Result::vec2PrimVar( const PrimitiveVariable &pv ) const
{
	return primVar<V2f>( pv, m_coefficients );
}

float CurvesPrimitiveEvaluator::Result::floatPrimVar( const PrimitiveVariable &pv ) const
{
	return primVar<float>( pv, m_coefficients );
}

int CurvesPrimitiveEvaluator::Result::intPrimVar( const PrimitiveVariable &pv ) const
{
	return primVar<int>( pv, m_coefficients );
}

const std::string &CurvesPrimitiveEvaluator::Result::stringPrimVar( const PrimitiveVariable &pv ) const
{
	switch( pv.interpolation )
	{
		case PrimitiveVariable::Constant :
			{
				const TypedData<string> *d = static_cast<TypedData<string> *>( pv.data.get() );
				return d->readable();
			}
			break;
		case PrimitiveVariable::Uniform :
			{
				const vector<string> &d = static_cast<TypedData<vector<string> > *>( pv.data.get() )->readable();
				return d[m_curveIndex];
			}
		default :
			{
				throw InvalidArgumentException( "Can only evaluate string PrimitiveVariables with Constant or Uniform interpolation." );
			}
	}
}

Imath::Color3f CurvesPrimitiveEvaluator::Result::colorPrimVar( const PrimitiveVariable &pv ) const
{
	return primVar<Color3f>( pv, m_coefficients );
}

half CurvesPrimitiveEvaluator::Result::halfPrimVar( const PrimitiveVariable &pv ) const
{
	return primVar<half>( pv, m_coefficients );
}

template<bool linear, bool periodic>
void CurvesPrimitiveEvaluator::Result::init( unsigned curveIndex, float v, const CurvesPrimitiveEvaluator *evaluator )
{
	m_curveIndex = curveIndex;
	m_v = v;

	int numVertices = evaluator->m_verticesPerCurve[curveIndex];
	const CubicBasisf &basis = evaluator->m_curvesPrimitive->basis();

	unsigned numSegments = 0;
	if( linear )
	{
		if( periodic )
		{
			numSegments = numVertices;
		}
		else
		{
			numSegments = numVertices - 1;
		}
	}
	else
	{
		if( periodic )
		{
			numSegments = numVertices / basis.step;
		}
		else
		{
			numSegments = (numVertices - 4 ) / basis.step + 1;
		}
	}
	
	float vv = v * numSegments;
	unsigned segment = min( (unsigned)fastFloatFloor( vv ), numSegments - 1 );
	m_segmentV = vv - segment;
	
	unsigned o = evaluator->m_vertexDataOffsets[m_curveIndex];
	unsigned i = segment * basis.step;
	
	if( linear )
	{
		m_coefficients[0] = 1.0f - m_segmentV;
		m_coefficients[1] = m_segmentV;
		m_derivativeCoefficients[0] = 1.0f;
		m_derivativeCoefficients[1] = -1.0f;
		m_vertexDataIndices[0] = m_varyingDataIndices[0] = o + i;
		if( periodic )
		{
			m_vertexDataIndices[1] = m_varyingDataIndices[1] = o + ( ( i + 1 ) % numVertices );
		}
		else
		{
			m_vertexDataIndices[1] = m_varyingDataIndices[1] = m_vertexDataIndices[0] + 1;
		}
	}
	else
	{
		basis.coefficients( m_segmentV, m_coefficients );
		basis.derivativeCoefficients( m_segmentV, m_derivativeCoefficients );
		
		if( periodic )
		{
			m_vertexDataIndices[0] = o + i;
			m_vertexDataIndices[1] = o + ( ( i + 1 ) % numVertices );
			m_vertexDataIndices[2] = o + ( ( i + 2 ) % numVertices );
			m_vertexDataIndices[3] = o + ( ( i + 3 ) % numVertices );

			m_varyingDataIndices[0] = evaluator->m_varyingDataOffsets[m_curveIndex] + segment;
			m_varyingDataIndices[1] = evaluator->m_varyingDataOffsets[m_curveIndex] + ( segment % numSegments );
		}
		else
		{
			m_vertexDataIndices[0] = evaluator->m_vertexDataOffsets[m_curveIndex] + segment * basis.step;
			m_vertexDataIndices[1] = m_vertexDataIndices[0] + 1;
			m_vertexDataIndices[2] = m_vertexDataIndices[1] + 1;
			m_vertexDataIndices[3] = m_vertexDataIndices[2] + 1;

			m_varyingDataIndices[0] = evaluator->m_varyingDataOffsets[m_curveIndex] + segment;
			m_varyingDataIndices[1] = m_varyingDataIndices[0] + 1;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Implementation of CurvesPrimitiveEvaluator::Line
//////////////////////////////////////////////////////////////////////////

struct CurvesPrimitiveEvaluator::Line
{
	public :
	
		Line( const V3f &p1, const V3f &p2, unsigned curveIndex, float vMin, float vMax )
			:	m_lineSegment( p1, p2 ), m_curveIndex( curveIndex ), m_vMin( vMin ), m_vMax( vMax )
		{
		}
	
		/// \todo I wonder if this could be derived on the fly from a min/max flag per axis and the
		/// bound for the Line? It would save memory and perhaps lead to speedups.
		const LineSegment3f &lineSegment() const { return m_lineSegment; }
		int curveIndex() const { return m_curveIndex; }
		float vMin() const { return m_vMin; }
		float vMax() const { return m_vMax; }
	
		static int linesPerCurveSegment() { return 20; };
	
	private :
	
		LineSegment3f m_lineSegment;
		int m_curveIndex;
		float m_vMin;
		float m_vMax;
		
};
				
//////////////////////////////////////////////////////////////////////////
// Implementation of Evaluator
//////////////////////////////////////////////////////////////////////////

CurvesPrimitiveEvaluator::CurvesPrimitiveEvaluator( ConstCurvesPrimitivePtr curves )
	:	m_curvesPrimitive( curves->copy() ), m_verticesPerCurve( m_curvesPrimitive->verticesPerCurve()->readable() ), m_haveTree( false )
{
	m_vertexDataOffsets.reserve( m_verticesPerCurve.size() );
	m_varyingDataOffsets.reserve( m_verticesPerCurve.size() );
	int vertexDataOffset = 0;
	int varyingDataOffset = 0;
	for( unsigned i=0; i<m_verticesPerCurve.size(); i++ )
	{
		m_vertexDataOffsets.push_back( vertexDataOffset );
		vertexDataOffset += m_verticesPerCurve[i];
		
		m_varyingDataOffsets.push_back( varyingDataOffset );
		varyingDataOffset += m_curvesPrimitive->variableSize( PrimitiveVariable::Varying, i );
	}
	
	PrimitiveVariableMap::iterator pIt = m_curvesPrimitive->variables.find( "P" );
	if( pIt==m_curvesPrimitive->variables.end() )
	{
		throw InvalidArgumentException( "No PrimitiveVariable named P on CurvesPrimitive." );
	}
	m_p = pIt->second;
}

CurvesPrimitiveEvaluator::~CurvesPrimitiveEvaluator()
{
}

PrimitiveEvaluatorPtr CurvesPrimitiveEvaluator::create( ConstPrimitivePtr primitive )
{
	ConstCurvesPrimitivePtr points = runTimeCast<const CurvesPrimitive>( primitive );
	assert( points );
	return new CurvesPrimitiveEvaluator( points );
}

ConstPrimitivePtr CurvesPrimitiveEvaluator::primitive() const
{
	return m_curvesPrimitive;
}

PrimitiveEvaluator::ResultPtr CurvesPrimitiveEvaluator::createResult() const
{
	return new Result( m_p, m_curvesPrimitive->basis() == CubicBasisf::linear(), m_curvesPrimitive->periodic() );
}

void CurvesPrimitiveEvaluator::validateResult( PrimitiveEvaluator::Result *result ) const
{
	if( ! dynamic_cast<CurvesPrimitiveEvaluator::Result *>( result ) )
	{
		throw InvalidArgumentException( "CurvesPrimitiveEvaluator: Invalid result type" );
	}
}

float CurvesPrimitiveEvaluator::surfaceArea() const
{
	throw NotImplementedException( __PRETTY_FUNCTION__ );
}

float CurvesPrimitiveEvaluator::volume() const
{
	throw NotImplementedException( __PRETTY_FUNCTION__ );
}

Imath::V3f CurvesPrimitiveEvaluator::centerOfGravity() const
{
	throw NotImplementedException( __PRETTY_FUNCTION__ );
}

bool CurvesPrimitiveEvaluator::closestPoint( const Imath::V3f &p, PrimitiveEvaluator::Result *result ) const
{
	if( !m_verticesPerCurve.size() )
	{
		return false;
	}

	Result *typedResult = static_cast<Result *>( result );
	// the cast isn't pretty but i think is the best of the alternatives. we want to delay building the tree until the first
	// closestPoint() query so people don't pay the overhead if they're just using other queries. the alternative to
	// the cast is to make the tree members mutable, but i'd rather keep them immutable so that the compiler tells us if
	// we do anything wrong during the query.
	const_cast<CurvesPrimitiveEvaluator *>( this )->buildTree();

	unsigned curveIndex = 0;
	float v = -1;
	float distSquared = Imath::limits<float>::max();
	closestPointWalk( m_tree.rootIndex(), p, curveIndex, v, distSquared );
	(typedResult->*typedResult->m_init)( curveIndex, v, this );
	
	return true;
}

void CurvesPrimitiveEvaluator::closestPointWalk( Box3fTree::NodeIndex nodeIndex, const Imath::V3f &p, unsigned &curveIndex, float &v, float &closestDistSquared ) const
{
	assert( m_haveTree );

	const Box3fTree::Node &node = m_tree.node( nodeIndex );
	if( node.isLeaf() )
	{
		Box3fTree::Iterator *permLast = node.permLast();
		for( Box3fTree::Iterator *perm = node.permFirst(); perm!=permLast; perm++ )
		{
			const Line &line = m_treeLines[*perm - m_treeBounds.begin()];
			
			float t;
			V3f cp = line.lineSegment().closestPointTo( p, t );
			float d2 = (cp - p).length2();
			
			if( d2 < closestDistSquared )
			{
				closestDistSquared = d2;
				curveIndex = line.curveIndex();
				v = lerp( line.vMin(), line.vMax(), t );
			}
		}
	}
	else
	{

		Box3fTree::NodeIndex lowChild = Box3fTree::lowChildIndex( nodeIndex );
		Box3fTree::NodeIndex highChild = Box3fTree::highChildIndex( nodeIndex );

		float d2Low = ( closestPointInBox( p, m_tree.node( lowChild ).bound() ) - p ).length2();
		float d2High = ( closestPointInBox( p, m_tree.node( highChild ).bound() ) - p ).length2();
	
		if( d2Low < d2High )
		{
			closestPointWalk( lowChild, p, curveIndex, v, closestDistSquared );
			if( d2High < closestDistSquared )
			{
				closestPointWalk( highChild, p, curveIndex, v, closestDistSquared );
			}
		}
		else
		{
			closestPointWalk( highChild, p, curveIndex, v, closestDistSquared );
			if( d2Low < closestDistSquared )
			{
				closestPointWalk( lowChild, p, curveIndex, v, closestDistSquared );
			}
		}

	}
}

bool CurvesPrimitiveEvaluator::pointAtUV( const Imath::V2f &uv, PrimitiveEvaluator::Result *result ) const
{
	return pointAtV( 0, uv[1], result );
}

bool CurvesPrimitiveEvaluator::intersectionPoint( const Imath::V3f &origin, const Imath::V3f &direction,
	PrimitiveEvaluator::Result *result, float maxDistance ) const
{
	throw NotImplementedException( __PRETTY_FUNCTION__ );
}

int CurvesPrimitiveEvaluator::intersectionPoints( const Imath::V3f &origin, const Imath::V3f &direction,
	std::vector<PrimitiveEvaluator::ResultPtr> &results, float maxDistance ) const
{
	throw NotImplementedException( __PRETTY_FUNCTION__ );
}

bool CurvesPrimitiveEvaluator::pointAtV( unsigned curveIndex, float v, PrimitiveEvaluator::Result *result ) const
{
	if( curveIndex >= m_verticesPerCurve.size() || v < 0.0f || v > 1.0f )
	{
		return false;
	}
	Result *typedResult = static_cast<Result *>( result );
	(typedResult->*typedResult->m_init)( curveIndex, v, this );
	return true;
}


float CurvesPrimitiveEvaluator::integrateCurve( unsigned curveIndex, float vStart, float vEnd, int samples, Result& typedResult ) const
{
	// get first curve point:
	(typedResult.*typedResult.m_init)( curveIndex, vStart, this );
	Imath::V3f current;
	Imath::V3f previous = typedResult.point();
	
	// take ten samples along the curve, and measure the length of the resulting polyline:
	const float vStep = ( vEnd - vStart ) / samples;
	float length = 0;
	float v;
	for( int i=1; i <= samples; ++i )
	{
		v = vStart + vStep * i;
		(typedResult.*typedResult.m_init)( curveIndex, v, this );
		current = typedResult.point();
		
		length += ( current - previous ).length();
		
		previous = current;
	}
	
	return length;
}


float CurvesPrimitiveEvaluator::curveLength( unsigned curveIndex, float vStart, float vEnd ) const
{
	
	if( curveIndex >= m_verticesPerCurve.size() || vStart >= vEnd || vStart < 0.0f || vStart > 1.0f || vEnd < 0.0f || vEnd > 1.0f )
	{
		return 0.0f;
	}
	
	size_t nSegments = m_curvesPrimitive->numSegments( curveIndex );
	if ( m_curvesPrimitive->basis() == CubicBasisf::linear() )
	{
		
		size_t curvePts = m_curvesPrimitive->variableSize( PrimitiveVariable::Vertex, curveIndex );
		
		const std::vector<V3f> &p = static_cast<const V3fVectorData *>( m_p.data.get() )->readable();
		
		float lowerPos = vStart * nSegments;
		float upperPos = vEnd * nSegments;
		
		// find curve relative vertex indices that bound the interval we're measuring:
		size_t lowerVertex = (unsigned)floor( lowerPos );
		size_t upperVertex = (unsigned)ceil( upperPos );
		
		if( upperVertex == lowerVertex + 1 )
		{
			// the interval lands entirely in one segment, so we just measure the length of the segment
			// and scale it by the appropriate factor:
			
			if( upperVertex == curvePts )
			{
				// this can happen if the curve's periodic:
				upperVertex = 0;
			}
			
			// offset into the main vertex list:
			lowerVertex += m_vertexDataOffsets[ curveIndex ];
			upperVertex += m_vertexDataOffsets[ curveIndex ];
			
			return ( p[ upperVertex ] - p[ lowerVertex ] ).length() * ( upperPos - lowerPos );
		}
		else
		{
			float length = 0.0f;
			
			float lowerFrac = lowerVertex + 1 - lowerPos;
			float upperFrac = upperPos - ( upperVertex - 1 );
			
			// offset into the main vertex list:
			lowerVertex += m_vertexDataOffsets[ curveIndex ];
			upperVertex += m_vertexDataOffsets[ curveIndex ];
			
			// work out length in lower interval:
			length = ( p[ lowerVertex + 1 ] - p[ lowerVertex ] ).length() * lowerFrac;
			
			// work out length in upper interval:
			if( upperVertex - m_vertexDataOffsets[ curveIndex ] == curvePts )
			{
				// periodic!
				length += ( p[ m_vertexDataOffsets[ curveIndex ] ] - p[ upperVertex - 1 ] ).length() * upperFrac;
			}
			else
			{
				length += ( p[ upperVertex ] - p[ upperVertex - 1 ] ).length() * upperFrac;
			}
			
			// work out the intervals in between:
			for( size_t currentVertex = lowerVertex + 1; currentVertex < ( upperVertex - 1 ); ++currentVertex )
			{
				length += ( p[ currentVertex + 1 ] - p[ currentVertex ] ).length();
			}
			return length;
		}
		
	}
	else
	{
		// find curve relative vertex indices that bound the interval we're measuring:
		size_t lowerVertex = (unsigned)floor( vStart * nSegments );
		size_t upperVertex = (unsigned)ceil( vEnd * nSegments );
		
		Result typedResult( m_p, false, m_curvesPrimitive->periodic() );
		
		const int curveSamples = 10;
		
		if( upperVertex - lowerVertex == 1 )
		{
			// entirely in a single interval, so just directly integrate with ten samples:
			return integrateCurve( curveIndex, vStart, vEnd, curveSamples, typedResult );
		}
		else
		{
			// integrate from vStart to first control point after vStart:
			float length = integrateCurve( curveIndex, vStart, ( lowerVertex + 1.0f ) / nSegments, curveSamples, typedResult );
			
			// measure the intervals between the start and end intervals:
			for( size_t currentVertex = lowerVertex + 1; currentVertex < ( upperVertex - 1 ); ++currentVertex )
			{
				length += integrateCurve( curveIndex, float(currentVertex) / nSegments, float(currentVertex + 1) / nSegments, curveSamples, typedResult );
			}
			
			// integrate from the last control point before vEnd to vEnd:
			length += integrateCurve( curveIndex, ( upperVertex - 1.0f ) / nSegments, vEnd, curveSamples, typedResult );
			
			return length;
		}
	}
}

void CurvesPrimitiveEvaluator::buildTree()
{
	if( m_haveTree )
	{
		return;
	}
	
	TreeMutex::scoped_lock lock( m_treeMutex );
	if( m_haveTree )
	{
		// another thread may have built the tree while we waited for the mutex
		return;
	}
	
	bool linear = m_curvesPrimitive->basis() == CubicBasisf::linear();
	const std::vector<V3f> &p = static_cast<const V3fVectorData *>( m_p.data.get() )->readable();
	PrimitiveEvaluator::ResultPtr result = createResult();
	
	size_t numCurves = m_curvesPrimitive->numCurves();
	for( size_t curveIndex = 0; curveIndex<numCurves; curveIndex++ )
	{
		if( linear )
		{
			int numVertices = m_verticesPerCurve[curveIndex];
			int vertIndex = m_vertexDataOffsets[curveIndex];
			float prevV = 0.0f;
			for( int i=0; i<numVertices; i++, vertIndex++ )
			{
				float v = clamp( (float)i/(float)(numVertices-1), 0.0f, 1.0f );
				if( i!=0 )
				{
					Box3f b;
					b.extendBy( p[vertIndex-1] );
					b.extendBy( p[vertIndex] );
					m_treeBounds.push_back( b );
					m_treeLines.push_back( Line(  p[vertIndex-1], p[vertIndex], curveIndex, prevV, v ) );
				}
				prevV = v;				
			}
		}
		else
		{
			unsigned numSegments = m_curvesPrimitive->numSegments( curveIndex );
			int steps = numSegments * Line::linesPerCurveSegment();
			V3f prevP( 0 );
			float prevV = 0;
			for( int i=0; i<steps; i++ )
			{
				float v = clamp( (float)i/(float)(steps-1), 0.0f, 1.0f );
				pointAtV( curveIndex, v, result.get() );
				V3f p = result->point();
				if( i!=0 )
				{
					Box3f b;
					b.extendBy( prevP );
					b.extendBy( p );
					m_treeBounds.push_back( b );
					m_treeLines.push_back( Line( prevP, p, curveIndex, prevV, v ) );
				}

				prevP = p;
				prevV = v;
			}
		}
	}
	
	m_tree.init( m_treeBounds.begin(), m_treeBounds.end() );
	m_haveTree = true;
}

const std::vector<int> &CurvesPrimitiveEvaluator::verticesPerCurve() const
{
	return m_verticesPerCurve;
}

const std::vector<int> &CurvesPrimitiveEvaluator::vertexDataOffsets() const
{
	return m_vertexDataOffsets;
}

const std::vector<int> &CurvesPrimitiveEvaluator::varyingDataOffsets() const
{
	return m_varyingDataOffsets;
}
