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

#ifndef IE_CORE_BGEOPARTICLEREADER_H
#define IE_CORE_BGEOPARTICLEREADER_H

#include "IECore/CompoundData.h"
#include "IECore/ParticleReader.h"
#include "IECore/VectorTypedData.h"

# include <string>

namespace IECore
{

/// The BGEOParticleReader class implements the ParticleReader
/// interface for Houdini .bgeo format particle caches.
/// All points are treated as particles, primitives are ignored
class BGEOParticleReader : public ParticleReader
{

	public :

		IE_CORE_DECLARERUNTIMETYPED( BGEOParticleReader, ParticleReader );

		BGEOParticleReader( );
		BGEOParticleReader( const std::string &fileName );
		virtual ~BGEOParticleReader();

		static bool canRead( const std::string &fileName );

		virtual unsigned long numParticles();
		virtual void attributeNames( std::vector<std::string> &names );
		virtual DataPtr readAttribute( const std::string &name );

	protected :

		/// Returns a PointsPrimitive object containing all the
		/// This method overwrites the base class implementation.
		virtual IECore::ObjectPtr doOperation( IECore::ConstCompoundObjectPtr operands );
	
	private :

		static const ReaderDescription<BGEOParticleReader> m_readerDescription;

		enum AttributeType
		{
			Float = 0,
			Integer = 1,
			Vector = 5,
		};
		
		struct Record
		{
			std::string name;
			AttributeType type;
			int size;
		};
		
		struct AttrInfo
		{
			Record info;
			IECore::DataPtr targetData;
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
			int numPoints;
			int numPrims;
			int numPointGroups;
			int numPrimGroups;
			int numPointAttribs;
			int numVertexAttribs;
			int numPrimAttribs;
			int numDetailAttribs;
			int firstPointPosition;
			std::vector<Record> attributes;
		} m_header;
		
		template<typename T>
		void readAttributeData( T *buffer, unsigned long n ) const;
		
		template<typename T, typename F>
		boost::intrusive_ptr<T> filterAttr( boost::intrusive_ptr<F> attr, float percentage );
		
		// reads all attributes from m_header.attributes and returns CoumpoundData containing the results
		IECore::CompoundDataPtr readAttributes();
};

IE_CORE_DECLAREPTR( BGEOParticleReader );

} // namespace IECore

#endif // IE_CORE_BGEOPARTICLEREADER_H
