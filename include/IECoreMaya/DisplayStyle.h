//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREMAYA_DISPLAYSTYLE_H
#define IECOREMAYA_DISPLAYSTYLE_H

#include "boost/noncopyable.hpp"

#include "IECoreGL/State.h"

#include "maya/M3dView.h"

namespace IECoreMaya
{

/// Maya specifies how things should be drawn using the M3dView::DisplayStyle
/// enum, whereas IECoreGL uses State objects to specify the equivalent things
/// (and more). When using IECoreGL to draw within maya nodes it then becomes
/// necessary to translate from the maya definition into an IECoreGL::State
/// object. This class performs that translation. Typically one would be held as
/// member data in a node and baseState() would be called upon in the draw() method.
class DisplayStyle : public boost::noncopyable
{

	public :
	
		DisplayStyle();
		~DisplayStyle();
	
		/// Returns a base state suitable for representing objects in the the style specified
		/// by maya. If setCurrentColor is true then the current gl color is also translated
		/// appropriately into the State (for bounding box, wireframe and points modes only).
		IECoreGL::ConstStatePtr baseState( M3dView::DisplayStyle style, bool transferCurrentColor=true );
	
	private :
			
		struct Data;
		
		Data *m_data;
		
};

} // namespace IECoreMaya

#endif // IECOREMAYA_DISPLAYSTYLE_H
