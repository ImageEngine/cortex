//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012-2013, Image Engine Design Inc. All rights reserved.
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

#include "boost/python.hpp"

#include "IECoreGL/bindings/SelectorBinding.h"

#include "IECoreGL/Selector.h"
#include "IECoreGL/State.h"

using namespace boost::python;

namespace IECoreGL
{

class SelectorContext
{

	public :

		SelectorContext( const Imath::Box2f &region, Selector::Mode mode, boost::python::list hits )
			:	m_region( region ), m_mode( mode ), m_hitsList( hits ), m_selector()
		{
		}

		void enter()
		{
			m_selector = boost::shared_ptr<Selector>( new Selector( m_region, m_mode, m_hitsVector ) );
		}

		void loadName( GLuint name )
		{
			if( m_selector )
			{
				m_selector->loadName( name );
			}
		}

		StatePtr baseState()
		{
			return StatePtr( m_selector ? m_selector->baseState() : nullptr );
		}

		void exit( object type, object value, object traceBack )
		{
			m_selector = boost::shared_ptr<Selector>();
			for( std::vector<HitRecord>::const_iterator it=m_hitsVector.begin(); it!=m_hitsVector.end(); it++ )
			{
				m_hitsList.append( *it );
			}
		}

	private :

		Imath::Box2f m_region;
		Selector::Mode m_mode;
		std::vector<HitRecord> m_hitsVector;
		boost::python::list m_hitsList;
		boost::shared_ptr<Selector> m_selector;


};

void bindSelector()
{
	scope s = class_<SelectorContext, boost::noncopyable>( "Selector", init<const Imath::Box2f &, Selector::Mode, boost::python::list>() )
		.def( "loadName", &SelectorContext::loadName )
		.def( "baseState", &SelectorContext::baseState )
		.def( "__enter__", &SelectorContext::enter, return_self<>())
		.def( "__exit__", &SelectorContext::exit )
	;

	enum_<Selector::Mode>( "Mode" )
		.value( "GLSelect", Selector::GLSelect )
		.value( "OcclusionQuery", Selector::OcclusionQuery )
		.value( "IDRender", Selector::IDRender )
	;
}

} // namespace IECoreGL
