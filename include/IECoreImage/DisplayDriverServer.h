//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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

//! \file DisplayDriverServer.h
/// Defines the DisplayDriverServer class.

#ifndef IECOREIMAGE_DISPLAYDRIVERSERVER
#define IECOREIMAGE_DISPLAYDRIVERSERVER

#include "IECoreImage/DisplayDriver.h"
#include "IECoreImage/Export.h"

#include "IECore/RunTimeTyped.h"
#include "IECore/VectorTypedData.h"

#include "boost/system/error_code.hpp"

namespace IECoreImage
{

/// Server class that receives images from ClientDisplayDriver connections and forwards the data to local display drivers.
/// The type of the local display drivers is defined by the 'remoteDisplayType' parameter.
///
/// The server object creates a thread to control the socket connection. The thread dies when the object is destroyed.
/// \ingroup renderingGroup
class IECOREIMAGE_API DisplayDriverServer : public IECore::RunTimeTyped
{
	public:

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( DisplayDriverServer, DisplayDriverServerTypeId, IECore::RunTimeTyped );

		/// A port number of 0 causes a free port to be chosen
		/// automatically. Call `portNumber()` after construction
		/// to retrieve the actual number.
		DisplayDriverServer( int portNumber = 0 );
		~DisplayDriverServer() override;

		int portNumber();

	private:

		// Session class
		// Takes care of one client connection.
		class Session;
		IE_CORE_DECLAREPTR( Session );

		void serverThread();
		void handleAccept( DisplayDriverServer::SessionPtr session, const boost::system::error_code& error);

		class PrivateData;
		IE_CORE_DECLAREPTR( PrivateData );
		PrivateDataPtr m_data;
};

IE_CORE_DECLAREPTR( DisplayDriverServer )

} // namespace IECoreImage

#endif // IECOREIMAGE_DISPLAYDRIVERSERVER
