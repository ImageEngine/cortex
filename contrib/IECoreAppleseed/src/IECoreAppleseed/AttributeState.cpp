//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2014, Esteban Tovagliari. All rights reserved.
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

#include "foundation/math/scalar.h"

#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"

#include "IECoreAppleseed/private/AttributeState.h"

using namespace std;
using namespace IECore;

namespace asf = foundation;
namespace asr = renderer;

IECoreAppleseed::AttributeState::AttributeState()
{
	m_attributes = new CompoundData;
}

IECoreAppleseed::AttributeState::AttributeState( const AttributeState &other )
{
	m_attributes = other.m_attributes->copy();
	m_shadingState = other.m_shadingState;
	m_visibilityDictionary = other.m_visibilityDictionary;
}

void IECoreAppleseed::AttributeState::setAttribute( const string &name, ConstDataPtr value )
{
	m_attributes->writable()[name] = value->copy();

	if( name == "name" )
	{
		if( ConstStringDataPtr f = runTimeCast<const StringData>( value ) )
		{
			m_name = f->readable();
		}
		else
		{
			msg( Msg::Error, "IECoreAppleseed::RendererImplementation::setAttribute", "name attribute expects a StringData value." );
		}
	}
	else if( name == "as:alpha_map" )
	{
		if( ConstStringDataPtr f = runTimeCast<const StringData>( value ) )
		{
			m_shadingState.setAlphaMap( f->readable() );
		}
		else
		{
			msg( Msg::Error, "IECoreAppleseed::RendererImplementation::setAttribute", "as:alpha_map attribute expects a StringData value." );
		}
	}
	else if( name == "as:shading_samples" )
	{
		if( ConstIntDataPtr f = runTimeCast<const IntData>( value ) )
		{
			m_shadingState.setShadingSamples( f->readable() );
		}
		else
		{
			msg( Msg::Error, "IECoreAppleseed::RendererImplementation::setAttribute", "as:shading_samples attribute expects an IntData value." );
		}
	}
	else if( name == "gaffer:deformationBlurSegments" )
	{
		if( ConstIntDataPtr f = runTimeCast<const IntData>( value ) )
		{
			// round samples to the next power of 2 as
			// appleseed only supports power of 2 number of deformation segments.
			int samples = asf::next_pow2( f->readable() );
			m_attributes->writable()[name] = new IntData( samples );
		}
		else
		{
			msg( Msg::Error, "IECoreAppleseed::RendererImplementation::setAttribute", "as:shading_samples attribute expects an IntData value." );
		}
	}
	else if( 0 == name.compare( 0, 14, "as:visibility:" ) )
	{
		if( ConstBoolDataPtr f = runTimeCast<const BoolData>( value ) )
		{
			string flag_name( name, 14, string::npos );
			m_visibilityDictionary.insert( flag_name.c_str(), f->readable() ? "true" : "false" );
		}
		else
		{
			msg( Msg::Error, "IECoreAppleseed::RendererImplementation::setAttribute", "visibility attributes expect a BoolData value." );
		}
	}
}

ConstDataPtr IECoreAppleseed::AttributeState::getAttribute( const string &name ) const
{
	return m_attributes->member<Data>( name );
}

const string &IECoreAppleseed::AttributeState::name() const
{
	return m_name;
}

const asf::Dictionary &IECoreAppleseed::AttributeState::visibilityDictionary() const
{
	return m_visibilityDictionary;
}

void IECoreAppleseed::AttributeState::addOSLShader( ConstShaderPtr shader )
{
	m_shadingState.addOSLShader( shader );
}

void IECoreAppleseed::AttributeState::setOSLSurface( ConstShaderPtr surface )
{
	m_shadingState.setOSLSurface( surface );
}

bool IECoreAppleseed::AttributeState::shadingStateValid() const
{
	return m_shadingState.valid();
}

const MurmurHash&IECoreAppleseed::AttributeState::shaderGroupHash() const
{
	return m_shadingState.shaderGroupHash();
}

const MurmurHash&IECoreAppleseed::AttributeState::materialHash() const
{
	return m_shadingState.materialHash();
}

string IECoreAppleseed::AttributeState::createShaderGroup( asr::Assembly &assembly )
{
	return m_shadingState.createShaderGroup( assembly );
}

string IECoreAppleseed::AttributeState::createMaterial( asr::Assembly &assembly, const string &shaderGroupName, const asf::SearchPaths &searchPaths )
{
	return m_shadingState.createMaterial( assembly, shaderGroupName, searchPaths );
}
