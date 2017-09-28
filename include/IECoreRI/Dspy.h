//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORERI_DSPY_H
#define IECORERI_DSPY_H

#include "IECoreRI/Export.h"

#include "ndspy.h"

namespace IECoreRI
{

/// Defines functions to allow the RenderMan Dspy interface to be
/// implemented in terms of IECore::DisplayDrivers. These functions are
/// automatically registered with RenderMan using DspyRegisterDriverTable,
/// but are public to allow their use in implementing display driver dsos
/// as well.
class IECORERI_API Dspy
{

	public :

		/// A function suitable for implementing DspyImageOpen. This uses IECore::DisplayDriver::create() to make a driver. The reference count is incremented on the
		/// driver and a pointer to it is placed in image.
		static PtDspyError imageOpen( PtDspyImageHandle *image, const char *driverName, const char *fileName, int width, int height, int paramcount, const UserParameter *parameters, int formatCount, PtDspyDevFormat *format, PtFlagStuff *flags );
		/// Expects image to have been created by the imageOpen above, but currently implements no queries.
		static PtDspyError imageQuery( PtDspyImageHandle image, PtDspyQueryType type, int size, void *data );
		/// Expects image to have been created by the imageOpen above, and uses data to make a call do DisplayDriver::imageData().
		static PtDspyError imageData( PtDspyImageHandle image, int xMin, int xMaxPlusOne, int yMin, int yMaxPlusOne, int entrySize, const unsigned char *data );
		/// Decrements the reference count, destroying the IECore::DisplayDriver if nothing else owns a reference.
		static PtDspyError imageClose( PtDspyImageHandle image );

	private :

		class Registration;
		static Registration g_registration;

};

}  // namespace IECore

#endif // IECORERI_DSPY_H
