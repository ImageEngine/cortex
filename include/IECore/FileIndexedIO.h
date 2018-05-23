//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_FILEINDEXEDIO_H
#define IE_CORE_FILEINDEXEDIO_H

#include "IECore/Export.h"
#include "IECore/StreamIndexedIO.h"

namespace IECore
{

/// An implementation of StreamIndexedIO which operates within a single file on disk.
/// \ingroup ioGroup
class IECORE_API FileIndexedIO : public StreamIndexedIO
{
	public:

		IE_CORE_DECLARERUNTIMETYPED( FileIndexedIO, StreamIndexedIO );


		static IndexedIOPtr create(const std::string &path, const IndexedIO::EntryIDList &root, IndexedIO::OpenMode mode, const CompoundData *options = nullptr);

		static bool canRead( const std::string &path );

		/// Open or create an file at the given root location
		/// options CompoundData and contain the following:
		/// 	"compressor" : String [ 'blosclz' | 'lz4' | 'lz4hc' | 'snappy' | 'zlib']
		///		"compressionLevel" : Int [ 0 = no compression, 9 = max compression ]
		///		"maxCompressedBlockSize" : UInt [ size of compression block ]
		FileIndexedIO(const std::string &path, const IndexedIO::EntryIDList &root, IndexedIO::OpenMode mode, const CompoundData *options = nullptr);

		~FileIndexedIO() override;

		/// Returns the full file name accessed by this object.
		const std::string &fileName() const;

	protected:

		FileIndexedIO();

		FileIndexedIO( StreamIndexedIO::Node &rootNode );

		// duplicates this object by mapping it to a different root node.
		IndexedIO *duplicate(StreamIndexedIO::Node &rootNode) const override;

		class StreamFile;
};

IE_CORE_DECLAREPTR( FileIndexedIO )

}

#endif // IE_CORE_FILEINDEXEDIO_H
