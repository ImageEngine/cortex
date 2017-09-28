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

#ifndef IE_CORE_INDEXEDIO_H
#define IE_CORE_INDEXEDIO_H

#include <string>
#include <vector>
#include <map>
#include <stdint.h>

#include "OpenEXR/half.h"

#include "IECore/Export.h"
#include "IECore/RunTimeTyped.h"
#include "IECore/InternedString.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( IndexedIO );

/// Abstract interface to define operations on a random-access indexed input/output device. All methods throw an instance of IOException,
/// or one of its subclasses, if an error is encountered.
/// \ingroup ioGroup
class IECORE_API IndexedIO : public RunTimeTyped
{
	public:

		IE_CORE_DECLARERUNTIMETYPED( IndexedIO, RunTimeTyped );

		/// General enums and low level structures
		enum OpenModeFlags
		{
			Read      = 1L << 0,
			Write     = 1L << 1,
			Append    = 1L << 2,

			Shared    = 1L << 3,
			Exclusive = 1L << 4,
		} ;

		typedef unsigned OpenMode;

		typedef enum
		{
			Directory=0,
			File
		} EntryType;


		typedef enum
		{
			Invalid=0,
			Float,
			FloatArray,
			Double,
			DoubleArray,
			Int,
			IntArray,
			Long, /// Obsolete
			LongArray, /// Obsolete
			String,
			StringArray,
			UInt,
			UIntArray,
			Char,
			CharArray,
			UChar,
			UCharArray,
			Half,
			HalfArray,
			Short,
			ShortArray,
			UShort,
			UShortArray,
			Int64,
			Int64Array,
			UInt64,
			UInt64Array,
			InternedStringArray
		} DataType;

		/// Enum used to specify behavior when querying child directories.
		typedef enum {
			ThrowIfMissing = 0,
			NullIfMissing,
			CreateIfMissing
		} MissingBehaviour;

		typedef InternedString EntryID;
		typedef std::vector< EntryID > EntryIDList;
		class Entry;
		// singleton representing the root name
		static const EntryID rootName;
		// singleton representing the root location (to be passed in the factory function)
		static const EntryIDList rootPath;

		typedef IndexedIOPtr (*CreatorFn)(const std::string &, const EntryIDList &, IndexedIO::OpenMode );

		/// Create an instance of a subclass which is able to open the IndexedIO structure found at "path".
		/// Files can be opened for Read, Overwrite, or Append.
		/// During "Read" operations it is not permitted to make any modifications to the underlying files.
		/// When opening a device in "Write" mode its contents below the root directory are removed.
		/// For "Append" operations it is possible to write new files, or overwrite existing ones. It is not possible to overwrite entire directories, however.
		/// \param path A file or directory on disk. The appropriate reader for reading/writing is determined by the path's extension.
		/// \param root The root point to 'mount' the structure.
		/// \param mode A bitwise-ORed combination of constants which determine how the file system should be accessed.
		static IndexedIOPtr create( const std::string &path, const EntryIDList &root, IndexedIO::OpenMode mode);

		/// Fills the passed vector with all the extensions for which an IndexedIO implementation is
		/// available. Extensions are of the form "fio" - ie without a preceding '.'.
		static void supportedExtensions( std::vector<std::string> &extensions );

		/// Static instantation of one of these (with a supported file extension) using a subclass as the template parameter  will register it
		/// as a supported IndexedIO. This allows read and write operations to be performed generically, with the correct interface to
		/// use being automatically determined by the system.
		template<class T>
		struct Description
		{
			Description(const std::string &extension) { IndexedIO::registerCreator( extension, &T::create ); }
		};

		virtual ~IndexedIO();

		/// Returns the mode with which the interface was created.
		virtual IndexedIO::OpenMode openMode() const = 0;

		/// Retrieve the current directory. Returns empty list at the root location.
		virtual void path( IndexedIO::EntryIDList & ) const = 0;

		/// Returns whether the given entry exists in the file.
		virtual bool hasEntry( const IndexedIO::EntryID &name ) const = 0;

		/// Returns the EntryID for the current directory in the file. The root location has a special name "/".
		virtual const IndexedIO::EntryID &currentEntryId() const = 0;

		/// Stores in the given array all the ids of all files and directories
		virtual void entryIds( IndexedIO::EntryIDList &names ) const = 0;

