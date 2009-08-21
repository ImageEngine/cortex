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

#ifndef IECORE_POINTDISTRIBUTION_INL
#define IECORE_POINTDISTRIBUTION_INL

#include "IECore/FastFloat.h"

#include <cassert>

namespace IECore
{

struct PointDistribution::Tile
{
	int n, e, s, w;
	
	std::vector<Tile *> subTiles;
	
	typedef std::vector<Imath::V2f> PointVector;
	PointVector points;
	PointVector subPoints;
};

template<typename DensityFunction, typename PointFunction>
void PointDistribution::operator () ( const Imath::Box2f &bounds, float density, DensityFunction &densitySampler, PointFunction &pointEmitter ) const
{
	Imath::Box2i bi;
	bi.min.x = fastFloatFloor( bounds.min.x );
	bi.max.x = fastFloatFloor( bounds.max.x );
	bi.min.y = fastFloatFloor( bounds.min.y );
	bi.max.y = fastFloatFloor( bounds.max.y );

	for( int x=bi.min.x; x<=bi.max.x; x++ )
	{
		for( int y=bi.min.y; y<=bi.max.y; y++ )
		{
			unsigned sw = hash( x, y );
			unsigned nw = hash( x, y+1 );
			unsigned ne = hash( x+1, y+1 );
			unsigned se = hash( x+1, y );
			int w = (sw + nw) % 2;
			int n = (nw + ne) % 2;
			int e = (ne + se) % 2;
			bool tileFound = false;
			for( unsigned i=0; i<m_tiles.size(); i++ )
			{
				const Tile *tile = &m_tiles[i];
				if( tile->w==w && tile->n==n && tile->e==e )
				{
					processTile( *tile, Imath::V2f( x, y ), bounds, density, densitySampler, pointEmitter );
					tileFound = true;
					break;
				}
			}
			assert( tileFound );
		}
	}

}

template<typename DensityFunction, typename PointFunction>
void PointDistribution::processTile( const Tile &tile, const Imath::V2f &bottomLeft, const Imath::Box2f &bounds, float density, DensityFunction &densitySampler, PointFunction &pointEmitter ) const
{
	unsigned potentialPoints = std::min( tile.points.size(), (size_t)density );
	float factor = 1.0f / density;
	for( unsigned i=0; i<potentialPoints; i++ )
	{
		const Imath::V2f &p = bottomLeft + tile.points[i];
		if( !bounds.intersects( p ) )
		{
			continue;
		}
		
		if( densitySampler( p ) < i * factor )
		{
			continue;
		}
		
		pointEmitter( p );
	}

	recurseTile( tile, bottomLeft, 0, bounds, density, densitySampler, pointEmitter );
}

template<typename DensityFunction, typename PointFunction>
void PointDistribution::recurseTile( const Tile &tile, const Imath::V2f &bottomLeft, unsigned level, const Imath::Box2f &bounds, float density, DensityFunction &densitySampler, PointFunction &pointEmitter ) const
{
	float tileSize = 1.0f / powf( (float)m_numSubTiles, (float)level );
	
	Imath::Box2f tileBound( bottomLeft, bottomLeft + Imath::V2f( tileSize ) );
	if( !tileBound.intersects( bounds ) )
	{
		return;
	}
	
	float tileArea = tileSize * tileSize;
	float numPointsInTile = density * tileArea;
	unsigned potentialPoints = std::min( tile.subPoints.size(), (size_t)numPointsInTile - tile.points.size() );
	
	float factor = 1.0f / ( numPointsInTile );
	
	for( unsigned i=0; i<potentialPoints; i++ )
	{
		const Imath::V2f &p = bottomLeft + tile.subPoints[i] * tileSize;
		if( !bounds.intersects( p ) )
		{
			continue;
		}
		
		if( densitySampler( p ) < ( i + tile.points.size() ) * factor )
		{
			continue;
		}
		
		pointEmitter( p );
	}
	
	if( numPointsInTile - tile.points.size() > tile.subPoints.size() )
	{
		for( int y=0; y<m_numSubTiles; y++ )
		{
			for( int x=0; x<m_numSubTiles; x++ )
			{
				Imath::V2f newBottomLeft = bottomLeft + Imath::V2f( x, y ) * tileSize / m_numSubTiles;
				recurseTile( *(tile.subTiles[y*m_numSubTiles + x]), newBottomLeft, level + 1, bounds, density, densitySampler, pointEmitter );
			}
		}
	}
}

inline unsigned int PointDistribution::hash( int x, int y ) const
{
	unsigned int h = m_perm[x & (m_permSize-1)];
	return m_perm[h + (y & (m_permSize-1))];
}
		
} // namespace IECore

#endif // IECORE_POINTDISTRIBUTION_INL
