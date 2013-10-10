//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREMAYA_SCENESHAPEINTERFACECOMPONENTBOUNDITERATOR_H
#define IECOREMAYA_SCENESHAPEINTERFACECOMPONENTBOUNDITERATOR_H

#include "maya/MPxGeometryIterator.h"
#include "maya/MPoint.h"
#include "maya/MBoundingBox.h"
#include "maya/MObjectArray.h"

#include "OpenEXR/ImathBox.h"

#include "IECoreMaya/SceneShapeInterface.h"

namespace IECoreMaya
{

/// The SceneShapeInterfaceComponentBoundIterator allows maya to iterate over the bounding box corners of
/// the SceneShapeInterface components. It's currently used so you can frame scene shape components
/// in the maya viewport.
class SceneShapeInterfaceComponentBoundIterator : public MPxGeometryIterator
{

	public:

		SceneShapeInterfaceComponentBoundIterator( void *userGeometry, MObjectArray &components );
		SceneShapeInterfaceComponentBoundIterator( void *userGeometry, MObject &components );
		~SceneShapeInterfaceComponentBoundIterator();
		
		virtual bool isDone() const;
		virtual void next();
		virtual void reset();
		virtual void component( MObject &component );
		virtual bool hasPoints() const;
		virtual int iteratorCount() const;
		virtual MPoint point() const;
		virtual void setPoint(const MPoint &) const;
		virtual int setPointGetNext(MPoint &);
		virtual int index() const;
		virtual bool hasNormals() const;
		virtual int indexUnsimplified() const;
		
	private:
		
		void computeNumComponents();
		
		SceneShapeInterface* m_sceneShapeInterface;
		unsigned m_idx;
		MObjectArray m_components;
		unsigned m_numComponents;
	
};

} // namespace IECoreMaya

#endif // IECOREMAYA_SCENESHAPEINTERFACECOMPONENTBOUNDITERATOR_H
