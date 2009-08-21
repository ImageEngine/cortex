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

#ifndef IECORE_POINTDISTRIBUTION_H
#define IECORE_POINTDISTRIBUTION_H

#include "OpenEXR/ImathVec.h"
#include "OpenEXR/ImathBox.h"

#include "boost/noncopyable.hpp"

#include <vector>

namespace IECore
{

/// An implementation of the following paper for producing nice 2d point
/// distributions with varying density.
///
///	Recursive Wang Tiles for Real-Time Blue Noise
///	Johannes Kopf, Daniel Cohen-Or, Oliver Deussen, Dani Lischinski
///	In ACM Transactions on Graphics 25, 3 (Proc. SIGGRAPH 2006)
class PointDistribution : public boost::noncopyable
{

	public :
	
		/// Constructor takes the filename of a tile set. We use the set
		/// found at http://johanneskopf.de/publications/blue_noise/tilesets/tileset_2048.dat
		PointDistribution( const std::string &tileSet );
	
		/// Returns points in the box specified by bounds.
		///
		/// density specifies the number of points generated per unit area if the densitySampler always returns 1.
		/// 
		/// densitySampler must have signature float( const Imath::V2f &pos ) and return a density in the range 0-1.
		///
		/// pointEmitter is called for each point generated and must have the signature void( const Imath::V2f &pos ).
		template<typename DensityFunction, typename PointFunction>
		void operator () ( const Imath::Box2f &bounds, float density, DensityFunction &densitySampler, PointFunction &pointEmitter ) const;
	
		/// Returns a reference to a static PointDistribution which can be shared by anyone who needs one. This
		/// distribution uses the tile set pointed to by the CORTEX_POINTDISTRIBUTION_TILESET environment variable.
		static const PointDistribution &defaultInstance();
	
	private :

		struct Tile;
		
		template<typename DensityFunction, typename PointFunction>
		void processTile( const Tile &tile, const Imath::V2f &bottomLeft, const Imath::Box2f &bounds, float density, DensityFunction &densitySampler, PointFunction &pointEmitter ) const;

		template<typename DensityFunction, typename PointFunction>
		void recurseTile( const Tile &tile, const Imath::V2f &bottomLeft, unsigned level, const Imath::Box2f &bounds, float density, DensityFunction &densitySampler, PointFunction &pointEmitter ) const;
	
		typedef std::vector<Tile> TileVector;
		TileVector m_tiles;
		
		int m_numSubTiles; // number of subtiles in one axis (so there are m_numSubTiles * m_numSubTiles child tiles really)
		
		inline unsigned int hash( int x, int y ) const;
		static const unsigned int m_permSize = 256;
		std::vector<unsigned int> m_perm;
		
};

} // namespace IECore

#include "IECore/PointDistribution.inl"

#endif // IECORE_POINTDISTRIBUTION_H
