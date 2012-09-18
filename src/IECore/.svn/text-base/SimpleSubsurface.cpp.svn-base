//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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
#include "OpenEXR/ImathBoxAlgo.h"

#include "IECore/SimpleSubsurface.h"
#include "IECore/ColorAlgo.h"
#include "IECore/Lookup.h"

using namespace IECore;
using namespace std;
using namespace Imath;

/// This class is used to monkey around with the spline values used for the falloff.
class SplineRemapper
{
	public :

		SplineRemapper( const SplinefColor3f &s )
			:	m_mult( 1.0f ), m_spline( s )
		{
			// compute a normalisation factor for the spline.
			// the intention here is that a flat white surface with white illumination
			// of value 1 should have a subsurface value of 1 regardless of the falloff curve.
			// this allows falloff to be adjusted freely while overall intensity remains
			// pretty much constant. we compute the normalization just by integrating the falloff
			// function over a disk of unit radius.
			float sum = 0;
			const unsigned numSteps = 1000;
			for( unsigned i=0; i<numSteps; i++ )
			{
				float r = (float)i/(numSteps-1);
				float v = luminance( (*this)( r ) );
				float a = 2.0f * M_PI * r;
				sum += a * v;
			}
			sum /= numSteps;
			m_mult = 1.0f/sum;
		}

		Color3f operator () ( float t ) const
		{
			// the division by t causes samples at smaller radii to be weighted much more
			// strongly than those at the outer radius. this seems to give more intuitive
			// results as otherwise the values at the inner end of the falloff spline don't
			// get much of a look in against the values at the outer end of the falloff (there
			// are far more samples on the outer radius than at the centre).
			float w = m_mult / max( 0.01f, t );
			return w * m_spline( t );
		};
	
	private :
	
		float m_mult;
		SplinefColor3f m_spline;
		
};

struct SimpleSubsurface::PrivateData
{

	V3fVectorDataPtr points;
	Color3fVectorDataPtr colors;

	V3fTree tree;
	
	LookupfColor3f falloff;
		
	std::vector<Imath::V3f> nodeCentroids;
	std::vector<Imath::Color3f> nodeColors;
	std::vector<Imath::Box3f> nodeBounds;

};

SimpleSubsurface::SimpleSubsurface( ConstV3fVectorDataPtr p, ConstColor3fVectorDataPtr c, ConstFloatVectorDataPtr a, const SplinefColor3f &falloff )
{
	m_privateData = boost::shared_ptr<PrivateData>( new PrivateData );
	
	m_privateData->points = p->copy();
	m_privateData->colors = c->copy();
	
	const vector<float> &areas = a->readable();
	vector<Color3f> &colors = m_privateData->colors->writable();
	for( size_t i=0; i<colors.size(); i++ )
	{
		colors[i] *= areas[i];
	}
	
	m_privateData->tree.init( m_privateData->points->readable().begin(), m_privateData->points->readable().end() );

	m_privateData->falloff.init( SplineRemapper( falloff ), 0, 1, 100 );
	
	m_privateData->nodeCentroids.resize( m_privateData->tree.numNodes() );
	m_privateData->nodeColors.resize( m_privateData->tree.numNodes() );
	m_privateData->nodeBounds.resize( m_privateData->tree.numNodes() );
	buildWalk( m_privateData->tree.rootIndex() );
	
}

SimpleSubsurface::~SimpleSubsurface()
{
}

