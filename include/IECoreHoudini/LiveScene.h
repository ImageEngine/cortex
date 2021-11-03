//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013-2014, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREHOUDINI_LIVESCENE_H
#define IECOREHOUDINI_LIVESCENE_H

#include "IECoreHoudini/DetailSplitter.h"
#include "IECoreHoudini/Export.h"
#include "IECoreHoudini/TypeIds.h"

#include "IECoreScene/SceneInterface.h"

#include "OP/OP_Node.h"
#include "UT/UT_String.h"

#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"

namespace IECoreHoudini
{

IE_CORE_FORWARDDECLARE( LiveScene );

/// A read-only class for representing a live Houdini scene as an IECore::SceneInterface
/// Note that this class treats time by SceneInterface standards, starting at Frame 0,
/// as opposed to Houdini standards, which start at Frame 1.
class IECOREHOUDINI_API LiveScene : public IECoreScene::SceneInterface
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( LiveScene, LiveSceneTypeId, IECoreScene::SceneInterface );

		LiveScene();
		LiveScene( const UT_String &nodePath, const Path &contentPath, const Path &rootPath, double defaultTime = std::numeric_limits<double>::infinity() );
		~LiveScene() = default;

		std::string fileName() const override;

		Name name() const override;
		void path( Path &p ) const override;

		Imath::Box3d readBound( double time ) const override;
		void writeBound( const Imath::Box3d &bound, double time ) override;

		IECore::ConstDataPtr readTransform( double time ) const override;
		Imath::M44d readTransformAsMatrix( double time ) const override;
		/// \todo: consider making these methods of SceneInterface itself
		IECore::ConstDataPtr readWorldTransform( double time ) const;
		Imath::M44d readWorldTransformAsMatrix( double time ) const;
		void writeTransform( const IECore::Data *transform, double time ) override;

		bool hasAttribute( const Name &name ) const override;
		void attributeNames( NameList &attrs ) const override;
		IECore::ConstObjectPtr readAttribute( const Name &name, double time ) const override;
		void writeAttribute( const Name &name, const IECore::Object *attribute, double time ) override;

		bool hasTag( const Name &name, int filter = SceneInterface::LocalTag ) const override;
		void readTags( NameList &tags, int filter = SceneInterface::LocalTag ) const override;
		void writeTags( const NameList &tags ) override;

		NameList setNames( bool includeDescendantSets = true ) const override;
		IECore::PathMatcher readSet( const Name &name, bool includeDescendantSets = true, const IECore::Canceller *canceller = nullptr ) const override;
		void writeSet( const Name &name, const IECore::PathMatcher &set ) override;
		void hashSet( const Name& setName, IECore::MurmurHash &h ) const override;

		bool hasObject() const override;
		IECore::ConstObjectPtr readObject( double time, const IECore::Canceller *canceller = nullptr ) const override;
		IECoreScene::PrimitiveVariableMap readObjectPrimitiveVariables( const std::vector<IECore::InternedString> &primVarNames, double time ) const override;
		void writeObject( const IECore::Object *object, double time ) override;

		void childNames( NameList &childNames ) const override;
		bool hasChild( const Name &name ) const override;
		IECoreScene::SceneInterfacePtr child( const Name &name, MissingBehaviour missingBehaviour = SceneInterface::ThrowIfMissing ) override;
		IECoreScene::ConstSceneInterfacePtr child( const Name &name, MissingBehaviour missingBehaviour = SceneInterface::ThrowIfMissing ) const override;
		IECoreScene::SceneInterfacePtr createChild( const Name &name ) override;

		IECoreScene::SceneInterfacePtr scene( const Path &path, MissingBehaviour missingBehaviour = SceneInterface::ThrowIfMissing ) override;
		IECoreScene::ConstSceneInterfacePtr scene( const Path &path, MissingBehaviour missingBehaviour = SceneInterface::ThrowIfMissing ) const override;

		/// Currently raises an exception
		void hash( HashType hashType, double time, IECore::MurmurHash &h ) const override;

		/// Convenience method to access the Houdini node this scene refers to
		const OP_Node *node() const;

		/// Convenience method to determine if this scene refers to hierarchy embedded inside a SOP
		bool embedded() const;