		// Stores in the given array all the ids for the given entry type
		virtual void entryIds( IndexedIO::EntryIDList &names, IndexedIO::EntryType type ) const = 0;

		/// Returns a new interface for the child or if missing then consults missingBehaviour and throws exception if ThrowIfMissing,
		//returns Null pointer if NullIfMissing or creates the child directory if CreateIfMissing.
		virtual IndexedIOPtr subdirectory( const IndexedIO::EntryID &name, MissingBehaviour missingBehaviour = ThrowIfMissing ) = 0;

		/// Returns read-only interface for the child directory or if missing then consults missingBehaviour and throws exception if
		// ThrowIfMissing or CreateIfMissing, returns Null pointer if NullIfMissing.
		virtual ConstIndexedIOPtr subdirectory( const IndexedIO::EntryID &name, MissingBehaviour missingBehaviour = ThrowIfMissing ) const = 0;

		/// Return details of a specific child entry or raises an exception if it doesn't exist.
		virtual IndexedIO::Entry entry( const IndexedIO::EntryID &name ) const = 0;

		/// Creates a subdirectory and returns a writable interface for it or Throws an exception if the subdirectory already exists.
		virtual IndexedIOPtr createSubdirectory( const IndexedIO::EntryID &name ) = 0;

		/// Remove a specified child file or directory.
		/// Any IndexedIO instances to child directories will be in a invalid state and should not be used after remove is called.
		virtual void remove( const IndexedIO::EntryID &name ) = 0;

		/// Remove all entries.
		/// Any IndexedIO instances to child directories will be in a invalid state and should not be used after remove is called.
		virtual void removeAll() = 0;

		/// Commit the contents of the current directory to the file, further changes on this directory or it's subdirectories are not allowed.
		/// This helps freeing memory and also gives hints to the implementation classes to structure the file format in a sensible way.
		/// The commit() method is called by Object::save function.
		/// Any IndexedIO instances to child directories will be in a invalid state and should not be used after commit is called.
		virtual void commit() = 0;

		/// Returns a new interface for the parent of this node in the file or a NULL pointer if it's the root.
		virtual IndexedIOPtr parentDirectory() = 0;

		/// Returns a read-only interface for the parent of this node in the file or a NULL pointer if it's the root.
		virtual ConstIndexedIOPtr parentDirectory() const = 0;

		/// Returns a new interface for the given path in the file.
		virtual IndexedIOPtr directory( const IndexedIO::EntryIDList &path, MissingBehaviour missingBehaviour = ThrowIfMissing ) = 0;

		/// Returns a read-only interface for the given path in the file.
		virtual ConstIndexedIOPtr directory( const IndexedIO::EntryIDList &path, MissingBehaviour missingBehaviour = ThrowIfMissing ) const = 0;

		/// Create a new file containing the specified float array contents
		/// \param name The name of the file to be written
		/// \param x The data to write
		/// \param arrayLength The number of elements in the array
		virtual void write(const IndexedIO::EntryID &name, const float *x, unsigned long arrayLength) = 0;

		/// Create a new file containing the specified double array contents
		/// \param name The name of the file to be written
		/// \param x The data to write
		/// \param arrayLength The number of elements in the array
		virtual void write(const IndexedIO::EntryID &name, const double *x, unsigned long arrayLength) = 0;

		/// Create a new file containing the specified half array contents
		/// \param name The name of the file to be written
		/// \param x The data to write
		/// \param arrayLength The number of elements in the array
		virtual void write(const IndexedIO::EntryID &name, const half *x, unsigned long arrayLength) = 0;

		/// Create a new file containing the specified unsigned int array contents
		/// \param name The name of the file to be written
		/// \param x The data to write
		/// \param arrayLength The number of elements in the array
		virtual void write(const IndexedIO::EntryID &name, const int *x, unsigned long arrayLength) = 0;

		/// Create a new file containing the specified unsigned long array contents
		/// \param name The name of the file to be written
		/// \param x The data to write
		/// \param arrayLength The number of elements in the array
		virtual void write(const IndexedIO::EntryID &name, const int64_t *x, unsigned long arrayLength) = 0;

		/// Create a new file containing the specified unsigned long array contents
		/// \param name The name of the file to be written
		/// \param x The data to write
		/// \param arrayLength The number of elements in the array
		virtual void write(const IndexedIO::EntryID &name, const uint64_t *x, unsigned long arrayLength) = 0;

