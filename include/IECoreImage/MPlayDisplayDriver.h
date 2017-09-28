//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, John Haddon. All rights reserved.
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

#ifndef IECOREIMAGE_MPLAYDISPLAYDRIVER_H
#define IECOREIMAGE_MPLAYDISPLAYDRIVER_H

#include "IECoreImage/Export.h"
#include "IECoreImage/DisplayDriver.h"

namespace IECoreImage
{

class IECOREIMAGE_API MPlayDisplayDriver : public DisplayDriver
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( MPlayDisplayDriver, MPlayDisplayDriverTypeId, DisplayDriver );

		MPlayDisplayDriver( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, const std::vector<std::string> &channelNames, IECore::ConstCompoundDataPtr parameters );
		virtual ~MPlayDisplayDriver();

		virtual void imageData( const Imath::Box2i &box, const float *data, size_t dataSize );
		virtual void imageClose();
		virtual bool scanLineOrderOnly() const;
		virtual bool acceptsRepeatedData() const;

	private :

		static const DisplayDriverDescription<MPlayDisplayDriver> g_description;

		struct ImageHeader;
		struct PlaneHeader;
		struct TileHeader;

		struct Plane
		{
			Plane( const std::string &n ) : name( n ) {}
			std::string name;
			std::vector<std::string> channelNames;
			std::vector<size_t> channelIndices;
		};

		typedef std::vector<Plane> PlaneVector;

		FILE *m_imDisplayStdIn;
		PlaneVector m_planes;

};

IE_CORE_DECLAREPTR( MPlayDisplayDriver )

}  // namespace IECoreImage

#endif // IECOREIMAGE_MPLAYDISPLAYDRIVER_H
