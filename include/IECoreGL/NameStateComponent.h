//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREGL_NAMESTATECOMPONENT_H
#define IECOREGL_NAMESTATECOMPONENT_H

#include <set>

#include "tbb/spin_rw_mutex.h"

#include "boost/multi_index_container.hpp"
#include "boost/multi_index/member.hpp"
#include "boost/multi_index/ordered_index.hpp"

#include "IECoreGL/Export.h"
#include "IECoreGL/StateComponent.h"
#include "IECore/InternedString.h"

namespace IECoreGL
{

/// The NameStateComponent class is used to specify the names of objects being rendered.
/// It maps from a public name stored as a string to an integer name which is used
/// with the Selector::loadName() method to perform selection of rendered objects.
class IECOREGL_API NameStateComponent : public StateComponent
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreGL::NameStateComponent, NameStateComponentTypeId, StateComponent );

		NameStateComponent( const std::string &name="unnamed" );
		virtual ~NameStateComponent();

		/// The name in string form.
		const std::string &name() const;
		/// The name in integer form. The value 0 is reserved to represent an invalid
		/// name, and will never be returned here.
		GLuint glName() const;

		/// Calls Selector::currentSelector()->loadName() as necessary.
		virtual void bind() const;

		/// Returns the public (string) name from the internal OpenGL name value, which
		/// typically will come from the contents of the select buffer. Raises an Exception
		/// if glName does not map to a NameStateComponent name.
		static const std::string &nameFromGLName( GLuint glName );
		/// Returns the internal GL name used for the given public (string) name. If no
		//// internal name has been created yet then throws an Exception, unless createIfMissing
		/// is true in which case an appropriate GL name is created. The value 0 represents an
		/// invalid name and will never be returned.
		static GLuint glNameFromName( const std::string &name, bool createIfMissing=false );

	private :

		typedef std::pair<IECore::InternedString, unsigned int> NamePair;
		typedef boost::multi_index::multi_index_container<
			NamePair,
			boost::multi_index::indexed_by<
				boost::multi_index::ordered_unique<
					boost::multi_index::member<NamePair, IECore::InternedString, &NamePair::first>
				>,
				boost::multi_index::ordered_unique<
					boost::multi_index::member<NamePair, unsigned int, &NamePair::second>
				>
			>
		> NameMap;
		typedef NameMap::nth_index_const_iterator<0>::type ConstNameIterator;

		static ConstNameIterator iteratorFromName( const std::string &name, bool createIfMissing );

		ConstNameIterator m_it;

		static NameMap g_nameMap;
		typedef tbb::spin_rw_mutex Mutex;
		static Mutex g_nameMapMutex;

		static Description<NameStateComponent> g_description;

};

IE_CORE_DECLAREPTR( NameStateComponent );

} // namespace IECoreGL

#endif // IECOREGL_NAMESTATECOMPONENT_H
