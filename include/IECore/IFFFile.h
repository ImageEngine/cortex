//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_IFFFILE_H
#define IE_CORE_IFFFILE_H

#include "IECore/Export.h"
#include "IECore/RefCounted.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "OpenEXR/ImathVec.h"
IECORE_POP_DEFAULT_VISIBILITY

#include <fstream>
#include <vector>

namespace IECore
{

IE_CORE_FORWARDDECLARE( IFFFile );

/// The IFFFile class defines a low level class for reading IFF files.
/// For specific IFF file types use a more specific implementation (i.e. NParticleReader, IFFHairReader, IFFImageReader).
class IECORE_API IFFFile : public RefCounted
{
	public :

		IFFFile( const std::string &fileName );
		~IFFFile() override;

		class Tag;
		class Chunk;

		Chunk *root();

		/// A Tag is a 4 char sequence that identifies the type of Chunk at this point in the file
		/// For efficient comparision Tags store the integer equivalent of the 4 char sequence as an id
		/// Common tag ids may be predefined by the interface that recognizes them. Default tags are defined by GroupTagID
		class IECORE_API Tag
		{
			public :

				Tag( );
				Tag( const char *buffer );
				Tag( std::string str );
				std::string name();
				int id();
				char alignmentByte();
				bool isGroup();

				// FORM is currently the only accepted group type.
				// CAT, LIST, and PROP have not been implemented.
				enum GroupTagID
				{
					kFORM = 1179603533,
					kFOR4 = 1179603508,
					kFOR8 = 1179603512,
				};

				static const int TagSize = 4;

			private :

				char m_a;
				char m_b;
				char m_c;
				char m_d;
				int m_id;
		};

		/// A Chunk is the most basic structure in an IFF file.
		/// It consists of a Tag, an integer representing it's data size, and dataSize number of bytes of arbitrary data.
		/// If the Chunk is a group, it will have an additional Tag indicating the group name.
		/// Groups may contain child Chunks which can be accesed through the ChunkIterators.
		/// \todo: the current implementation only supports reading. It may not be well suited for writing.
		class IECORE_API Chunk
		{
			public :

				friend class IFFFile;

				Tag type();
				unsigned int dataSize();

				bool isGroup();
				Tag groupName();

				typedef std::vector<Chunk>::iterator ChunkIterator;
				ChunkIterator childrenBegin();
				ChunkIterator childrenEnd();

				/// read Chunk data as a single value
				template<typename T>
				void read( T &data );

				/// read Chunk data as a string
				void read( std::string &data );

				/// read Chunk data as a vector of values
				template<typename T>
				size_t read( std::vector<T> &data );

				/// read Chunk data as a vector of Imath::Vec3 values
				template<typename T>
				size_t read( std::vector<Imath::Vec3<T> > &data );

			private :

				Chunk( );
				Chunk( std::string type, unsigned int dataSize, IFFFilePtr file, std::streampos filePosition, int alignmentQuota );

				Tag m_type;
				unsigned int m_dataSize;

				IFFFilePtr m_file;
				std::streampos m_filePosition;

				Tag m_groupName;
				int m_alignmentQuota;
				std::vector<Chunk> m_children;

				// fills m_children
				void ls();

				// reads most member variables from m_file, starting at pos
				void readHeader( std::streampos *pos );

				// reads the data from m_file, storing it in dataBuffer
				template<typename T>
				void readData( T *dataBuffer, unsigned long n );

				// returns the proper byte alignment value for m_type
				int alignmentQuota();

				// returns the number of bytes that can be skipped to fill the group's alignment quota
				int skippableBytes();
		};

	private :

		bool open();
		std::ifstream *m_iStream;
		std::string m_streamFileName;

		Chunk *m_root;

		// reads data from the char buffer into a more specific buffer, accounting for byte order
		template<typename T>
		static void readData( const char *dataBuffer, T *attrBuffer, unsigned long n );
};

} // namespace IECore

#include "IFFFile.inl"

#endif // IE_CORE_IFFFILE_H
