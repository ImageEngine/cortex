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

#ifndef IE_COREMAYA_POSTLOADCALLBACK_H
#define IE_COREMAYA_POSTLOADCALLBACK_H

#include "IECore/RefCounted.h"

#include "maya/MCallbackIdArray.h"

namespace IECoreMaya
{

/// A base class which executes a callback whenever a Maya scene is opened, referenced, or imported. User-defined
/// classes should derive from this, and implement any custom behaviour in the postLoad() method. Refcounting
/// ensures that all Maya callbacks are removed when the instance is deleted.
class PostLoadCallback : public IECore::RefCounted
{
	friend class PostLoadCallbackData;

	public:

		/// Construct a new callback instance
		PostLoadCallback();
		virtual ~PostLoadCallback();

	protected:

		/// To be overridden by derived classes to implement custom behaviour
		virtual void postLoad() = 0;

		class PostLoadCallbackData;
		PostLoadCallbackData* m_data;


};

IE_CORE_DECLAREPTR( PostLoadCallback );

} // namespace IECoreMaya

#endif //  IE_COREMAYA_POSTLOADCALLBACK_H
