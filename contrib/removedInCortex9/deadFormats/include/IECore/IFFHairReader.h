//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_IFFHAIRREADER_H
#define IE_CORE_IFFHAIRREADER_H

#include <vector>

#include "IECore/Reader.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/SimpleTypedParameter.h"
#include "IECore/VectorTypedParameter.h"
#include "IECore/NumericParameter.h"
#include "IECore/IFFFile.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( CurvesPrimitive );

/// The IFFHairReader class defines a class for reading Maya IFF hair cache files onto a CurvesPrimitive.
/// \ingroup ioGroup
class IFFHairReader : public Reader
{
	public:
		
		IE_CORE_DECLARERUNTIMETYPED( IFFHairReader, Reader );
		
		IFFHairReader( );
		IFFHairReader( const std::string &fileName );
		virtual ~IFFHairReader();
		
		/// An enum for the values accepted by realTypeParameter().
		enum RealType
		{
			Native = 0,
			Float = 1,
			Double = 2,
		};
		
		static bool canRead( const std::string &filename );
		
		/// returns IntVectorData of all frames contained in the hair cache
		/// the frameIndex parameter should be set using an index into this IntVectorData
		ConstIntVectorDataPtr frameTimes();
		
		/// returns the number of hairs present at the frame specified by m_frameParameter
		unsigned long numHairs();
	
	private:
		
		static const ReaderDescription<IFFHairReader> g_readerDescription;
		
		// makes sure that m_iffFile is open and that m_header is full.
		// returns true on success and false on failure.
		bool open();
		IFFFilePtr m_iffFile;
		std::string m_iffFileName;
		IntParameterPtr m_frameParameter;
		IntParameterPtr m_realTypeParameter;
		
		enum HairCacheTagID
		{
			// header tags
			kCACH = 1128350536,
			kSTIM = 1398032717,
			kETIM = 1163151693,
			kTYPE = 1415139397,
			kRATE = 1380013125,

			// body tags
			kHAIR = 1212238162,
			kTIME = 1414090053,
			kNMHA = 1313687617,
			kNMCV = 1313686358,
			kPOSS = 1347375955,
			kVELS = 1447382099,
		};
				
		struct
		{
			bool valid;
			int startTime;
			int endTime;
			int rate;
			char type;
		} m_header;
		
		IntVectorDataPtr m_frames;
		std::map<int, IFFFile::Chunk::ChunkIterator> frameToRootChildren;
		
		/// Returns a CurvesPrimitive object containing all the hairs at the given frame
		virtual ObjectPtr doOperation( const CompoundObject * operands );
		
		// loads the current channel into the data vector
		void loadData( IFFFile::Chunk::ChunkIterator channel, V3dVectorData * channelData, int numCVs, bool fromFile=true );
		
		// convenience function to get the realType
		RealType realType() const;
		
		template<typename T, typename F>
		IntrusivePtr<T> convertAttr( IntrusivePtr<F> attr );
};

IE_CORE_DECLAREPTR( IFFHairReader );

} // namespace IECore

#endif // IE_CORE_IFFHAIRREADER_H
