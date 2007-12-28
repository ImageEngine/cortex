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

#include <iostream>
#include <cassert>
#include <sstream>
#include <map>

#include <boost/format.hpp>
#include <boost/regex.hpp> 
#include <boost/filesystem/operations.hpp> 

#include "sqlite/sqlite3.h"

#include "IECore/SQLiteIndexedIO.h"

using namespace IECore;
namespace fs = boost::filesystem;


static IndexedIOInterface::Description<SQLiteIndexedIO> registrar(".sql");

// The name we give to the internal database table
const char* SQLiteIndexedIO::g_tableName("SQLiteIndexedIO");

/// A reference-counted SQL statement
class SQLiteIndexedIO::SQLStatement : public RefCounted
{			
	public:
		SQLStatement(sqlite3 *handle, const std::string &sql);				
		virtual ~SQLStatement();
		inline sqlite3_stmt *get() const;
	
		// Execute the statement so we can then fetch the result
		inline int		step() const;
		
		inline void		reset() const;
	
		// Obtain the integer held in the i'th column from the current result
		inline int		getInt(int i) const;
	
		// Obtain the string held in the i'th column from the current result
		inline std::string	getString(int i) const;
	
		// Obtain the blob held in the i'th column from the current result
		inline const void *	getBlob(int i) const;
	
		// Obtain the number of columns from the current result
		inline int		columnCount() const;
	

	protected:
				
		sqlite3_stmt *m_statement;

};	

/// A reference-counted handle to a database connection
class SQLiteIndexedIO::SQLiteDBHandle : public RefCounted
{
	sqlite3 *m_dbHandle;
	
	typedef std::map< std::pair< sqlite3*, std::string> , SQLStatementPtr > StatementMap;
	
	static StatementMap g_statementMap;

	public:
		SQLiteDBHandle();

		SQLiteDBHandle( const std::string &filename );

		virtual ~SQLiteDBHandle();

		inline sqlite3* get();
		
		SQLStatementPtr prepareStatement( const std::string &sql );
						
		static void flushStatements()
		{
			g_statementMap.clear();			
		}
};

class SQLiteIndexedIO::SQLiteDirectorySet : public std::set<std::string>, public RefCounted
{
};


SQLiteIndexedIO::SQLiteDBHandle::StatementMap SQLiteIndexedIO::SQLiteDBHandle::g_statementMap;

SQLiteIndexedIO::SQLiteDBHandle::SQLiteDBHandle()
{	
	m_dbHandle = 0;	
}

SQLiteIndexedIO::SQLiteDBHandle::SQLiteDBHandle( const std::string &filename )
{
	
	int rc = sqlite3_open(filename.c_str(), &m_dbHandle);

	if (rc != SQLITE_OK)
	{
		throw IOException(filename);
	}
	
	assert(m_dbHandle);
	
	for (StatementMap::iterator it = g_statementMap.begin(); it != g_statementMap.end(); ++it)
	{
		assert (it->first.first != m_dbHandle);
	}
}

SQLiteIndexedIO::SQLiteDBHandle::~SQLiteDBHandle()
{
	assert(m_dbHandle);
	
	/// Remove any cached statements for this instance's database
	for (StatementMap::iterator it = g_statementMap.begin(); it != g_statementMap.end(); ++it)
	{
		if (it->first.first == m_dbHandle)
		{
			g_statementMap.erase(it);
		}
	}	
	
	int rc = sqlite3_close(m_dbHandle);

	(void) rc;
}

sqlite3* SQLiteIndexedIO::SQLiteDBHandle::get()
{
	assert(m_dbHandle);
	
	return m_dbHandle;
}

SQLiteIndexedIO::SQLStatementPtr SQLiteIndexedIO::SQLiteDBHandle::prepareStatement( const std::string &sql )
{
	StatementMap::key_type key( m_dbHandle, sql );
	
	StatementMap::const_iterator it = g_statementMap.find(key);
	
	if (it != g_statementMap.end())
	{
		it->second->reset();
				
		return it->second;
	}
	else
	{
		SQLStatementPtr stmt = new SQLStatement( m_dbHandle, sql );
		g_statementMap.insert( StatementMap::value_type(key, stmt ) );
		
		return stmt;
	}
}

