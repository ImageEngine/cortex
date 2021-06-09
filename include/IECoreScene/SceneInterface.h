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

#ifndef IECORESCENE_SCENEINTERFACE_H
#define IECORESCENE_SCENEINTERFACE_H

#include "IECoreScene/Export.h"
#include "IECoreScene/PrimitiveVariable.h"
#include "IECoreScene/Renderable.h"
#include "IECoreScene/TypeIds.h"

#include "IECore/Canceller.h"
#include "IECore/Export.h"
#include "IECore/Object.h"
#include "IECore/PathMatcher.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "OpenEXR/ImathBox.h"
#include "OpenEXR/ImathMatrix.h"
IECORE_POP_DEFAULT_VISIBILITY

namespace IECoreScene
{

IE_CORE_FORWARDDECLARE( SceneInterface );

/// A pure virtual base class for navigating a hierarchical animated 3D scene.
/// A scene is defined by a hierarchy of named 3D transforms.
/// Each SceneInterface instance maps to a specific transform in a scene, uniquely identified by it's path.
/// A path is an array of transform names.
/// Using the method child, you can explore the hierarchy (and create new transforms).
/// Each transform on the hierarchy has a unique name and contains the 3D transformation, custom attributes, tags,
/// a bounding box, a main object and more child transforms. All of them can be animated.
/// Animation is stored by providing the time and the value. And it's retrieved by querying it's value at any time, and if the
/// animation is inherently sampled, interpolation will be applied for queries on attributes, objects, transforms and bounds.
/// The path to the root transform is an empty array. The name of the root transform is "/" though.
/// The root transform by definition cannot store transformation or an object. Attributes and Tags are allowed.
/// Tags are string labels assigned to any location in a scene and they are propagated up and down in the hierarchy
/// when the scene is saved to files, so they can be used for efficiently filtering the hierarchy.
/// Check on the Maya and Houdini scene reader nodes for examples on how to filter by tag.
/// \ingroup ioGroup
/// \todo Implement a TransformStack class that can represent any custom
/// transformation that could be interpolated and consider using it here as the
/// returned type as opposed to DataPtr.
class IECORESCENE_API SceneInterface : public IECore::RunTimeTyped
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( SceneInterface, SceneInterfaceTypeId, IECore::RunTimeTyped );

		typedef IECore::IndexedIO::EntryID Name;
		typedef IECore::IndexedIO::EntryIDList NameList;
		typedef IECore::IndexedIO::EntryIDList Path;

		typedef enum {
			ThrowIfMissing = IECore::IndexedIO::ThrowIfMissing,
			NullIfMissing = IECore::IndexedIO::NullIfMissing,
			CreateIfMissing = IECore::IndexedIO::CreateIfMissing
		} MissingBehaviour;

		enum TagFilter {
			DescendantTag = 1,
			LocalTag = 2,
			AncestorTag = 4,
			EveryTag = DescendantTag | LocalTag | AncestorTag
		};

		/// Defines the type of hash to be computed.
		/// The hierarchy hash includes all the other types of hash for the queried location and all it's children locations.
		enum HashType {
			TransformHash,
			AttributesHash,
			BoundHash,
			ObjectHash,
			ChildNamesHash,
			HierarchyHash,
		};

		/// Constant name assigned to the root location "/".
		static const Name &rootName;
		/// Utility variable that can be used anytime you want to refer to the root path in the Scene.
		static const Path &rootPath;
		/// Name of the visibility attribute
		static const Name &visibilityName;

		/// Create an instance of a subclass which is able to open the file found at "path".
		/// Files can be opened for Read, Write, or Append depending on the derived classes.
		/// During "Read" operations it is not permitted to make any modifications to the underlying files.
		/// When opening a scene file in "Write" mode its contents below the root directory are removed.
		/// For "Append" operations (if supported) it is possible to write new files, or overwrite existing ones.
		/// \param path A file on disk. The appropriate scene interface for reading/writing is determined by the path's extension.
		/// \param mode A bitwise-ORed combination of constants which determine how the file system should be accessed.
		static SceneInterfacePtr create(const std::string &path, IECore::IndexedIO::OpenMode mode);

		/// Returns all the file extensions for which a SceneInterface implementation is
		/// available for the given access mode(s). Extensions do not include the preceding dot character ('.').
		static std::vector<std::string> supportedExtensions( IECore::IndexedIO::OpenMode modes = IECore::IndexedIO::Read|IECore::IndexedIO::Write|IECore::IndexedIO::Append );

