//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_PDCPARTICLEREADER_H
#define IE_CORE_PDCPARTICLEREADER_H

#include "IECore/ParticleReader.h"
#include "IECore/VectorTypedData.h"

namespace IECore
{

/// The PDCParticleReader class implements the ParticleReader
/// interface for Maya .pdc format particle caches. Percentage filtering
/// of loaded particles is seeded using the particleId attribute, so
/// is not only repeatable but also consistent from frame to frame.
class PDCParticleReader : public ParticleReader
{

	public :

		IE_CORE_DECLARERUNTIMETYPED( PDCParticleReader, ParticleReader );

		PDCParticleReader( );
		PDCParticleReader( const std::string &fileName );
		virtual ~PDCParticleReader();

		static bool canRead( const std::string &fileName );
		
		virtual unsigned long numParticles();
		virtual void attributeNames( std::vector<std::string> &names );
		virtual DataPtr readAttribute( const std::string &name );
					
	private :
	
		static const ReaderDescription<PDCParticleReader> m_readerDescription;
	
		enum AttributeType
		{
			Integer = 0,
			IntegerArray = 1,
			Double = 2,
			DoubleArray = 3,
			Vector = 4,
			VectorArray = 5,
		};
	
		struct Record
		{
			int type;
			std::streampos position;
		};
	
		// makes sure that m_iStream is open and that m_header is full.
		// returns true on success and false on failure.
		bool open();
		std::ifstream *m_iStream;
		std::string m_streamFileName;
		struct
		{
			bool valid;
			int version;
			bool reverseBytes;
			int numParticles;
			std::map<std::string, Record> attributes;
		} m_header;
	
		template<typename T>
		void readElements( T *buffer, std::streampos pos, unsigned long n ) const;
		
		template<typename T, typename F>
		typename T::Ptr filterAttr( typename F::ConstPtr attr, float percentage );
	
		// loads particleId in a completely unfiltered state
		ConstDoubleVectorDataPtr idAttribute();
		DoubleVectorDataPtr m_idAttribute;
				
};

IE_CORE_DECLAREPTR( PDCParticleReader );

} // namespace IECore

#endif // IE_CORE_PDCPARTICLEREADER_H
