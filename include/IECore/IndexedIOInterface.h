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

#ifndef IE_CORE_INDEXEDIOINTERFACE_H
#define IE_CORE_INDEXEDIOINTERFACE_H

#include <string>
#include <map>

#include <boost/filesystem/path.hpp>
#include <boost/noncopyable.hpp>

#include <OpenEXR/half.h>

#include "IECore/RefCounted.h"
#include "IECore/IndexedIO.h"
#include "IECore/IndexedIOFilter.h"
#include "IECore/IndexedIOPath.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( IndexedIOInterface );
	
/// Abstract interface to define operations on a random-access indexed input/output device. All methods throw an instance of IOException,
/// or one of its subclasses, if an error is encountered. 
class IndexedIOInterface : public RefCounted, private boost::noncopyable
{
	public:	
		typedef IndexedIOInterfacePtr (*CreatorFn)(const std::string &, const IndexedIO::EntryID &, IndexedIO::OpenMode );
	
		/// Create an instance of a subclass which is able to open the IndexedIO structure found at "path".
		/// Files can be opened for Read, Overwrite, or Append. 
		/// During "Read" operations it is not permitted to make any modifications to the underlying files. 
		/// When opening a device in "Write" mode its contents below the root directory are removed. 
		/// For "Append" operations it is possible to write new files, or overwrite existing ones. It is not possible to overwrite entire directories, however.
		/// \param path A file or directory on disk. The appropriate reader for reading/writing is determined by the path's extension.
		/// \param root The root point to 'mount' the structure. Paths above the root in the hierarchy are inaccessible.
		/// \param mode A bitwise-ORed combination of constants which determine how the file system should be accessed.
		static IndexedIOInterfacePtr create( const std::string &path, const IndexedIO::EntryID &root, IndexedIO::OpenMode mode);
		
		/// Fills the passed vector with all the extensions for which an IndexedIOInterface implementation is
		/// available. Extensions are of the form "fio" - ie without a preceding '.'.
		static void supportedExtensions( std::vector<std::string> &extensions );
		
		/// Static instantation of one of these (with a supported file extension) using a subclass as the template parameter  will register it
		/// as a supported IndexedIOInterface. This allows read and write operations to be performed generically, with the correct interface to 
		/// use being automatically determined by the system.
		template<class T>
		struct Description
		{
			Description(const std::string &extension) { IndexedIOInterface::registerCreator( extension, &T::create ); }
		};
							
		virtual ~IndexedIOInterface();
		
		/// Returns the mode with which the interface was created.
		virtual IndexedIO::OpenMode openMode() const = 0;
		
		/// Returns a new interface with the root set to the current directory.
		virtual IndexedIOInterfacePtr resetRoot() const = 0;
		
		/// Relocate to a different directory within the current device.  Attempting to navigate above the current root directory will throw an exception.
		/// \param name The directory to relocate to. Can be an absolute or relative path. Special directory names such as ".", "..", and "/" are supported.
		virtual void chdir(const IndexedIO::EntryID &name) = 0;
		
		/// Create a new directory. Automatically creates parent directories if needed.
		/// \param name The directory to create. 
		virtual void mkdir(const IndexedIO::EntryID &name) = 0;
	
		/// Retrieve the current directorys, relative to the root.
		virtual IndexedIO::EntryID pwd() = 0;
	
		/// Find file and/or directory names contained within the current index. Short names to the files/directories are always returned.
		/// \param f A subclass of IndexedIOFilter to allow removal of entries which are of no conern.
		virtual IndexedIO::EntryList ls(IndexedIOFilterPtr f=0) = 0;
	
		/// Return details of a specific entry.
		virtual IndexedIO::Entry ls(const IndexedIO::EntryID &name) = 0;
	
		/// Remove a specified file or directory. If an attempt is made to delete the current directory, the operation
		/// is permitted but the current directory is then undefined and must be reset using chdir with an absolute path.
		virtual unsigned long rm(const IndexedIO::EntryID &name) = 0;
					
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
		virtual void write(const IndexedIO::EntryID &name, const long *x, unsigned long arrayLength) = 0;			
		
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
		virtual void write(const IndexedIO::EntryID &name, const long &x) = 0;
		
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
		virtual void read(const IndexedIO::EntryID &name, float *&x, unsigned long arrayLength) = 0;
		
		/// Read a double array from an existing file.
		/// \param name The name of the file to be read
		/// \param x The buffer to fill. If 0 is passed, then memory is allocated and should be freed by the caller.
		/// \param arrayLength The number of elements in the array
		virtual void read(const IndexedIO::EntryID &name, double *&x, unsigned long arrayLength) = 0;
		
		/// Read a half array from an existing file.
		/// \param name The name of the file to be read
		/// \param x The buffer to fill. If 0 is passed, then memory is allocated and should be freed by the caller.
		/// \param arrayLength The number of elements in the array
		virtual void read(const IndexedIO::EntryID &name, half *&x, unsigned long arrayLength) = 0;
		
		/// Read an int array from an existing file.
		/// \param name The name of the file to be read
		/// \param x The buffer to fill. If 0 is passed, then memory is allocated and should be freed by the caller.
		/// \param arrayLength The number of elements in the array
		virtual void read(const IndexedIO::EntryID &name, int *&x, unsigned long arrayLength) = 0;
		