		/// Static instantation of one of these (with a supported file extension) using a subclass as the template parameter  will register it
		/// as a supported SceneInterface file format. This allows read and write operations to be performed generically, with the correct interface to
		/// use being automatically determined by the system.
		template< class T >
		struct FileFormatDescription
		{
			public :
				FileFormatDescription(const std::string &extension, IECore::IndexedIO::OpenMode modes);

			private :
				static SceneInterfacePtr creator( const std::string &fileName, IECore::IndexedIO::OpenMode mode );
		};

		~SceneInterface() override = 0;

		/// Returns the file that this scene is mapped to. Throws exception if there's no file.
		virtual std::string fileName() const = 0;

		/// Returns the name of the scene location which this instance is referring to. The root path returns "/".
		virtual Name name() const = 0;
		/// Returns the path scene this instance is referring to.
		virtual void path( Path &p ) const = 0;

		/*
		 * Bounding box
		 */

		/// Returns true if a bounding box is available for reading, false if not.
		/// Default implementation returns true.
		virtual bool hasBound() const;
		/// Returns the bounding box for the entire scene contents from
		/// path() down, inclusive of the object at this path, but
		/// exclusive of the transform at this path.
		virtual Imath::Box3d readBound( double time ) const = 0;
		/// Writes the bound for this path, overriding the default bound
		/// that would be written automatically. Note that it might be useful
		/// when writing objects which conceptually have a bound but don't
		/// derive from VisibleRenderable.
		virtual void writeBound( const Imath::Box3d &bound, double time ) = 0;

		/*
		 * Transform
		 */

		/// Returns the interpolated transform object of this node at the specified
		/// point in time.
		virtual IECore::ConstDataPtr readTransform( double time ) const = 0;
		/// Returns the transform of this node at the specified
		/// point in time as a matrix.
		virtual Imath::M44d readTransformAsMatrix( double time ) const = 0;
		/// Writes the transform applied to this path within the scene.
		/// Raises an exception if you try to write transform in the root path
		/// Currently it only accepts M44dData or TransformationMatrixdData.
		virtual void writeTransform( const IECore::Data *transform, double time ) = 0;

		/*
		 * Attributes
		 */

		/// Convenience method to determine if an attribute exists without reading it
		virtual bool hasAttribute( const Name &name ) const = 0;
		/// Fills attrs with the names of all attributes available in the current directory
		virtual void attributeNames( NameList &attrs ) const = 0;
		/// Returns the attribute value at the given time.
		virtual IECore::ConstObjectPtr readAttribute( const Name &name, double time ) const = 0;
		/// Writers the attribute to this path within the scene
		/// Raises an exception if you try to write an attribute in the root path with a time different than 0.
		virtual void writeAttribute( const Name &name, const IECore::Object *attribute, double time ) = 0;

		/*
		 * Tags
		 */

		/// Utility function that quickly checks for the existence of one tag relative to the current scene location and the given filter.
		/// \param filter Will filter the results based on a combination of flags DescendantTag, LocalTag and AncestorTag. Use LocalTag for tags stored in the current scene location (default). DescendantTags for tags stored in child locations and AncestorTags for tags stored in parent locations.
		/// Some implementations may not support all combinations of these flags and will ignore them.
		virtual bool hasTag( const Name &name, int filter = LocalTag ) const = 0;
		/// Reads all the tags relative to the current scene location and the filter. Does not guarantee unique set of tags to be returned.
		/// \param filter Will filter the results based on a combination of flags DescendantTag, LocalTag and AncestorTag. Use LocalTag for tags stored in the current scene location (default). DescendantTags for tags stored in child locations and AncestorTags for tags stored in parent locations.
		/// Some implementations may not support all combinations of these flags and will ignore them.
		virtual void readTags( NameList &tags, int filter = LocalTag ) const = 0;
		/// Adds tags to the current scene location.
		virtual void writeTags( const NameList &tags ) = 0;

		/*
		 * Sets
		 */

		/// Returns the names of all sets containing objects in this location and all of its descendants.
		virtual NameList setNames( bool includeDescendantSets = true ) const = 0;
		/// Reads the named set. All paths returned are relative to the current location.
		/// If provided, the Canceller will periodically be checked, terminating the read with an exception if
		/// the result is no longer needed.
		virtual IECore::PathMatcher readSet( const Name &name, bool includeDescendantSets = true, const IECore::Canceller *canceller = nullptr ) const = 0;
		/// Writes a set at the current location. All paths are specified relative to the current location.
		virtual void writeSet( const Name &name, const IECore::PathMatcher &set ) = 0;
		/// Hash a named set at the current location.
		virtual void hashSet( const Name& setName, IECore::MurmurHash &h ) const = 0;