SQLiteIndexedIO::SQLStatement::SQLStatement(sqlite3 *handle, const std::string &sql)
{
	assert(handle);		

	int rc = sqlite3_prepare( handle, sql.c_str(), sql.length(), &m_statement, 0);
	if (rc != SQLITE_OK)
	{
		throw IOException(sql);
	}
	
	assert( rc == SQLITE_OK );	
	assert(m_statement);	
}
		
SQLiteIndexedIO::SQLStatement::~SQLStatement()
{
	assert(m_statement);

	// Free the associated memory
	int rc = sqlite3_finalize( m_statement );
	assert(rc == SQLITE_OK);
	(void) rc;
}
	
sqlite3_stmt *SQLiteIndexedIO::SQLStatement::get() const
{
	assert(m_statement);
	
	return m_statement;
}

void SQLiteIndexedIO::SQLStatement::reset() const
{
	assert(m_statement);
	
	sqlite3_reset(m_statement);
}

int SQLiteIndexedIO::SQLStatement::step() const
{
	assert(m_statement);
	
	return sqlite3_step(m_statement);
}

int SQLiteIndexedIO::SQLStatement::getInt(int i) const
{
	assert( i >= 0 );
	assert( i < columnCount());
	
	return sqlite3_column_int(m_statement, i);
}

std::string SQLiteIndexedIO::SQLStatement::getString(int i) const
{
	assert( i >= 0 );
	assert( i < columnCount());
	
	const char *str = (const char *)(sqlite3_column_text(m_statement, i));
	assert(str);
	
	return str;
}

const void *SQLiteIndexedIO::SQLStatement::getBlob(int i) const
{
	assert( i >= 0 );
	assert( i < columnCount());
	
	return sqlite3_column_blob(m_statement, i);
}

int SQLiteIndexedIO::SQLStatement::columnCount() const
{
	return sqlite3_column_count(m_statement);
}
	
// Our own custom sqlite function which allows queries using the REGEXP operator. 
void SQLiteIndexedIO::regexp(sqlite3_context* ctx,int argc,sqlite3_value** argv)
{
	int ret=0;
	
	const char *regexp=(const char*)(sqlite3_value_text(argv[0]));
	const char *str=(const char*)(sqlite3_value_text(argv[1]));
	
	if (regexp && str)
	{
		boost::regex exp;
		
		// We try to avoid unnecessary regexp compilation by storing them in a map
		SQLiteIndexedIOPtr io = static_cast<SQLiteIndexedIO *>(sqlite3_user_data(ctx));
		SQLiteIndexedIO::RegExpMap &regExpMap = io->m_regExps;
		SQLiteIndexedIO::RegExpMap::const_iterator it = regExpMap.find( regexp );
		
		if (it == regExpMap.end())
		{
			exp = boost::regex(regexp);
			regExpMap.insert( SQLiteIndexedIO::RegExpMap::value_type( regexp, exp ) );
		}
		else
		{
			exp = it->second;
		}
		
		boost::cmatch what; 
		if (regex_match(str, what, exp)) 
			ret = 1;
	}
	
	// Pass the result of the regexp match back to SQLite
	sqlite3_result_int(ctx, ret);
}

IndexedIOInterfacePtr SQLiteIndexedIO::create(const std::string &path, const IndexedIO::EntryID &root, IndexedIO::OpenMode mode)
{
	return new SQLiteIndexedIO(path, root, mode);
}

SQLiteIndexedIO::SQLiteIndexedIO(const SQLiteIndexedIO &other, const IndexedIO::EntryID &root, IndexedIO::OpenMode mode  )
{
	m_mode = mode;
	m_filename = other.m_filename;
	m_dbHandle = other.m_dbHandle;
	
	m_validDirs = other.m_validDirs;
	
	m_currentDirectory = IndexedIOPath(root);
	chdir("/");
}

