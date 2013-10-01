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

#include "IECoreHoudini/DetailSplitter.h"
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
		HoudiniScene( const UT_String &nodePath, const Path &contentPath, const Path &rootPath );
		
		virtual ~HoudiniScene();
		
		virtual std::string fileName() const;

		virtual Name name() const;
		virtual void path( Path &p ) const;
		
		virtual Imath::Box3d readBound( double time ) const;
		virtual void writeBound( const Imath::Box3d &bound, double time );

		virtual IECore::ConstDataPtr readTransform( double time ) const;
		virtual Imath::M44d readTransformAsMatrix( double time ) const;
		virtual void writeTransform( const IECore::Data *transform, double time );

		virtual bool hasAttribute( const Name &name ) const;
		virtual void attributeNames( NameList &attrs ) const;
		virtual IECore::ConstObjectPtr readAttribute( const Name &name, double time ) const;
		virtual void writeAttribute( const Name &name, const IECore::Object *attribute, double time );

		virtual bool hasTag( const Name &name, bool includeChildren = true ) const;
		virtual void readTags( NameList &tags, bool includeChildren = true ) const;
		virtual void writeTags( const NameList &tags );

		virtual bool hasObject() const;
		virtual IECore::ConstObjectPtr readObject( double time ) const;
		virtual IECore::PrimitiveVariableMap readObjectPrimitiveVariables( const std::vector<IECore::InternedString> &primVarNames, double time ) const;
		virtual void writeObject( const IECore::Object *object, double time );

		virtual void childNames( NameList &childNames ) const;
		virtual bool hasChild( const Name &name ) const;
		virtual IECore::SceneInterfacePtr child( const Name &name, MissingBehaviour missingBehaviour = SceneInterface::ThrowIfMissing );
		virtual IECore::ConstSceneInterfacePtr child( const Name &name, MissingBehaviour missingBehaviour = SceneInterface::ThrowIfMissing ) const;
		virtual IECore::SceneInterfacePtr createChild( const Name &name );
		
		virtual IECore::SceneInterfacePtr scene( const Path &path, MissingBehaviour missingBehaviour = SceneInterface::ThrowIfMissing );
		virtual IECore::ConstSceneInterfacePtr scene( const Path &path, MissingBehaviour missingBehaviour = SceneInterface::ThrowIfMissing ) const;
		
		/// Convenience method to access the Houdini node this scene refers to
		const OP_Node *node() const;
		
		/// The parameter name used to identify user defined tags on any OBJ node. This will be accessed
		/// by hasTag and readTags as a string parameter, and will be split on spaces to separate tags.
		static PRM_Name pTags;
		
		typedef boost::function<bool (const OP_Node *)> HasFn;
		typedef boost::function<IECore::ConstObjectPtr (const OP_Node *)> ReadFn;
		typedef boost::function<bool (const OP_Node *, const Name &)> HasTagFn;
		typedef boost::function<void (const OP_Node *, NameList &, bool)> ReadTagsFn;
		
		// Register callbacks for custom named attributes.
		// The has function will be called during hasAttribute and it stops in the first one that returns true.
		// The read method is called if the has method returns true, so it should return a valid Object pointer or raise an Exception.
		static void registerCustomAttribute( const Name &attrName, HasFn hasFn, ReadFn readFn );
		
		// Register callbacks for nodes to define custom tags
		// The functions will be called during hasTag and readTags.
		// readTags will return the union of all custom ReadTagsFns.
		static void registerCustomTags( HasTagFn hasFn, ReadTagsFn readFn );
		
	private :
		
		HoudiniScene( const UT_String &nodePath, const Path &contentPath, const Path &rootPath, DetailSplitter *splitter );
		void constructCommon( const UT_String &nodePath, const Path &contentPath, const Path &rootPath, DetailSplitter *splitter );
		
		OP_Node *retrieveNode( bool content = false, MissingBehaviour missingBehaviour = SceneInterface::ThrowIfMissing ) const;
		OP_Node *locateContent( OP_Node *node ) const;
		OP_Node *retrieveChild( const Name &name, Path &contentPath, MissingBehaviour missingBehaviour = SceneInterface::ThrowIfMissing ) const;
		IECore::SceneInterfacePtr retrieveScene( const Path &path, MissingBehaviour missingBehaviour = SceneInterface::ThrowIfMissing ) const;
		bool hasInput( const OP_Node *node ) const;
		
		void calculatePath( const Path &contentPath, const Path &rootPath );
		const char *matchPath( const char *value ) const;
		bool matchPattern( const char *value, const char *pattern ) const;
		std::pair<const char *, size_t> nextWord( const char *value ) const;
		const char *contentPathValue() const;
		
		/// Struct for registering readers for custom Attributes.
		struct CustomReader
		{
			HasFn m_has;
			ReadFn m_read;
		};
		
		/// Struct for registering readers for custom Tags.
		struct CustomTagReader
		{
			HasTagFn m_has;
			ReadTagsFn m_read;
		};
		
		static std::map<Name, CustomReader> &customAttributeReaders();
		static std::vector<CustomTagReader> &customTagReaders();
		
		UT_String m_nodePath;
		size_t m_rootIndex;
		size_t m_contentIndex;
		IECore::SceneInterface::Path m_path;
		
		// used by instances which track the hierarchy inside a SOP
		mutable DetailSplitterPtr m_splitter;

};

} // namespace IECoreHoudini

#endif // IECOREHOUDINI_HOUDINISCENE_H
