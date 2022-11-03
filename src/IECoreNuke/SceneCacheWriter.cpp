//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2022, Image Engine Design Inc. All rights reserved.
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

#include "IECoreNuke/SceneCacheWriter.h"

using namespace IECore;
using namespace IECoreNuke;
using namespace IECoreScene;

DD::Image::GeoWriter::Description SceneCacheWriter::description( "scc\0", SceneCacheWriter::Build );

DD::Image::GeoWriter* SceneCacheWriter::Build( DD::Image::WriteGeo* readNode )
{
	return new SceneCacheWriter( readNode );
}

SceneCacheWriter::SceneCacheWriter( DD::Image::WriteGeo* writeNode ) :
	DD::Image::GeoWriter( writeNode )
{
}

void SceneCacheWriter::execute( DD::Image::Scene& scene )
{
	open();
	if ( auto geoOp = dynamic_cast<DD::Image::GeoOp*>( geo ) )
	{
		m_liveScene = new IECoreNuke::LiveScene( geoOp );
	}

	IECoreScene::SceneInterface::NameList names;
	m_liveScene->childNames( names );
	for ( const auto& name : names )
	{
		writeLocation( m_liveScene, m_writer, name );
	}
}

bool SceneCacheWriter::animation() const
{
	return true;
}

bool SceneCacheWriter::open()
{
	if ( !m_writer )
	{
		m_writer = IECoreScene::SceneInterface::create( std::string( filename() ), IECore::IndexedIO::Write );
	}

	return true;
}

void SceneCacheWriter::writeLocation( ConstSceneInterfacePtr inScene, SceneInterfacePtr outScene, const IECore::InternedString& childName )
{
	auto time = LiveScene::frameToTime( frame() );
	ConstSceneInterfacePtr inChild = inScene->child( childName, SceneInterface::MissingBehaviour::ThrowIfMissing );
	auto outChild = outScene->child( childName, IECoreScene::SceneInterface::MissingBehaviour::CreateIfMissing );
	try
	{
		outChild->writeTransform( inChild->readTransform( time ).get(), time );
	}
	catch ( ... )
	{
	}
	if ( inChild->hasObject() )
	{
		try
		{
			outChild->writeObject( inChild->readObject( time ).get(), time );
		}
		catch ( ... )
		{
		}
	}

	// recursion
	SceneInterface::NameList grandChildNames;
	inChild->childNames( grandChildNames );
	for ( auto& grandChildName : grandChildNames )
	{
		writeLocation( inChild, outChild, grandChildName );
	}
}
