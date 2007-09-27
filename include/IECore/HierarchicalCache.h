//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_HIERARCHICALCACHE_H
#define IE_CORE_HIERARCHICALCACHE_H

#include <string>
#include <vector>

#include <IECore/RefCounted.h>
#include <IECore/IndexedIO.h>
#include <OpenEXR/ImathMatrix.h>
#include <OpenEXR/ImathBox.h>
#include <IECore/VisibleRenderable.h>
#include <IECore/TreeGraphDependency.h>


namespace IECore
{

IE_CORE_FORWARDDECLARE( Object );
IE_CORE_FORWARDDECLARE( CompoundObject );
IE_CORE_FORWARDDECLARE( Group );
IE_CORE_FORWARDDECLARE( IndexedIOInterface );

///A simple means of storing VisibleRenderable objects in a hierarchical way.
///Uses an IndexedIOInterface object to access the file. The file is organized as following:
///- /headersH/<headerName> (Object)
///- /children/<objName>/attributes/<attrName>
///- /children/<objName>/boundingBox (Box3f)
///- /children/<objName>/transformMatrix (M44f) or /children/<objName>/shape/ ( VisibleRenderable ).
///- /children/<objName>/children/...
///Will throw an exception derived from IEException if any errors are encountered.
///\todo Create a base class for this one and AttributeCache.
class HierarchicalCache : public RefCounted
{
	public:
				
		typedef IndexedIO::EntryID ObjectHandle;
		typedef IndexedIO::EntryID HeaderHandle;
		typedef IndexedIO::EntryID AttributeHandle;
		
		///Open the cache, using the specified open mode
		HierarchicalCache( const std::string &filename, IndexedIO::OpenMode mode );

		~HierarchicalCache();
		
		///Write a piece of data associated with the specified object and attribute to the cache.
		///This function will create every necessary intermediate node found in the absolute object name.
		void write( const ObjectHandle &obj, const AttributeHandle &attr, ObjectPtr data );	

		///Write data associated with the specified header to the cache.
		void writeHeader( const HeaderHandle &hdr, ObjectPtr data );
	
		///Read a piece of data associated with the specified object and attribute from the cache.
		///Throws an exception if the requested data is not present in the cache.
		ObjectPtr read( const ObjectHandle &obj, const AttributeHandle &attr );

		///Read a piece of data associated with the specified object from the cache.
		///Returns a CompoundObject with attribute as keys.
		///Throws an exception if the requested data is not present in the cache.
		CompoundObjectPtr read( const ObjectHandle &obj );

		///Read data associated with the specified header from the cache.
		///Throws an exception if the requested header is not present in the cache.
		ObjectPtr readHeader( const HeaderHandle &hdr );

		///Read all header data present in the cache. 
		///Creates a CompoundObject with the header names as keys.
		CompoundObjectPtr readHeader( );
		
		///Retrieve the list of object handles from the cache
		void objects(std::vector<ObjectHandle> &objs);

		///Retrieve the list of header handles from the cache
		void headers(std::vector<HeaderHandle> &hds);
		
		///Retrieve the list of attribute handles from the specified objects. Throws
		///an exception if the object is not within the cache.
		void attributes(const ObjectHandle &obj, std::vector<AttributeHandle> &attrs);
		
		///Retrieve the list of attribute handles that match the given regex from the specified objects.
		/// Throws an exception if the object is not within the cache.
		void attributes(const ObjectHandle &obj, const std::string regex, std::vector<AttributeHandle> &attrs);

		///Determines whether or not the cache contains the specified object
		bool contains( const ObjectHandle &obj );
		
		///Determines whether or not the cache contains the specified object and attribute
		bool contains( const ObjectHandle &obj, const AttributeHandle &attr );

		///Removes an object from the cache file.
		void remove( const ObjectHandle &obj );
	
		///Removes an object's attribute from the cache file.
		void remove( const ObjectHandle &obj, const AttributeHandle &attr );

		///Removes a header from the cache file.
		void removeHeader( const HeaderHandle &hdr );

		///Write a M44f object to the given transform node.
		///Overwrites shape nodes without errors.
		///This function will create every necessary intermediate node found in the absolute object name.
		void write( const ObjectHandle &obj, const Imath::M44f &matrix );

		///Write a VisibleRenderable object to the given shape node.
		///Overwrites transform nodes without errors.
		///This function will create every necessary intermediate node found in the absolute object name.
		void write( const ObjectHandle &obj, ConstVisibleRenderablePtr shape );