IndexedIOInterfacePtr SQLiteIndexedIO::resetRoot() const
{		
	IndexedIO::OpenMode mode = m_mode;
	
	if (mode & IndexedIO::Write)
	{
		mode &= ~IndexedIO::Write;
		assert( (mode & IndexedIO::Shared) == (m_mode & IndexedIO::Shared) );
		assert( (mode & IndexedIO::Exclusive) == (m_mode & IndexedIO::Exclusive) );
		assert( !(mode & IndexedIO::Write) );		
		mode |= IndexedIO::Append;
	}
		
	return new SQLiteIndexedIO(*this, m_currentDirectory.fullPath(), mode);
}

SQLiteIndexedIO::SQLiteIndexedIO(const std::string &path, const IndexedIO::EntryID &root, IndexedIO::OpenMode mode)
: m_dbHandle(0)
{
	m_validDirs = new SQLiteDirectorySet();
	SQLiteDBHandle::flushStatements();
	
	validateOpenMode(mode);
	const fs::path p = fs::path(path);
	m_filename = p.native_file_string();
	m_currentDirectory = IndexedIOPath(root);

	if (mode & IndexedIO::Read)
	{
		if (! fs::exists(m_filename) )
		{
			throw FileNotFoundIOException(m_filename);
		}
	}
	
	
	m_dbHandle = new SQLiteDBHandle( m_filename );
	
	// Register our own "regexp" function to allow use of REGEXP keyword from SQL
	int rc = sqlite3_create_function(
		m_dbHandle->get(),
		"regexp",
		2,
		SQLITE_ANY,
		this,
		SQLiteIndexedIO::regexp,
		0,
		0);
	
	assert(rc == SQLITE_OK);
	
	// Setup a busy-wait timeout duration when dealing with concurrent access.
	rc = sqlite3_busy_timeout(m_dbHandle->get(), 2000);
	
	SQLStatementPtr stmt = 0;
	
	// Performance optimisations
	
	sqlite3_enable_shared_cache(1);
	
	stmt = m_dbHandle->prepareStatement( "PRAGMA cache_size = 100000000;" );
	rc = stmt->step();
	assert(rc == SQLITE_DONE);
	
	stmt = m_dbHandle->prepareStatement( "PRAGMA temp_store = MEMORY;" );
	rc = stmt->step();
	assert(rc == SQLITE_DONE);
		
	stmt = m_dbHandle->prepareStatement( "PRAGMA count_changes = 0;" );
	rc = stmt->step();
	assert(rc == SQLITE_DONE);
	
	stmt = m_dbHandle->prepareStatement( "PRAGMA synchronous = OFF;" );
	rc = stmt->step();
	assert(rc == SQLITE_DONE);
	
	// Table initilisation

	static const boost::format queryTableFormat("SELECT name FROM sqlite_master WHERE type='table' AND name='%1%';");
	
	std::string sql = (boost::format(queryTableFormat) % g_tableName).str();
	
	stmt = m_dbHandle->prepareStatement( sql );	
	
	rc = stmt->step();
	
	if (rc == SQLITE_DONE)
	{
		static const boost::format createTableFormat("CREATE TABLE %1% ( ID VARCHAR PRIMARY KEY, size INTEGER, arrayLength INTEGER, entryType INTEGER, dataType INTEGER, data BLOB);");
		sql = (boost::format(createTableFormat) % g_tableName).str();
		
		rc = sqlite3_exec( m_dbHandle->get(), sql.c_str(), 0, 0, 0);		
		assert( rc == SQLITE_OK );				
	}

	if (mode & IndexedIO::Read)
	{
		if(!exists(m_currentDirectory.fullPath(), IndexedIO::Directory))
		{
			throw IOException(m_filename);
		}
	}
	else
	{	
		if (mode & IndexedIO::Write)
		{
			if (exists(m_currentDirectory.fullPath(), IndexedIO::Directory))
			{		
				rm("/");
			} 
		}
		
		mkdir("/");
	} 	
	chdir("/");		
}

