//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREMAYA_PROCEDURALHOLDERUI_H
#define IECOREMAYA_PROCEDURALHOLDERUI_H

#include <map>

#include "IECore/Object.h"

#include "IECoreMaya/ProceduralHolder.h"

#include "maya/MPxSurfaceShapeUI.h"

namespace IECoreGL
{
IE_CORE_FORWARDDECLARE( State );
IE_CORE_FORWARDDECLARE( BoxPrimitive );
IE_CORE_FORWARDDECLARE( Group );
IE_CORE_FORWARDDECLARE( StateComponent );
}

namespace IECoreMaya
{

class ProceduralHolderUI : public MPxSurfaceShapeUI
{

	public :
	
		ProceduralHolderUI();
		virtual ~ProceduralHolderUI();

		virtual void getDrawRequests( const MDrawInfo &info, bool objectAndActiveOnly, MDrawRequestQueue &requests );
		virtual void draw( const MDrawRequest &request, M3dView &view ) const;
		virtual bool select( MSelectInfo &selectInfo, MSelectionList &selectionList, MPointArray &worldSpaceSelectPts ) const;
							
		static void *creator();

	private :
	
		enum DrawMode
		{
			SceneDrawMode,
			BoundDrawMode,
		};
	
		static void setWireFrameColors( MDrawRequest &request, M3dView::DisplayStatus status );
	
		IECoreGL::StatePtr baseState( M3dView::DisplayStyle style ) const;

		IECoreGL::BoxPrimitivePtr m_boxPrimitive;
		
		typedef std::map< IECoreGL::Group*, IECoreGL::StatePtr > StateMap;
				
		void hiliteGroups( const ProceduralHolder::ComponentToGroupMap::mapped_type &groups, IECoreGL::StateComponentPtr hilite, IECoreGL::StateComponentPtr base ) const;
		void unhiliteGroupChildren( const std::string &name, IECoreGL::GroupPtr group, IECoreGL::StateComponentPtr base ) const;
		void resetHilites() const;
		
	public :
	
		/// \todo Move members into class on next major version change
		struct MemberData;

};

} // namespace IECoreMaya

#endif // IECOREMAYA_PROCEDURALHOLDERUI_H
