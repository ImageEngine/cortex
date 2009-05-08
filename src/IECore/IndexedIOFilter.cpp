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

#include "IECore/IndexedIOFilter.h"
#include <iostream>

using namespace IECore;

IndexedIOFilter::IndexedIOFilter()
: m_next(0)
{
}

IndexedIOFilter::~IndexedIOFilter()
{
}

void IndexedIOFilter::add(const IndexedIOFilterPtr &f)
{
	IndexedIOFilterPtr current = this;

	while (current)
	{
		if (current == f)
		{
			return;
		}
		current = current->m_next;
	}

	m_next = f;
}

bool IndexedIOFilter::filter( const IndexedIO::Entry &e) const
{
	if (m_next)
		return m_next->filter( e );
	else
		return false;
}

struct Filter
{
	IndexedIOFilterPtr m_f;
	Filter(const IndexedIOFilterPtr &f) : m_f(f)
	{
	}

	bool operator()(const IndexedIO::Entry& e) const
	{
		return m_f->filter(e);
	}

};

unsigned int IndexedIOFilter::apply( IndexedIO::EntryList &l)
{
	unsigned sizeBefore = l.size();

	l.remove_if( Filter(this) );

	return sizeBefore - l.size();
}

//

IndexedIONullFilter::IndexedIONullFilter()
{
}

bool IndexedIONullFilter::filter( const IndexedIO::Entry &e) const
{
	return IndexedIOFilter::filter(e) || false;
}

//

IndexedIOEntryTypeFilter::IndexedIOEntryTypeFilter(IndexedIO::EntryType typ)
: m_entryType(typ)
{
}

bool IndexedIOEntryTypeFilter::filter( const IndexedIO::Entry &e) const
{
	return IndexedIOFilter::filter(e) || (e.entryType() != m_entryType);
}

//

IndexedIORegexFilter::IndexedIORegexFilter(const std::string &regex)
{
	m_regex = boost::regex(regex);
}

bool IndexedIORegexFilter::filter( const IndexedIO::Entry &e) const
{
	boost::cmatch what;

	return IndexedIOFilter::filter(e) || ! regex_match(e.id().c_str(), what, m_regex);
}