SQLiteIndexedIO::~SQLiteIndexedIO()
{
}

IndexedIO::EntryID SQLiteIndexedIO::pwd()
{
	readable(".");
	
	return m_currentDirectory.relativePath();
}

void SQLiteIndexedIO::mkdir(const IndexedIO::EntryID &name)
{		
	assert(m_dbHandle->get());
		
	writable(name);
	
	const IndexedIOPath d = m_currentDirectory.appended(name);

	static const boost::format fmt("INSERT INTO %1% VALUES('%2%', %3%, 0, 0, 0, NULL);");
	std::string sql = (boost::format(fmt) % g_tableName % d.fullPath() % int(IndexedIO::Directory)).str();
	
	int rc = sqlite3_exec( m_dbHandle->get(), sql.c_str(), 0, 0, 0);
	if (rc == SQLITE_CONSTRAINT)
	{
		// Already exists. 
		// \todo Do we really want to throw an exception here?
	} 
	else
	{
		if (!(rc == SQLITE_DONE || rc == SQLITE_OK))
		{
			throw IOException(name);
		}
	}
	
	m_validDirs->insert( d.fullPath() );
}

void SQLiteIndexedIO::chdir(const IndexedIO::EntryID &name)
{	
	assert(m_dbHandle->get());

	readable(name);
	IndexedIOPath d = m_currentDirectory.appended(name);
	
	if ( m_validDirs->find( d.fullPath() ) != m_validDirs->end() )
	{
		m_currentDirectory = d;		
	}
	else
	{	
		if (!exists(d.fullPath(), IndexedIO::Directory))
		{		
			throw IOException(name);
		}
		else
		{	
			m_currentDirectory = d;
		}
	}
}

IndexedIO::EntryList SQLiteIndexedIO::ls(IndexedIOFilterPtr f)
{	
	assert(m_dbHandle->get());
	
	readable(".");
	
	IndexedIO::EntryList result;
	
	std::string prefix = m_currentDirectory.fullPath();
	
	// Ensure path contains trailing slash
	if (*(prefix.rbegin()) != '/')
		prefix += "/";
	
	std::string match = "^" + prefix + "[^/]+";

	static const boost::format fmt("SELECT ID, arrayLength, entryType, dataType FROM %1% WHERE REGEXP('%2%', ID);");
	std::string sql = (boost::format(fmt) % g_tableName % match).str();
	
	SQLStatementPtr stmt = m_dbHandle->prepareStatement( sql );	
	
	int rc = stmt->step();

	while (rc == SQLITE_ROW)
	{
		// Extract results
		std::string name = stmt->getString(0);
				
		unsigned long arrayLength = stmt->getInt(1);
		
		int entryType = stmt->getInt(2);
		assert((IndexedIO::EntryType)entryType == IndexedIO::Directory || (IndexedIO::EntryType)entryType == IndexedIO::File);
		
		int dataType = stmt->getInt(3);
			
		// Our regex should ensure this
		assert(name.size() > prefix.size());
			
		// Trim the prefix from result name
		name.assign(name, prefix.size(), prefix.size()-name.size()-1);
		result.push_back( IndexedIO::Entry( name, (IndexedIO::EntryType)entryType, (IndexedIO::DataType)dataType, arrayLength ) );
		
		rc = stmt->step();		
	}
	
	if (f)
	{
		f->apply(result);
	}

	return result;
}

unsigned long SQLiteIndexedIO::rm(const IndexedIO::EntryID &name)
{
	return rm(name, true);
}