		///Returns true if the given node is a shape node.
		///Returns false for the root.
		bool isShape( const ObjectHandle &obj );

		///Returns true if the given node is a transform node.
		///Returns false for the root.
		bool isTransform( const ObjectHandle &obj );

		///Returns the list of objects that are children of the given transform node.
		///The first level in the hierarchy is "/" ( returned by rootObject() ).
		void children(  const ObjectHandle &obj, std::vector<ObjectHandle> &children );

		///Returns the M44f object stored in the given transform node.
		/// Throws an exception if it's not a transform node.
		Imath::M44f transformMatrix( const ObjectHandle &obj );

		///Returns the VisibleRenderable object stored in the given shape node.
		/// Throws an exception if it's not a shape node.
		VisibleRenderablePtr shape( const ObjectHandle &obj );

		///Returns the world matrix up to the given node.
		///Works for any kind of node.
		Imath::M44f globalTransformMatrix( const ObjectHandle &obj );

		///Returns the bounding box in local space for the given node.
		Imath::Box3f bound( const ObjectHandle &obj );

		///Returns the full name for an object ( includes parent names and separators ).
		///All the other functions in this class require absolute names.
		static ObjectHandle absoluteName( const ObjectHandle &relativeName, const ObjectHandle &parent = rootName() );

		///Returns the relative name for an object ( does not include parent names or separators ).
		static ObjectHandle relativeName( const ObjectHandle &obj );

		///Returns the parent node name given a child object name.
		///Throws an exception for the root node.
		static ObjectHandle parentName( const ObjectHandle &obj );

		///Returns the root node name: "/"
		static ObjectHandle rootName()
		{
			return "/";
		}

	protected:

		///Converts an absoluteName to canonical name by eliminating the last backslash in the path.
		static ObjectHandle canonicalName( const ObjectHandle &obj );

		IndexedIOInterfacePtr m_io;

		///Internal dependency graph for lazy computation of bounding boxes.
		///Parent nodes are dependent on their child nodes.
		class CacheDependency : public TreeGraphDependency< std::string >
		{
			public:
				CacheDependency( HierarchicalCache *cache ) : m_cache( cache ) {};

				///Returns the root node name
				virtual std::string rootNode() const;

				///Returns true if node1 is parented directly or indirectly to node2.
				///Throws Exception if the node names are not full path.
				virtual bool isDescendant( const std::string &node1, const std::string &node2 ) const;

				///Updates a node. It's guarantee that all dependent nodes are updated.
				virtual void compute( const std::string &node );

			private:

				HierarchicalCache *m_cache;
			
		};
		boost::intrusive_ptr< CacheDependency > m_dependency;

		friend class CacheDependency;

		///Utility function used by globalTransformMatrix().
		Imath::M44f recursiveTransformMatrix( const ObjectHandle &obj, const Imath::M44f &world );

		///Utility function used by objects() function to extract all object names saved in the file.
		void recursiveObjects(std::vector<ObjectHandle> &objs, const ObjectHandle parent = rootName(), size_t totalSize = 0 );

		///Returns the internal IndexedIO path for a given object
		static void objectPath( const ObjectHandle &obj, IndexedIO::EntryID &path );

		///Returns the internal IndexedIO path to the attributes directory for a given object
		static void attributesPath( const ObjectHandle &obj, IndexedIO::EntryID &path );

		///Returns the internal IndexedIO path for a given attribute on a given object
		static void attributePath( const ObjectHandle &obj, const AttributeHandle &attr, IndexedIO::EntryID &path );

		///Guarantees object existence
		IndexedIO::EntryID guaranteeObject( const ObjectHandle &obj );

		///Make sure all unsaved data is saved on the file.
		void flush();

		///Computes the current bounding box and updates the object in the file
		void updateNode( const ObjectHandle &obj );

		///Saves the object bounding box in the file
		void updateBound( const ObjectHandle &obj, Imath::Box3f box );

		///Returns the VisibleRenderable object stored in the current directory or NULL if there's no shape.
		VisibleRenderablePtr loadShape( );

		///Returns the Box3fData object stored in the current directory or NULL if there's no bounding box.
		bool loadBound( Imath::Box3f &b );

		///Loads the M44f object stored in the current directory. Returns false if could not read the object.
		bool loadTransform( Imath::M44f &m );
};

IE_CORE_DECLAREPTR( HierarchicalCache );
	
} // namespace IECore

#endif //IE_CORE_HIERARCHICALCACHE_H
