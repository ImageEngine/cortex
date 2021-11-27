//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2018, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_INDEXEDIOALGO_H
#define IE_CORE_INDEXEDIOALGO_H

#include "IECore/Export.h"
#include "IECore/IndexedIO.h"

#include <array>
#include <iostream>
#include <iomanip>
#include <array>

namespace IECore
{
namespace IndexedIOAlgo
{

/// Histogram of blockSize and byte totals for 64 power 2 bins.
/// Template function so we can use atomics for multithreaded writes but can be converted
/// easily to another integer type.
template<typename T>
struct FileStats
{
	static constexpr unsigned int numBins = 64;
	/// number of bytes per bin
	std::array<T, numBins> numBytes;
	/// number of blocks per bin
	std::array<T, numBins> numBlocks;

	FileStats()
	{
		for (unsigned int i = 0; i < numBins;++i)
		{
			numBytes[i] = 0UL;
			numBlocks[i] = 0UL;
		}
	}

	template<typename S>
	FileStats(const FileStats<S> &other )
	{
		for (unsigned int i = 0; i < numBins;++i)
		{
			numBytes[i] = other.numBytes[i];
			numBlocks[i] = other.numBlocks[i];
		}

	}

	void addBlock(size_t blockSize)
	{
		for (unsigned int i = 0; i < numBins; ++i)
		{
			if ( blockSize <= (1U << i) )
			{
				numBlocks[i]++;
				numBytes[i]+= blockSize;
				break;
			}
		}
	}

	int maxNonZeroBin() const
	{
		int maxBlock = 0;
		for (unsigned int i = 0; i < numBins; ++i)
		{
			if (numBlocks[i] > 0)
			{
				maxBlock = i;
			}
		}
		return maxBlock;
	}

	size_t totalBlocks() const
	{
		size_t total = 0;
		for (unsigned int i = 0; i < numBins;++i)
		{
			total += numBlocks[i];
		}
		return total;
	}

	size_t totalBytes() const
	{
		size_t total = 0;
		for (unsigned int i = 0; i < numBins;++i)
		{
			total += numBytes[i];
		}
		return total;
	}
};

/// Recursively copy from 'src' to 'dst'
IECORE_API void copy(const IndexedIO *src, IndexedIO *dst );

/// Completely read an IndexedIO in parallel gathering statistics as we read.
/// This function is used for performance monitoring
IECORE_API FileStats<size_t> parallelReadAll( const IndexedIO *src );

template<typename T>
inline std::ostream &operator <<( std::ostream &s, const FileStats<T> &stats)
{
	int column0 = 8;
	int column1 = 24;
	int column2 = 16;
	int column3 = 22;
	int column4 = 16;
	int column5 = 22;

	s << std::setw(column0) << "bin" << std::setw( column1 ) << "bin Size" << std::setw( column2 );
	s << "num blocks" << std::setw(column3) << "% blocks";
	s << std::setw( column4 ) <<  "num bytes" <<  std::setw(column5) << "% bytes" << std::endl;
	s << std::setw(column0) << std::string( column0 - 1, '-' )
	  << std::setw(column1) << std::string( column1 - 1, '-' )
	  << std::setw(column2) << std::string( column2 - 1, '-' )
	  << std::setw(column3) << std::string( column3 - 1, '-' )
	  << std::setw(column4) << std::string( column4 - 1, '-' )
	  << std::setw(column4) << std::string( column5 - 1, '-' )
	  << std::endl;

	size_t totalBlocks = stats.totalBlocks();
	size_t totalBytes = stats.totalBytes();

	size_t previousBinSize = 0;
	for (int i = 0; i <= stats.maxNonZeroBin(); ++i )
	{
		size_t binSize = 1 << i;
		std::stringstream ss;
		ss << "(" << previousBinSize << " - " << binSize << "]";

		s << std::setw( column0 ) << i <<  std::setw( column1 ) << ss.str()
		  << std::setw( column2 ) << stats.numBlocks[i] << std::setw( column3 ) << std::string( stats.numBlocks[i] * 20 / totalBlocks, '*' )
		  << std::setw( column4 ) << stats.numBytes[i] << std::setw( column5 ) << std::string( stats.numBytes[i] * 20 / totalBytes, '*' )
		  << std::endl;

		previousBinSize = binSize;
	}

	s << std::setw( column0 ) << "" << std::setw( column1 ) << "" << std::setw( column2 ) << totalBlocks << std::setw( column3 ) << "" << std::setw( column4 ) <<  totalBytes << std::endl;

	return s;
}

}
}




#endif // IE_CORE_INDEXEDIOALGO_H