unsigned long SQLiteIndexedIO::rm(const IndexedIO::EntryID &name, bool throwIfNonExistent)
{
	assert(m_dbHandle->get());
	
	writable(name);
	
	if (!exists(name))
	{
		if (throwIfNonExistent)
		{
			throw FileNotFoundIOException(name );
		}
		else
		{
			return 0;
		}
	}
	
	m_validDirs->clear();
	
	IndexedIOPath d = m_currentDirectory.appended(name);
		
	std::string prefix = d.fullPath();
	const std::string &shortName = d.fullPath();
	
	// Ensure prefix contains trailing slash
	if (*(prefix.rbegin()) != '/')
		prefix += "/";
	
	static const boost::format fmt("DELETE FROM %1% WHERE ID='%2%' OR ID LIKE '%3%%%'");
	std::string sql = (boost::format(fmt) % g_tableName % shortName % prefix).str();
		
	SQLStatementPtr stmt =0;
	int rc;
	
/*	stmt = m_dbHandle->prepareStatement( "PRAGMA count_changes = 1;" );

 rc = stmt->step();
	assert(rc == SQLITE_DONE);*/
	
	stmt = m_dbHandle->prepareStatement( sql );	
	rc = stmt->step();
/*	
	int numChanges = sqlite3_changes( m_dbHandle->get() );
		
	stmt = m_dbHandle->prepareStatement( "PRAGMA count_changes = 0;" );
	rc = stmt->step();
	assert(rc == SQLITE_DONE);*/
	
	return 0;
}

bool SQLiteIndexedIO::exists(const IndexedIO::EntryID &name)
{
	assert(m_dbHandle->get());
	readable(name);
		
	std::string key = m_currentDirectory.appended(name).fullPath();
	std::string sql;
	static const boost::format fmt("SELECT ID FROM %1% WHERE ID='%2%';");
	sql = (boost::format(fmt) % g_tableName % key).str();
	
	SQLStatementPtr stmt = m_dbHandle->prepareStatement(sql);	
	
	int rc = stmt->step();	
		
	if (rc != SQLITE_ROW)
	{	
		return false;
	}
	
	assert(stmt->columnCount() == 1);
		
	return true;
}

bool SQLiteIndexedIO::exists(const IndexedIOPath &path, IndexedIO::EntryType e)
{
	assert(m_dbHandle->get());
		
	std::string key = path.fullPath();
	std::string sql;
	static const boost::format fmt("SELECT ID, entryType FROM %1% WHERE ID='%2%' AND entryType=%3%;");
	sql = (boost::format(fmt) % g_tableName % key % int(e)).str();	
	
	SQLStatementPtr stmt = m_dbHandle->prepareStatement(sql);	
	
	int rc = stmt->step();	
		
	if (rc != SQLITE_ROW)
	{	
		return false;
	}

	assert(stmt->columnCount() == 2);
	
	return true;
}


IndexedIO::Entry SQLiteIndexedIO::ls(const IndexedIO::EntryID &name)
{
	assert(m_dbHandle->get());
	
	readable(name);

	std::string key = m_currentDirectory.appended(name).fullPath();
	
	static const boost::format fmt("SELECT ID, arrayLength, entryType, dataType FROM %1% WHERE ID='%2%';");
	std::string sql = (boost::format(fmt) % g_tableName % key).str();
	
	SQLStatementPtr stmt = m_dbHandle->prepareStatement( sql );
		
	int rc = stmt->step();

	if (rc != SQLITE_ROW)
	{		
		throw IOException(name);
	}

#ifndef NDEBUG	
	std::string id = stmt->getString(0);
	assert(key == id);
#endif	
	
	unsigned long arrayLength = stmt->getInt(1);
	int entryType = stmt->getInt(2);	
	int dataType = stmt->getInt(3);
	
	return IndexedIO::Entry(key, (IndexedIO::EntryType)entryType, (IndexedIO::DataType)dataType, arrayLength);
}

