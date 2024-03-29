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

#ifndef IECOREMAYA_LIVESCENE_H
#define IECOREMAYA_LIVESCENE_H

#include "boost/function.hpp"
#include "tbb/mutex.h"

#include "IECoreScene/SceneInterface.h"
#include "IECoreMaya/TypeIds.h"
#include "IECoreMaya/Export.h"

#include "maya/MDagPath.h"

namespace IECoreMaya
{

IE_CORE_FORWARDDECLARE( LiveScene );

/// A class for navigating a maya scene.
/// Each LiveScene instance maps to a specific transform in a scene, uniquely identified by it's dag path.
/// Shapes are interpreted as objects living on their parent - eg a scene with the objects |pSphere1 and
/// |pSphere1|pSphereShape1 in it will map to a LiveScene at "/", with a child called "pSphere1", with a
/// MeshPrimitive as its object, and no children.
/// This interface currently only supports read operations, which can only be called with the current maya
/// time in seconds. For example, if you're currently on frame 1 in your maya session, your scene's frame rate
/// is 24 fps, and you want to read an object from your LiveScene instance, you must call
/// LiveSceneInstance.readObject( 1.0 / 24 ), or it will throw an exception.

class IECOREMAYA_API LiveScene : public IECoreScene::SceneInterface
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( LiveScene, LiveSceneTypeId, IECoreScene::SceneInterface );

		LiveScene();
		~LiveScene() = default;

		std::string fileName() const override;

		/// Returns the name of the scene location which this instance is referring to. The root path returns "/".
		Name name() const override;
		/// Returns the tokenized dag path this instance is referring to.
		void path( Path &p ) const override;

		/// Name of maya attribute overriding `IECoreScene::SceneInterface::visibilityName`
		static IECoreScene::SceneInterface::Name visibilityOverrideName;

		/*
		 * Bounding box
		 */

		/// Returns the local bounding box of this node at the
		/// specified point in time, which must be equal to the current maya time in seconds.
		Imath::Box3d readBound( double time ) const override;
		/// Not currently supported - will throw an exception.
		void writeBound( const Imath::Box3d &bound, double time ) override;

		/*
		 * Transform
		 */

		/// Returns the local transform of this node at the specified
		/// point in time, which must be equal to the current maya time in seconds.
		IECore::ConstDataPtr readTransform( double time ) const override;
		/// Returns the transform of this node at the specified
		/// point in time as a matrix.
		Imath::M44d readTransformAsMatrix( double time ) const override;
		/// Not currently supported - will throw an exception.
		void writeTransform( const IECore::Data *transform, double time ) override;

		/*
		 * Attributes
		 */

		bool hasAttribute( const Name &name ) const override;
		/// Fills attrs with the names of all attributes available in the current directory
		void attributeNames( NameList &attrs ) const override;
		/// Returns the attribute value at the given time, which must be equal to the current maya time in seconds.
		IECore::ConstObjectPtr readAttribute( const Name &name, double time ) const override;
		/// Not currently supported - will throw an exception.
		void writeAttribute( const Name &name, const IECore::Object *attribute, double time ) override;

		/*
		 * Tags
		 */

		/// Uses the custom registered tags to return whether a given tag is present in the scene location or not.
		bool hasTag( const Name &name, int filter = SceneInterface::LocalTag ) const override;
		/// Uses the custom registered tags to list all the tags present in the scene location.
		void readTags( NameList &tags, int filter = SceneInterface::LocalTag ) const override;
		/// Not currently supported - will throw an exception.
		void writeTags( const NameList &tags ) override;

		/*
		 * Sets
		 */

		/// Returns the names of all sets containing objects in this location and all of its descendants.
		NameList setNames( bool includeDescendantSets = true ) const override;
		/// Reads the named set. All paths returned are relative to the current location.
		IECore::PathMatcher readSet( const Name &name, bool includeDescendantSets = true, const IECore::Canceller *canceller = nullptr ) const override;
		/// Writes a set at the current location. All paths are specified relative to the current location.
		void writeSet( const Name &name, const IECore::PathMatcher &set ) override;
		/// Hash a named set at the current location.
		void hashSet( const Name& setName, IECore::MurmurHash &h ) const override;

		/*
		 * Object
		 */

		/// Checks if there are objects in the scene (convertible from Maya or registered as custom objects)
		bool hasObject() const override;
		/// Reads the object stored at this path in the scene at the given time - may
		/// return 0 when no object has been stored. Time must be equal to the current maya time in seconds
		IECore::ConstObjectPtr readObject( double time, const IECore::Canceller *canceller = nullptr ) const override;
		/// Reads primitive variables from the object of type Primitive stored at this path in the scene at the given time.
		/// Raises exception if it turns out not to be a Primitive object.
		IECoreScene::PrimitiveVariableMap readObjectPrimitiveVariables( const std::vector<IECore::InternedString> &primVarNames, double time ) const override;
		/// Not currently supported - will throw an exception.
		void writeObject( const IECore::Object *object, double time ) override;

		/*
		 * Hierarchy
		 */