		/// Create a new file containing the specified unsigned int array contents
		/// \param name The name of the file to be written
		/// \param x The data to write
		/// \param arrayLength The number of elements in the array
		virtual void write(const IndexedIO::EntryID &name, const unsigned int *x, unsigned long arrayLength) = 0;

		/// Create a new file containing the specified char array contents
		/// \param name The name of the file to be written
		/// \param x The data to write
		/// \param arrayLength The number of elements in the array
		virtual void write(const IndexedIO::EntryID &name, const char *x, unsigned long arrayLength) = 0;

		/// Create a new file containing the specified unsigned char array contents
		/// \param name The name of the file to be written
		/// \param x The data to write
		/// \param arrayLength The number of elements in the array
		virtual void write(const IndexedIO::EntryID &name, const unsigned char *x, unsigned long arrayLength) = 0;

		/// Create a new file containing the specified short array contents
		/// \param name The name of the file to be written
		/// \param x The data to write
		/// \param arrayLength The number of elements in the array
		virtual void write(const IndexedIO::EntryID &name, const short *x, unsigned long arrayLength) = 0;

		/// Create a new file containing the specified unsigned short array contents
		/// \param name The name of the file to be written
		/// \param x The data to write
		/// \param arrayLength The number of elements in the array
		virtual void write(const IndexedIO::EntryID &name, const unsigned short *x, unsigned long arrayLength) = 0;

		/// Create a new file containing the specified string array contents
		/// \param name The name of the file to be written
		/// \param x The data to write
		/// \param arrayLength The number of elements in the array
		virtual void write(const IndexedIO::EntryID &name, const std::string *x, unsigned long arrayLength) = 0;

		/// Create a new file containing the specified InternedString array contents
		/// \param name The name of the file to be written
		/// \param x The data to write
		/// \param arrayLength The number of elements in the array
		virtual void write(const IndexedIO::EntryID &name, const InternedString *x, unsigned long arrayLength) = 0;

		/// Create a new file containing the specified float
		/// \param name The name of the file to be written
		/// \param x The data to write
		virtual void write(const IndexedIO::EntryID &name, const float &x) = 0;

		/// Create a new file containing the specified double
		/// \param name The name of the file to be written
		/// \param x The data to write
		virtual void write(const IndexedIO::EntryID &name, const double &x) = 0;

		/// Create a new file containing the specified half
		/// \param name The name of the file to be written
		/// \param x The data to write
		virtual void write(const IndexedIO::EntryID &name, const half &x) = 0;

		/// Create a new file containing the specified int
		/// \param name The name of the file to be written
		/// \param x The data to write
		virtual void write(const IndexedIO::EntryID &name, const int &x) = 0;

		/// Create a new file containing the specified long
		/// \param name The name of the file to be written
		/// \param x The data to write
		virtual void write(const IndexedIO::EntryID &name, const int64_t &x) = 0;

		/// Create a new file containing the specified long
		/// \param name The name of the file to be written
		/// \param x The data to write
		virtual void write(const IndexedIO::EntryID &name, const uint64_t &x) = 0;

		/// Create a new file containing the specified string
		/// \param name The name of the file to be written
		/// \param x The data to write
		virtual void write(const IndexedIO::EntryID &name, const std::string &x) = 0;

		/// Create a new file containing the specified unsigned int
		/// \param name The name of the file to be written
		/// \param x The data to write
		virtual void write(const IndexedIO::EntryID &name, const unsigned int &x) = 0;

		/// Create a new file containing the specified char
		/// \param name The name of the file to be written
		/// \param x The data to write
		virtual void write(const IndexedIO::EntryID &name, const char &x) = 0;

		/// Create a new file containing the specified unsigned char
		/// \param name The name of the file to be written
		/// \param x The data to write
		virtual void write(const IndexedIO::EntryID &name, const unsigned char &x) = 0;

		/// Create a new file containing the specified short
		/// \param name The name of the file to be written
		/// \param x The data to write
		virtual void write(const IndexedIO::EntryID &name, const short &x) = 0;

		/// Create a new file containing the specified unsigned short
		/// \param name The name of the file to be written
		/// \param x The data to write
		virtual void write(const IndexedIO::EntryID &name, const unsigned short &x) = 0;