		/// Read a long array from an existing file.
		/// \param name The name of the file to be read
		/// \param x The buffer to fill. If 0 is passed, then memory is allocated and should be freed by the caller.
		/// \param arrayLength The number of elements in the array
		virtual void read(const IndexedIO::EntryID &name, long *&x, unsigned long arrayLength) = 0;
		
		/// Read an unsigned int array from an existing file.
		/// \param name The name of the file to be read
		/// \param x The buffer to fill. If 0 is passed, then memory is allocated and should be freed by the caller.
		/// \param arrayLength The number of elements in the array
		virtual void read(const IndexedIO::EntryID &name, unsigned int *&x, unsigned long arrayLength) = 0;
		
		/// Read a char array from an existing file.
		/// \param name The name of the file to be read
		/// \param x The buffer to fill. If 0 is passed, then memory is allocated and should be freed by the caller.
		/// \param arrayLength The number of elements in the array
		virtual void read(const IndexedIO::EntryID &name, char *&x, unsigned long arrayLength) = 0;
		
		/// Read an unsigned char array from an existing file.
		/// \param name The name of the file to be read
		/// \param x The buffer to fill. If 0 is passed, then memory is allocated and should be freed by the caller.
		/// \param arrayLength The number of elements in the array
		virtual void read(const IndexedIO::EntryID &name, unsigned char *&x, unsigned long arrayLength) = 0;
		
		/// Read a short array from an existing file.
		/// \param name The name of the file to be read
		/// \param x The buffer to fill. If 0 is passed, then memory is allocated and should be freed by the caller.
		/// \param arrayLength The number of elements in the array
		virtual void read(const IndexedIO::EntryID &name, short *&x, unsigned long arrayLength) = 0;
		
		/// Read an unsigned short array from an existing file.
		/// \param name The name of the file to be read
		/// \param x The buffer to fill. If 0 is passed, then memory is allocated and should be freed by the caller.
		/// \param arrayLength The number of elements in the array
		virtual void read(const IndexedIO::EntryID &name, unsigned short *&x, unsigned long arrayLength) = 0;		
		
		/// Read a string array from an existing file.
		/// \param name The name of the file to be read
		/// \param x The buffer to fill. If 0 is passed, then memory is allocated and should be freed by the caller.
		/// \param arrayLength The number of elements in the array
		virtual void read(const IndexedIO::EntryID &name, std::string *&x, unsigned long arrayLength) = 0;
		
		/// Read a float from an existing file.
		/// \param name The name of the file to be read
		/// \param x Returns the data read.
		virtual void read(const IndexedIO::EntryID &name, float &x) = 0;
		
		/// Read a double from an existing file
		/// \param name The name of the file to be read
		/// \param x Returns the data read.
		virtual void read(const IndexedIO::EntryID &name, double &x) = 0;
		
		/// Read a half from an existing file
		/// \param name The name of the file to be read
		/// \param x Returns the data read.
		virtual void read(const IndexedIO::EntryID &name, half &x) = 0;
		
		/// Read an int from an existing file
		/// \param name The name of the file to be read
		/// \param x Returns the data read.
		virtual void read(const IndexedIO::EntryID &name, int &x) = 0;
		
		/// Read a long from an existing file
		/// \param name The name of the file to be read
		/// \param x Returns the data read.
		virtual void read(const IndexedIO::EntryID &name, long &x) = 0;
		
		/// Read a string from an existing file
		/// \param name The name of the file to be read
		/// \param x Returns the data read.
		virtual void read(const IndexedIO::EntryID &name, std::string &x) = 0;
		
		/// Read an unsigned int from an existing file
		/// \param name The name of the file to be read
		/// \param x Returns the data read.
		virtual void read(const IndexedIO::EntryID &name, unsigned int &x) = 0;
		
		/// Read a char from an existing file
		/// \param name The name of the file to be read
		/// \param x Returns the data read.
		virtual void read(const IndexedIO::EntryID &name, char &x) = 0;
		
		/// Read an unsigned char from an existing file
		/// \param name The name of the file to be read
		/// \param x Returns the data read.
		virtual void read(const IndexedIO::EntryID &name, unsigned char &x) = 0;
		
		/// Read a short from an existing file
		/// \param name The name of the file to be read
		/// \param x Returns the data read.
		virtual void read(const IndexedIO::EntryID &name, short &x) = 0;
		
		/// Read an unsigned char from an existing file
		/// \param name The name of the file to be read
		/// \param x Returns the data read.
		virtual void read(const IndexedIO::EntryID &name, unsigned short &x) = 0;
		
	protected:
								
		// Throw an exception if the entry is not readable
		virtual void readable(const IndexedIO::EntryID &name) const;
		
		// Throw an exception if the entry is not writable
		virtual void writable(const IndexedIO::EntryID &name) const;
		
		virtual void validateOpenMode(IndexedIO::OpenMode &mode);
		
	private:
		/// Register a new subclass that can handle the given extension
		static void registerCreator( const std::string &extension, CreatorFn f );
	
		typedef std::map<std::string, CreatorFn> CreatorMap;
		static CreatorMap &getCreateFns() { static CreatorMap *g_createFns = new CreatorMap(); return *g_createFns; }
	
};

} // namespace IECore

#endif // IE_CORE_INDEXEDIOINTERFACE_H
