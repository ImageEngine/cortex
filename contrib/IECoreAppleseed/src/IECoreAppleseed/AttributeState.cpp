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
	m_photonTarget = false;
}

IECoreAppleseed::AttributeState::AttributeState( const AttributeState &other )
{
	m_attributes = other.m_attributes->copy();
	m_shadingState = other.m_shadingState;
	m_alphaMap = other.m_alphaMap;
	m_photonTarget = other.m_photonTarget;
	m_visibilityDictionary = other.m_visibilityDictionary;
}

void IECoreAppleseed::AttributeState::setAttribute( const string &name, ConstDataPtr value )
{
	m_attributes->writable()[name] = value->copy();

	if( name == "name" )
	{
		if( const StringData *f = runTimeCast<const StringData>( value.get() ) )
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
		if( const StringData *f = runTimeCast<const StringData>( value.get() ) )
		{
			m_alphaMap = f->readable();
		}
		else
		{
			msg( Msg::Error, "IECoreAppleseed::RendererImplementation::setAttribute", "as:alpha_map attribute expects a StringData value." );
		}
	}
	else if( name == "as:shading_samples" )
	{
		if( const IntData *f = runTimeCast<const IntData>( value.get() ) )
		{
			m_shadingState.setShadingSamples( f->readable() );
		}
		else
		{
			msg( Msg::Error, "IECoreAppleseed::RendererImplementation::setAttribute", "as:shading_samples attribute expects an IntData value." );
		}
	}
	else if( name == "as:photon_target" )
	{
		if( const BoolData *f = runTimeCast<const BoolData>( value.get() ) )
		{
			m_photonTarget = f->readable();
		}
		else
		{
			msg( Msg::Error, "IECoreAppleseed::RendererImplementation::setAttribute", "photon_target attribute expect a BoolData value." );
		}
	}
	else if( 0 == name.compare( 0, 14, "as:visibility:" ) )
	{
		if( const BoolData *f = runTimeCast<const BoolData>( value.get() ) )
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

const std::string &IECoreAppleseed::AttributeState::alphaMap() const
{
	return m_alphaMap;
}

bool IECoreAppleseed::AttributeState::photonTarget() const
{
	return m_photonTarget;
}

void IECoreAppleseed::AttributeState::attributesHash( MurmurHash &hash ) const
{
	hash.append( m_alphaMap );
	hash.append( m_photonTarget );
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

void IECoreAppleseed::AttributeState::shaderGroupHash( MurmurHash &hash ) const
{
	m_shadingState.shaderGroupHash( hash );
}

void IECoreAppleseed::AttributeState::materialHash( MurmurHash &hash ) const
{
	m_shadingState.materialHash( hash );
}

string IECoreAppleseed::AttributeState::createShaderGroup( asr::Assembly &assembly )
{
	return m_shadingState.createShaderGroup( assembly, name() );
}

void IECoreAppleseed::AttributeState::editShaderGroup( renderer::Assembly &assembly, const string &name )
{
	m_shadingState.editShaderGroup( assembly, name );
}

string IECoreAppleseed::AttributeState::createMaterial( asr::Assembly &assembly, const string &shaderGroupName )
{
	return m_shadingState.createMaterial( assembly, name(), shaderGroupName );
}