		/// Read a float array from an existing file.
		/// \param name The name of the file to be read
		/// \param x The buffer to fill. If 0 is passed, then memory is allocated and should be freed by the caller.
		/// \param arrayLength The number of elements in the array
		virtual void read(const IndexedIO::EntryID &name, float *&x, unsigned long arrayLength) const = 0;

		/// Read a double array from an existing file.
		/// \param name The name of the file to be read
		/// \param x The buffer to fill. If 0 is passed, then memory is allocated and should be freed by the caller.
		/// \param arrayLength The number of elements in the array
		virtual void read(const IndexedIO::EntryID &name, double *&x, unsigned long arrayLength) const  = 0;

		/// Read a half array from an existing file.
		/// \param name The name of the file to be read
		/// \param x The buffer to fill. If 0 is passed, then memory is allocated and should be freed by the caller.
		/// \param arrayLength The number of elements in the array
		virtual void read(const IndexedIO::EntryID &name, half *&x, unsigned long arrayLength) const  = 0;

		/// Read an int array from an existing file.
		/// \param name The name of the file to be read
		/// \param x The buffer to fill. If 0 is passed, then memory is allocated and should be freed by the caller.
		/// \param arrayLength The number of elements in the array
		virtual void read(const IndexedIO::EntryID &name, int *&x, unsigned long arrayLength) const  = 0;

		/// Read a long array from an existing file.
		/// \param name The name of the file to be read
		/// \param x The buffer to fill. If 0 is passed, then memory is allocated and should be freed by the caller.
		/// \param arrayLength The number of elements in the array
		virtual void read(const IndexedIO::EntryID &name, int64_t *&x, unsigned long arrayLength) const  = 0;

		/// Read a long array from an existing file.
		/// \param name The name of the file to be read
		/// \param x The buffer to fill. If 0 is passed, then memory is allocated and should be freed by the caller.
		/// \param arrayLength The number of elements in the array
		virtual void read(const IndexedIO::EntryID &name, uint64_t *&x, unsigned long arrayLength) const  = 0;

		/// Read an unsigned int array from an existing file.
		/// \param name The name of the file to be read
		/// \param x The buffer to fill. If 0 is passed, then memory is allocated and should be freed by the caller.
		/// \param arrayLength The number of elements in the array
		virtual void read(const IndexedIO::EntryID &name, unsigned int *&x, unsigned long arrayLength) const  = 0;

		/// Read a char array from an existing file.
		/// \param name The name of the file to be read
		/// \param x The buffer to fill. If 0 is passed, then memory is allocated and should be freed by the caller.
		/// \param arrayLength The number of elements in the array
		virtual void read(const IndexedIO::EntryID &name, char *&x, unsigned long arrayLength) const  = 0;

		/// Read an unsigned char array from an existing file.
		/// \param name The name of the file to be read
		/// \param x The buffer to fill. If 0 is passed, then memory is allocated and should be freed by the caller.
		/// \param arrayLength The number of elements in the array
		virtual void read(const IndexedIO::EntryID &name, unsigned char *&x, unsigned long arrayLength) const  = 0;

		/// Read a short array from an existing file.
		/// \param name The name of the file to be read
		/// \param x The buffer to fill. If 0 is passed, then memory is allocated and should be freed by the caller.
		/// \param arrayLength The number of elements in the array
		virtual void read(const IndexedIO::EntryID &name, short *&x, unsigned long arrayLength) const  = 0;

		/// Read an unsigned short array from an existing file.
		/// \param name The name of the file to be read
		/// \param x The buffer to fill. If 0 is passed, then memory is allocated and should be freed by the caller.
		/// \param arrayLength The number of elements in the array
		virtual void read(const IndexedIO::EntryID &name, unsigned short *&x, unsigned long arrayLength) const  = 0;

		/// Read a string array from an existing file.
		/// \param name The name of the file to be read
		/// \param x The buffer to fill. If 0 is passed, then memory is allocated and should be freed by the caller.
		/// \param arrayLength The number of elements in the array
		virtual void read(const IndexedIO::EntryID &name, std::string *&x, unsigned long arrayLength) const  = 0;

		/// Read an InternedString array from an existing file
		/// \param name The name of the file to be read
		/// \param x The buffer to fill. If 0 is passed, then memory is allocated and should be freed by the caller.
		/// \param arrayLength The number of elements in the array
		virtual void read(const IndexedIO::EntryID &name, InternedString *&x, unsigned long arrayLength) const  = 0;