template<typename T>
void SQLiteIndexedIO::write(const IndexedIO::EntryID &name, const T *x, unsigned long arrayLength)
{
	assert(m_dbHandle->get());

	writable(name);
	
	if (m_mode & IndexedIO::Write || m_mode & IndexedIO::Append)
	{
		rm(name, false);
	}

	std::string key = m_currentDirectory.appended(name).fullPath();
	
	unsigned long size = IndexedIO::DataSizeTraits<T*>::size(x, arrayLength);	
	int dataType = IndexedIO::DataTypeTraits<T*>::type();
	
	static const boost::format fmt("INSERT INTO %1% VALUES ('%2%', %3%, %4%, %5%, %6%, ?)");
	std::string sql = (boost::format(fmt) 
				% g_tableName 
				% key 
				% size 
				% arrayLength
				% int(IndexedIO::File) 
				% dataType).str();
	

	SQLStatementPtr stmt = m_dbHandle->prepareStatement( sql );	

	const char *data = IndexedIO::DataFlattenTraits<T*>::flatten(x, arrayLength);
	const void *blob = static_cast<const void *>(data);
	
	int rc = sqlite3_bind_blob( stmt->get(), 1, blob, size, SQLITE_TRANSIENT);
	assert( rc != SQLITE_RANGE );
		
	rc = stmt->step();
	
	IndexedIO::DataFlattenTraits<T>::free(data);
	
	if (rc == SQLITE_CONSTRAINT)
	{
		// File exists
		throw IOException(name);
	}
	
}

template<typename T>
void SQLiteIndexedIO::write(const IndexedIO::EntryID &name, const T &x)
{
	assert(m_dbHandle->get());

	writable(name);
		
	if (m_mode & IndexedIO::Write || m_mode & IndexedIO::Append)
	{
		rm(name, false);
	}
	
	std::string key = m_currentDirectory.appended(name).fullPath();
	
	unsigned long size = IndexedIO::DataSizeTraits<T>::size(x);
	
	int dataType = IndexedIO::DataTypeTraits<T>::type();
	
	static const boost::format fmt("INSERT INTO %1% VALUES ('%2%', %3%, %4%, %5%, %6%, ?)");
	std::string sql = (boost::format(fmt) 
				% g_tableName 
				% key 
				% size 
				% 0 /* arrayLength */
				% int(IndexedIO::File) 
				% dataType).str();

	SQLStatementPtr stmt = m_dbHandle->prepareStatement( sql );

	const char *blob = IndexedIO::DataFlattenTraits<T>::flatten(x);
	
	int rc = sqlite3_bind_blob( stmt->get(), 1, blob, size, SQLITE_TRANSIENT);
	assert( rc != SQLITE_RANGE );
		
	rc = stmt->step();
	
	IndexedIO::DataFlattenTraits<T>::free(blob);
	
	if (rc == SQLITE_CONSTRAINT)
	{
		// File exists
		throw IOException(name);
	}	
}

template<typename T>
void SQLiteIndexedIO::read(const IndexedIO::EntryID &name, T *&x, unsigned long arrayLength) const
{
	assert(m_dbHandle->get());
	
	readable(name);
	
	std::string key = m_currentDirectory.appended(name).fullPath();
	
	static const boost::format fmt("SELECT ID, size, arrayLength, entryType, dataType, data FROM %1% WHERE ID='%2%';");
	std::string sql = (boost::format(fmt) % g_tableName % key).str();
	
	SQLStatementPtr stmt = m_dbHandle->prepareStatement( sql );	
	
	int rc = stmt->step();

	if (rc != SQLITE_ROW)
	{
		throw IOException(name);
	}

#ifndef NDEBUG		
	std::string id = stmt->getString(0);	
	assert(key == id);
#endif	
	
	unsigned long size = stmt->getInt(1);	
	
	unsigned long length = stmt->getInt(2);
	if (length != arrayLength)
	{	
		throw IOException(name);
	}
		
	int entryType = stmt->getInt(3);
	if ((IndexedIO::EntryType)entryType != IndexedIO::File)
	{
		throw IOException(name);		
	}
	
	// Make sure were of the correct dataType
	int dataType = stmt->getInt(4);
	if ((IndexedIO::DataType)dataType != IndexedIO::DataTypeTraits<T*>::type())
	{
		throw IOException(name);
	}
	
	const char *data = static_cast<const char*>(stmt->getBlob(5));
	
	if (data)
	{
		IndexedIO::DataFlattenTraits<T*>::unflatten( data, x, arrayLength );
	}
	else if (size != 0)
	{
		throw IOException(name);	
	}		

}