		/// These methods provide a default cooking time for methods that do not accept time
		/// as an argument (e.g. hasObject or childNames). In a LiveScene which points at
		/// a SOP, it is necessary to use time in these methods. The default time will pass
		/// through to children automatically. If left unset, CHgetEvalTime() will be used
		/// for these queries. See ROP_SceneCacheWriter for a use case.
		double getDefaultTime() const;
		void setDefaultTime( double time );

		/// The parameter name used to identify user defined tags on any OBJ node. This will be accessed
		/// by hasTag and readTags as a string parameter, and will be split on spaces to separate tags.
		static PRM_Name pTags;

		typedef boost::function<bool (const OP_Node *)> HasFn;
		typedef boost::function<IECore::ConstObjectPtr (const OP_Node *, double &)> ReadFn;
		typedef boost::function<IECore::ConstObjectPtr (const OP_Node *, const Name &, double &)> ReadAttrFn;
		typedef boost::function<bool (const OP_Node *, const Name &, int)> HasTagFn;
		typedef boost::function<void (const OP_Node *, NameList &, int)> ReadTagsFn;
		typedef boost::function<void (const OP_Node *, NameList &)> ReadNamesFn;

		// Register callbacks for custom named attributes.
		// The names function will be called during attributeNames and hasAttribute.
		// The read method is called if the names method returns the expected attribute, so it should return a valid Object pointer or raise an Exception.
		static void registerCustomAttributes( ReadNamesFn namesFn, ReadAttrFn readFn, bool callEmbedded );
		static void clearCustomAttributeReaders();

		// Register callbacks for nodes to define custom tags
		// The functions will be called during hasTag and readTags.
		// readTags will return the union of all custom ReadTagsFns.
		static void registerCustomTags( HasTagFn hasFn, ReadTagsFn readFn, bool callEmbedded );
		static void clearCustomTagReaders();

	protected :

		LiveScene( const UT_String &nodePath, const Path &contentPath, const Path &rootPath,  const LiveScene& parent);

		virtual LiveScenePtr create() const;
		virtual LiveScenePtr duplicate( const UT_String &nodePath, const Path &contentPath, const Path &rootPath) const;

	private :

		void constructCommon( const UT_String &nodePath, const Path &contentPath, const Path &rootPath, DetailSplitter *splitter );

		OP_Node *retrieveNode( bool content = false, MissingBehaviour missingBehaviour = SceneInterface::ThrowIfMissing ) const;
		OP_Node *locateContent( OP_Node *node ) const;
		OP_Node *retrieveChild( const Name &name, Path &contentPath, MissingBehaviour missingBehaviour = SceneInterface::ThrowIfMissing ) const;
		IECoreScene::SceneInterfacePtr retrieveScene( const Path &path, MissingBehaviour missingBehaviour = SceneInterface::ThrowIfMissing ) const;
		bool hasInput( const OP_Node *node ) const;
		// We need to adjust the time internally, because SceneInterfaces treat time
		// starting at Frame 0, while Houdini treats time starting at Frame 1.
		double adjustTime( double time ) const;
		double adjustedDefaultTime() const;

		void calculatePath( const Path &contentPath, const Path &rootPath );
		const char *matchPath( const char *value ) const;
		bool matchPattern( const char *value, const char *pattern ) const;
		std::pair<const char *, size_t> nextWord( const char *value ) const;
		void relativeContentPath( IECoreScene::SceneInterface::Path &path ) const;
		GU_DetailHandle contentHandle() const;

		/// Struct for registering readers for custom Attributes.
		struct CustomAttributeReader
		{
			ReadNamesFn m_names;
			ReadAttrFn m_read;
			bool m_callEmbedded;
		};

		/// Struct for registering readers for custom Tags.
		struct CustomTagReader
		{
			HasTagFn m_has;
			ReadTagsFn m_read;
			bool m_callEmbedded;
		};

		static std::vector<CustomAttributeReader> &customAttributeReaders();
		static std::vector<CustomTagReader> &customTagReaders();

		UT_String m_nodePath;
		size_t m_rootIndex;
		size_t m_contentIndex;
		IECoreScene::SceneInterface::Path m_path;

		// used by instances which track the hierarchy inside a SOP
		mutable DetailSplitterPtr m_splitter;

		// used as the default cook time for methods that do not accept a time
		double m_defaultTime;

};

} // namespace IECoreHoudini

#endif // IECOREHOUDINI_LIVESCENE_H
