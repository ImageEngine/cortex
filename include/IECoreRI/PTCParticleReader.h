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

#ifndef IE_CORERI_PTCPARTICLEREADER_H
#define IE_CORERI_PTCPARTICLEREADER_H

#include "IECore/CompoundData.h"
#include "IECore/ParticleReader.h"
#include "IECore/VectorTypedData.h"
#include "IECoreRI/TypeIds.h"
#include "IECoreRI/PTCParticleIO.h"

namespace IECoreRI
{

/// The PTCParticleReader class implements the ParticleReader
/// interface for Renderman .ptc format particle caches.
class PTCParticleReader : public IECore::ParticleReader
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( PTCParticleReader, PTCParticleReaderTypeId, IECore::ParticleReader );

		PTCParticleReader( );
		PTCParticleReader( const std::string &fileName );
		virtual ~PTCParticleReader();

		static bool canRead( const std::string &fileName );

		virtual unsigned long numParticles();
		virtual void attributeNames( std::vector<std::string> &names );
		virtual IECore::DataPtr readAttribute( const std::string &name );

	protected :

		/// Returns a PointsPrimitive object containing all the
		/// attributes requested. It also fills up the blindData dictionary under "PTCParticleIO" key 
		/// with the information available in the pointcloud header file 
		/// (worldToEye, worldToNdc, boundingBox, xResolution, yResolution and aspectRatio, variableTypes). 
		/// This method overwrites the base class implementation.
		virtual IECore::ObjectPtr doOperation( IECore::ConstCompoundObjectPtr operands );

	private :

		static const ReaderDescription<PTCParticleReader> m_readerDescription;

		void *m_ptcFile;

		// makes sure that m_ptcFile is open.
		// returns true on success and false on failure.
		bool open();

		// closes current ptcFile.
		void close();

		struct AttrInfo 
		{
			PTCParticleIO::VarType type;
			const float *sourcePtr;
			IECore::DataPtr targetData;
		};

		std::string m_streamFileName;

		struct PTCParticleIO::PTCHeader m_header;
		float *m_userDataBuffer;
	
		template<typename T, typename F>
		boost::intrusive_ptr<T> filterAttr( boost::intrusive_ptr<F> attr, float percentage );

		// reads several attributes in one operation and returns them in a CompoundData.
		IECore::CompoundDataPtr readAttributes( const std::vector<std::string> &names );				
};

IE_CORE_DECLAREPTR( PTCParticleReader );

} // namespace IECoreRI

#endif // IE_CORERI_PTCPARTICLEREADER_H