template<typename T>
void SQLiteIndexedIO::read(const IndexedIO::EntryID &name, T &x) const
{
	assert(m_dbHandle->get());
	
	readable(name);
	
	std::string key = m_currentDirectory.appended(name).fullPath();
	
	static const boost::format fmt("SELECT ID, size, arrayLength, entryType, dataType, data FROM %1% WHERE ID='%2%';");
	std::string sql = (boost::format(fmt) % g_tableName % key).str();
	
	SQLStatementPtr stmt = m_dbHandle->prepareStatement( sql );	
	
	int rc = stmt->step();

	if (rc != SQLITE_ROW)
	{		
		throw IOException(name);
	}
	
#ifndef NDEBUG	
	std::string id = stmt->getString(0);	
	assert(key == id);
#endif	
	
	unsigned long arrayLength = stmt->getInt(2);
	if (arrayLength != 0)
	{
		throw IOException(name);
	}
			
	int entryType = stmt->getInt(3);
	if ((IndexedIO::EntryType)entryType != IndexedIO::File)
	{		
		throw IOException(name);		
	}
	
	// Make sure were of the correct dataType
	int dataType = stmt->getInt(4);
	if ((IndexedIO::DataType)dataType != IndexedIO::DataTypeTraits<T>::type())
	{
		throw IOException(name);
	}
	
	// Read and unflatten the actual data
	const char *data = static_cast<const char*>(stmt->getBlob(5));
	IndexedIO::DataFlattenTraits<T>::unflatten( data, x );	
	
	// Check that the data we read was of the expected size
	unsigned long size = stmt->getInt(1);
	if (size != IndexedIO::DataSizeTraits<T>::size(x) )
	{
		x = T();
		throw IOException(name);
	}

}

// Write

void SQLiteIndexedIO::write(const IndexedIO::EntryID &name, const float *x, unsigned long arrayLength)
{
	write<float>(name, x, arrayLength);
}

void SQLiteIndexedIO::write(const IndexedIO::EntryID &name, const double *x, unsigned long arrayLength)
{
	write<double>(name, x, arrayLength);
}

void SQLiteIndexedIO::write(const IndexedIO::EntryID &name, const half *x, unsigned long arrayLength)
{
	write<half>(name, x, arrayLength);
}

void SQLiteIndexedIO::write(const IndexedIO::EntryID &name, const int *x, unsigned long arrayLength)
{
	write<int>(name, x, arrayLength);
}

void SQLiteIndexedIO::write(const IndexedIO::EntryID &name, const long *x, unsigned long arrayLength)
{
	write<long>(name, x, arrayLength);
}

void SQLiteIndexedIO::write(const IndexedIO::EntryID &name, const unsigned int *x, unsigned long arrayLength)
{
	write<unsigned int>(name, x, arrayLength);
}

void SQLiteIndexedIO::write(const IndexedIO::EntryID &name, const char *x, unsigned long arrayLength)
{
	write<char>(name, x, arrayLength);
}

void SQLiteIndexedIO::write(const IndexedIO::EntryID &name, const unsigned char *x, unsigned long arrayLength)
{
	write<unsigned char>(name, x, arrayLength);
}

void SQLiteIndexedIO::write(const IndexedIO::EntryID &name, const std::string *x, unsigned long arrayLength)
{
	write<std::string>(name, x, arrayLength);
}

