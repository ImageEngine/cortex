//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2015, Esteban Tovagliari. All rights reserved.
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

#include "IECoreAppleseed/private/MotionBlockHandler.h"

#include "IECore/MessageHandler.h"

#include "IECoreAppleseed/private/AppleseedUtil.h"

using namespace std;
using namespace Imath;
using namespace IECore;

namespace asf = foundation;
namespace asr = renderer;

IECoreAppleseed::MotionBlockHandler::MotionBlockHandler( TransformStack &transformStack,
	PrimitiveConverter &primitiveConverter )
	: m_transformStack( transformStack ), m_primitiveConverter( primitiveConverter )
{
	m_blockType = NoBlock;
}

bool IECoreAppleseed::MotionBlockHandler::insideMotionBlock() const
{
	return !m_times.empty();
}

void IECoreAppleseed::MotionBlockHandler::motionBegin( const set<float> &times )
{
	m_times = times;
	m_transforms.clear();
	m_primitives.clear();
}

void IECoreAppleseed::MotionBlockHandler::motionEnd( const AttributeState &attrState,
	renderer::Assembly *mainAssembly )
{
	assert( !m_times.empty() );

	size_t numCalls = ( m_blockType == PrimitiveBlock ) ?
		m_primitives.size() : m_transforms.size();

	if( numCalls != m_times.size() )
	{
		msg( MessageHandler::Error, "IECoreAppleseed::RendererImplementation::motionEnd", "Wrong number of calls in motion block." );
	}

	switch( m_blockType )
	{
		case SetTransformBlock:
			m_transformStack.setTransform( m_times, m_transforms );
		break;

		case ConcatTransformBlock:
			m_transformStack.concatTransform( m_times, m_transforms );
		break;

		case PrimitiveBlock:
			assert( mainAssembly );

			msg( MessageHandler::Warning, "IECoreAppleseed::RendererImplementation::motionEnd", "Deformation motion blur is not supported yet." );

			if( const asr::Assembly *assembly = m_primitiveConverter.convertPrimitive( m_times, m_primitives, attrState, m_materialName, *mainAssembly ) )
			{
				string assemblyName = assembly->get_name();
				string assemblyInstanceName = attrState.name() + "_instance";

				asr::ParamArray params;
				params.insert( "visibility", attrState.visibilityDictionary() );

				asf::auto_release_ptr<asr::AssemblyInstance> assemblyInstance = asr::AssemblyInstanceFactory::create( assemblyInstanceName.c_str(), params, assemblyName.c_str() );
				assemblyInstance->transform_sequence() = m_transformStack.top();
				insertEntityWithUniqueName( mainAssembly->assembly_instances(), assemblyInstance, assemblyInstanceName );
			}
		break;

		case NoBlock:
			// should not happen...
			assert( false );
		break;
	}

	m_times.clear();
	m_blockType = NoBlock;
}

void IECoreAppleseed::MotionBlockHandler::setTransform( const M44f &m )
{
	if( m_blockType == NoBlock )
	{
		assert( m_transforms.empty() );

		m_blockType = SetTransformBlock;
	}
	else
	{
		if( m_blockType != SetTransformBlock )
		{
			msg( MessageHandler::Error, "IECoreAppleseed::RendererImplementation::setTransform", "Bad call in motion block." );
		}
	}

	m_transforms.push_back( m );
}

void IECoreAppleseed::MotionBlockHandler::concatTransform( const M44f &m )
{
	if( m_blockType == NoBlock )
	{
		assert( m_transforms.empty() );

		m_blockType = ConcatTransformBlock;
	}
	else
	{
		if( m_blockType != ConcatTransformBlock )
		{
			msg( MessageHandler::Error, "IECoreAppleseed::RendererImplementation::concatTransform", "Bad call in motion block." );
		}
	}

	m_transforms.push_back( m );
}

void IECoreAppleseed::MotionBlockHandler::primitive( PrimitivePtr primitive, const string &materialName )
{
	if( m_blockType == NoBlock )
	{
		assert( m_primitives.empty() );

		m_blockType = PrimitiveBlock;
		m_primitiveType = primitive->typeId();
		m_materialName = materialName;
	}
	else
	{
		if( m_blockType != PrimitiveBlock )
		{
			msg( MessageHandler::Error, "IECoreAppleseed::RendererImplementation::primitive", "Bad call in motion block." );
		}

		if( m_primitiveType != primitive->typeId() )
		{
			msg( MessageHandler::Error, "IECoreAppleseed::RendererImplementation::primitive", "Cannot mix primitive types in motion block." );
		}
	}

	m_primitives.push_back( primitive );
}