void SimpleSubsurface::buildWalk( Tree::NodeIndex nodeIndex )
{
	const Tree::Node &node = m_privateData->tree.node( nodeIndex );
		
	if( node.isLeaf() )
	{
		float totalWeight = 0;
		Color3f nodeColor( 0 );
		V3f centroid( 0 );
		Box3f bound;
		vector<V3f>::const_iterator pointsBegin =  m_privateData->points->readable().begin();
		for( Tree::Iterator *p = node.permFirst(); p!=node.permLast(); p++ )
		{
			const Color3f &c =  m_privateData->colors->readable()[*p - pointsBegin];
			float weight = luminance( c );
			nodeColor += c;
			centroid += **p * weight;
			totalWeight += weight;
			bound.extendBy( **p );
		}
		
		m_privateData->nodeCentroids[nodeIndex] = centroid / ( totalWeight > 0.0f ? totalWeight : 1.0f );
		m_privateData->nodeColors[nodeIndex] = nodeColor;
		m_privateData->nodeBounds[nodeIndex] = bound;
	}
	else
	{
		Tree::NodeIndex lowIndex =  m_privateData->tree.lowChildIndex( nodeIndex );
		Tree::NodeIndex highIndex =  m_privateData->tree.highChildIndex( nodeIndex );
		buildWalk( lowIndex );
		buildWalk( highIndex );
		
		Box3f bound;
		bound.extendBy( m_privateData->nodeBounds[lowIndex] );
		bound.extendBy( m_privateData->nodeBounds[highIndex] );
		m_privateData->nodeBounds[nodeIndex] = bound;
		
		float wLow = luminance( m_privateData->nodeColors[lowIndex] );
		float wHigh = luminance( m_privateData->nodeColors[highIndex] );
		float wSum = wLow + wHigh;
		m_privateData->nodeCentroids[nodeIndex] = ( wLow * m_privateData->nodeCentroids[lowIndex] + wHigh * m_privateData->nodeCentroids[highIndex] ) / ( wSum > 0.0f ? wSum : 1.0f );
		
		m_privateData->nodeColors[nodeIndex] = m_privateData->nodeColors[lowIndex] + m_privateData->nodeColors[highIndex];
	}
}

Imath::Color3f SimpleSubsurface::lookup( const Imath::V3f &p, float radius ) const
{	
	Color3f result( 0 );
	lookupWalk( m_privateData->tree.rootIndex(), p, radius, result );
	return result;
}

inline void SimpleSubsurface::accum( const Imath::V3f &p, float radius, const Imath::V3f &sampleP, const Imath::Color3f &sampleC, Imath::Color3f &result ) const
{
	float d = (p - sampleP).length();	
	Color3f w = m_privateData->falloff( d/radius );
	result += w * sampleC;
}

void SimpleSubsurface::lookupWalk( Tree::NodeIndex nodeIndex, const Imath::V3f &p, float radius, Imath::Color3f &result ) const
{
	
	const Tree::Node &node = m_privateData->tree.node( nodeIndex );
	
	if( node.isLeaf() )
	{
		Tree::Iterator *permLast = node.permLast();
		const std::vector<V3f>::const_iterator pointsBegin = m_privateData->points->readable().begin();
		const std::vector<Color3f> &colors = m_privateData->colors->readable();
		for( Tree::Iterator *pp = node.permFirst(); pp!=permLast; pp++ )
		{
			accum( p, radius, **pp, colors[*pp - pointsBegin], result );
		}
	}
	else
	{
		float d = (m_privateData->nodeCentroids[nodeIndex] - p).length();
		float o = m_privateData->nodeBounds[nodeIndex].size().length();
		float t = Imath::Math<float>::atan2( o, d );
		if( t < 0.4f ) /// \todo probably needs tuning!!!
		{
			accum( p, radius, m_privateData->nodeCentroids[nodeIndex], m_privateData->nodeColors[nodeIndex], result );
		}
		else
		{
			// keep on walking
			
			float d = p[node.cutAxis()] - node.cutValue();
			Tree::NodeIndex firstChild, secondChild;
			if( d > 0.0 )
			{
				firstChild = m_privateData->tree.highChildIndex( nodeIndex );
				secondChild = m_privateData->tree.lowChildIndex( nodeIndex );
			}
			else
			{
				firstChild = m_privateData->tree.lowChildIndex( nodeIndex );
				secondChild = m_privateData->tree.highChildIndex( nodeIndex );
			}

			lookupWalk( firstChild, p, radius, result );
			if( d < radius )
			{
				lookupWalk( secondChild, p, radius, result );
			}

		}
		
	}
}