void SQLiteIndexedIO::write(const IndexedIO::EntryID &name, const float &x)
{
	write<float>(name, x);
}

void SQLiteIndexedIO::write(const IndexedIO::EntryID &name, const double &x)
{
	write<double>(name, x);
}

void SQLiteIndexedIO::write(const IndexedIO::EntryID &name, const half &x)
{
	write<half>(name, x);
}

void SQLiteIndexedIO::write(const IndexedIO::EntryID &name, const int &x)
{
	write<int>(name, x);
}

void SQLiteIndexedIO::write(const IndexedIO::EntryID &name, const long &x)
{
	write<long>(name, x);
}

void SQLiteIndexedIO::write(const IndexedIO::EntryID &name, const std::string &x)
{
	write<std::string>(name, x);
}

void SQLiteIndexedIO::write(const IndexedIO::EntryID &name, const unsigned int &x)
{
	write<unsigned int>(name, x);
}

void SQLiteIndexedIO::write(const IndexedIO::EntryID &name, const char &x)
{
	write<char>(name, x);
}

void SQLiteIndexedIO::write(const IndexedIO::EntryID &name, const unsigned char &x)
{
	write<unsigned char>(name, x);
}

// Read

void SQLiteIndexedIO::read(const IndexedIO::EntryID &name, float *&x, unsigned long arrayLength)
{
	read<float>(name, x, arrayLength);
}

void SQLiteIndexedIO::read(const IndexedIO::EntryID &name, double *&x, unsigned long arrayLength)
{
	read<double>(name, x, arrayLength);
}

void SQLiteIndexedIO::read(const IndexedIO::EntryID &name, half *&x, unsigned long arrayLength)
{
	read<half>(name, x, arrayLength);
}

void SQLiteIndexedIO::read(const IndexedIO::EntryID &name, int *&x, unsigned long arrayLength)
{
	read<int>(name, x, arrayLength);
}

void SQLiteIndexedIO::read(const IndexedIO::EntryID &name, long *&x, unsigned long arrayLength)
{
	read<long>(name, x, arrayLength);
}

void SQLiteIndexedIO::read(const IndexedIO::EntryID &name, unsigned int *&x, unsigned long arrayLength)
{
	read<unsigned int>(name, x, arrayLength);
}

void SQLiteIndexedIO::read(const IndexedIO::EntryID &name, char *&x, unsigned long arrayLength)
{
	read<char>(name, x, arrayLength);
}

void SQLiteIndexedIO::read(const IndexedIO::EntryID &name, unsigned char *&x, unsigned long arrayLength)
{
	read<unsigned char>(name, x, arrayLength);
}

void SQLiteIndexedIO::read(const IndexedIO::EntryID &name, std::string *&x, unsigned long arrayLength)
{
	read<std::string>(name, x, arrayLength);
}

void SQLiteIndexedIO::read(const IndexedIO::EntryID &name, float &x)
{
	read<float>(name, x);
}

void SQLiteIndexedIO::read(const IndexedIO::EntryID &name, double &x)
{
	read<double>(name, x);
}

void SQLiteIndexedIO::read(const IndexedIO::EntryID &name, half &x)
{
	read<half>(name, x);
}

void SQLiteIndexedIO::read(const IndexedIO::EntryID &name, int &x)
{
	read<int>(name, x);
}

void SQLiteIndexedIO::read(const IndexedIO::EntryID &name, long &x)
{
	read<long>(name, x);
}

void SQLiteIndexedIO::read(const IndexedIO::EntryID &name, std::string &x)
{
	read<std::string>(name, x);
}

void SQLiteIndexedIO::read(const IndexedIO::EntryID &name, unsigned int &x)
{
	read<unsigned int>(name, x);
}

void SQLiteIndexedIO::read(const IndexedIO::EntryID &name, char &x)
{
	read<char>(name, x);
}

void SQLiteIndexedIO::read(const IndexedIO::EntryID &name, unsigned char &x)
{
	read<unsigned char>(name, x);
}
