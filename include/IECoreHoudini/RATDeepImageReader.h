//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011-2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREHOUDINI_RATDEEPIMAGEREADER_H
#define IECOREHOUDINI_RATDEEPIMAGEREADER_H

#include "UT/UT_Version.h"

// We haven't updated deep rat support for the changes in Houdini 15,
// since we're considering dropping deep rat support entirely.
#if UT_MAJOR_VERSION_INT < 15

#include "IMG/IMG_DeepShadow.h"

#include "IECore/CompoundObject.h"
#include "IECore/DeepImageReader.h"

#include "IECoreHoudini/TypeIds.h"

namespace IECoreHoudini
{

/// The RATDeepImageReader class reads Houdini deep texture files. Currently, it
/// only supports reading C and Of channels (RGBA or A). In the case of a DCM, 
/// (C and Of) it will assume Of is identical to the A sub-channel of C and can
/// be discarded. In the case of a DSM (Of only) the first Of sub-channel will
/// be used as A and the other 2 sub-channels will be discarded.
/// \todo: add support for arbitrary channels
/// \ingroup deepCompositingGroup
/// \ingroup ioGroup
class RATDeepImageReader : public IECore::DeepImageReader
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( RATDeepImageReader, RATDeepImageReaderTypeId, IECore::DeepImageReader );

		RATDeepImageReader();
		RATDeepImageReader( const std::string &filename );

		virtual ~RATDeepImageReader();
		
		static bool canRead( const std::string &filename );

		virtual void channelNames( std::vector<std::string> &names );
		virtual bool isComplete();
		virtual Imath::Box2i dataWindow();
		virtual Imath::Box2i displayWindow();
		virtual Imath::M44f worldToCameraMatrix();
		virtual Imath::M44f worldToNDCMatrix();

	protected :

		virtual IECore::DeepPixelPtr doReadPixel( int x, int y );

	private :

		static const ReaderDescription<RATDeepImageReader> g_readerDescription;

		/// Tries to open the file, returning true on success and false on failure. On success,
		/// all of the private members will be valid. If throwOnFailure is true then a descriptive
		/// Exception is thrown rather than false being returned.
		bool open( bool throwOnFailure = false );
		
		IMG_DeepShadow *m_inputFile;
		IMG_DeepPixelReader *m_ratPixel;
		const IMG_DeepShadowChannel *m_depthChannel;
		const IMG_DeepShadowChannel *m_opacityChannel;
		const IMG_DeepShadowChannel *m_colorChannel;
		std::string m_inputFileName;
		std::string m_channelNames;
		Imath::Box2i m_dataWindow;
		Imath::M44f m_worldToCamera;
		Imath::M44f m_worldToNDC;

};

IE_CORE_DECLAREPTR( RATDeepImageReader );

} // namespace IECoreHoudini

#endif // UT_MAJOR_VERSION_INT

#endif // IECOREHOUDINI_RATDEEPIMAGEREADER_H