		/// Queries the names of any existing children of path() within
		/// the scene.
		void childNames( NameList &childNames ) const override;
		/// Queries weather the named child exists.
		bool hasChild( const Name &name ) const override;
		/// Returns an object for the specified child location in the scene.
		/// If the child does not exist then it will behave according to the
		/// missingBehavior parameter. May throw and exception, may return a NULL pointer,
		/// or may create the child (if that is possible).
		/// Bounding boxes will be automatically propagated up from the children
		/// to the parent as it is written.
		IECoreScene::SceneInterfacePtr child( const Name &name, MissingBehaviour missingBehaviour = SceneInterface::ThrowIfMissing ) override;
		/// Returns a read-only interface for a child location in the scene.
		IECoreScene::ConstSceneInterfacePtr child( const Name &name, MissingBehaviour missingBehaviour = SceneInterface::ThrowIfMissing ) const override;
		/// Returns a writable interface to a new child. Throws an exception if it already exists.
		/// Bounding boxes will be automatically propagated up from the children
		/// to the parent as it is written.
		IECoreScene::SceneInterfacePtr createChild( const Name &name ) override;


		/// Returns an object for querying the scene at the given path (full path).
		IECoreScene::SceneInterfacePtr scene( const Path &path, MissingBehaviour missingBehaviour = SceneInterface::ThrowIfMissing ) override;

		/// Returns an object for querying the scene at the given path (full path).
		IECoreScene::ConstSceneInterfacePtr scene( const Path &path, MissingBehaviour missingBehaviour = SceneInterface::ThrowIfMissing ) const override;

		/// Currently raises an exception
		void hash( HashType hashType, double time, IECore::MurmurHash &h ) const override;

		/*
		* Utility functions
		*/

		/// Returns the MDagPath object to the scene node
		MDagPath dagPath() const;

		/// Returns the scene path corresponding to the maya dag path
		static void dagPathToPath( MDagPath dagPath, IECoreScene::SceneInterface::Path &path );

		/// Returns the maya dag path corresponding to the scene path
		static void pathToDagPath( const IECoreScene::SceneInterface::Path &path, MDagPath &dagPath );

		/// Translates cortex attribute name to maya attribute name
		/// Returns an empty string if there are no valid mappings
		static Name toMayaAttributeName( const Name &name );

		/// Translates maya attribute name to cortex attribute name
		/// Returns an empty string if there are no valid mappings
		static Name fromMayaAttributeName( const Name &name );

		typedef boost::function<bool (const MDagPath &)> HasFn;
		typedef boost::function<IECore::ConstObjectPtr (const MDagPath &)> ReadFn;
		typedef boost::function<IECore::ConstObjectPtr (const MDagPath &, const Name &)> ReadAttrFn;
		typedef boost::function<bool (const MDagPath &, const Name &, int )> HasTagFn;
		typedef boost::function<void (const MDagPath &, NameList &, int)> ReadTagsFn;
		typedef boost::function<void (const MDagPath &, NameList &)> NamesFn;
		typedef boost::function<bool (const MDagPath &, const Name &)> MightHaveFn;

		/// Register callbacks for custom objects.
		/// The has function will be called during hasObject and it stops in the first one that returns true.
		/// The read method is called if the has method returns true, so it should return a valid Object pointer or raise an Exception.
		static void registerCustomObject( HasFn hasFn, ReadFn readFn );

		/// Register callbacks for custom attributes.
		/// The names function will be called during attributeNames and hasAttribute.
		/// The readAttr method is called if the names method returns the expected attribute, so it should return a valid Object pointer or raise an Exception.
		/// If the mightHave function is specified, it will be called before names function for early out, to see if the names function can return the expected attribute.
		static void registerCustomAttributes( NamesFn namesFn, ReadAttrFn readFn );
		static void registerCustomAttributes( NamesFn namesFn, ReadAttrFn readFn, MightHaveFn mightHaveFn);

		/// Register callbacks for nodes to define custom tags
		/// The functions will be called during hasTag and readTags.
		/// readTags will return the union of all custom ReadTagsFns.
		static void registerCustomTags( HasTagFn hasFn, ReadTagsFn readFn );

	private :

		IECoreScene::SceneInterfacePtr retrieveScene( const Path &path, MissingBehaviour missingBehaviour = SceneInterface::ThrowIfMissing ) const;
		IECoreScene::SceneInterfacePtr retrieveChild( const Name &name, MissingBehaviour missingBehaviour = SceneInterface::ThrowIfMissing ) const;
		IECoreScene::SceneInterfacePtr retrieveParent() const;

		void getChildDags( const MDagPath& dagPath, MDagPathArray& paths ) const;

		/// Struct for registering readers for custom Object.
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

		/// Struct for registering readers for custom Attributes.
		struct CustomAttributeReader
		{
			NamesFn m_names;
			ReadAttrFn m_read;
			MightHaveFn m_mightHave;
		};

		static std::vector< CustomReader > &customObjectReaders();
		static std::vector< CustomAttributeReader > &customAttributeReaders();
		static std::vector< CustomTagReader > &customTagReaders();

	protected:

		/// constructor for a specific dag path:
		LiveScene( const MDagPath& p, bool isRoot = false );

		MDagPath m_dagPath;
		bool m_isRoot;

		/// calls the constructor for a specific dag path. Derived classes can override this so their child() and scene() methods can
		/// return instances of the derived class
		virtual LiveScenePtr duplicate( const MDagPath& p, bool isRoot = false ) const;

		typedef tbb::mutex Mutex;
		static Mutex s_mutex;

};

} // namespace IECoreMaya

#endif // IECOREMAYA_LIVESCENE_H
