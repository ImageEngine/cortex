//////////////////////////////////////////////////////////////////////////
//
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

#ifndef IECOREHOUDINI_HOUDINISCENE_H
#define IECOREHOUDINI_HOUDINISCENE_H

#include "OP/OP_Node.h" 
#include "UT/UT_String.h"

#include "IECore/SceneInterface.h"
#include "IECoreHoudini/TypeIds.h"

namespace IECoreHoudini
{

IE_CORE_FORWARDDECLARE( HoudiniScene );

/// A read-only class for representing a live Houdini scene as an IECore::SceneInterface
class HoudiniScene : public IECore::SceneInterface
{
	public :
		
		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( HoudiniScene, HoudiniSceneTypeId, IECore::SceneInterface );
		
		HoudiniScene();
		HoudiniScene( const UT_String &nodePath, const Path &relativePath );
		HoudiniScene( const std::string &fileName, IECore::IndexedIO::OpenMode );
		
		virtual ~HoudiniScene();
		
		static FileFormatDescription<HoudiniScene> s_description;
		
		virtual Name name() const;
		virtual void path( Path &p ) const;
		
		virtual Imath::Box3d readBound( double time ) const;
		virtual void writeBound( const Imath::Box3d &bound, double time );

		virtual IECore::DataPtr readTransform( double time ) const;
		virtual Imath::M44d readTransformAsMatrix( double time ) const;
		virtual void writeTransform( const IECore::Data *transform, double time );

		virtual bool hasAttribute( const Name &name ) const;
		virtual void readAttributeNames( NameList &attrs ) const;
		virtual IECore::ObjectPtr readAttribute( const Name &name, double time ) const;
		virtual void writeAttribute( const Name &name, const IECore::Object *attribute, double time );

		virtual bool hasObject() const;
		virtual IECore::ObjectPtr readObject( double time ) const;
		virtual void writeObject( const IECore::Object *object, double time );

		virtual void childNames( NameList &childNames ) const;
		virtual bool hasChild( const Name &name ) const;
		virtual IECore::SceneInterfacePtr child( const Name &name, MissingBehaviour missingBehaviour = SceneInterface::ThrowIfMissing );
		virtual IECore::ConstSceneInterfacePtr child( const Name &name, MissingBehaviour missingBehaviour = SceneInterface::ThrowIfMissing ) const;
		virtual IECore::SceneInterfacePtr createChild( const Name &name );
		
		virtual IECore::SceneInterfacePtr scene( const Path &path, MissingBehaviour missingBehaviour = SceneInterface::ThrowIfMissing );
		virtual IECore::ConstSceneInterfacePtr scene( const Path &path, MissingBehaviour missingBehaviour = SceneInterface::ThrowIfMissing ) const;

	private :
		
		OP_Node *retrieveNode( bool content = false, MissingBehaviour missingBehaviour = SceneInterface::ThrowIfMissing ) const;
		OP_Node *locateContent( OP_Node *node ) const;
		OP_Node *retrieveChild( const Name &name, Path &relativePath, MissingBehaviour missingBehaviour = SceneInterface::ThrowIfMissing ) const;
		IECore::SceneInterfacePtr retrieveScene( const Path &path, MissingBehaviour missingBehaviour = SceneInterface::ThrowIfMissing ) const;
		
		void relativePath( const char *value, Path &result ) const;
		
		UT_String m_nodePath;
		UT_String m_contentPath;
		IECore::SceneInterface::Path m_relativePath;

};

} // namespace IECoreHoudini

#endif // IECOREHOUDINI_HOUDINISCENE_H
