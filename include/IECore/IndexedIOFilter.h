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

#ifndef IE_CORE_INDEXEDIOFILTER_H
#define IE_CORE_INDEXEDIOFILTER_H 

#include <boost/regex.hpp>

#include "RefCounted.h"
#include "IndexedIO.h"

namespace IECore
{
	
class IndexedIOFilter;
typedef boost::intrusive_ptr<IndexedIOFilter> IndexedIOFilterPtr;	

/// An interface to allow arbitrary filtering of IndexedIO::EntryList objects. For example,
/// to filter out all files that are larger than a certain size, or of a particular data type.
class IndexedIOFilter : public RefCounted
{
				
	public:
		IndexedIOFilter();
		virtual ~IndexedIOFilter();
			
		/// Chains an additional filter to the end, resulting in a logical OR.
		virtual void add(const IndexedIOFilterPtr &f);
		
		/// Applies the filter to an entry list
		virtual unsigned int apply( IndexedIO::EntryList &l);		
	
		/// Derived classes should implement this method, returning 'true' if they
		/// want to filter out the passed in Entry.
		virtual bool filter( const IndexedIO::Entry &e) const = 0;
	
	private:
		IndexedIOFilterPtr m_next;
};

/// A Null filter. Performs no filtering.
class IndexedIONullFilter : public IndexedIOFilter
{
	public:		
		IndexedIONullFilter();
	
		virtual bool filter( const IndexedIO::Entry &e) const;
};

typedef boost::intrusive_ptr<IndexedIONullFilter> IndexedIONullFilterPtr;


/// A class to filter out Entries which don't have the specified type.
class IndexedIOEntryTypeFilter : public IndexedIOFilter
{
	public:		
		IndexedIOEntryTypeFilter(IndexedIO::EntryType typ);
	
		virtual bool filter( const IndexedIO::Entry &e) const;
	
	protected:
		IndexedIO::EntryType m_entryType;
			
};

typedef boost::intrusive_ptr<IndexedIOEntryTypeFilter> IndexedIOEntryTypeFilterPtr;

/// A class to filter out Entries whose names don't match the specified regular expression
class IndexedIORegexFilter : public IndexedIOFilter
{
	public:		
		IndexedIORegexFilter(const std::string &regex);
	
		virtual bool filter( const IndexedIO::Entry &e) const;
	
	protected:
		boost::regex m_regex;
			
};

typedef boost::intrusive_ptr<IndexedIORegexFilter> IndexedIORegexFilterPtr;

}

#endif //IE_CORE_INDEXEDIOFILTER_H
