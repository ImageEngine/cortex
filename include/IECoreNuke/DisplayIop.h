//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, J3P LLC. All rights reserved.
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

#ifndef IECORENUKE_DISPLAYIOP_H
#define IECORENUKE_DISPLAYIOP_H

#include "IECoreNuke/Export.h"

#include "IECoreImage/DisplayDriverServer.h"

#include "DDImage/Iop.h"

namespace IECoreNuke
{

IE_CORE_FORWARDDECLARE( NukeDisplayDriver );

class IECORENUKE_API DisplayIop : public DD::Image::Iop
{

	public :

		DisplayIop( Node *node );
		virtual ~DisplayIop();

		virtual const char *Class() const;
		virtual const char *node_help() const;

		virtual void knobs( DD::Image::Knob_Callback f );
		virtual int knob_changed( DD::Image::Knob *knob );
		virtual void append( DD::Image::Hash &hash );
		virtual void _validate( bool forReal );
		virtual void engine( int y, int x, int r, const DD::Image::ChannelSet &channels, DD::Image::Row &row );

	private :

		static const Description g_description;
		static DD::Image::Op *build( Node *node );

		DisplayIop *firstDisplayIop();
		void driverCreated( NukeDisplayDriver *driver );
		void connectToDriver( NukeDisplayDriver *driver );
		void driverDataReceived( NukeDisplayDriver *driver, const Imath::Box2i &box );

		int m_portNumber;

		DD::Image::Format m_format;
		DD::Image::Format m_fullSizeFormat;

		IECoreImage::DisplayDriverServerPtr m_server;
		// we only bother updating these for firstDisplayIop(),
		// and then refer to them from all other instances. this
		// avoids problems where nuke might make new ops mid render
		// and those ops would have missed the display driver creation.
		unsigned int m_updateCount;
		IECoreNuke::NukeDisplayDriverPtr m_driver;

};

} // namespace IECoreNuke

#endif // IECORENUKE_DISPLAYIOP_H
