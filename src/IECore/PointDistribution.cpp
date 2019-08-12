#include "IECore/PointDistribution.h"
#include "IECore/Exception.h"
#include "IECore/ImathRandAdapter.h"

#include "OpenEXR/ImathRandom.h"

#include <fstream>
#include <algorithm>

using namespace IECore;

PointDistribution::PointDistribution( const std::string &tileSet )
	:	m_numSubTiles( 0 )
{
	std::ifstream f( tileSet.c_str(), std::ios_base::binary | std::ios_base::in );
	if( f.fail() || !f.is_open() )
	{
		throw IOException( "Unable to open file." );
	}

	// read the header
	//////////////////

	int numTiles = 0;
	f.read( (char *)&numTiles, sizeof( numTiles ) );

	f.read( (char *)&m_numSubTiles, sizeof( m_numSubTiles ) );

	// the subdivs is unused here. the example code to accompany the
	// paper always indexes subdivs[0] in recurseTile() so it seems unclear
	// what to do with more subdivs if they existed.
	int subdivs = 0;
	f.read( (char *)&subdivs, sizeof( subdivs ) );
	assert( subdivs==1 );

	// read each tile
	/////////////////

	m_tiles.resize( numTiles );
	for( int i=0; i<numTiles; i++ )
	{
		Tile &tile = m_tiles[i];

		// wang colors
		f.read( (char *)&tile.n, sizeof( tile.n ) );
		f.read( (char *)&tile.e, sizeof( tile.e ) );
		f.read( (char *)&tile.s, sizeof( tile.s ) );
		f.read( (char *)&tile.w, sizeof( tile.w ) );
		tile.n = tile.n % 2; // the cohen et al tileset is actually made up of two rather than four colors,
		tile.s = tile.s % 2; // as green/red is exclusively north/south and yellow/blue is exclusively east/west

		// pointers to subtiles
		const int subTilesSqr = m_numSubTiles * m_numSubTiles;
		tile.subTiles.resize( subTilesSqr );
		for( int j=0; j<subTilesSqr; j++ )
		{
			int subTileIndex = 0;
			f.read( (char *)&subTileIndex, sizeof( subTileIndex ) );
			tile.subTiles[j] = &(m_tiles[subTileIndex]);
		}

		// points
		int numPoints = 0;
		f.read( (char *)&numPoints, sizeof( numPoints ) );
		tile.points.resize( numPoints );
		for( int j=0; j<numPoints; j++ )
		{
			f.read( (char *)&(tile.points[j]), sizeof( Imath::V2f ) );
			f.seekg( sizeof( int ) * 4, std::ios_base::cur ); // skip chunk of unknown data, presumably used during the tile generation process?
		}

		// subpoints
		int numSubPoints = 0;
		f.read( (char *)&numSubPoints, sizeof( numSubPoints ) );
		tile.subPoints.resize( numSubPoints );
		for( int j=0; j<numSubPoints; j++ )
		{
			f.read( (char *)&(tile.subPoints[j]), sizeof( Imath::V2f ) );
			f.seekg( sizeof( int ) * 4, std::ios_base::cur ); // skip chunk of unknown data, presumably used during the tile generation process?
		}
	}

	// build the permutation table. this is similar to the perlin noise permutation table
	// and is used to hash the integer coordinates of the tiles into random values for
	// choosing which tile to use at the top level.
	//////////////////////////////////////////////////////////////////////////////////////

	m_perm.resize( m_permSize * 2 );

	// fill first half with the values 0-m_permSize
	unsigned int value=0;
	std::vector<unsigned int>::iterator it = m_perm.begin();
	while (it != m_perm.begin()+m_permSize)
	{
		*it++ = value++;
	}
	ImathRandAdapter<Imath::Rand32> random( 101 );
	std::random_shuffle( m_perm.begin(), m_perm.begin() + m_permSize, random );
	// fill second half of table with a copy of first half
	std::copy( m_perm.begin(), m_perm.begin() + m_permSize, m_perm.begin() + m_permSize );

}

const PointDistribution &PointDistribution::defaultInstance()
{
	static PointDistribution *p = nullptr;
	if( !p )
	{
		const char *f = getenv( "CORTEX_POINTDISTRIBUTION_TILESET" );
		if( !f )
		{
			throw Exception( "CORTEX_POINTDISTRIBUTION_TILESET environment variable not set." );
		}
		p = new PointDistribution( f );
	}
	return *p;
}