		/// Read a float from an existing file.
		/// \param name The name of the file to be read
		/// \param x Returns the data read.
		virtual void read(const IndexedIO::EntryID &name, float &x) const  = 0;

		/// Read a double from an existing file
		/// \param name The name of the file to be read
		/// \param x Returns the data read.
		virtual void read(const IndexedIO::EntryID &name, double &x) const  = 0;

		/// Read a half from an existing file
		/// \param name The name of the file to be read
		/// \param x Returns the data read.
		virtual void read(const IndexedIO::EntryID &name, half &x) const  = 0;

		/// Read an int from an existing file
		/// \param name The name of the file to be read
		/// \param x Returns the data read.
		virtual void read(const IndexedIO::EntryID &name, int &x) const  = 0;

		/// Read a long from an existing file
		/// \param name The name of the file to be read
		/// \param x Returns the data read.
		virtual void read(const IndexedIO::EntryID &name, int64_t &x) const  = 0;

		/// Read a long from an existing file
		/// \param name The name of the file to be read
		/// \param x Returns the data read.
		virtual void read(const IndexedIO::EntryID &name, uint64_t &x) const  = 0;

		/// Read a string from an existing file
		/// \param name The name of the file to be read
		/// \param x Returns the data read.
		virtual void read(const IndexedIO::EntryID &name, std::string &x) const  = 0;

		/// Read an unsigned int from an existing file
		/// \param name The name of the file to be read
		/// \param x Returns the data read.
		virtual void read(const IndexedIO::EntryID &name, unsigned int &x) const  = 0;

		/// Read a char from an existing file
		/// \param name The name of the file to be read
		/// \param x Returns the data read.
		virtual void read(const IndexedIO::EntryID &name, char &x) const  = 0;

		/// Read an unsigned char from an existing file
		/// \param name The name of the file to be read
		/// \param x Returns the data read.
		virtual void read(const IndexedIO::EntryID &name, unsigned char &x) const  = 0;

		/// Read a short from an existing file
		/// \param name The name of the file to be read
		/// \param x Returns the data read.
		virtual void read(const IndexedIO::EntryID &name, short &x) const  = 0;

		/// Read an unsigned char from an existing file
		/// \param name The name of the file to be read
		/// \param x Returns the data read.
		virtual void read(const IndexedIO::EntryID &name, unsigned short &x) const  = 0;

		/// A representation of a single file/directory
		class IECORE_API Entry
		{
			public:
				Entry();

				Entry( const EntryID &id, EntryType eType, DataType dType, unsigned long arrayLength);

				/// ID, or name, of the file/directory
				const EntryID &id() const;

				/// Returns either Directory or File.
				EntryType entryType() const;

				/// Should only be called on instances which represent files. Returns the type of data held by in the file. If this entry does not represent a file
				/// an IOException is thrown.
				DataType dataType() const;

				/// Convenience method to return if entry respresents an array. If Entry's datatype is not an array then an IOException is thrown.
				bool isArray() const;

				/// Convenience method to return size of array. If Entry's datatype is not an array then an IOException is thrown.
				unsigned long arrayLength() const;

				/// Convenience method to return if a data is an array or not
				static bool isArray( DataType dType );

			protected:

				EntryID m_ID;
				EntryType m_entryType;
				DataType m_dataType;
				unsigned long m_arrayLength;
		};

		// Method for establishing flattened size of a data object
		template<typename T>
		struct DataSizeTraits;

		// Method for flatting/unflattening data objects
		template<typename T>
		struct DataFlattenTraits;

		template<typename T>
		struct DataTypeTraits;

	protected:

		// Throw an exception if the entry is not readable
		virtual void readable(const IndexedIO::EntryID &name) const;

		// Throw an exception if the entry is not writable
		virtual void writable(const IndexedIO::EntryID &name) const;

		static void validateOpenMode(IndexedIO::OpenMode &mode);

	private:
		/// Register a new subclass that can handle the given extension
		static void registerCreator( const std::string &extension, CreatorFn f );

		typedef std::map<std::string, CreatorFn> CreatorMap;
		static CreatorMap &getCreateFns() { static CreatorMap *g_createFns = new CreatorMap(); return *g_createFns; }

};

} // namespace IECore

#include "IndexedIO.inl"

#endif // IE_CORE_INDEXEDIO_H
