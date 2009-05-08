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

#ifndef IE_COREMAYA_VIEWPORTPOSTPROCESS_H
#define IE_COREMAYA_VIEWPORTPOSTPROCESS_H

#include <string>

#include "IECore/RefCounted.h"
#include "IECore/ImagePrimitive.h"

#include "maya/MString.h"
#include "maya/MImage.h"
#include "maya/M3dView.h"

namespace IECoreMaya
{

/// A class which defines a post-process on a viewport. Use ViewportPostProcessCallback to register the post-process with a panel.
class ViewportPostProcess : public IECore::RefCounted
{
	friend class ViewportPostProcessCallback;

	public:

		IE_CORE_DECLAREMEMBERPTR( ViewportPostProcess );

		ViewportPostProcess();
		virtual ~ViewportPostProcess();

		virtual bool needsDepth() const;

	protected:

		virtual void preRender( const std::string &panelName );

		/// Derived classes should implement this, and modify the image in-place.
		virtual void postRender( const std::string &panelName, MImage &image ) = 0;

	private :

		ViewportPostProcess( const ViewportPostProcess &other );
		ViewportPostProcess& operator =( const ViewportPostProcess &other );
};


IE_CORE_DECLAREPTR( ViewportPostProcess );

}

#endif
