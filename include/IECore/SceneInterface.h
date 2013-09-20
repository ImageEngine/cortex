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

#ifndef IECORE_SCENEINTERFACE_H
#define IECORE_SCENEINTERFACE_H

#include "OpenEXR/ImathBox.h"
#include "OpenEXR/ImathMatrix.h"

#include "IECore/Object.h"
#include "IECore/Renderable.h"
#include "IECore/PrimitiveVariable.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( SceneInterface );

/// A pure virtual base class for navigating a hierarchical animated 3D scene.
/// A scene is defined by a hierarchy of 3D transforms.
/// Each SceneInterface instance maps to a specific transform in a scene, uniquely identified by it's path.
/// A path is an array of transform names.
/// Using the method child, you can explore the hierarchy (and create new transforms).
/// Each transform on the hierarchy has a unique name and contains the 3D transformation, custom attributes, tags,
/// a bounding box, a main object and more child transforms. All of them can be animated.
/// Animation is stored by providing the time and the value. When retrieving animation, the interface allows 
/// for reading the individual stored samples or the interpolated value at any given time. 
/// The path to the root transform is an empty array. The name of the root transform is "/" though.
/// The root transform by definition cannot store transformation or an object. Attributes and Tags are allowed.
/// Tags is simply a set of labels assigned to any location in a scene and they are propagated up in the hierarchy
/// when the scene is saved, so they can be used to filter the scene when reading. For example, some geo could be 
/// tagged as "proxy", then readers could easily only display the proxy geometry for quick previsualization.
/// Care must be taken when a tag is set in the middle of the hierarchy as child locations will not inherit the tag.
/// \ingroup ioGroup
/// \todo Implement a TransformStack class that can represent any custom 
/// transformation that could be interpolated and consider using it here as the
/// returned type as opposed to DataPtr.
class SceneInterface : public RunTimeTyped
{
	public :

		IE_CORE_DECLARERUNTIMETYPED( SceneInterface, RunTimeTyped );

		typedef IndexedIO::EntryID Name;
		typedef IndexedIO::EntryIDList NameList;
		typedef IndexedIO::EntryIDList Path;

		typedef enum {
			ThrowIfMissing = IndexedIO::ThrowIfMissing,
			NullIfMissing = IndexedIO::NullIfMissing,
			CreateIfMissing = IndexedIO::CreateIfMissing
		} MissingBehaviour;

		/// Constant name assigned to the root location "/".
		static const Name &rootName;
		/// Utility variable that can be used anytime you want to refer to the root path in the Scene.
		static const Path &rootPath;

		/// Create an instance of a subclass which is able to open the file found at "path".
		/// Files can be opened for Read, Write, or Append depending on the derived classes.
		/// During "Read" operations it is not permitted to make any modifications to the underlying files.
		/// When opening a scene file in "Write" mode its contents below the root directory are removed.
		/// For "Append" operations (if supported) it is possible to write new files, or overwrite existing ones.
		/// \param path A file on disk. The appropriate scene interface for reading/writing is determined by the path's extension.
		/// \param mode A bitwise-ORed combination of constants which determine how the file system should be accessed.
		static SceneInterfacePtr create(const std::string &path, IndexedIO::OpenMode mode);
		
		/// Returns all the file extensions for which a SceneInterface implementation is
		/// available for the given access mode(s). Extensions do not include the preceding dot character ('.').
		static std::vector<std::string> supportedExtensions( IndexedIO::OpenMode modes = IndexedIO::Read|IndexedIO::Write|IndexedIO::Append );

		/// Static instantation of one of these (with a supported file extension) using a subclass as the template parameter  will register it
		/// as a supported SceneInterface file format. This allows read and write operations to be performed generically, with the correct interface to
		/// use being automatically determined by the system.
		template< class T >
		struct FileFormatDescription
		{
			public :
				FileFormatDescription(const std::string &extension, IndexedIO::OpenMode modes);

			private :
				static SceneInterfacePtr creator( const std::string &fileName, IndexedIO::OpenMode mode );
		};
		
		virtual ~SceneInterface() = 0;

		/// Returns the file that this scene is mapped to. Throws exception if there's no file.
		virtual std::string fileName() const = 0;

		/// Returns the name of the scene location which this instance is referring to. The root path returns "/".
		virtual Name name() const = 0;
		/// Returns the path scene this instance is referring to. 
		virtual void path( Path &p ) const = 0;

		/*
		 * Bounding box
		 */

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
		virtual ConstDataPtr readTransform( double time ) const = 0;
		/// Returns the transform of this node at the specified 
		/// point in time as a matrix.
		virtual Imath::M44d readTransformAsMatrix( double time ) const = 0;
		/// Writes the transform applied to this path within the scene.
		/// Raises an exception if you try to write transform in the root path
		/// Currently it only accepts M44dData or TransformationMatrixdData.
		virtual void writeTransform( const Data *transform, double time ) = 0;

		/*
		 * Attributes
		 */

		/// Convenience method to determine if an attribute exists without reading it
		virtual bool hasAttribute( const Name &name ) const = 0;
		/// Fills attrs with the names of all attributes available in the current directory
		virtual void attributeNames( NameList &attrs ) const = 0;
		/// Returns the attribute value at the given time.
		virtual ConstObjectPtr readAttribute( const Name &name, double time ) const = 0;
		/// Writers the attribute to this path within the scene
		/// Raises an exception if you try to write an attribute in the root path with a time different than 0.
		virtual void writeAttribute( const Name &name, const Object *attribute, double time ) = 0;

		/*
		 * Tags
		 */

		/// Utility function that quickly checks for the existence of one tag in the scene
		/// \param includeChildren If false, then it will return true only if the given tag was written in the current scene location.
		/// Otherwise, will return true if the tag was set on any child location.
		virtual bool hasTag( const Name &name, bool includeChildren = true ) const = 0;
		/// Reads all the tags on the current scene location.
		/// \param includeChildren If false, then it will return tags that were written only in the current scene location.
		/// Otherwise, will include the union of all tags from the children as well which is usually the expected behavior. 
		/// Some implementations may not support recursing to the children due to performance reasons and will ignore this flag.
		virtual void readTags( NameList &tags, bool includeChildren = true ) const = 0;
		/// Adds tags to the current scene location.
		virtual void writeTags( const NameList &tags ) = 0;

		/*
		 * Object
		 */

		/// Convenience method to determine if a piece of geometry exists without reading it
		virtual bool hasObject() const = 0;
		/// Reads the object stored at this path in the scene at the given time.
		virtual ConstObjectPtr readObject( double time ) const = 0;
		/// Reads primitive variables from the object of type Primitive stored at this path in the scene at the given time. 
		/// Raises exception if it turns out not to be a Primitive object.
		virtual PrimitiveVariableMap readObjectPrimitiveVariables( const std::vector<InternedString> &primVarNames, double time ) const = 0;
		/// Writes a geometry to this path in the scene.
		/// Raises an exception if you try to write an object in the root path.
		virtual void writeObject( const Object *object, double time ) = 0;

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

		typedef SceneInterfacePtr (*CreatorFn)(const std::string &, IndexedIO::OpenMode );
		class CreatorMap;
		static CreatorMap &fileCreators();
		static void registerCreator( const std::string &extension, IndexedIO::OpenMode modes, CreatorFn f );

};

} // namespace IECore

#include "IECore/SceneInterface.inl"

#endif // IECORE_SCENEINTERFACE_H
