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

#ifndef IE_CORE_NPARTICLEREADER_H
#define IE_CORE_NPARTICLEREADER_H

#include <vector>

#include "IECore/Export.h"
#include "IECore/ParticleReader.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/IFFFile.h"

namespace IECore
{

/// The NParticleReader class defines a class for reading IFF cache files (Maya nCaches) onto a PointsPrimitive.
/// \ingroup ioGroup
class IECORE_API NParticleReader : public ParticleReader
{
	public:
		
		IE_CORE_DECLARERUNTIMETYPED( NParticleReader, ParticleReader );
		
		NParticleReader( );
		NParticleReader( const std::string &fileName );
		virtual ~NParticleReader();
		
		static bool canRead( const std::string &filename );
		
		virtual unsigned long numParticles();
		virtual void attributeNames( std::vector<std::string> &names );
		virtual DataPtr readAttribute( const std::string &name );
		
		/// returns IntVectorData of all frames contained in the nCache
		/// the frameIndex parameter should be set using an index into this IntVectorData
		const IntVectorData * frameTimes();
	
	protected:
		
		// Returns the name of the position primVar
		virtual std::string positionPrimVarName();
		
	private:
		
		static const ReaderDescription<NParticleReader> m_readerDescription;
		
		// makes sure that m_iffFile is open and that m_header is full.
		// returns true on success and false on failure.
		bool open();
		IFFFilePtr m_iffFile;
		std::string m_iffFileName;
		IntParameterPtr m_frameParameter;
		
		enum NCacheTagID
		{
			// header tags
			kCACH = 1128350536,
			kVRSN = 1448235854,
			kSTIM = 1398032717,
			kETIM = 1163151693,

			// body tags
			kMYCH = 1297695560,
			kTIME = 1414090053,
			kCHNM = 1128812109,
			kSIZE = 1397316165,
			kDBLA = 1145195585,
			kDVCA = 1146504001,
			kFVCA = 1180058433,
		};
		
		struct
		{
			bool valid;
			std::string version;
			int startTime;
			int endTime;
		} m_header;
		
		IntVectorDataPtr m_frames;
		std::map<int, IFFFile::Chunk::ChunkIterator> frameToRootChildren;
		
		template<typename T, typename F>
		typename T::Ptr filterAttr( const F * attr, float percentage );
};

IE_CORE_DECLAREPTR( NParticleReader );

} // namespace IECore

#endif // IE_CORE_NPARTICLEREADER_H
