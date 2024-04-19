#include "IECore/PointDistribution.h"
#include "IECore/Exception.h"
#include "IECore/ImathRandAdapter.h"

#include "Imath/ImathRandom.h"

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

	// This was originally generated using `std::random_shuffle`, which did not yield the
	// same results on different platforms and which was removed in C++17. The results here
	// were generated on Linux with `libstdc++`, maintaining compatibility with the majority
	// of previous usage.

	m_perm = { 147, 228, 196, 166, 243, 130, 108, 90, 46, 3, 162, 12, 221, 180, 56, 194, 77,
		84, 241, 8, 175, 250, 224, 34, 44, 246, 40, 23, 103, 26, 106, 212, 189, 98, 30, 114,
		135, 80, 21, 136, 187, 208, 184, 144, 171, 64, 201, 74, 131, 13, 170, 14, 254, 214,
		62, 31, 94, 51, 16, 240, 186, 104, 193, 53, 235, 82, 4, 158, 203, 120, 225, 110, 245,
		59, 101, 102, 54, 206, 93, 70, 33, 69, 88, 41, 251, 24, 49, 142, 139, 43, 249, 52, 220,
		7, 237, 79, 173, 71, 164, 83, 146, 17, 178, 218, 226, 86, 132, 87, 112, 160, 29, 85,
		163, 65, 19, 126, 255, 42, 138, 67, 47, 238, 128, 75, 37, 72, 153, 192, 11, 123, 6,
		129, 183, 113, 197, 252, 222, 127, 157, 152, 213, 81, 39, 97, 60, 174, 105, 73, 143,
		91, 61, 216, 116, 154, 205, 188, 134, 150, 177, 115, 195, 35, 229, 210, 122, 27, 2,
		76, 5, 148, 141, 89, 247, 99, 121, 230, 219, 204, 200, 172, 231, 117, 140, 167, 227,
		156, 253, 181, 236, 232, 209, 9, 45, 100, 78, 161, 22, 63, 179, 1, 25, 248, 182, 198,
		50, 111, 57, 151, 15, 107, 66, 48, 217, 211, 176, 159, 137, 58, 18, 32, 199, 165, 36,
		95, 68, 185, 10, 202, 242, 92, 190, 191, 149, 234, 55, 38, 119, 133, 20, 223, 124, 118,
		244, 155, 0, 233, 168, 169, 109, 145, 28, 215, 125, 96, 239, 207
	};

	assert( m_perm.size() == m_permSize );

	// fill second half of table with a copy of first half
	m_perm.resize( m_permSize * 2 );
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