		/*
		 * IECore::Object
		 */

		/// Convenience method to determine if a piece of geometry exists without reading it
		virtual bool hasObject() const = 0;
		/// Reads the object stored at this path in the scene at the given time.
		/// If provided, the Canceller will periodically be checked, terminating the read with an exception if
		/// the result is no longer needed.
		virtual IECore::ConstObjectPtr readObject( double time, const IECore::Canceller *canceller = nullptr ) const = 0;
		/// Reads primitive variables from the object of type Primitive stored at this path in the scene at the given time.
		/// Raises exception if it turns out not to be a Primitive object.
		/// Note that according to both internal comments and a brief code search, this function is now unused,
		/// and should probably be deprecated
		virtual PrimitiveVariableMap readObjectPrimitiveVariables( const std::vector<IECore::InternedString> &primVarNames, double time ) const = 0;
		/// Writes a geometry to this path in the scene.
		/// Raises an exception if you try to write an object in the root path.
		virtual void writeObject( const IECore::Object *object, double time ) = 0;

		/*
		 * Hierarchy
		 */

		/// Convenience method to determine if a child exists
		virtual bool hasChild( const Name &name ) const = 0;
		/// Queries the names of any existing children of path() within
		/// the scene.
		virtual void childNames( NameList &childNames ) const = 0;
		/// Returns an object for the specified child location in the scene.
		/// If the child does not exist then it will behave according to the
		/// missingBehavior parameter. May throw and exception, may return a NULL pointer,
		/// or may create the child (if that is possible).
		/// Bounding boxes will be automatically propagated up from the children
		/// to the parent as it is written.
		virtual SceneInterfacePtr child( const Name &name, MissingBehaviour missingBehaviour = ThrowIfMissing ) = 0;
		/// Returns a read-only interface for a child location in the scene.
		virtual ConstSceneInterfacePtr child( const Name &name, MissingBehaviour missingBehaviour = ThrowIfMissing ) const = 0;
		/// Returns a writable interface to a new child. Throws an exception if it already exists.
		/// Bounding boxes will be automatically propagated up from the children
		/// to the parent as it is written.
		virtual SceneInterfacePtr createChild( const Name &name ) = 0;
		/// Returns a interface for querying the scene at the given path (full path).
		virtual SceneInterfacePtr scene( const Path &path, MissingBehaviour missingBehaviour = ThrowIfMissing ) = 0;
		/// Returns a const interface for querying the scene at the given path (full path).
		virtual ConstSceneInterfacePtr scene( const Path &path, MissingBehaviour missingBehaviour = ThrowIfMissing ) const = 0;

		/*
		 * Hash
		 */

		/// Computes the requested type of hash for the current location on the scene at the given time.
		/// The hash returned is not content-based, but it uniquely identifies the queried information so that
		/// it can be used for memory caches, for example, used by ComputationCache objects.
		/// This function is only available when reading scenes and it raises an exception otherwise.
		/// The base class implementation only adds the class typeId information to garantee that the hash won't
		/// collide with other Cortex objects and derived classes are responsible to call the base class implementation
		/// as well as add the time dependency as applicable.
		virtual void hash( HashType hashType, double time, IECore::MurmurHash &h ) const;

		/*
		 * Utility functions
		 */

		/// Converts a internal Path to a path-like string. The root path results in "/".
		static void pathToString( const Path &p, std::string &path );
		/// Returns the object path within the scene that this class is
		/// referring to. For instances created using the constructor
		/// this will be "/". Use the childNames() and child() methods
		/// to traverse to other parts of the scene.
		static void stringToPath( const std::string &path, Path &p );

	protected:

		typedef SceneInterfacePtr (*CreatorFn)(const std::string &, IECore::IndexedIO::OpenMode );
		class CreatorMap;
		static CreatorMap &fileCreators();
		static void registerCreator( const std::string &extension, IECore::IndexedIO::OpenMode modes, CreatorFn f );

};

IECORESCENE_API std::ostream &operator <<( std::ostream &o, const SceneInterface::Path &path );

} // namespace IECoreScene

#include "IECoreScene/SceneInterface.inl"

#endif // IECORESCENE_SCENEINTERFACE_H
